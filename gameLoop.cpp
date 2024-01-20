#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
using namespace std;

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 540
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool gameIsRunning = false,
     gameIsStarted = false,
     collisionDetected = false,
     snakeAteFruit = false,
     snakeAteBonusFruit = false,
     keyWasPressed = false,
     gameIsPaused = false,
     fontIsInitialized = false,
     increaseOpacity = false,
     collisionSoundPlayed = false,
     bonusFruitTimerFlag = false,
     bonusFruitPositionFound = false,
     pauseScreenSoundPlayed = false,
     newHighScore = false,
     startTheGameNow = false,
     newLuckyInterVal = false;

// global variable for updating states of the game
const int totalWaterTexture = 2,
          totalBoarderTexture = 1,
          totalMenuTexture = 1,
          totalFruitTexture = 6,
          totalFruitEatingSound = 3,
          totalCollisionSound = 3,
          totalBackgroundSound = 3,
          snakeInitialVelocity = 6,
          snakeColorRed = 100,
          snakeColorGreen = 255,
          snakeColorBlue = 100,
          snakeColorAlpha = SDL_ALPHA_OPAQUE,
          boarderWidth = 15,
          boarderHeight = 15,
          fruitWidth = 30,
          fruitHeight = 30,
          snakeWidth = 20,
          snakeHeight = 20;
int snakeVelocity = snakeInitialVelocity,
    pickFruit,
    totalScore = 0,
    taskBackgroundFinished = 0,
    vSyncActive,
    prevResult = 0;

uint32_t startTime;
Uint8 textOpacity = 255;
char snakeCurrentDirection = 'r', // by default snake is facing the right side
    pressedKey;

vector<SDL_Rect> snakeBody;
string userName = "";
SDL_Rect rect1, rect2, rect3, rect4, boarderTopRect, boarderBottomRect, boarderLeftRect, boarderRightRect,
    fruitControl, bonusFruitControl, middleScreenTextRect, scoreTextRect, screenTopTextRect, screenBottomBoxRectangle, screenBottomLeftBox;
SDL_Texture *textureWater[totalWaterTexture],
    *textureBoarder[totalBoarderTexture],
    *textureMenu[totalMenuTexture],
    *textureFruit[totalFruitTexture],
    *textureSnakeSkin;

void processInput(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            gameIsRunning = false;
            break;
        case SDL_KEYDOWN:
            keyWasPressed = true;

            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
            case SDLK_w:
                pressedKey = 'w';
                break;
            case SDLK_DOWN:
            case SDLK_s:
                pressedKey = 's';
                break;
            case SDLK_RIGHT:
            case SDLK_d:
                pressedKey = 'd';
                break;
            case SDLK_LEFT:
            case SDLK_a:
                pressedKey = 'a';
                break;
            case SDLK_SPACE:
            case SDLK_k:
                keyWasPressed = false;
                if (!gameIsStarted && !collisionDetected && startTheGameNow)
                    gameIsStarted = true;
                else
                {
                    if (gameIsPaused)
                        gameIsPaused = false;
                    else
                        gameIsPaused = true;
                }
                if (collisionDetected && collisionSoundPlayed)
                    pressedKey = 'k'; // k and space both key will do the same thing
                break;
            case SDLK_q:
                gameIsRunning = false;
                break;
            default:
                keyWasPressed = false;
                break;
            }
            break;
        default:
            break;
        }
    }
}

class Snake
{
public:
    int incraseSpeedAfter;
    // intialize snake using constructor
    Snake(string);
    void updateSnakePosition(int);
    void updateSnakeSize(void);
    void updateSnakeDirection(char);

private:
    SDL_Rect initialSnake;
};

class Fruit
{
public:
    // initialize the fruit using constructor
    Fruit();
};

class Font
{
public:
    Font()
    {
        if (TTF_Init() == -1)
        {
            printf("TTF failed to initialize\nError: %s\n",
                   TTF_GetError());
        }
    }
};

class Music
{
public:
    Music(string filePath, bool isMusic)
    {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
            cout << "Audio Library Is Not Working\nError: " << Mix_GetError() << endl;

        if (isMusic)
            music = Mix_LoadMUS(filePath.c_str());
        else
            chunk = Mix_LoadWAV(filePath.c_str());
    }

    void playMusic(int loops)
    {
        // loops = 0 for 0
        //-1 for foreever loop
        if (music != nullptr)
            Mix_PlayMusic(music, loops);
    }

    int playChuck(int channel, int loops)
    {
        if (chunk != nullptr)
            return Mix_PlayChannel(channel, chunk, loops);
        return -1;
    }

    void pauseMusic()
    {
        Mix_PauseMusic();
    }

