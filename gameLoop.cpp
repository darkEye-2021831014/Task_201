#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <stdio.h>
#include <vector>
using namespace std;

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 540
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool gameIsRunning = false,
     gameIsStarted = false,
     snakeAteFruit = false,
     keyWasPressed = false,
     gameIsPaused = false,
     fontIsInitialized = false,
     increaseOpacity = false;

// global variable for updating states of the game
const int totalWaterTexture = 2,
          totalBoarderTexture = 1,
          totalMenuTexture = 1,
          totalFruitTexture = 4,
          snakeColorRed = 100,
          snakeColorGreen = 255,
          snakeColorBlue = 100,
          snakeColorAlpha = SDL_ALPHA_OPAQUE,
          boarderWidth = 15,
          boarderHeight = 15,
          fruitWidth = 30,
          fruitHeight = 30,
          snakeWidth = 20,
          snakeHeight = 20,
          snakeVelocity = 8;
int pickFruit = rand() % totalFruitTexture,
    totalScore = 0;
Uint8 textOpacity = 255;
char snakeCurrentDirection = 'r', // by default snake is facing the right side
    pressedKey;

vector<SDL_Rect> snakeBody;
SDL_Rect rect1, rect2, rect3, rect4, rect5, rect6, rect7, rect8,
    fruitControl, middleScreenTextRect, scoreTextRect;
SDL_Texture *textureWater[totalWaterTexture],
    *textureBoarder[totalBoarderTexture],
    *textureMenu[totalMenuTexture],
    *textureFruit[totalFruitTexture];

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
                if (!gameIsStarted)
                    gameIsStarted = true;
                else
                {
                    if (gameIsPaused)
                        gameIsPaused = false;
                    else
                        gameIsPaused = true;
                }
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
};

class drawFunction
{
public:
    void drawBackground(void);
    void drawFruit(void);
    SDL_Texture *drawText(const char *, const char *, int, SDL_Color);
    void drawMiddleScreenText(drawFunction &, const char *);
    void drawScore(drawFunction &, const char *);

private:
    SDL_Texture *texture;
};

class Snake
{
public:
    void updateSnakePosition(int);
    void updateSnakeSize(void);
    void updateSnakeDirection(char);
};

void Update(void)
{
    SDL_RenderClear(renderer);

    drawFunction draw;
    Snake snake;
    draw.drawBackground();
    draw.drawFruit();
    draw.drawScore(draw, to_string(totalScore).c_str()); // convert int to string then character array then pass to drawScore

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

        if (snakeAteFruit)
        {
            totalScore++;
            snake.updateSnakeSize();
            snakeAteFruit = false;
        }
    }
    else
        draw.drawMiddleScreenText(draw, "Press Space To Start");

    if (gameIsPaused || !gameIsStarted)
        snake.updateSnakePosition(0); // This Will Draw Current State Of The Snake

    SDL_Delay(20);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    basicFunction initilize;
    gameIsRunning = initilize.initializeWindow();
    fontIsInitialized = initilize.initializeFont();
    initilize.initializeBackground();
    initilize.initializeFruit();
    initilize.initializeSnake();

    while (gameIsRunning)
    {
        processInput();
        Update();
    }

    destroyWindow();
}

// implimentation of all prototype functions
bool basicFunction ::initializeWindow(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
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
    middleScreenTextRect.w = 500;
    middleScreenTextRect.h = 100;
    middleScreenTextRect.x = (SCREEN_WIDTH - middleScreenTextRect.w) / 2;
    middleScreenTextRect.y = (SCREEN_HEIGHT - middleScreenTextRect.h) / 2;
}

void basicFunction ::initializeFruit(void)
{
    textureFruit[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/apple.bmp"));
    textureFruit[1] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/grapes.bmp"));
    textureFruit[2] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/cherry.bmp"));
    textureFruit[3] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/banana.bmp"));

    fruitControl.w = fruitWidth;
    fruitControl.h = fruitHeight;
    fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
    fruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
}

void basicFunction ::initializeSnake(void)
{
    SDL_Rect initialSnake;
    initialSnake.w = snakeWidth;
    initialSnake.h = snakeHeight;
    initialSnake.x = boarderWidth;
    initialSnake.y = (SCREEN_HEIGHT - initialSnake.w) / 2;

    snakeBody.push_back(initialSnake);
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
        // spawn new fruit. i.e., pick a random fruit
        pickFruit = rand() % totalFruitTexture;
        fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - boarderWidth - scoreTextRect.w));
        fruitControl.y = scoreTextRect.h + (rand() % (SCREEN_HEIGHT - fruitHeight - boarderHeight - scoreTextRect.h));
    }

    SDL_RenderCopy(renderer, textureFruit[pickFruit], NULL, &fruitControl);
}

SDL_Texture *drawFunction ::drawText(const char *fontFile, const char *message, int size, SDL_Color textColor)
{
    TTF_Font *font = TTF_OpenFont(fontFile, size);
    SDL_Surface *surface = TTF_RenderText_Solid(font, message, textColor);
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
                   draw.drawText("Fonts/novaMono.ttf", message, 24, {255, 255, 255, textOpacity}),
                   NULL, &middleScreenTextRect);
}

void drawFunction ::drawScore(drawFunction &draw, const char *score)
{
    SDL_RenderCopy(renderer,
                   draw.drawText("Fonts/unispace.ttf", score, 18, {0, 0, 0, 255}),
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

    SDL_SetRenderDrawColor(renderer, snakeColorRed, snakeColorGreen, snakeColorBlue, snakeColorAlpha);
    for (int i = 0; i < snakeBody.size(); i++)
        SDL_RenderFillRect(renderer, &snakeBody[i]);
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