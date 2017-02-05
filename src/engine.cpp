#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <sp2/graphics/opengl.h>
#include <sp2/scene/scene.h>
#include <sp2/io/keybinding.h>
#include <SFML/Window/Event.hpp>

namespace sp {

P<Engine> Engine::engine;

Engine::Engine()
{
    sp2assert(!engine, "SP2 does not support more then 1 engine.");
    engine = this;

    game_speed = 1.0;
    paused = false;

    maximum_frame_time = 0.5f;
    minimum_frame_time = 0.001f;
    fixed_update_accumulator = 0.0;
}

Engine::~Engine()
{
}

void Engine::run()
{
    sp2assert(Window::window, "Before the engine is run, a Window needs to be created.");
    
    glewInit();
    
    sf::Clock frame_time_clock;
    int fps_count = 0;
    sf::Clock fps_counter_clock;
    LOG(Info, "Engine started");
    while(Window::window->render_window.isOpen())
    {
        sf::Event event;
        while (Window::window->render_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                Window::window->render_window.close();
            Window::window->handleEvent(event);
        }
        io::Keybinding::updateAll();
        float delta = frame_time_clock.restart().asSeconds();
        delta = std::min(maximum_frame_time, delta);
        delta = std::max(minimum_frame_time, delta);
        delta *= game_speed;
        if (paused)
            delta = 0;
        fixed_update_accumulator += delta;
        while(fixed_update_accumulator > fixed_update_delta)
        {
            fixed_update_accumulator -= fixed_update_delta;
            for(Scene* scene : Scene::scenes)
            {
                if (scene->isEnabled())
                    scene->fixedUpdate();
            }
        }
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

        if (Window::window->render_window.isOpen())
            Window::window->render();
        
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
    Window::window->render_window.close();
}

};//!namespace sp