    void stopMusic()
    {
        Mix_HaltMusic();
        Mix_HaltChannel(-1); //-1 means halt all channels
    }

private:
    Mix_Music *music;
    Mix_Chunk *chunk;
};

class Background
{
public:
    Background() : pauseSound("./Sounds/pauseGame.wav", false),
                   resumeSound("./Sounds/resumeGame.wav", false),
                   bonusFruitSound("./Sounds/eatBonusFruit.wav", false)
    {
        gameIsRunning = initializeWindow();
        initializeBackground();
    }
    bool initializeWindow();
    void initializeBackground();
    void resetGame();
    void backgroundActivities();
    void getUserName();

private:
    vector<Music> backgroundMusic, fruitEatingSound, collisionSound;
    Music pauseSound, resumeSound, bonusFruitSound;
};

class drawFunction
{
public:
    void drawBackground(void);
    void drawFruit(void);
    void drawBonusFruit(void);
    SDL_Texture *drawText(const char *, const char *, int, SDL_Color);
    void drawMiddleScreenText(drawFunction &, const char *);
    void drawScore(drawFunction &, const char *);
    void drawAllTimeBestScore(drawFunction &, Snake *);

private:
    SDL_Texture *texture;
};

class Collision
{
public:
    bool detectCollision(void);
};

void destroyBackground(Fruit *fruit, Snake *snake, Font *font, Background *background)
{
    delete fruit;
    delete snake;
    delete font;
    delete background;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    for (int i = 0; i < totalWaterTexture; i++)
        SDL_DestroyTexture(textureWater[i]);
    for (int i = 0; i < totalBoarderTexture; i++)
        SDL_DestroyTexture(textureBoarder[i]);
    for (int i = 0; i < totalFruitTexture; i++)
        SDL_DestroyTexture(textureFruit[i]);
    for (int i = 0; i < totalMenuTexture; i++)
        SDL_DestroyTexture(textureMenu[i]);
    SDL_DestroyTexture(textureSnakeSkin);
    TTF_Quit();
    SDL_Quit();
}

void Update(Snake *snake, drawFunction &draw, Collision &collision, Background *background)
{
    SDL_RenderClear(renderer);

    draw.drawBackground();
    draw.drawFruit();
    draw.drawScore(draw, to_string(totalScore).c_str()); // convert int to string then character array then pass to drawScore

    // detect collision
    if (collision.detectCollision())
    {
        draw.drawAllTimeBestScore(draw, snake);
        string message = "       Game Over\n     Final Score: " + to_string(totalScore) + "\nPress Space To Play Again";
        draw.drawMiddleScreenText(draw, message.c_str());
        gameIsStarted = false;
        collisionDetected = true;
    }

    if (gameIsStarted)
    {
        if (gameIsPaused)
            draw.drawMiddleScreenText(draw, "Press Space To Resume"); // draw pause screen
        else
        {
            if (keyWasPressed)
            {
                snake->updateSnakeDirection(pressedKey);
                keyWasPressed = false;
            }
            snake->updateSnakePosition(snakeVelocity);
        }

        if (bonusFruitTimerFlag)
        {
            draw.drawBonusFruit();
            double elapsedTime = (SDL_GetTicks() - startTime) / 1000.0;
            if (elapsedTime >= 4)
            {
                bonusFruitPositionFound = false;
                bonusFruitTimerFlag = false;
            }
        }

        if (snakeAteFruit)
        {
            totalScore++;
            snake->updateSnakeSize();
            snakeAteFruit = false;

            // check for bonus fruit
            if (!bonusFruitTimerFlag && totalScore % 7 == 0)
            {
                bonusFruitTimerFlag = true;
                startTime = SDL_GetTicks();
            }
        }

        // increase snake speed up to 15;
        if ((totalScore / snake->incraseSpeedAfter) > prevResult && snakeVelocity < 15)
        {
            snakeVelocity++;
            SDL_Log("New Snake Speed: %d->%d", snakeVelocity - 1, snakeVelocity);
            prevResult = (totalScore / snake->incraseSpeedAfter);
        }
    }
    else if (!collisionDetected)
    {
        // do these operations before starting the game
        while (userName.empty())
            background->getUserName();

        if (!userName.empty())
        {
            draw.drawMiddleScreenText(draw, "Press Space To Start");
            startTheGameNow = true;
        }
    }

    if (gameIsPaused || !gameIsStarted || collisionDetected)
        snake->updateSnakePosition(0); // This Will Draw Current State Of The Snake

    // vSync failed to activate so a 20 ms delay is the adjustment
    if (vSyncActive != 0)
        SDL_Delay(20);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    // make class objects
    drawFunction draw;
    Collision collision;

    Background *background = new Background();
    Font *font = new Font();
    Snake *snake = new Snake("./Snake/skin.bmp");
    Fruit *fruit = new Fruit();

    // run background activities in a diffrent thread
    thread taskThread(&Background::backgroundActivities, background);

    while (gameIsRunning)
    {
        processInput();
        // reset the game
        if (collisionDetected && pressedKey == 'k')
        {
            delete snake;
            delete fruit;
            background->resetGame();
            snake = new Snake("./Snake/skin.bmp");
            fruit = new Fruit();
        }
        Update(snake, draw, collision, background);
    }

    taskThread.join(); // wait for the thread to finish
    destroyBackground(fruit, snake, font, background);
}

