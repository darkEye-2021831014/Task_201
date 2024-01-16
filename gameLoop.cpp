#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <stdio.h>
#include <vector>
#include <thread>
using namespace std;

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 540
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool gameIsRunning = false,
     gameIsStarted = false,
     collisionDetected = false,
     snakeAteFruit = false,
     keyWasPressed = false,
     gameIsPaused = false,
     fontIsInitialized = false,
     increaseOpacity = false,
     collisionSoundPlayed = false,
     bonusFruitTimerFlag = false;

// global variable for updating states of the game
const int totalWaterTexture = 2,
          totalBoarderTexture = 1,
          totalMenuTexture = 1,
          totalFruitTexture = 6,
          totalFruitEatingSound = 3,
          snakeInitialVelocity = 9,
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
    pickFruit = rand() % (totalFruitTexture - 1),
    totalScore = 0,
    taskBackgroundFinished = 0;
uint32_t startTime;
Uint8 textOpacity = 255;
char snakeCurrentDirection = 'r', // by default snake is facing the right side
    pressedKey;

vector<SDL_Rect> snakeBody;
string soundEating[totalFruitEatingSound];
SDL_Rect rect1, rect2, rect3, rect4, rect5, rect6, rect7, rect8,
    fruitControl, bonusFruitControl, middleScreenTextRect, scoreTextRect;
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
                if (!gameIsStarted && !collisionDetected)
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

void destroyWindow(void)
{
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
    TTF_Quit();
    SDL_Quit();
}

class basicFunction
{
public:
    bool initializeWindow(void);
    bool initializeFont(void);
    void initializeBackground(void);
    void initializeFruit(void);
    void initializeSnake(void);
    void backgroundActivities(void);
    void resetGame(void);
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

private:
    SDL_Texture *texture;
};

class Snake
{
public:
    int incraseSpeedAfter = 8 + rand() % 8;
    void updateSnakePosition(int);
    void updateSnakeSize(void);
    void updateSnakeDirection(char);
};

class Collision
{
public:
    bool detectCollision(void);
};

class Sound
{
public:
    void playWAVSound(const char *);
    void eatFruit(void);
};

void Update(Snake &snake, drawFunction &draw, Collision &collision)
{
    SDL_RenderClear(renderer);

    draw.drawBackground();
    draw.drawFruit();
    draw.drawScore(draw, to_string(totalScore).c_str()); // convert int to string then character array then pass to drawScore

    // detect collision
    if (collision.detectCollision())
    {
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
                snake.updateSnakeDirection(pressedKey);
                keyWasPressed = false;
            }
            snake.updateSnakePosition(snakeVelocity);
        }

        if (bonusFruitTimerFlag)
        {
            draw.drawBonusFruit();
            double elapsedTime = (SDL_GetTicks() - startTime) / 1000.0;
            if (elapsedTime >= 4)
            {
                bonusFruitTimerFlag = false;
            }
        }

        if (snakeAteFruit)
        {
            totalScore++;
            snake.updateSnakeSize();
            snakeAteFruit = false;

            // check for bonus fruit
            if (!bonusFruitTimerFlag && totalScore % 7 == 0)
            {
                bonusFruitTimerFlag = true;
                startTime = SDL_GetTicks();
            }

            // increase snake speed up to 15;
            if (!(totalScore % snake.incraseSpeedAfter) && snakeVelocity < 15)
                snakeVelocity++;
        }
    }
    else if (!collisionDetected)
        draw.drawMiddleScreenText(draw, "Press Space To Start");

    if (gameIsPaused || !gameIsStarted || collisionDetected)
        snake.updateSnakePosition(0); // This Will Draw Current State Of The Snake

    SDL_Delay(20);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    // make class objects
    basicFunction initilize;
    Sound sound;
    Snake snake;
    drawFunction draw;
    Collision collision;

    gameIsRunning = initilize.initializeWindow();
    fontIsInitialized = initilize.initializeFont();
    initilize.initializeBackground();
    initilize.initializeSnake();
    initilize.initializeFruit();

    // run background activities in a diffrent thread
    thread taskThread(&basicFunction::backgroundActivities, &initilize);

    while (gameIsRunning)
    {
        processInput();
        if (collisionDetected && pressedKey == 'k')
            initilize.resetGame();
        Update(snake, draw, collision);
    }

    taskThread.join(); // wait for the thread to finish
    destroyWindow();
}

