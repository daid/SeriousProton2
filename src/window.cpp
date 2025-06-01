#include <sp2/window.h>
#include <sp2/engine.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/graphicslayer.h>
#include <sp2/graphics/renderTexture.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/graphics/opengl.h>

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define GIF_FLIP_VERT
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif//__GNUC__
#include <stb/gif.h>
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif//__GNUC__
#ifdef _WIN32
#include <windows.h>
#endif

/* //NOTE: Disabled this for now. On battery power this seems to lower performance on laptops... (30fps limit)
extern "C"
{
    //Tell NVIDIA drivers to use the NVIDIA card if multiple cards are available (common in laptops)
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    //Tell AMD drivers to use the AMD card if multiple cards are available (common in laptops)
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
*/

namespace sp {

class ScreenRecorder : sp::NonCopyable
{
public:
    ScreenRecorder(Vector2i window_size)
    {
        writer = std::thread([this]() {writerThread(); });
        gif_writer = {0};
        char filename[64];
        std::time_t t = std::time(nullptr);
        std::strftime(filename, sizeof(filename), "record_%H%M%S_%d%m%Y.gif", std::localtime(&t));
        GifBegin(&gif_writer, filename, window_size.x, window_size.y, 1);
        LOG(Info, "Screen recording started:", filename);

        last_frame_time = std::chrono::steady_clock::now();
    }

    ~ScreenRecorder()
    {
        frames.put(nullptr);
        writer.join();
        GifEnd(&gif_writer);
        LOG(Info, "Screen recording stopped");
    }

    void grabFrame(Vector2i window_size)
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = now - last_frame_time;
        if (diff > std::chrono::milliseconds(50))
        {
            std::vector<uint32_t> pixels;
            pixels.resize(window_size.x * window_size.y);
            glReadPixels(0, 0, window_size.x, window_size.y, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<void*>(pixels.data()));
            Image* img = new Image(window_size, std::move(pixels));
            frames.put(img);
            last_frame_time += std::chrono::milliseconds(50);
        }
    }

private:
    void writerThread()
    {
        while(true)
        {
            Image* img = frames.get();
            if (!img)
                break;
            GifWriteFrame(&gif_writer, reinterpret_cast<const uint8_t*>(img->getPtr()), img->getSize().x, img->getSize().y, 5);
            delete img;
        }
    }

    GifWriter gif_writer;

    std::thread writer;
    threading::Queue<Image*> frames;
    std::chrono::steady_clock::time_point last_frame_time;
};


PList<Window> Window::windows;
void* Window::shared_render_context;

Window::Window()
{
    initialize();
    createRenderWindow();
}

Window::Window(float aspect_ratio)
{
    initialize();
    window_aspect_ratio = aspect_ratio;

    createRenderWindow();
}

Window::Window(Vector2f size_factor)
{
    initialize();
    max_window_size_ratio = size_factor;

    createRenderWindow();
}

Window::Window(Vector2f size_factor, float aspect_ratio)
{
    initialize();
    window_aspect_ratio = aspect_ratio;
    max_window_size_ratio = size_factor;

    createRenderWindow();
}

Window::~Window()
{
    close();
}