// implimentation of all prototype functions
bool Background ::initializeWindow(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("Error: SDL Failed to initialize\n SDL Error: %s", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "Snake Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0);

    if (!window)
    {
        printf("Error: Failed to open window\nSDL Error: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // set vsync on to adjust frame rate with the system
    // vSyncActive = 0 means success and any non zero value for failure
    vSyncActive = SDL_RenderSetVSync(renderer, 1);

    if (!renderer)
    {
        printf("Error: Failed to create renderer\nSDl Error: %s", SDL_GetError());
        return false;
    }

    return true;
}

void Background ::initializeBackground(void)
{
    // initialize score showing rectangle
    scoreTextRect.w = 60;
    scoreTextRect.h = 70;
    scoreTextRect.x = SCREEN_WIDTH - scoreTextRect.w;
    scoreTextRect.y = 0;

    // intitial position of the background image
    rect1.x = 0;
    rect1.y = 0;
    rect1.w = SCREEN_WIDTH;
    rect1.h = SCREEN_HEIGHT;

    rect2.x = -SCREEN_WIDTH;
    rect2.y = 0;
    rect2.w = SCREEN_WIDTH;
    rect2.h = SCREEN_HEIGHT;

    rect3.x = 0;
    rect3.y = 0;
    rect3.w = SCREEN_WIDTH;
    rect3.h = SCREEN_HEIGHT;

    rect4.x = 0;
    rect4.y = -SCREEN_HEIGHT;
    rect4.w = SCREEN_WIDTH;
    rect4.h = SCREEN_HEIGHT;

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    surface = SDL_LoadBMP("./Background/water.bmp");
    textureWater[0] = SDL_CreateTextureFromSurface(renderer, surface);
    textureWater[1] = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    // initialize boarder
    boarderTopRect.x = 0;
    boarderTopRect.y = 0;
    boarderTopRect.w = SCREEN_WIDTH - scoreTextRect.w;
    boarderTopRect.h = boarderHeight;

    boarderBottomRect.w = SCREEN_WIDTH;
    boarderBottomRect.h = boarderHeight;
    boarderBottomRect.x = 0;
    boarderBottomRect.y = SCREEN_HEIGHT - boarderBottomRect.h;

    boarderLeftRect.x = 0;
    boarderLeftRect.y = boarderHeight;
    boarderLeftRect.w = boarderWidth;
    boarderLeftRect.h = SCREEN_HEIGHT - (2 * boarderLeftRect.y);

    boarderRightRect.w = boarderWidth;
    boarderRightRect.x = SCREEN_WIDTH - boarderRightRect.w;
    boarderRightRect.y = scoreTextRect.h;
    boarderRightRect.h = SCREEN_HEIGHT - boarderRightRect.y - boarderHeight;

    textureBoarder[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Background/boarder.bmp"));

    // intialize middle screen message rectangle
    middleScreenTextRect.w = 600;
    middleScreenTextRect.h = 100;
    middleScreenTextRect.x = (SCREEN_WIDTH - middleScreenTextRect.w) / 2;
    middleScreenTextRect.y = (SCREEN_HEIGHT - middleScreenTextRect.h) / 2;

    // initialize top Screen Text rectangle
    screenTopTextRect.w = 800;
    screenTopTextRect.h = 140;
    screenTopTextRect.x = (SCREEN_WIDTH - screenTopTextRect.w) / 2;
    screenTopTextRect.y = boarderHeight + 30;

    // initialize bottom screen box rectangle
    screenBottomBoxRectangle.w = 200;
    screenBottomBoxRectangle.h = SCREEN_HEIGHT - boarderHeight - scoreTextRect.h;
    screenBottomBoxRectangle.x = (SCREEN_WIDTH - screenBottomBoxRectangle.w) - boarderWidth;
    screenBottomBoxRectangle.y = (SCREEN_HEIGHT - screenBottomBoxRectangle.h) - boarderHeight;

    // inititalize bottom left screen text rectangle
    screenBottomLeftBox.w = (middleScreenTextRect.x - boarderWidth);
    screenBottomLeftBox.h = SCREEN_HEIGHT - 2 * boarderHeight - screenTopTextRect.h - 30;
    screenBottomLeftBox.x = boarderWidth;
    screenBottomLeftBox.y = screenTopTextRect.y + screenTopTextRect.h;

    // initialize fruit eating and background sounds
    for (int i = 0; i < totalBackgroundSound; i++)
        backgroundMusic.emplace_back("./Sounds/backgroundTrack" + to_string(i + 1) + ".mp3", true);
    for (int i = 0; i < totalFruitEatingSound; i++)
        fruitEatingSound.emplace_back("./Sounds/eatFruit" + to_string(i + 1) + ".wav", false);
    for (int i = 0; i < totalCollisionSound; i++)
        collisionSound.emplace_back("./Sounds/gameOver" + to_string(i + 1) + ".wav", false);
}

void Background::backgroundActivities(void)
{
    // this function will run in a diffrent thread

    while (gameIsRunning)
    {
        if (gameIsStarted && !collisionDetected)
        {
            if (!Mix_PlayingMusic())
                backgroundMusic[rand() % totalBackgroundSound].playMusic(0);
            if (gameIsPaused && !pauseScreenSoundPlayed)
            {
                //-1 to play the chunk on the first free channel
                pauseSound.playChuck(0, 0);
                pauseScreenSoundPlayed = true;
            }
            if (!gameIsPaused && pauseScreenSoundPlayed)
            {
                resumeSound.playChuck(0, 0);
                pauseScreenSoundPlayed = false;
            }

            if (snakeAteBonusFruit)
            {
                bonusFruitSound.playChuck(0, 0);
                snakeAteBonusFruit = false;
            }
            if (snakeAteFruit)
                fruitEatingSound[rand() % totalFruitEatingSound].playChuck(0, 0);
        }
        else
        {
            if (collisionDetected && !collisionSoundPlayed) // play collision sound only once
            {
                int channel = collisionSound[rand() % totalCollisionSound].playChuck(-1, 0);

                while (Mix_Playing(channel) == 1)
                {
                    if (!gameIsRunning)
                        break;
                    continue;
                }
                collisionSoundPlayed = true;
            }
            pauseSound.stopMusic();
        }
    }
}

void Background ::resetGame()
{
    gameIsStarted = true;
    collisionDetected = false;
    snakeAteFruit = false;
    keyWasPressed = false;
    gameIsPaused = false;
    increaseOpacity = false;
    collisionSoundPlayed = false;
    bonusFruitTimerFlag = false;
    pauseScreenSoundPlayed = false;
    newHighScore = false;
    newLuckyInterVal = false;
    totalScore = 0;
    prevResult = 0;
    snakeVelocity = snakeInitialVelocity;
    snakeBody.clear();
    snakeCurrentDirection = 'r';
    pressedKey = '/'; // default value;
}

void Background ::getUserName(void)
{
    string userInput = "";
    cout << "Enter Your Name Here: ";
    getline(cin, userInput);
    for (int i = 0; i < userInput.size(); i++)
    {
        if (userInput[i] == ':')
            break;
        userName.push_back(userInput[i]);
    }
}

Fruit ::Fruit()
{
    // create texture for all available fruits
    textureFruit[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/apple.bmp"));
    textureFruit[1] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/grapes.bmp"));
    textureFruit[2] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/cherry.bmp"));
    textureFruit[3] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/banana.bmp"));
    textureFruit[4] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/pear.bmp"));
    textureFruit[5] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/bonusFruit.bmp"));

    // find the co-ordinate of the fruit
    fruitControl.w = fruitWidth;
    fruitControl.h = fruitHeight;
    fruitControl.x = rand() % SCREEN_WIDTH;
    fruitControl.y = rand() % SCREEN_HEIGHT;
    // avoid snake body & other obstacles
    while (SDL_HasIntersection(&fruitControl, &snakeBody[0]) ||
           SDL_HasIntersection(&fruitControl, &scoreTextRect) ||
           SDL_HasIntersection(&fruitControl, &boarderTopRect) ||
           SDL_HasIntersection(&fruitControl, &boarderBottomRect) ||
           SDL_HasIntersection(&fruitControl, &boarderLeftRect) ||
           SDL_HasIntersection(&fruitControl, &boarderRightRect))
    {
        fruitControl.x = rand() % SCREEN_WIDTH;
        fruitControl.y = rand() % SCREEN_HEIGHT;
    }

    // intialize bonus fruit
    bonusFruitControl.w = fruitWidth;
    bonusFruitControl.h = fruitHeight;

    // pick A random fruit;
    pickFruit = rand() % (totalFruitTexture - 1);
}

Snake ::Snake(string filePath)
{
    initialSnake.w = snakeWidth;
    initialSnake.h = snakeHeight;
    initialSnake.x = boarderWidth + 1;
    initialSnake.y = (SCREEN_HEIGHT - initialSnake.w) / 2;

    textureSnakeSkin = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP(filePath.c_str()));

    snakeBody.push_back(initialSnake);

    // speed increase after a certain score
    // use the current system time ass seed to generate pseudo-random values;
    srand((unsigned)time(0));
    incraseSpeedAfter = 10 + rand() % 11;
    SDL_Log("Increase Snake Speed After: %d", incraseSpeedAfter);
}

void drawFunction ::drawBackground(void)
{
    // draw background images
    rect1.x++;
    if (rect1.x >= SCREEN_WIDTH)
        rect1.x = -SCREEN_WIDTH;

    rect2.x++;
    if (rect2.x >= SCREEN_WIDTH)
        rect2.x = -SCREEN_WIDTH;

    rect3.y++;
    if (rect3.y >= SCREEN_HEIGHT)
        rect3.y = -SCREEN_HEIGHT;

    rect4.y++;
    if (rect4.y >= SCREEN_HEIGHT)
        rect4.y = -SCREEN_HEIGHT;

    SDL_RenderCopy(renderer, textureWater[0], NULL, &rect1);
    SDL_RenderCopy(renderer, textureWater[0], NULL, &rect2);

    SDL_RenderCopy(renderer, textureWater[1], NULL, &rect3);
    SDL_RenderCopy(renderer, textureWater[1], NULL, &rect4);

    SDL_SetTextureBlendMode(textureWater[1], SDL_BLENDMODE_MOD);

    // draw boarder
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &boarderTopRect);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &boarderBottomRect);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &boarderLeftRect);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &boarderRightRect);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &scoreTextRect);
    SDL_SetTextureBlendMode(textureBoarder[0], SDL_BLENDMODE_ADD);
}