// implimentation of all prototype functions
bool basicFunction ::initializeWindow(void)
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

    if (!renderer)
    {
        printf("Error: Failed to create renderer\nSDl Error: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool basicFunction ::initializeFont(void)
{
    if (TTF_Init() == -1)
    {
        printf("TTF failed to initialize\nError: %s\n",
               TTF_GetError());
        return false;
    }
    return true;
}

void basicFunction ::initializeBackground(void)
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
    rect5.x = 0;
    rect5.y = 0;
    rect5.w = SCREEN_WIDTH - scoreTextRect.w;
    rect5.h = boarderHeight;

    rect6.w = SCREEN_WIDTH;
    rect6.h = boarderHeight;
    rect6.x = 0;
    rect6.y = SCREEN_HEIGHT - rect6.h;

    rect7.x = 0;
    rect7.y = boarderHeight;
    rect7.w = boarderWidth;
    rect7.h = SCREEN_HEIGHT - (2 * rect7.y);

    rect8.w = boarderWidth;
    rect8.x = SCREEN_WIDTH - rect8.w;
    rect8.y = scoreTextRect.h;
    rect8.h = SCREEN_HEIGHT - rect8.y - boarderHeight;

    textureBoarder[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Background/boarder.bmp"));

    // intialize middle screen message rectangle
    middleScreenTextRect.w = 600;
    middleScreenTextRect.h = 100;
    middleScreenTextRect.x = (SCREEN_WIDTH - middleScreenTextRect.w) / 2;
    middleScreenTextRect.y = (SCREEN_HEIGHT - middleScreenTextRect.h) / 2;

    // initialize fruit eating and background sounds
    soundEating[0] = "Sounds/eatFruit.wav";
    soundEating[1] = "Sounds/eatFruit2.wav";
    soundEating[2] = "Sounds/eatFruit3.wav";
}

void basicFunction ::initializeFruit(void)
{
    textureFruit[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/apple.bmp"));
    textureFruit[1] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/grapes.bmp"));
    textureFruit[2] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/cherry.bmp"));
    textureFruit[3] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/banana.bmp"));
    textureFruit[4] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/pear.bmp"));
    textureFruit[5] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/bonusFruit.bmp"));

    fruitControl.w = fruitWidth;
    fruitControl.h = fruitHeight;
    fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
    fruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
    // avoid snake body
    while (((snakeBody[0].x + snakeWidth) >= fruitControl.x && snakeBody[0].x <= (fruitControl.x + fruitWidth)) &&
           ((snakeBody[0].y + snakeHeight) >= fruitControl.y && snakeBody[0].y <= (fruitControl.y + fruitHeight)))
    {
        fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
        fruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
    }

    // bonus fruit
    bonusFruitControl.w = fruitWidth;
    bonusFruitControl.h = fruitHeight;
    bonusFruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
    bonusFruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
    // avoid snake body
    while (((snakeBody[0].x + snakeWidth) >= bonusFruitControl.x && snakeBody[0].x <= (fruitControl.x + fruitWidth)) &&
           ((snakeBody[0].y + snakeHeight) >= bonusFruitControl.y && snakeBody[0].y <= (bonusFruitControl.y + fruitHeight)))
    {
        bonusFruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
        bonusFruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
    }
}

void basicFunction ::initializeSnake(void)
{
    SDL_Rect initialSnake;
    initialSnake.w = snakeWidth;
    initialSnake.h = snakeHeight;
    initialSnake.x = boarderWidth + 1;
    initialSnake.y = (SCREEN_HEIGHT - initialSnake.w) / 2;

    textureSnakeSkin = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Snake/skin.bmp"));

    snakeBody.push_back(initialSnake);
}

void basicFunction::backgroundActivities(void)
{
    // this function will run in a diffrent thread
    Sound sound;
    while (gameIsRunning)
    {
        if (gameIsStarted && !collisionDetected)
        {
            taskBackgroundFinished = 0;
            thread taskBackground(&Sound::playWAVSound, &sound, "Sounds/backgroundTrack.wav");

            thread taskEat(&Sound::eatFruit, &sound);
            taskBackground.join();
            taskBackgroundFinished = 1;
            taskEat.join();
        }
    }
}

void basicFunction ::resetGame(void)
{
    gameIsStarted = true;
    collisionDetected = false;
    snakeAteFruit = false;
    keyWasPressed = false;
    gameIsPaused = false;
    increaseOpacity = false;
    collisionSoundPlayed = false;
    bonusFruitTimerFlag = false;
    totalScore = 0;
    snakeVelocity = snakeInitialVelocity;
    snakeBody.clear();
    snakeCurrentDirection = 'r';
    pressedKey = '/'; // default value;

    // Reset necessary game states or variables here
    initializeSnake();
    initializeFruit();
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
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &rect5);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &rect6);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &rect7);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &rect8);
    SDL_RenderCopy(renderer, textureBoarder[0], NULL, &scoreTextRect);
    SDL_SetTextureBlendMode(textureBoarder[0], SDL_BLENDMODE_ADD);
}