void Window::initialize()
{
    sp2assert(Engine::getInstance(), "The sp::Engine needs to be created before the sp::Window is created");

#ifdef _WIN32
    //On Vista or newer windows, let the OS know we are DPI aware, so we won't have odd scaling issues.
    void* user_dll = SDL_LoadObject("USER32.DLL");
    if (user_dll)
    {
        BOOL(WINAPI *SetProcessDPIAware)(void);
        SetProcessDPIAware = reinterpret_cast<BOOL(WINAPI *)(void)>(SDL_LoadFunction(user_dll, "SetProcessDPIAware"));
        if (SetProcessDPIAware)
            SetProcessDPIAware();
    }

    //Move the console window to the top left corner, makes it easier to read the log output in debug mode.
    HWND console_handle = GetConsoleWindow();
    if (console_handle)
        SetWindowPos(console_handle, nullptr, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
#endif

    windows.add(this);
    antialiasing = 0;
    fullscreen = false;
    mouse_button_down_mask = 0;

    window_aspect_ratio = 0.0;
    clear_color = Color(0.1, 0.1, 0.1);

    render_window = nullptr;
}

void Window::setFullScreen(bool new_fullscreen)
{
    fullscreen = new_fullscreen;
    createRenderWindow();
}

bool Window::getFullScreen()
{
    return fullscreen;
}

void Window::setClearColor(sp::Color color)
{
    clear_color = color;
}

void Window::hideCursor()
{
    SDL_ShowCursor(false); //TODO: This is global and not per window...
    cursor_texture = nullptr;
    cursor_mesh = nullptr;
}

void Window::setDefaultCursor()
{
    SDL_ShowCursor(false); //TODO: This is global and not per window...
    cursor_texture = nullptr;
    cursor_mesh = nullptr;
}

void Window::setCursor(Texture* texture, std::shared_ptr<MeshData> mesh)
{
    SDL_ShowCursor(false); //TODO: This is global and not per window...
    cursor_texture = texture;
    cursor_mesh = mesh;
}

void Window::setRelativeMouseMode(bool enabled)
{
    SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
}

void Window::setPosition(Vector2f position, int monitor_number)
{
    if (!render_window)
        return;

    SDL_Rect rect;
    if (SDL_GetDisplayUsableBounds(monitor_number, &rect) < 0)
    {
        SDL_GetDisplayUsableBounds(0, &rect);
    }

    Vector2i size;
    SDL_GetWindowSize(render_window, &size.x, &size.y);

    //GetWindowSize and SetWindowPosition work on the client area, so we need to adjust for borders to ensure borders are on screen.
    int top, left, bottom, right;
    SDL_GetWindowBordersSize(render_window, &top, &left, &bottom, &right);
    size.x += left + right;
    size.y += top + bottom;

    SDL_SetWindowPosition(render_window, rect.x + left + (rect.w - size.x) * position.x, rect.y + top + (rect.h - size.y) * position.y);
}

Vector2i Window::getSize()
{
    Vector2i size;
    if (render_window)
        SDL_GetWindowSize(render_window, &size.x, &size.y);
    return size;
}

int Window::getMonitorCount()
{
    return SDL_GetNumVideoDisplays();
}

void Window::addLayer(P<GraphicsLayer> layer)
{
    graphics_layers.add(layer);
}

void Window::close()
{
    if (render_window)
    {
        SDL_DestroyWindow(render_window);
        render_window = nullptr;
    }
}

void Window::createRenderWindow()
{
    SDL_Rect display_mode;
#ifdef __EMSCRIPTEN__
    display_mode.w = EM_ASM_INT({ return window.innerWidth; });
    display_mode.h = EM_ASM_INT({ return window.innerHeight; });
#else
    if (SDL_GetDisplayBounds(0, &display_mode))
    {
        LOG(Warning, "Failed to get desktop size.");
        display_mode.w = 640;
        display_mode.h = 480;
    }
#endif//__EMSCRIPTEN__

#ifdef ANDROID
    fullscreen = true;
#endif
    
    float window_width = display_mode.w;
    float window_height = display_mode.h;
    if (!fullscreen)
    {
        SDL_GetDisplayUsableBounds(0, &display_mode);
#ifdef __EMSCRIPTEN__
        display_mode.w = EM_ASM_INT({ return window.innerWidth; });
        display_mode.h = EM_ASM_INT({ return window.innerHeight; });
#endif//__EMSCRIPTEN__
        window_width = display_mode.w;
        window_height = display_mode.h;

        if (max_window_size_ratio.x != 0.0 && max_window_size_ratio.y != 0.0)
        {
            window_width *= max_window_size_ratio.x;
            window_height *= max_window_size_ratio.y;
        }
        else
        {
#ifndef __EMSCRIPTEN__
            //Make sure the window fits on the screen with some edge around it.
            while(window_width >= int(display_mode.w) || window_height >= int(display_mode.h))
            {
                window_width *= 0.9;
                window_height *= 0.9;
            }
#endif
        }

        if (window_aspect_ratio != 0.0)
        {
            if (window_width > window_height * window_aspect_ratio)
                window_width = window_height * window_aspect_ratio;
            else if (window_height > window_width / window_aspect_ratio)
                window_height = window_width / window_aspect_ratio;
        }
    }

    if (window_aspect_ratio != 0.0)
    {
        //For Android, set the orientation depending on if we want a wide or tall window.
        if (window_aspect_ratio > 1.0)
            SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight LandscapeLeft");
        else
            SDL_SetHint(SDL_HINT_ORIENTATIONS, "Portrait PortraitUpsideDown");
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef ANDROID
    //Try to get a ES 2.0 from android.
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

    if (antialiasing > 1)
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, antialiasing);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    int flags = SDL_WINDOW_OPENGL;
    if (fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN;
    else if (window_aspect_ratio == 0.0 && (max_window_size_ratio.x == 0.0 || max_window_size_ratio.y == 0.0))
        flags |= SDL_WINDOW_RESIZABLE;
    if (render_window)
    {
        SDL_SetWindowSize(render_window, window_width, window_height);
        SDL_SetWindowFullscreen(render_window, flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP));
    }
    else
    {
        render_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED_DISPLAY(0), SDL_WINDOWPOS_CENTERED_DISPLAY(0), window_width, window_height, flags);
    }
    if (!render_window)
    {
        LOG(Warning, "Failed to create SDL window.");
        return;
    }

    if (!shared_render_context)
    {
        shared_render_context = SDL_GL_CreateContext(render_window);
        initOpenGL();

        int major_version, minor_version;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major_version);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor_version);
        LOG(Info, "OpenGL version:", major_version, minor_version);
        LOG(Info, "OpenGL driver vendor:", glGetString(GL_VENDOR));
        LOG(Info, "OpenGL driver renderer:", glGetString(GL_RENDERER));
        LOG(Info, "OpenGL driver version:", glGetString(GL_VERSION));
    }

    //Enable VSync.
    if (SDL_GL_SetSwapInterval(-1))
        SDL_GL_SetSwapInterval(1);
}