void drawFunction ::drawFruit(void)
{
    // snake ate the fruit
    if (SDL_HasIntersection(&fruitControl, &snakeBody[0]))
    {
        snakeAteFruit = true;
        // spawn new fruit. i.e., pick a random fruit & also avoid snake body
        pickFruit = rand() % (totalFruitTexture - 1);
        // avoid snake body & bonus fruit
        bool fruitInsideSnake = true;
        while (fruitInsideSnake)
        {
            fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - 2 * boarderWidth));
            fruitControl.y = boarderHeight + (rand() % (SCREEN_HEIGHT - fruitHeight - 2 * boarderHeight));

            if (SDL_HasIntersection(&fruitControl, &scoreTextRect))
                continue;

            // avoid bonus fruit if exists
            if (bonusFruitPositionFound)
            {
                if (SDL_HasIntersection(&bonusFruitControl, &fruitControl))
                    continue;
            }
            // avoid snake body
            for (int i = 0; i < snakeBody.size(); i++)
            {
                if (SDL_HasIntersection(&snakeBody[i], &fruitControl))
                {
                    fruitInsideSnake = true;
                    break;
                }
                else
                    fruitInsideSnake = false;
            }
        }
    }
    SDL_RenderCopy(renderer, textureFruit[pickFruit], NULL, &fruitControl);
}

