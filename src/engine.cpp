#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/opengl.h>
#include <sp2/scene/scene.h>
#include <sp2/multiplayer/server.h>
#include <sp2/multiplayer/client.h>
#include <sp2/multiplayer/registry.h>
#include <sp2/io/keybinding.h>

#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>

namespace sp {

P<Engine> Engine::engine;

#ifdef DEBUG
io::Keybinding single_step_enable("single_step_enable", "Tilde");
io::Keybinding single_step_step("single_step_step", "Tab");
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
    
    in_fixed_update = false;
    
    // By creating a SoundBuffer we force SFML to load the sound subsystem.
    // Else this is done when the first sound is loaded, causing a delay at that point, which causes a hickup on windows.
    sf::SoundBuffer forceLoadBuffer;
    
    sp::multiplayer::ClassEntry::fillMappings();
}

Engine::~Engine()
{
}

void Engine::run()
{
    sp2assert(Window::windows.size() > 0, "Before the engine is run, a Window needs to be created.");
    
    glewInit();
    
    sf::Clock frame_time_clock;
    int fps_count = 0;
    sf::Clock fps_counter_clock;
    LOG(Info, "Engine started");
    
    while(Window::anyWindowOpen())
    {
        sf::Event event;
        for(Window* window : Window::windows)
        {
            while (window->render_window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                    window->render_window.close();
                window->handleEvent(event);
                io::Keybinding::handleEvent(event);
            }
        }
        float delta = frame_time_clock.restart().asSeconds();
        delta = std::min(maximum_frame_time, delta);
        delta = std::max(minimum_frame_time, delta);
        delta *= game_speed;
        if (paused)
            delta = 0;
#ifdef DEBUG
        if (single_step_enable.getDown())
            single_step_enabled = !single_step_enabled;
        if (single_step_enabled)
        {
            delta = 0;
            if (single_step_step.getDown())
                delta = fixed_update_delta;
        }
#endif
        fixed_update_accumulator += delta;
        in_fixed_update = true;
        while(fixed_update_accumulator > fixed_update_delta)
        {
            fixed_update_accumulator -= fixed_update_delta;
            for(Scene* scene : Scene::scenes)
            {
                if (scene->isEnabled())
                {
                    scene->fixedUpdate();
                    scene->postFixedUpdate(fixed_update_accumulator);
                }
            }
            io::Keybinding::allPostFixedUpdate();
        }
        in_fixed_update = false;
        for(Scene* scene : Scene::scenes)
        {
            if (scene->isEnabled())
                scene->postFixedUpdate(fixed_update_accumulator);
        }
        for(Scene* scene : Scene::scenes)
        {
            if (scene->isEnabled())
                scene->update(delta);
        }
        
        if (multiplayer::Server::getInstance())
            multiplayer::Server::getInstance()->update();
        if (multiplayer::Client::getInstance())
            multiplayer::Client::getInstance()->update();

        for(Window* window : Window::windows)
        {
            if (window->render_window.isOpen())
                window->render();
        }

        io::Keybinding::allPostUpdate();

        fps_count++;
        if (fps_counter_clock.getElapsedTime().asSeconds() > 5.0)
        {
            double fps = double(fps_count) / fps_counter_clock.restart().asSeconds();
            fps_count = 0;
            LOG(Debug, "FPS:", fps);
        }
    }
    LOG(Info, "Engine closing down");
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
    for(Window* window : Window::windows)
        window->render_window.close();
}

};//!namespace sp