void Window::render()
{
    Vector2i window_size;
    SDL_GetWindowSize(render_window, &window_size.x, &window_size.y);

    graphics_layers.sort([](const P<GraphicsLayer>& a, const P<GraphicsLayer>& b){
        return a->priority - b->priority;
    });

    queue.add([this, window_size]()
    {
        SDL_GL_MakeCurrent(render_window, shared_render_context);
        glViewport(0, 0, window_size.x, window_size.y);
        glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    });
    for(P<GraphicsLayer> layer : graphics_layers)
    {
        if (layer->isEnabled())
        {
            if (layer->getTarget())
            {
                queue.add([layer]()
                {
                    layer->getTarget()->activateRenderTarget();
                    glViewport(0, 0, layer->getTarget()->getSize().x, layer->getTarget()->getSize().y);
                });
                queue.setTargetAspectSize(float(layer->getTarget()->getSize().x) / float(layer->getTarget()->getSize().y));
            }
            else
            {
                queue.add([layer, window_size]()
                {
                    Rect2d viewport = layer->viewport;
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    glViewport(viewport.position.x * window_size.x, viewport.position.y * window_size.y, viewport.size.x * window_size.x, viewport.size.y * window_size.y);
                });
                queue.setTargetAspectSize(float(window_size.x) / float(window_size.y));
            }
            layer->render(queue);
        }
    }

    if (cursor_mesh && cursor_texture && (SDL_GetWindowFlags(render_window) & SDL_WINDOW_MOUSE_FOCUS))
    {
        queue.add([this, window_size]()
        {
            SDL_GL_MakeCurrent(render_window, shared_render_context);
            glViewport(0, 0, window_size.x, window_size.y);
        });

        Vector2i mouse_pos;
        SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);

        Vector2d position(mouse_pos.x, mouse_pos.y);
        position.x = position.x - window_size.x / 2.0;
        position.y = window_size.y / 2.0 - position.y;

        queue.setCamera(Matrix4x4f::scale(2.0/float(window_size.x), 2.0/float(window_size.y), 1), Matrix4x4f::identity());
        RenderData rd;
        rd.type = RenderData::Type::Normal;
        rd.shader = Shader::get("internal:basic.shader");
        rd.mesh = cursor_mesh;
        rd.texture = cursor_texture;
        queue.add(Matrix4x4f::translate(position.x, position.y, 0), rd);
    }
    queue.add([this, window_size]()
    {
        if (recorder)
            recorder->grabFrame(window_size);
        SDL_GL_SwapWindow(render_window);
    });

    queue.render();
}

