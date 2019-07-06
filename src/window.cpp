#include <sp2/window.h>
#include <sp2/engine.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/graphicslayer.h>
#include <sp2/graphics/renderTexture.h>
#include <sp2/graphics/scene/renderqueue.h>
#include <sp2/graphics/opengl.h>

#include <SDL.h>

#ifdef __WIN32__
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

PList<Window> Window::windows;
void* Window::shared_render_context;

Window::Window()
{
    sp2assert(Engine::getInstance(), "The sp::Engine needs to be created before the sp::Window is created");

#ifdef __WIN32__
    //On Vista or newer windows, let the OS know we are DPI aware, so we won't have odd scaling issues.
    void* user_dll = SDL_LoadObject("USER32.DLL");
    if (user_dll)
    {
        BOOL(WINAPI *SetProcessDPIAware)(void);
        SetProcessDPIAware = (BOOL(WINAPI *)(void)) SDL_LoadFunction(user_dll, "SetProcessDPIAware");
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

    createRenderWindow();
}

Window::Window(float aspect_ratio)
: Window()
{
    window_aspect_ratio = aspect_ratio;

    createRenderWindow();
}

Window::Window(Vector2f size_factor)
: Window()
{
    max_window_size_ratio = size_factor;

    createRenderWindow();
}

Window::Window(Vector2f size_factor, float aspect_ratio)
: Window()
{
    window_aspect_ratio = aspect_ratio;
    max_window_size_ratio = size_factor;

    createRenderWindow();
}

Window::~Window()
{
    close();
}

void Window::setFullScreen(bool fullscreen)
{
    this->fullscreen = fullscreen;
    createRenderWindow();
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
    if (SDL_GetDisplayBounds(0, &display_mode))
    {
        LOG(Warning, "Failed to get desktop size.");
        display_mode.w = 640;
        display_mode.h = 480;
    }
#ifdef ANDROID
    fullscreen = true;
#endif
    
    float window_width = display_mode.w;
    float window_height = display_mode.h;
    if (!fullscreen)
    {
        if (max_window_size_ratio.x != 0.0 && max_window_size_ratio.y != 0.0)
        {
            window_width *= max_window_size_ratio.x;
            window_height *= max_window_size_ratio.y;
        }
        else
        {
            //Make sure the window fits on the screen with some edge around it.
            while(window_width >= int(display_mode.w) || window_height >= int(display_mode.h) - 100)
            {
                window_width *= 0.9;
                window_height *= 0.9;
            }
        }

        if (window_aspect_ratio != 0.0)
        {
            if (window_width > window_height * window_aspect_ratio)
                window_width = window_height * window_aspect_ratio;
            else if (window_height > window_width / window_aspect_ratio)
                window_height = window_width / window_aspect_ratio;
        }
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
    if (render_window)
    {
        SDL_SetWindowSize(render_window, window_width, window_height);
        SDL_SetWindowFullscreen(render_window, flags & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_FULLSCREEN_DESKTOP));
    }
    else
    {
        render_window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, flags);
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
    queue.add([this]()
    {
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
        break;
    case SDL_MOUSEBUTTONUP:
        mouse_button_down_mask &=~(1 << int(event.button.button));
        if (!mouse_button_down_mask)
            pointerUp(screenToGLPosition(event.button.x, event.button.y), -1);
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
    case SDL_KEYDOWN:
        if (focus_layer)
        {
            switch(event.key.keysym.sym)
            {
            case SDLK_KP_4: if (event.key.keysym.mod & KMOD_NUM) break;
            case SDLK_LEFT: focus_layer->onTextInput(TextInputEvent::Left); break;
            case SDLK_KP_6: if (event.key.keysym.mod & KMOD_NUM) break;
            case SDLK_RIGHT: focus_layer->onTextInput(TextInputEvent::Right); break;
            case SDLK_KP_8: if (event.key.keysym.mod & KMOD_NUM) break;
            case SDLK_UP: focus_layer->onTextInput(TextInputEvent::Up); break;
            case SDLK_KP_2: if (event.key.keysym.mod & KMOD_NUM) break;
            case SDLK_DOWN: focus_layer->onTextInput(TextInputEvent::Down); break;
            case SDLK_KP_7: if (event.key.keysym.mod & KMOD_NUM) break;
            case SDLK_HOME:
                if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::TextStart);
                else
                    focus_layer->onTextInput(TextInputEvent::LineStart);
                break;
            case SDLK_KP_1: if (event.key.keysym.mod & KMOD_NUM) break;
            case SDLK_END:
                if (event.key.keysym.mod & KMOD_CTRL)
                    focus_layer->onTextInput(TextInputEvent::TextEnd);
                else
                    focus_layer->onTextInput(TextInputEvent::LineEnd);
                break;
            case SDLK_KP_PERIOD: if (event.key.keysym.mod & KMOD_NUM) break;
            case SDLK_DELETE: focus_layer->onTextInput(TextInputEvent::Delete); break;
            case SDLK_BACKSPACE: focus_layer->onTextInput(TextInputEvent::Backspace); break;
            case SDLK_KP_ENTER:
            case SDLK_RETURN:
                focus_layer->onTextInput(TextInputEvent::Return);
                break;
            }
        }
        break;
    case SDL_WINDOWEVENT:
        switch(event.window.event)
        {
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

void Window::pointerDown(io::Pointer::Button button, Vector2d position, int id)
{
    for(auto l : graphics_layers)
    {
        if (l->onPointerDown(button, position, id))
        {
            pointer_focus_layer[id] = l;
            focus_layer = l;
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

Vector2d Window::screenToGLPosition(int x, int y)
{
    Vector2i size;
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

};//namespace sp
