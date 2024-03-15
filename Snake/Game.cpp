#include "Game.h"

#include <string>

#include "randomness.h"
bool writeBuffer(const std::vector<char>& frameData, HANDLE consoleBuffer) {
	DWORD ret = SetConsoleCursorPosition(consoleBuffer, { 0, 0 }); //I use WINAPI instead of virtual terminal sequences because when I tried them they were buggy and wouldnt render the game properly.
	if (ret == 0)
	{
		DWORD error = GetLastError();
		std::string errString = "Failed to initialize (HANDLE): " + std::to_string(error);
		MessageBoxA(NULL, errString.c_str(), "FATAL ERROR", 0);
		return false;

	}
	WriteConsoleA(consoleBuffer, frameData.data(), frameData.size(), nullptr,nullptr);
	return true;
}
bool Game::isValidApplePos(short X, short Y) {
	for (const auto& bodyCell : dangerNoodle.body) {
		if (X == bodyCell.X && Y == bodyCell.Y) {
			return false;
		}
	}
	return true;
}
void Game::drawBoard() {
	static bool firstFrame = true;

	size_t writeOffset = 0;
	std::string scoreText = "Score: " + std::to_string(score);
	std::copy(scoreText.begin(), scoreText.end(), frameData[0].begin());
	std::string spaceFiller(gameSize.X + 3 - scoreText.size(), ' ');
	spaceFiller.back() = '\n';
	std::copy(spaceFiller.begin(), spaceFiller.end(),
		frameData[0].begin() + scoreText.size());
	if (!firstFrame) // board shouldnt be overwritten by anything so we need to draw it only once and then only score needs updating.
	{
		return;
	}
	writeOffset += scoreText.size();
	for (size_t i = 0; i < realSize.X; i++) {
		if (i == realSize.X - 1) {
			frameData[1][i] = '\n';
			break;
		}
		frameData[1][i] = '#';
	}

	for (size_t i = 0; i < gameSize.Y; i++) {
		frameData[2 + i][0] = '#';
		for (size_t ii = 0; ii < gameSize.X; ii++) {
			frameData[2 + i][ii + 1] = ' ';
		}
		frameData[2 + i][realSize.X - 2] = '#';
		frameData[2 + i][realSize.X - 1] = '\n';
	}
	for (size_t i = 0; i < realSize.X - 1; i++) {
		frameData[realSize.Y - 1][i] = '#';
	}

	short spawnX = randomInt<short>(5, gameSize.X - 5);
	short spawnY = randomInt<short>(5, gameSize.Y - 5);
	Direction dir = static_cast<Direction>(
		randomInt<short>(1, 4)); // short because 1 byte types arent allowed in
	// uniform_int_distribution
	COORD head = { spawnX, spawnY };
	dangerNoodle.body.push_back(head);
	dangerNoodle.headPos = { spawnX, spawnY };
	frameData[spawnY][spawnX] = '%';
	COORD bodyCell;
	switch (dir) {
	case up: {
		dangerNoodle.dir = up;
		bodyCell = { spawnX, spawnY-- };
		break;
	}
	case down: {
		dangerNoodle.dir = down;
		bodyCell = { spawnX, spawnY++ };
		break;
	}
	case left: {
		dangerNoodle.dir = left;
		bodyCell = { spawnX++, spawnY };
		break;
	}
	case right: {
		dangerNoodle.dir = right;
		bodyCell = { spawnX--, spawnY };
		break;
	}
	}
	dangerNoodle.body.push_back(bodyCell);
	frameData[bodyCell.Y][bodyCell.X] = 'X';
	firstFrame = false;
}
void Game::drawApple() {
	if (!isFoodThere) {
		bool validPlace = false;
		short spawnX;
		short spawnY;
		while (validPlace == false) {
			spawnX = randomInt<short>(1, gameSize.X);
			spawnY = randomInt<short>(2, gameSize.Y);
			if (isValidApplePos(spawnX, spawnY)) {
				validPlace = true;
			}
		}
		foodPos = { spawnX, spawnY };
		frameData[spawnY][spawnX] = '@';
		isFoodThere = true;
		return;
	}
	else
		return;
}
void Game::handleInputs() {
	INPUT_RECORD irec;
	DWORD cc;
	DWORD numberOfInputs = 0;
	GetNumberOfConsoleInputEvents(consoleInHandle, &numberOfInputs);
	if (numberOfInputs == 0) {
		DWORD err = GetLastError();
		return; //we ignore this error because its not really critical, game still works just inputs dont
	}
	for (size_t i = 0; i < numberOfInputs; i++) {
		ReadConsoleInput(consoleInHandle, &irec, 1, &cc); //same here
		if (irec.EventType == KEY_EVENT &&
			((KEY_EVENT_RECORD&)irec.Event).bKeyDown) {
			KEY_EVENT_RECORD krec = irec.Event.KeyEvent;
			switch (krec.wVirtualKeyCode) { //numbered cases are for WASD
			case VK_UP:
			case 0x57: {
				if (dangerNoodle.dir != down) {
					inputs.push_back(up);
				}
				return;
			}
			case VK_DOWN:
			case 0x53: {
				if (dangerNoodle.dir != up) {
					inputs.push_back(down);
				}
				return;
			}
			case VK_LEFT:
			case 0x41: {
				if (dangerNoodle.dir == right) {
					return;
				}
				inputs.push_back(left);
				return;
			}
			case VK_RIGHT:
			case 0x44: {
				if (dangerNoodle.dir == left) {
					return;
				}
				inputs.push_back(right);
				return;
			}
			}
		}
	}
}
bool Game::checkCollision(COORD coords)
{
	if (coords.Y == 1 || coords.Y == realSize.Y - 1)
	{
		return true;
	}
	else if (coords.X == 0 || coords.X == realSize.X - 2) 
	{
		return true;
	}
	if (coords.X == dangerNoodle.body.back().X && coords.Y == dangerNoodle.body.back().Y)
	{
		return false;
	}
	for (const auto& cell : dangerNoodle.body)
	{
		if (coords.X == cell.X && coords.Y == cell.Y)
		{
			return true;
		}
	}
	return false;
}
void Game::handleSnek()
{
	if (!inputs.empty()) {
		dangerNoodle.dir = inputs.front();
		inputs.pop_front();
	}
	COORD newCoords = { dangerNoodle.headPos.X, dangerNoodle.headPos.Y };
	switch (dangerNoodle.dir)
	{
	case up:
	{
		newCoords.Y--;
		break;
	}
	case down:
	{
		newCoords.Y++;
		break;
	}
	case left:
	{
		newCoords.X--;
		break;
	}
	case right:
	{
		newCoords.X++;
		break;
	}
	}

	bool willCollide = checkCollision(newCoords);
	if (willCollide)
	{
		gameOver = true;
		return;
	}
	bool ateApple = false;
	if (newCoords.X == foodPos.X && newCoords.Y == foodPos.Y)
	{
		score++;
		if (score % 10 == 0 && fps < 12)
		{
			fps++; //lets speed up the game to make it harder over time
		}
		isFoodThere = false;
		dangerNoodle.body.push_front(newCoords);
		frameData[newCoords.Y][newCoords.X] = '%';
		frameData[dangerNoodle.headPos.Y][dangerNoodle.headPos.X] = 'X';
		dangerNoodle.headPos = newCoords;
		ateApple = true;
	}
	if (!ateApple)
	{
		COORD tailPos = dangerNoodle.body.back();
		frameData[tailPos.Y][tailPos.X] = ' ';
		dangerNoodle.body.pop_back();
		dangerNoodle.body.push_front(newCoords);
		frameData[newCoords.Y][newCoords.X] = '%';
		frameData[dangerNoodle.headPos.Y][dangerNoodle.headPos.X] = 'X';
		dangerNoodle.headPos = newCoords;
	}
	if (!inputs.empty())
	{
		handleSnek();
	}
}
bool Game::writeFrame()
{
	return writeBuffer(frameDatav, consoleOutHandle);
}