void Window::handleEvent(const SDL_Event& event)
{
    switch(event.type)
    {
    case SDL_MOUSEBUTTONDOWN:
        {
            io::Pointer::Button button = io::Pointer::Button::Unknown;
            switch(event.button.button)
            {
            case SDL_BUTTON_LEFT: button = io::Pointer::Button::Left; break;
            case SDL_BUTTON_MIDDLE: button = io::Pointer::Button::Middle; break;
            case SDL_BUTTON_RIGHT: button = io::Pointer::Button::Right; break;
            default: break;
            }
            mouse_button_down_mask |= 1 << int(event.button.button);
            pointerDown(button, screenToGLPosition(event.button.x, event.button.y), -1);
        }
        break;
    case SDL_MOUSEMOTION:
        if (mouse_button_down_mask)
            pointerDrag(screenToGLPosition(event.motion.x, event.motion.y), -1);
        else
            pointerMove(screenToGLPosition(event.motion.x, event.motion.y), -1);
        break;
    case SDL_MOUSEBUTTONUP:
        mouse_button_down_mask &=~(1 << int(event.button.button));
        if (!mouse_button_down_mask)
        {
            pointerUp(screenToGLPosition(event.button.x, event.button.y), -1);
            pointerMove(screenToGLPosition(event.button.x, event.button.y), -1);
        }
        break;
    case SDL_FINGERDOWN:
        pointerDown(io::Pointer::Button::Touch, Vector2d(event.tfinger.x * 2.0 - 1.0, 1.0 - event.tfinger.y * 2.0), event.tfinger.fingerId);
        break;
    case SDL_FINGERMOTION:
        pointerDrag(Vector2d(event.tfinger.x * 2.0 - 1.0, 1.0 - event.tfinger.y * 2.0), event.tfinger.fingerId);
        break;
    case SDL_FINGERUP:
        pointerUp(Vector2d(event.tfinger.x * 2.0 - 1.0, 1.0 - event.tfinger.y * 2.0), event.tfinger.fingerId);
        break;
    case SDL_TEXTINPUT:
        if (focus_layer)
            focus_layer->onTextInput(event.text.text);
        break;
    case SDL_MOUSEWHEEL:
        if (event.wheel.y > 0)
            mousewheelMove(io::Pointer::Wheel::Up);
        else if (event.wheel.y < 0)
            mousewheelMove(io::Pointer::Wheel::Down);
        if (event.wheel.x > 0)
            mousewheelMove(io::Pointer::Wheel::Right);
        else if (event.wheel.x < 0)
            mousewheelMove(io::Pointer::Wheel::Left);
        break;
    case SDL_KEYDOWN:
#ifdef DEBUG
        if (event.key.keysym.sym == SDLK_F3)
        {
            if (recorder)
            {
                recorder = nullptr;
            }
            else
            {
                Vector2i size;
                SDL_GetWindowSize(render_window, &size.x, &size.y);
                recorder = std::make_shared<ScreenRecorder>(size);
            }
        }
#endif
        if (focus_layer)
        {
            switch(event.key.keysym.sym)
            {
            case SDLK_KP_4:
                if (event.key.keysym.mod & KMOD_NUM)
                    break;
                //fallthrough
            case SDLK_LEFT:
                if (event.key.keysym.mod & KMOD_SHIFT && event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::WordLeftWithSelection);
                else if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::WordLeft);
                else if (event.key.keysym.mod & KMOD_SHIFT)
                    focus_layer->onTextInput(TextInputEvent::LeftWithSelection);
                else
                    focus_layer->onTextInput(TextInputEvent::Left);
                break;
            case SDLK_KP_6:
                if (event.key.keysym.mod & KMOD_NUM)
                    break;
                //fallthrough
            case SDLK_RIGHT:
                if (event.key.keysym.mod & KMOD_SHIFT && event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::WordRightWithSelection);
                else if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::WordRight);
                else if (event.key.keysym.mod & KMOD_SHIFT)
                    focus_layer->onTextInput(TextInputEvent::RightWithSelection);
                else
                    focus_layer->onTextInput(TextInputEvent::Right);
                break;
            case SDLK_KP_8:
                if (event.key.keysym.mod & KMOD_NUM)
                    break;
                //fallthrough
            case SDLK_UP:
                if (event.key.keysym.mod & KMOD_SHIFT)
                    focus_layer->onTextInput(TextInputEvent::UpWithSelection);
                else
                    focus_layer->onTextInput(TextInputEvent::Up);
                break;
            case SDLK_KP_2:
                if (event.key.keysym.mod & KMOD_NUM)
                    break;
                //fallthrough
            case SDLK_DOWN:
                if (event.key.keysym.mod & KMOD_SHIFT)
                    focus_layer->onTextInput(TextInputEvent::DownWithSelection);
                else
                    focus_layer->onTextInput(TextInputEvent::Down);
                break;
            case SDLK_KP_7:
                if (event.key.keysym.mod & KMOD_NUM)
                    break;
                //fallthrough
            case SDLK_HOME:
                if (event.key.keysym.mod & KMOD_SHIFT && event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::TextStartWithSelection);
                else if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::TextStart);
                else if (event.key.keysym.mod & KMOD_SHIFT)
                    focus_layer->onTextInput(TextInputEvent::LineStartWithSelection);
                else
                    focus_layer->onTextInput(TextInputEvent::LineStart);
                break;
            case SDLK_KP_1:
                if (event.key.keysym.mod & KMOD_NUM)
                    break;
                //fallthrough
            case SDLK_END:
                if (event.key.keysym.mod & KMOD_SHIFT && event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::TextEndWithSelection);
                else if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::TextEnd);
                else if (event.key.keysym.mod & KMOD_SHIFT)
                    focus_layer->onTextInput(TextInputEvent::LineEndWithSelection);
                else
                    focus_layer->onTextInput(TextInputEvent::LineEnd);
                break;
            case SDLK_KP_PERIOD:
                if (event.key.keysym.mod & KMOD_NUM)
                    break;
                //fallthrough
            case SDLK_DELETE:
                focus_layer->onTextInput(TextInputEvent::Delete);
                break;
            case SDLK_BACKSPACE:
                focus_layer->onTextInput(TextInputEvent::Backspace);
                break;
            case SDLK_KP_ENTER:
            case SDLK_RETURN:
                focus_layer->onTextInput(TextInputEvent::Return);
                break;
            case SDLK_TAB:
            case SDLK_KP_TAB:
                if (event.key.keysym.mod & KMOD_SHIFT)
                    focus_layer->onTextInput(TextInputEvent::Unindent);
                else
                    focus_layer->onTextInput(TextInputEvent::Indent);
                break;
            case SDLK_a:
                if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::SelectAll);
                break;
            case SDLK_c:
                if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::Copy);
                break;
            case SDLK_v:
                if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::Paste);
                break;
            case SDLK_x:
                if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::Cut);
                break;
            }
        }
        break;
    case SDL_WINDOWEVENT:
        switch(event.window.event)
        {
        case SDL_WINDOWEVENT_LEAVE:
            if (!SDL_GetMouseState(nullptr, nullptr))
            {
                pointerLeave(-1);
            }
            break;
        case SDL_WINDOWEVENT_CLOSE:
            close();
            break;
        }
        break;
    case SDL_QUIT:
        close();
        break;
    default:
        break;
    }
}

