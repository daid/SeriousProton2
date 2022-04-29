#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/updatable.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/opengl.h>
#include <sp2/audio/audioSource.h>
#include <sp2/scene/scene.h>
#include <sp2/multiplayer/server.h>
#include <sp2/multiplayer/client.h>
#include <sp2/multiplayer/registry.h>
#include <sp2/io/keybinding.h>

#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif//__EMPSCRIPTEN__

#include <signal.h>


namespace sp {

P<Engine> Engine::engine;

#ifdef DEBUG
static io::Keybinding single_step_enable("single_step_enable", "`");
static io::Keybinding single_step_step("single_step_step", "Tab");
static bool single_step_enabled = false;
#endif

static void requestShutdownSignal(int signal)
{
    LOG(Info, "SIGTERM received, requesting shutdown");
    Engine::getInstance()->shutdown();
}

#ifdef ANDROID
#undef main
extern "C" int main(int argc, char *argv[]);
extern "C" DECLSPEC int SDL_main(int argc, char *argv[])
{
    if (Engine::getInstance())
        return 0;
    chdir(SDL_AndroidGetInternalStoragePath());
    exit(main(argc, argv));
    return 0;
}
#endif

Engine::Engine()
{
    sp2assert(!engine, "SP2 does not support more then 1 engine.");
    engine = this;

    game_speed = 1.0;
    paused = false;

    maximum_frame_time = 0.5f;
    minimum_frame_time = 0.001f;
    fixed_update_accumulator = 0.0;
    last_update_timing = {0};

    in_fixed_update = false;
    update_delta = 0.0f;
    elapsed_game_time = 0.0;

    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
    SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#ifdef SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH
    SDL_SetHint(SDL_HINT_ANDROID_SEPARATE_MOUSE_AND_TOUCH, "1");
#elif defined(SDL_HINT_MOUSE_TOUCH_EVENTS)
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "0");
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#endif
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    SDL_Init(SDL_INIT_EVERYTHING);
    signal(SIGTERM, requestShutdownSignal);
    atexit(SDL_Quit);
}

Engine::~Engine()
{
}

void Engine::initialize()
{
    if (initialized)
        return;

    multiplayer::ClassEntry::fillMappings();

    initialized = true;

#ifndef __EMSCRIPTEN__
    audio::AudioSource::startAudioSystem();
#endif
}

class Clock
{
public:
    Clock()
    {
        start_time = std::chrono::steady_clock::now();
    }

    float restart()
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = now - start_time;
        start_time = now;
        return diff.count();
    }

    float getElapsedTime()
    {
        std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = now - start_time;
        return diff.count();
    }

private:
    std::chrono::steady_clock::time_point start_time;
};

#ifdef __EMSCRIPTEN__
static void engineUpdateFunction()
{
    static Clock frame_time_clock;

    float delta = frame_time_clock.restart();

    Engine* engine = Engine::getInstance();
    engine->processEvents();
    engine->update(delta);
}
#endif//__EMSCRIPTEN__

void Engine::run()
{
    initialize();
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(engineUpdateFunction, 0, 1);
#else
    Clock frame_time_clock;
    int fps_count = 0;
    Clock fps_counter_clock;
    LOG(Info, "Engine started");

    while(!shutdown_flag && (Window::windows.empty() || Window::anyWindowOpen()))
    {
        processEvents();

        float delta = frame_time_clock.restart();
        delta = std::min(maximum_frame_time, delta);
        delta = std::max(minimum_frame_time, delta);
        update(delta);

        fps_count++;
        if (fps_counter_clock.getElapsedTime() > 5.0)
        {
            double time = fps_counter_clock.restart();
            current_fps = double(fps_count) / time;
            fps_count = 0;
            if (Window::windows.empty())
                LOG(Debug, "UPS:", current_fps);
            else
                LOG(Debug, "FPS:", current_fps);
        }
    }
    LOG(Info, "Engine closing down");
#endif//!__EMSCRIPTEN__
}