bool Game::initialize()
{
	consoleOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 1;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleOutHandle, &info);
	consoleInHandle = GetStdHandle(STD_INPUT_HANDLE);
	if (consoleOutHandle == INVALID_HANDLE_VALUE || consoleInHandle == INVALID_HANDLE_VALUE)
	{
		DWORD error = GetLastError();
		std::string errString = "Failed to initialize (HANDLE): " + std::to_string(error);
		MessageBoxA(NULL, errString.c_str(), "FATAL ERROR", 0);
		return false;
	}
	else if (consoleOutHandle == NULL || consoleInHandle == NULL)
	{
		MessageBoxA(NULL, "The game was launched in an environment without handles.", "FATAL ERROR", 0);
		return false;
	}
	else {
		CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
		BOOL ret = GetConsoleScreenBufferInfo(consoleOutHandle, &bufferInfo);
		if (ret == 0)
		{
			DWORD error = GetLastError();
			std::string errString = "Failed to initialize (BUFFERINFO): " + std::to_string(error);
			MessageBoxA(NULL, errString.c_str(), "FATAL ERROR", 0);
			return false;
		}
		bool setBuffer = false;
		if (bufferInfo.dwSize.X < realSize.X)
		{
			setBuffer = true;
			bufferInfo.dwSize.X = realSize.X;
		}
		if (bufferInfo.dwSize.Y < realSize.Y)
		{
			setBuffer = true;
			bufferInfo.dwSize.Y = realSize.Y;
		}
		if (setBuffer)
		{
			ret = SetConsoleScreenBufferSize(consoleOutHandle, bufferInfo.dwSize);
			if (ret == 0)
			{
				DWORD error = GetLastError();
				std::string errString = "Failed to initialize (BUFFERSIZE): " + std::to_string(error);
				MessageBoxA(NULL, errString.c_str(), "FATAL ERROR", 0);
				return false;
			}
		}
		return true;
	}
}