void drawFunction ::drawFruit(void)
{
    // snake ate the fruit
    if (((snakeBody[0].x + snakeWidth) >= fruitControl.x && snakeBody[0].x <= (fruitControl.x + fruitWidth)) &&
        ((snakeBody[0].y + snakeHeight) >= fruitControl.y && snakeBody[0].y <= (fruitControl.y + fruitHeight)))
    {
        snakeAteFruit = true;
        // spawn new fruit. i.e., pick a random fruit & also avoid snake body
        pickFruit = rand() % (totalFruitTexture - 1);
        // avoid snake body
        bool fruitInsideSnake = true;
        while (fruitInsideSnake)
        {
            fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
            fruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
            for (int i = 0; i < snakeBody.size(); i++)
            {
                if (((snakeBody[i].x + snakeWidth) >= fruitControl.x && snakeBody[i].x <= (fruitControl.x + fruitWidth)) &&
                    ((snakeBody[i].y + snakeHeight) >= fruitControl.y && snakeBody[i].y <= (fruitControl.y + fruitHeight)))
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

int flag = 0;
void drawFunction ::drawBonusFruit(void)
{
    int bonusFruitIndex = 5; // index of bonus fruit
    if (!flag)
    {
        flag = 1;
        // spawn bonus fruit and avoid snake body or the other fruit;
        // avoid snake body
        bool fruitInsideSnake = true;
        while (fruitInsideSnake)
        {
            bonusFruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
            bonusFruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
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
            // // check if the bonus fruit is inside the regular one
            // if (bonusFruitControl.x >= fruitControl.x &&
            //     bonusFruitControl.x <= (fruitControl.x + fruitWidth) &&
            //     bonusFruitControl.y >= fruitControl.y &&
            //     bonusFruitControl.y <= (fruitControl.y + fruitHeight) &&
            //     (bonusFruitControl.y + fruitHeight) >= fruitControl.y &&
            //     (bonusFruitControl.y + fruitHeight) <= (fruitControl.y + fruitHeight) &&
            //     (bonusFruitControl.y + fruitHeight) >= fruitControl.x &&
            //     (bonusFruitControl.y + fruitHeight) <= (fruitControl.x + fruitWidth))
            // {
            //     fruitInsideRegularFruit = true;
            // }
            // else
            //     fruitInsideSnake = false;
        }
    }
    SDL_RenderCopy(renderer, textureFruit[bonusFruitIndex], NULL, &bonusFruitControl);

    // check whether the snake ate the bonus fruit
    if (((snakeBody[0].x + snakeWidth) >= bonusFruitControl.x && snakeBody[0].x <= (bonusFruitControl.x + fruitWidth)) &&
        ((snakeBody[0].y + snakeHeight) >= bonusFruitControl.y && snakeBody[0].y <= (bonusFruitControl.y + fruitHeight)))
    {
        flag = 0;
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

void Sound::playWAVSound(const char *filePath)
{

    SDL_AudioSpec wavSpec;
    Uint32 wavLength;
    Uint8 *wavBuffer;

    if (SDL_LoadWAV(filePath, &wavSpec, &wavBuffer, &wavLength) == nullptr)
    {
        printf("Could not load audio file.\nError: %s\n",
               SDL_GetError());
        return;
    }

    SDL_AudioDeviceID deviceId = SDL_OpenAudioDevice(nullptr, 0, &wavSpec, nullptr, 0);
    if (deviceId == 0)
    {
        printf("Failed to open audio device.\nError: %s\n",
               SDL_GetError());
        SDL_FreeWAV(wavBuffer);
        return;
    }

    SDL_QueueAudio(deviceId, wavBuffer, wavLength);

    SDL_PauseAudioDevice(deviceId, 0);

    while (SDL_GetQueuedAudioSize(deviceId) > 0 && gameIsRunning && !collisionSoundPlayed)
    {
        SDL_Delay(10); // Add a short delay to avoid busy-waiting
    }

    SDL_CloseAudioDevice(deviceId);
    SDL_FreeWAV(wavBuffer);
}

void Sound::eatFruit(void)
{
    int pickSoundTrack = 0;
    while (!taskBackgroundFinished)
    {
        pickSoundTrack = rand() % totalFruitEatingSound;
        if (snakeAteFruit)
            playWAVSound(soundEating[pickSoundTrack].c_str());
        if (collisionDetected && !collisionSoundPlayed) // play collision sound only once
        {
            playWAVSound("SOunds/gameOver.wav");
            collisionSoundPlayed = true;
        }
    }
}