void Window::pointerMove(Vector2d position, int id)
{
    auto it = pointer_focus_layer.find(id);
    for(auto layer_iterator = graphics_layers.rbegin(); layer_iterator != graphics_layers.rend(); ++layer_iterator)
    {
        auto layer = *layer_iterator;
        if (layer->onPointerMove(position, id))
        {
            if (it != pointer_focus_layer.end() && it->second && it->second != layer)
                it->second->onPointerLeave(id);
            pointer_focus_layer[id] = layer;
            return;
        }
    }
    if (it != pointer_focus_layer.end())
        pointerLeave(id);
}

void Window::pointerLeave(int id)
{
    auto it = pointer_focus_layer.find(id);
    if (it != pointer_focus_layer.end())
    {
        if (it->second)
            it->second->onPointerLeave(id);
        pointer_focus_layer.erase(it);
    }
}

void Window::pointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    for(auto layer_iterator = graphics_layers.rbegin(); layer_iterator != graphics_layers.rend(); ++layer_iterator)
    {
        auto layer = *layer_iterator;
        if (layer->onPointerDown(button, position, id))
        {
            pointer_focus_layer[id] = layer;
            focus_layer = layer;
            return;
        }
    }
}

void Window::pointerDrag(Vector2d position, int id)
{
    auto it = pointer_focus_layer.find(id);
    if (it != pointer_focus_layer.end() && it->second)
        it->second->onPointerDrag(position, id);
}

void Window::pointerUp(Vector2d position, int id)
{
    auto it = pointer_focus_layer.find(id);
    if (it != pointer_focus_layer.end() && it->second)
    {
        it->second->onPointerUp(position, id);
        pointer_focus_layer.erase(it);
    }
}

void Window::mousewheelMove(io::Pointer::Wheel direction)
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    auto position = screenToGLPosition(x, y);
    for(auto layer_iterator = graphics_layers.rbegin(); layer_iterator != graphics_layers.rend(); ++layer_iterator)
    {
        auto layer = *layer_iterator;
        if (layer->onWheelMove(position, direction))
            return;
    }
}

Vector2d Window::screenToGLPosition(int x, int y)
{
    Vector2i size;
    if (SDL_GetRelativeMouseMode())
        return {0.5, 0.5};
    SDL_GetWindowSize(render_window, &size.x, &size.y);
    return Vector2d((double(x) / double(size.x) - 0.5) * 2.0, (0.5 - double(y) / double(size.y)) * 2.0);
}

bool Window::anyWindowOpen()
{
    for(P<Window> window : windows)
    {
        if (window->render_window)
        {
            return true;
        }
    }
    return false;
}

}//namespace sp