void drawFunction ::drawBonusFruit(void)
{
    int bonusFruitIndex = 5; // index of bonus fruit
    if (!bonusFruitPositionFound)
    {
        bonusFruitPositionFound = true;
        // spawn bonus fruit and avoid snake body or the other fruit;
        // avoid snake body & regular fruit
        bool fruitInsideSnake = true;
        while (fruitInsideSnake)
        {
            bonusFruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
            bonusFruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));

            // avoid regular fruit
            if ((bonusFruitControl.x >= fruitControl.x && bonusFruitControl.x <= (fruitControl.x + fruitWidth)) || ((bonusFruitControl.x + fruitWidth) >= fruitControl.x && (bonusFruitControl.x + fruitWidth) <= (fruitControl.x + fruitWidth)))
                continue;
            // avoid snake body
            for (int i = 0; i < snakeBody.size(); i++)
            {
                if (((snakeBody[i].x + snakeWidth) >= bonusFruitControl.x && snakeBody[i].x <= (bonusFruitControl.x + fruitWidth)) &&
                    ((snakeBody[i].y + snakeHeight) >= bonusFruitControl.y && snakeBody[i].y <= (bonusFruitControl.y + fruitHeight)))
                {
                    fruitInsideSnake = true;
                    break;
                }
                else
                    fruitInsideSnake = false;
            }
        }
    }
    SDL_RenderCopy(renderer, textureFruit[bonusFruitIndex], NULL, &bonusFruitControl);

    // check whether the snake ate the bonus fruit
    if (((snakeBody[0].x + snakeWidth) >= bonusFruitControl.x && snakeBody[0].x <= (bonusFruitControl.x + fruitWidth)) &&
        ((snakeBody[0].y + snakeHeight) >= bonusFruitControl.y && snakeBody[0].y <= (bonusFruitControl.y + fruitHeight)))
    {
        snakeAteBonusFruit = true;
        bonusFruitPositionFound = false;
        bonusFruitTimerFlag = false;
        totalScore += 10;
    }
}

