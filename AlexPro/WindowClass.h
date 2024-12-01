#pragma once

#include <SFML/Graphics.hpp>
class Window {

	sf::RenderWindow* window = nullptr;

	void update() {
		window->clear();
		window->display();

	}
	void close()
	{
		delete window;
	}
	void loop()
	{
		sf::Event event;
		while (window->isOpen())
		{
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window->close();
				}
			}
			update();
		}
	}

public:
	void start(unsigned int Width, unsigned int Height, const char* Title)
	{
		window = new sf::RenderWindow(sf::VideoMode(Width, Height), Title);
		loop();

	}

};