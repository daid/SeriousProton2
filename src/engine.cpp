#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/assert.h>
#include <sp2/logging.h>
#include <SFML/Window/Event.hpp>

namespace sp {

P<Engine> Engine::engine;

Engine::Engine()
{
    sp2assert(!engine);
    engine = engine;
    
    game_speed = 1.0;
    paused = false;

    maximum_frame_time = 0.5f;
    minimum_frame_time = 0.001f;
}

Engine::~Engine()
{
}

void Engine::run()
{
    sf::Clock frameTimeClock;
    LOG(Info, "Engine started");
    while(Window::window->render_window.isOpen())
    {
        sf::Event event;
        while (Window::window->render_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                Window::window->render_window.close();
            //IO::System::handleEvent(event);
        }

        float delta = frameTimeClock.restart().asSeconds();
        delta = std::min(maximum_frame_time, delta);
        delta = std::min(minimum_frame_time, delta);
        delta *= game_speed;
        if (paused)
            delta = 0;

        //IOSystem::update();
        
        Window::window->render();
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

};//!namespace sp