SDL_Texture *drawFunction ::drawText(const char *fontFile, const char *message, int size, SDL_Color textColor)
{
    TTF_Font *font = TTF_OpenFont(fontFile, size);
    SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(font, message, textColor, middleScreenTextRect.w / 2 + 50);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    TTF_CloseFont(font);
    SDL_FreeSurface(surface);

    return texture;
}

void drawFunction ::drawMiddleScreenText(drawFunction &draw, const char *message)
{
    // create a blinking effect with opacity of the text
    if (textOpacity < 80)
        increaseOpacity = true;
    if (textOpacity >= 250)
        increaseOpacity = false;
    if (increaseOpacity)
        textOpacity += 10;
    else
        textOpacity -= 10;
    SDL_RenderCopy(renderer,
                   draw.drawText("Fonts/robotoMonoRegular.ttf", message, 24, {255, 255, 255, textOpacity}),
                   NULL, &middleScreenTextRect);
}

void drawFunction ::drawScore(drawFunction &draw, const char *score)
{
    SDL_RenderCopy(renderer,
                   draw.drawText("Fonts/robotoMonoBold.ttf", score, 18, {0, 0, 0, 255}),
                   NULL, &scoreTextRect);
}

void drawFunction ::drawAllTimeBestScore(drawFunction &draw, Snake *snake)
{
    // calculate all time best
    // there are five all time best scores are saved
    string highestVelocityFromFile;
    int totalSavedScores = 15;
    vector<pair<int, string>> allTimeBestScores;
    ifstream in;
    in.open("./Files/saveHighestScore.txt");
    string getLineFromFile;
    for (int i = 0; i < totalSavedScores; i++)
    {
        getline(in, getLineFromFile);
        string getScoreFromLine = "",
               getNameFromLine = "";
        bool startTakingScore = false;
        for (int i = 0; i < getLineFromFile.size(); i++)
        {
            if (startTakingScore)
                getScoreFromLine.push_back(getLineFromFile[i]);
            else
            {
                if (getLineFromFile[i] == ':')
                    startTakingScore = true;
                getNameFromLine.push_back(getLineFromFile[i]);
            }
        }

        allTimeBestScores.push_back({stoi(getScoreFromLine), getNameFromLine});
    }
    sort(allTimeBestScores.begin(), allTimeBestScores.end());
    in.close();

    // work with velocity file
    ifstream inVelocity;
    inVelocity.open("./Files/saveHighestVelocity.txt");
    getline(inVelocity, highestVelocityFromFile);
    inVelocity.close();

    int allTimeHighestVelocity = stoi(highestVelocityFromFile);
    if (snakeVelocity >= allTimeHighestVelocity)
        allTimeHighestVelocity = snakeVelocity;
    string highestVelocity = "All Time Best Velocity: " +
                             to_string(allTimeHighestVelocity);

    SDL_Color newScore = {255, 10, 10, SDL_ALPHA_OPAQUE},
              prevScore = {0, 255, 0, SDL_ALPHA_OPAQUE};
    if (totalScore > allTimeBestScores[0].first && !newHighScore)
    {
        newHighScore = true;
        allTimeBestScores[0].first = totalScore;
        allTimeBestScores[0].second = userName + ':';
        sort(allTimeBestScores.begin(), allTimeBestScores.end());
    }

    // work with luckiest interval file here
    int totalSavedIntervals = 10;
    vector<pair<int, string>> allTimeBestIntervals;
    ifstream inInterval;
    inInterval.open("./Files/luckiestInterval.txt");
    for (int i = 0; i < totalSavedIntervals; i++)
    {
        getline(inInterval, getLineFromFile);
        string getIntervalFromLine = "",
               getNameFromLine = "";
        bool startTakingInterval = false;
        for (int i = 0; i < getLineFromFile.size(); i++)
        {
            if (startTakingInterval)
                getIntervalFromLine.push_back(getLineFromFile[i]);
            else
            {
                if (getLineFromFile[i] == ':')
                    startTakingInterval = true;
                getNameFromLine.push_back(getLineFromFile[i]);
            }
        }

        allTimeBestIntervals.push_back({stoi(getIntervalFromLine), getNameFromLine});
    }
    sort(allTimeBestIntervals.begin(), allTimeBestIntervals.end());
    inInterval.close();

    if (snake->incraseSpeedAfter > allTimeBestIntervals[0].first && !newLuckyInterVal)
    {
        newLuckyInterVal = true;
        allTimeBestIntervals[0].first = snake->incraseSpeedAfter;
        allTimeBestIntervals[0].second = userName + ':';
        sort(allTimeBestIntervals.begin(), allTimeBestIntervals.end());
    }

    // start rendering text from here
    string highestScore = "All Time Best Score: " + to_string(allTimeBestScores[totalSavedScores - 1].first);
    SDL_RenderCopy(renderer,
                   draw.drawText("Fonts/robotoMonoRegular.ttf", (highestScore + "\n" + highestVelocity).c_str(), 20, (newHighScore) ? newScore : prevScore),
                   NULL, &screenTopTextRect);

    // write to the velocity file
    ofstream outVelocity;
    outVelocity.open("./Files/saveHighestVelocity.txt");
    outVelocity << allTimeHighestVelocity;
    outVelocity.close();

    // write to the score file
    ofstream out;
    out.open("./Files/saveHighestScore.txt");
    for (int i = totalSavedScores - 1; i >= 0; i--)
        out << allTimeBestScores[i].second << allTimeBestScores[i].first << endl;
    out.close();

    // show some best scores
    SDL_RenderCopy(renderer,
                   draw.drawText("Fonts/robotoMonoRegular.ttf", ("All Time Best Scores\n\n" + allTimeBestScores[totalSavedScores - 1].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 1].first) + "\n" + allTimeBestScores[totalSavedScores - 2].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 2].first) + "\n" + allTimeBestScores[totalSavedScores - 3].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 3].first) + "\n" + allTimeBestScores[totalSavedScores - 4].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 4].first) + "\n" + allTimeBestScores[totalSavedScores - 5].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 5].first) + "\n" + allTimeBestScores[totalSavedScores - 6].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 6].first) + "\n" + allTimeBestScores[totalSavedScores - 7].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 7].first) + "\n" + allTimeBestScores[totalSavedScores - 8].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 8].first) + "\n" + allTimeBestScores[totalSavedScores - 9].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 9].first) + "\n" + allTimeBestScores[totalSavedScores - 10].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 10].first) + "\n" + allTimeBestScores[totalSavedScores - 11].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 11].first) + "\n" + allTimeBestScores[totalSavedScores - 12].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 12].first) + "\n" + allTimeBestScores[totalSavedScores - 13].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 13].first) + "\n" + allTimeBestScores[totalSavedScores - 14].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 14].first) + "\n" + allTimeBestScores[totalSavedScores - 15].second + ' ' + to_string(allTimeBestScores[totalSavedScores - 15].first)).c_str(), 26, {255, 0, 255, SDL_ALPHA_OPAQUE}),
                   NULL, &screenBottomBoxRectangle);

    // write to the luckiest file
    ofstream outInterval;
    outInterval.open("./Files/luckiestInterval.txt");
    for (int i = totalSavedIntervals - 1; i >= 0; i--)
        outInterval << allTimeBestIntervals[i].second << allTimeBestIntervals[i].first << endl;
    outInterval.close();

    // show some best intervals
    SDL_RenderCopy(renderer,
                   draw.drawText("Fonts/robotoMonoRegular.ttf", ("\tAll Time Best\n\tIntervals\n\n\t" + allTimeBestIntervals[totalSavedIntervals - 1].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 1].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 2].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 2].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 3].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 3].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 4].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 4].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 5].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 5].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 6].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 6].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 7].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 7].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 8].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 8].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 9].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 9].first) + "\n\t" + allTimeBestIntervals[totalSavedIntervals - 10].second + ' ' + to_string(allTimeBestIntervals[totalSavedIntervals - 10].first)).c_str(), 26, {255, 0, 255, SDL_ALPHA_OPAQUE}),
                   NULL, &screenBottomLeftBox);
}

