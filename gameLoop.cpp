#include <SDL2/SDL.h>
#include <stdio.h>
#include <vector>
using namespace std;

#define SCREEN_WIDTH 1080
#define SCREEN_HEIGHT 540
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool gameIsRunning = false;

// global variable for updating states of the game
const int totalWaterTexture = 2,
          totalBoarderTexture = 1,
          totalFruitTexture = 4,
          boarderWidth = 15,
          boarderHeight = 15,
          fruitWidth = 30,
          fruitHeight = 30,
          snakeWidth = 15,
          snakeHeight = 15;

vector<SDL_Rect> snakeBody;
SDL_Rect rect1, rect2, rect3, rect4, rect5, rect6, rect7, rect8,
    fruitControl;
SDL_Texture *textureWater[totalWaterTexture],
    *textureBoarder[totalBoarderTexture],
    *textureFruit[totalFruitTexture];

bool initializeWindow(void)
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

void processInput(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        int diff = 0;
        switch (event.type)
        {
        case SDL_QUIT:
            gameIsRunning = false;
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
    for (int i = 0; i < totalFruitTexture; i++)
        SDL_DestroyTexture(textureFruit[i]);
    SDL_Quit();
}

class basicFunction
{
public:
    void initializeBackGround();
    void initializeFruit();
    void initializeSnake();
};

class drawFunction
{
public:
    void drawBackground(void);
};

void Update(void)
{
    SDL_RenderClear(renderer);

    drawFunction draw;
    draw.drawBackground();

    SDL_Delay(20);
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    gameIsRunning = initializeWindow();
    basicFunction initilize;
    initilize.initializeBackGround();
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

void basicFunction ::initializeBackGround(void)
{
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

    // draw boarder
    rect5.x = 0;
    rect5.y = 0;
    rect5.w = SCREEN_WIDTH;
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
    rect8.y = boarderHeight;
    rect8.h = SCREEN_HEIGHT - (2 * rect8.y);

    textureBoarder[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Background/boarder.bmp"));
}

void basicFunction ::initializeFruit(void)
{
    textureFruit[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/apple.bmp"));
    textureFruit[1] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/grapes.bmp"));
    textureFruit[2] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/cherry.bmp"));
    textureFruit[3] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Fruits/banana.bmp"));

    fruitControl.w = fruitWidth;
    fruitControl.h = fruitHeight;
    fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - 2 * boarderWidth));
    fruitControl.y = boarderHeight + (rand() % (SCREEN_HEIGHT - fruitHeight - 2 * boarderHeight));
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
    SDL_SetTextureBlendMode(textureBoarder[0], SDL_BLENDMODE_ADD);
}
