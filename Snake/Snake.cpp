#include <iostream>
#include <Windows.h>
#include <vector>
#include <thread>
#include <string>
#include <list>
#include "mdwrap.h"
#include "randomness.h"
static constexpr unsigned int fps = 3;
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
    COORD headPos;
    Direction dir = none;
    std::list<COORD> body;
};
struct Game
{
    static constexpr COORD gameSize = { 40,20 };
    static constexpr COORD realSize = { gameSize.X + 3,gameSize.Y + 3 };
    COORD foodPos;
    bool gameOver = false;
    bool isFoodThere = false;
    bool frameChanged = false;
    std::uint32_t score = 0;
    std::vector<char> frameData = std::vector<char>(static_cast<size_t>((realSize.X * realSize.Y)));
    mdwrap wr = mdwrap(frameData,realSize.X);
    std::list<Direction> inputs;
    Snake s;
};
COORD getscreensize(HANDLE firstbuffer)
{
    CONSOLE_SCREEN_BUFFER_INFO bufferinfo;
    GetConsoleScreenBufferInfo(firstbuffer, &bufferinfo);
    const auto newscreenwidth = bufferinfo.srWindow.Right - bufferinfo.srWindow.Left + 1;
    const auto newscreenheight = bufferinfo.srWindow.Bottom - bufferinfo.srWindow.Top + 1;

    return COORD{ static_cast<short>(newscreenwidth), static_cast<short>(newscreenheight) };
}
void writebuffer(std::vector<char>& framedata,HANDLE consolebuffer)
{
    static bool bufferswitch = true;
    SetConsoleCursorPosition(consolebuffer, { 0,0 });
    WriteConsoleA(consolebuffer, &framedata.front(), static_cast<short>(framedata.size()), nullptr, nullptr);
}
void drawBoard(Game& g)
{
    static std::uint32_t scoreB = 0;
    static bool firstFrame = true;

        if (g.score != scoreB)
        {
            g.frameChanged = true;
            scoreB = g.score;
        }
        else if(!firstFrame)
        {
            return;
        }
    
    size_t writeOffset = 0;
   std::string scoreText = "Score: " + std::to_string(g.score);
   std::copy(scoreText.begin(), scoreText.end(), g.frameData.begin());
   std::string spaceFiller(g.gameSize.X+3 - scoreText.size(), ' ');
   spaceFiller.back() = '\n';
   std::copy(spaceFiller.begin(), spaceFiller.end(), g.frameData.begin() + scoreText.size());
   if (!firstFrame)
   {
       return;
   }
   writeOffset += scoreText.size();
   for (size_t i = 0; i < g.realSize.X; i++)
   {
       if (i == g.realSize.X-1)
       {
           g.wr[1][i] = '\n';
           break;
       }
       g.wr[1][i] = '#';
   }

   for (size_t i = 0; i < g.gameSize.Y; i++)
   {
       g.wr[2+i][0] = '#';
       for (size_t ii = 0; ii < g.gameSize.X; ii++)
       {
           g.wr[2+i][ii+1] = ' ';
       }
       g.wr[2 + i][g.realSize.X - 2] = '#';
       g.wr[2 + i][g.realSize.X - 1] = '\n';
   }
   for (size_t i = 0; i < g.realSize.X-1; i++)
   {
       g.wr[g.realSize.Y - 1][i] = '#';
   }

   short spawnX = randomInt<short>(5, g.gameSize.X-5); //inconsistent types because windows and I dont like to cast
   short spawnY = randomInt<short>(5, g.gameSize.Y-5);
   Direction dir = static_cast<Direction>(randomInt<short>(1,4)); //short because 1 byte types arent allowed in uniform_int_distribution
   COORD head = { spawnX,spawnY };
   g.s.body.push_back(head);
   g.s.headPos = { spawnX,spawnY };
   g.wr[spawnY][spawnX] = '%';
   COORD bodyCell;
   switch (dir)
   {
         case up:
         {
             g.s.dir = up;
             bodyCell = { spawnX,static_cast<short>(spawnY - 1) };
             break;
         }
         case down:
         {
             g.s.dir = down;
             bodyCell = { spawnX,static_cast<short>(spawnY + 1) };
             break;
         }
         case left:
         {
             g.s.dir = left;
             bodyCell = { static_cast<short>(spawnX+1),spawnY};
             break;
         }
         case right:
         {
             g.s.dir = right;
             bodyCell = { static_cast<short>(spawnX - 1),spawnY };
             break;
         }
   }
   g.s.body.push_back(bodyCell);
   g.wr[bodyCell.Y][bodyCell.X] = 'X';
   firstFrame = false;
}
bool isValidApplePos(Snake& s, short X, short Y)
{
    for (auto& bodyCell : s.body)
    {
        if (X == bodyCell.X && Y == bodyCell.Y)
        {
            return false;
        }

    }
    return true;
}
void drawApple(Game& g)
{
    if (!g.isFoodThere)
    {
        bool validPlace = false;
        short spawnX;
        short spawnY;
        while (validPlace == false) {
            spawnX = randomInt<short>(1, g.gameSize.X); //inconsistent types because windows and I dont like to cast
            spawnY = randomInt<short>(2, g.gameSize.Y);
            if (isValidApplePos(g.s, spawnX, spawnY))
            {
                validPlace = true;
            }
        }
        g.foodPos = { spawnX,spawnY };
        g.wr[spawnY][spawnX] = '@';
        g.isFoodThere = true;
        g.frameChanged = true;
        return;
    }
    return;
}
void handleInputs(Game& g)
{
    INPUT_RECORD irec;
    DWORD cc;
    HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD numberOfInputs = 0;
    GetNumberOfConsoleInputEvents(stdHandle, &numberOfInputs);
    if (numberOfInputs == 0)
    {
        return;
    }
    for (size_t i = 0; i < numberOfInputs; i++)
    {
        ReadConsoleInput(stdHandle, &irec, 1, &cc);
        if (irec.EventType == KEY_EVENT && ((KEY_EVENT_RECORD&)irec.Event).bKeyDown)
        {
            KEY_EVENT_RECORD krec = irec.Event.KeyEvent;
            switch (krec.wVirtualKeyCode)
            {
            case VK_UP:
            case 0x57:
            {
                if (g.s.dir != down)
                {
                    g.inputs.push_back(up);
                }
                return;
            }
            case VK_DOWN:
            case 0x53:
            {
                if (g.s.dir != up)
                {
                    g.inputs.push_back(down);
                }
                return;
            }
            case VK_LEFT:
            case 0x41:
            {
                if (g.s.dir == right)
                {
                    return;
                }
                g.inputs.push_back(left);
                return;
            }
            case VK_RIGHT:
            case 0x44:
            {
                if (g.s.dir == left)
                {
                    return;
                }
                g.inputs.push_back(right);
                return;
            }
            }

        }
    }
}
bool checkCollision(COORD gameSize, Snake& s, COORD coords)
{
    if (coords.Y == 1 || coords.Y == gameSize.Y + 1)
    {
        return true;
    }
    else if(coords.X == 0 || coords.X == gameSize.X+1)
    {
        return true;
    }
    if (coords.X == s.body.back().X && coords.Y == s.body.back().Y)
    {
        return false;
    }
    for (auto& cell : s.body)
    {
        if (coords.X == cell.X && coords.Y == cell.Y)
        {
            return true;
        }
    }
    return false;
}
void handleSnek(Game& g)
{
    if (!g.inputs.empty()) {
        g.s.dir = g.inputs.front();
        g.inputs.pop_front();
    }
    COORD newCoords = { g.s.headPos.X,g.s.headPos.Y };
    switch (g.s.dir)
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

    bool willCollide = checkCollision(g.gameSize,g.s, newCoords);
    if (willCollide)
    {
        g.gameOver = true;
        return;
    }
    bool ateApple = false;
    if (newCoords.X == g.foodPos.X && newCoords.Y == g.foodPos.Y)
    {
        g.score++;
        g.isFoodThere = false;
        g.s.body.push_front(newCoords);
        g.wr[newCoords.Y][newCoords.X] = '%';
        g.wr[g.s.headPos.Y][g.s.headPos.X] = 'X';
        g.s.headPos = newCoords;
        ateApple = true;
        g.frameChanged = true;
    }
    if (!ateApple)
    {
        COORD tailPos = g.s.body.back();
        g.wr[tailPos.Y][tailPos.X] = ' ';
        g.s.body.pop_back();
        g.s.body.push_front(newCoords);
        g.wr[newCoords.Y][newCoords.X] = '%';
        g.wr[g.s.headPos.Y][g.s.headPos.X] = 'X';
        g.s.headPos = newCoords;
        g.frameChanged = true;
    }
    if (!g.inputs.empty())
    {
        handleSnek(g);
    }
}
int main()
{
    HANDLE consoleBuffer = GetStdHandle(STD_OUTPUT_HANDLE);
    const auto screenSize = getscreensize(consoleBuffer);
    Game g;
    SetConsoleScreenBufferSize(consoleBuffer, screenSize);
    while (g.gameOver != true)
    {
        drawBoard(g);
        drawApple(g);
        handleInputs(g);
        handleSnek(g);

        if (g.frameChanged) {
           writebuffer(g.frameData,consoleBuffer);
            g.frameChanged = false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps));
    }
    std::string scoreString = "Your score was: " + std::to_string(g.score);
    MessageBoxA(NULL, scoreString.c_str(), "You lost!", 0);
    return 0;
}