void Snake ::updateSnakePosition(int snakeVelocity)
{
    if (snakeVelocity)
    {
        switch (snakeCurrentDirection)
        {
        case 'r':
            for (int i = snakeBody.size() - 1; i >= 1; i--)
            {
                snakeBody[i].x = snakeBody[i - 1].x;
                snakeBody[i].y = snakeBody[i - 1].y;
            }
            snakeBody[0].x += snakeVelocity;
            break;
        case 'l':
            for (int i = snakeBody.size() - 1; i >= 1; i--)
            {
                snakeBody[i].x = snakeBody[i - 1].x;
                snakeBody[i].y = snakeBody[i - 1].y;
            }
            snakeBody[0].x -= snakeVelocity;
            break;
        case 'u':
            for (int i = snakeBody.size() - 1; i >= 1; i--)
            {
                snakeBody[i].x = snakeBody[i - 1].x;
                snakeBody[i].y = snakeBody[i - 1].y;
            }
            snakeBody[0].y -= snakeVelocity;
            break;
        case 'd':
            for (int i = snakeBody.size() - 1; i >= 1; i--)
            {
                snakeBody[i].x = snakeBody[i - 1].x;
                snakeBody[i].y = snakeBody[i - 1].y;
            }
            snakeBody[0].y += snakeVelocity;
            break;
        }
    }

    for (int i = 0; i < snakeBody.size(); i++)
        SDL_RenderCopy(renderer, textureSnakeSkin, NULL, &snakeBody[i]);
}

