#include <iostream>
#include <Windows.h>
#include <thread>
#include <string>
#include "Game.h"
#include <chrono>
int main()
{
	Game& g = Game::instance();
	if (!g.initialize())
	{
		return EXIT_FAILURE;
	}
	while (g.gameOver != true)
	{
		g.drawBoard();
		g.drawApple();
		g.handleInputs();
		g.handleSnek();
		if (!g.writeFrame())
		{
			return EXIT_FAILURE;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / g.fps));
	}
	std::string scoreString = "Your score was: " + std::to_string(g.score);
	MessageBoxA(NULL, scoreString.c_str(), "You lost!", 0);
	return EXIT_SUCCESS;
}

