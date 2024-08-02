#include <Windows.h>
#include <list>
#include <vector>
#include <cstdint>
#include "mdwrap.h"
enum Direction
{
	none,
	up,
	down,
	left,
	right
};

struct Snake
{
	COORD headPos = { 0,0 };
	Direction dir = none;
	std::list<COORD> body;
};
class Game
{
public:
	static constexpr COORD gameSize = { 40,20 };
	static constexpr COORD realSize = { gameSize.X + 3,gameSize.Y + 3 }; //because of walls and score

	Snake dangerNoodle;
	COORD foodPos = { 0, 0 };

	bool gameOver = false;
	bool isFoodThere = false;

	std::uint32_t score = 0;
	std::uint8_t fps = 3;

	mdwrap frameData = mdwrap(frameDatav, realSize.X);

	std::list<Direction> inputs;

	void handleSnek();
	void drawBoard();
	void drawApple();
	void handleInputs();
	bool checkCollision(COORD coords);
	bool isValidApplePos(short X, short Y);
	bool writeFrame();
	bool initialize();

private:
	HANDLE consoleOutHandle = INVALID_HANDLE_VALUE;
	HANDLE consoleInHandle = INVALID_HANDLE_VALUE;
	std::vector<char> frameDatav = std::vector<char>(static_cast<size_t>((realSize.X * realSize.Y)));
};