void Snake::updateSnakeSize(void)
{
    SDL_Rect snakeBodyPart;
    switch (snakeCurrentDirection)
    {
    case 'r':
        snakeBodyPart = {
            snakeBody[snakeBody.size() - 1].x - snakeWidth,
            snakeBody[snakeBody.size() - 1].y,
            snakeWidth, snakeHeight};
        break;
    case 'l':
        snakeBodyPart = {
            snakeBody[snakeBody.size() - 1].x + snakeWidth,
            snakeBody[snakeBody.size() - 1].y,
            snakeWidth, snakeHeight};
        break;
    case 'u':
        snakeBodyPart = {
            snakeBody[snakeBody.size() - 1].x,
            snakeBody[snakeBody.size() - 1].y + snakeHeight,
            snakeWidth, snakeHeight};
        break;
    case 'd':
        snakeBodyPart = {
            snakeBody[snakeBody.size() - 1].x,
            snakeBody[snakeBody.size() - 1].y - snakeHeight,
            snakeWidth, snakeHeight};
        break;
    }

    // the snake ate a fruit increase size
    snakeBody.push_back(snakeBodyPart);
}

void Snake ::updateSnakeDirection(char key)
{
    switch (key)
    {
    case 'w': // up direction
        if (snakeCurrentDirection != 'u' && snakeCurrentDirection != 'd')
            snakeCurrentDirection = 'u';
        break;
    case 's': // down direction
        if (snakeCurrentDirection != 'd' && snakeCurrentDirection != 'u')
            snakeCurrentDirection = 'd';
        break;
    case 'd': // right direction
        if (snakeCurrentDirection != 'r' && snakeCurrentDirection != 'l')
            snakeCurrentDirection = 'r';
        break;
    case 'a': // left direction
        if (snakeCurrentDirection != 'l' && snakeCurrentDirection != 'r')
            snakeCurrentDirection = 'l';
        break;
    }
}

bool Collision::detectCollision(void)
{
    // check collosion with boarder
    if (snakeBody[0].x + snakeWidth >= SCREEN_WIDTH - boarderWidth)
        return true;
    else if (snakeBody[0].x <= boarderWidth)
        return true;
    else if (snakeBody[0].y <= boarderHeight)
        return true;
    else if (snakeBody[0].y + snakeHeight >= SCREEN_HEIGHT - boarderHeight)
        return true;
    else if ((snakeBody[0].x + snakeWidth >= scoreTextRect.x) &&
             snakeBody[0].y <= scoreTextRect.h)
        return true;

    // check collision with snake body
    // check if snake head collides with any of its body part
    switch (snakeCurrentDirection)
    {
    case 'r':
        for (int i = 1; i < snakeBody.size(); i++)
        {
            if (snakeBody[0].x + snakeWidth >= snakeBody[i].x && snakeBody[0].x + snakeWidth <= snakeBody[i].x + snakeWidth && snakeBody[0].y + snakeHeight >= snakeBody[i].y && snakeBody[0].y <= snakeBody[i].y + snakeHeight)
                return true;
        }
        break;
    case 'l':
        for (int i = 1; i < snakeBody.size(); i++)
        {
            if (snakeBody[0].x <= snakeBody[i].x + snakeWidth && snakeBody[0].x >= snakeBody[i].x && snakeBody[0].y + snakeHeight >= snakeBody[i].y && snakeBody[0].y <= snakeBody[i].y + snakeHeight)
                return true;
        }
        break;
    case 'u':
        for (int i = 1; i < snakeBody.size(); i++)
        {
            if (snakeBody[0].x + snakeWidth >= snakeBody[i].x && snakeBody[0].x <= snakeBody[i].x + snakeWidth && snakeBody[0].y <= snakeBody[i].y + snakeHeight && snakeBody[0].y >= snakeBody[i].y)
                return true;
        }
        break;
    case 'd':
        for (int i = 1; i < snakeBody.size(); i++)
        {
            if (snakeBody[0].x + snakeWidth >= snakeBody[i].x && snakeBody[0].x <= snakeBody[i].x + snakeWidth && snakeBody[0].y + snakeHeight >= snakeBody[i].y && snakeBody[0].y + snakeHeight <= snakeBody[i].y + snakeHeight)
                return true;
        }
        break;
    }

    return false;
}