void Engine::processEvents()
{
#ifdef __EMSCRIPTEN__
    static bool audio_started = false;
#endif
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        unsigned int window_id = 0;
        switch(event.type)
        {
        case SDL_KEYDOWN:
#ifdef __EMSCRIPTEN__
            if (!audio_started)
            {
                audio::AudioSource::startAudioSystem();
                audio_started = true;
            }
#endif
        case SDL_KEYUP:
            window_id = event.key.windowID;
            break;
        case SDL_MOUSEMOTION:
            window_id = event.motion.windowID;
            break;
        case SDL_MOUSEBUTTONDOWN:
#ifdef __EMSCRIPTEN__
            if (!audio_started)
            {
                audio::AudioSource::startAudioSystem();
                audio_started = true;
            }
#endif
        case SDL_MOUSEBUTTONUP:
            window_id = event.button.windowID;
            break;
        case SDL_MOUSEWHEEL:
            window_id = event.wheel.windowID;
            break;
        case SDL_WINDOWEVENT:
            window_id = event.window.windowID;
            break;
        case SDL_FINGERDOWN:
#ifdef __EMSCRIPTEN__
            if (!audio_started)
            {
                audio::AudioSource::startAudioSystem();
                audio_started = true;
            }
#endif
        case SDL_FINGERUP:
        case SDL_FINGERMOTION:
            window_id = SDL_GetWindowID(SDL_GetMouseFocus());
            break;
        case SDL_TEXTEDITING:
            window_id = event.edit.windowID;
            break;
        case SDL_TEXTINPUT:
            window_id = event.text.windowID;
            break;
        }
        if (window_id != 0)
        {
            for(auto window : Window::windows)
                if (window->render_window && SDL_GetWindowID(window->render_window) == window_id)
                    window->handleEvent(event);
        }
        io::Keybinding::handleEvent(event);
    }
}

void Engine::update(float time_delta)
{
    UpdateTiming timing;
    Clock timing_clock;

    time_delta *= game_speed;
    if (paused)
        time_delta = 0;
#ifdef DEBUG
    if (single_step_enable.getDown())
        single_step_enabled = !single_step_enabled;
    if (single_step_enabled)
    {
        time_delta = 0;
        if (single_step_step.getDown())
            time_delta = fixed_update_delta;
    }
#endif
    update_delta = time_delta;
    elapsed_game_time += time_delta;

    fixed_update_accumulator += time_delta;
    in_fixed_update = true;
    while(fixed_update_accumulator > fixed_update_delta)
    {
        fixed_update_accumulator -= fixed_update_delta;
        for(P<Scene> scene : Scene::all())
        {
            if (scene->isEnabled())
            {
                scene->fixedUpdate();
            }
        }
        io::Keybinding::allPostFixedUpdate();
    }
    in_fixed_update = false;
    for(P<Scene> scene : Scene::all())
    {
        if (scene->isEnabled())
            scene->postFixedUpdate(fixed_update_accumulator);
    }
    timing.fixed_update = timing_clock.restart();
    for(P<Scene> scene : Scene::all())
    {
        if (scene->isEnabled())
            scene->update(time_delta);
    }
    for(P<Updatable> updatable : Updatable::updatables)
    {
        updatable->onUpdate(time_delta);
    }
    io::Keybinding::allPostUpdate();
    timing.dynamic_update = timing_clock.restart();

    for(P<Window> window : Window::windows)
    {
        if (window->render_window)
            window->render();
    }
    timing.render = timing_clock.restart();
    
    //If we are not rendering to any windows, limit the loop speed, so we do not burn CPU cycles to run insane amount of updates per second.
    if (Window::windows.empty())
    {
        int previous_total_time_ms = (last_update_timing.fixed_update + last_update_timing.dynamic_update + last_update_timing.render) * 1000.0f;
        if (previous_total_time_ms < 16)
        {
            SDL_Delay(16 - previous_total_time_ms);
        }
    }
    last_update_timing = timing;
}

void Engine::setGameSpeed(float speed)
{
    game_speed = std::max(0.0f, speed);
}

float Engine::getGameSpeed()
{
    return game_speed;
}

void Engine::setPause(bool pause)
{
    paused = pause;
}

bool Engine::getPause()
{
    return paused;
}

void Engine::shutdown()
{
    for(P<Window> window : Window::windows)
        window->close();
    shutdown_flag = true;
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif//__EMSCRIPTEN__
}

}//namespace sp
