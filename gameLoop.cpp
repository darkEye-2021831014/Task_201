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
#include <sstream>
using namespace std;

#define SCREEN_WIDTH 1180
#define SCREEN_HEIGHT 600
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
bool gameIsRunning = false,
     gameIsStarted = false,
     collisionDetected = false,
     keyWasPressed = false,
     gameIsPaused = false,
     fontIsInitialized = false,
     increaseOpacity = false,
     pauseScreenSoundPlayed = false,
     newHighScore = false,
     newMostSurvived = false,
     startTheGameNow = false,
     mainMenuActive = false,
     goToGameScreen = false,
     mouseButtonPressed = false,
     showScorecard = false,
     modeSelectionActive = false,
     absoluteSnakeVelocity = false;

// global variable for updating states of the game
const int totalWaterTexture = 2,
          totalBoarderTexture = 1,
          totalFruitTexture = 6,
          totalFruitEatingSound = 3,
          totalMenuTexture = 9,
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
    prevResult = 0,
    mouseX, mouseY;

uint32_t startTime,
    startPlaying,
    endPlaying,
    startPause;
double totalPausedTime = 0.0,
       timePlayed = 0.0;

char snakeCurrentDirection = 'r', // by default snake is facing the right side
    pressedKey;
string gameMode = "Easy",
       menuButtonSound, modeButtonSound;

vector<SDL_Rect> snakeBody;
SDL_Rect rect1, rect2, rect3, rect4, boarderTopRect, boarderBottomRect, boarderLeftRect, boarderRightRect,
    fruitControl, bonusFruitControl, middleScreenTextRect, scoreTextRect, screenTopTextRect, screenBottomBoxRectangle, screenBottomLeftBox, wholeObstacleOne, wholeObstacleTwo, wholeObstacleThree, wholeObstacleFour, wholeObstacleFive, wholeObstacleSix;
SDL_Texture *textureWater[totalWaterTexture],
    *textureBoarder[totalBoarderTexture],
    *textureFruit[totalFruitTexture],
    *textureSnakeSkin;

class Music
{
public:
    Music()
    {
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 4, 1024) == -1)
            cout << "Audio Library Is Not Working\nError: " << Mix_GetError() << endl;
    }

    ~Music()
    {
        Mix_FreeMusic(music);
        Mix_FreeChunk(chunk);
        Mix_CloseAudio();
    }

    void playMusic(string filePath, int loops, int ms)
    {
        music = Mix_LoadMUS(filePath.c_str());
        // loops = 0 for 0
        //-1 for foreever loop
        if (music != nullptr)
            Mix_FadeInMusic(music, loops, ms);
    }

    int playChuck(string filePath, int channel, int loops, int ms)
    {
        chunk = Mix_LoadWAV(filePath.c_str());
        if (chunk != nullptr)
            return Mix_FadeInChannel(channel, chunk, loops, ms);
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
    Mix_Music *music = nullptr;
    Mix_Chunk *chunk = nullptr;
};

void processInput(Music &music)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            gameIsRunning = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                // left mouse button is pressed check co-ordinates
                mouseX = event.button.x,
                mouseY = event.button.y;
                mouseButtonPressed = true;
            }
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
                if (!gameIsStarted && startTheGameNow)
                {
                    music.playChuck(menuButtonSound, 3, 0, 10);
                    gameIsStarted = true;
                }
                else if (startTheGameNow)
                {
                    if (gameIsPaused)
                    {
                        gameIsPaused = false;
                        totalPausedTime += (SDL_GetTicks() - startPause);
                    }
                    else
                    {
                        startPause = SDL_GetTicks();
                        gameIsPaused = true;
                    }
                }

                if (collisionDetected)
                    pressedKey = 'k'; // k and space both key will do the same thing
                break;
            case SDLK_q:
                music.playChuck(menuButtonSound, 3, 0, 10);
                gameIsRunning = false;
                break;
            case SDLK_p:
                if (mainMenuActive && !goToGameScreen)
                {
                    music.playChuck(menuButtonSound, 3, 0, 10);
                    goToGameScreen = true;
                    modeSelectionActive = false;
                    showScorecard = false;
                    mainMenuActive = false;
                }
                break;
            case SDLK_r:
                if (mainMenuActive)
                {
                    music.playChuck(menuButtonSound, 3, 0, 10);
                    if (!showScorecard)
                        showScorecard = true;
                    else
                        showScorecard = false;
                }
                break;
            case SDLK_m:
                if (mainMenuActive)
                {
                    music.playChuck(menuButtonSound, 3, 0, 10);
                    if (!modeSelectionActive)
                        modeSelectionActive = true;
                    else
                        modeSelectionActive = false;
                }
                break;
            case SDLK_e:
                if (modeSelectionActive && mainMenuActive)
                {
                    music.playChuck(modeButtonSound, 3, 0, 10);
                    gameMode = "Easy";
                }
                break;
            case SDLK_h:
                if (modeSelectionActive && mainMenuActive)
                {
                    music.playChuck(modeButtonSound, 3, 0, 10);
                    gameMode = "Hard";
                }
                break;
            case SDLK_i:
                if (modeSelectionActive && mainMenuActive)
                {
                    music.playChuck(modeButtonSound, 3, 0, 10);
                    gameMode = "Impossible";
                }
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
    void Render(SDL_Rect &snakePart, SDL_Texture &snakeTexture)
    {
        SDL_RenderCopy(renderer, &snakeTexture, NULL, &snakePart);
    }

private:
    SDL_Rect initialSnake;
};

class Fruit
{
public:
    bool bonusFruitTimerFlag = false;
    int bonusFruitIndex = 5; // index of bonus fruit
    // initialize the fruit using constructor
    Fruit();

    void Render(SDL_Rect &fruitRect, SDL_Texture &fruitTexture)
    {
        SDL_RenderCopy(renderer, &fruitTexture, NULL, &fruitRect);
    }

    void renderScore();
    void initializeBonusFruit();
};

class Background
{
public:
    SDL_Rect leftFirstObstacle, leftSecondObstacle, leftThirdObstacle, leftFourthObstacle,
        horizontalFirstObstacle, horizontalSecondObstacle, horizontalThirdObstacle, horizontalFourthObstacle;
    vector<string> backgroundMusic, fruitEatingSound, collisionSound;
    string pauseSound, resumeSound, bonusFruitSound,
        userName = "";

    const int baseSpeed = 2;
    int fadeInChunck = 40,
        fadeInMusic = 1000,
        obstacleMovingSpeed = baseSpeed,
        prevSpeed = baseSpeed;

    Background()
    {
        gameIsRunning = initializeWindow();
        initializeBackground();
        initializeObstacle();
        while (userName.empty())
            getUserName();
        mainMenuActive = true;
    }
    void simpleRenderer(SDL_Rect &rect, SDL_Texture &texture)
    {
        SDL_RenderCopy(renderer, &texture, NULL, &rect);
    }

    void renderObstacles();
    void renderExtraObstacles();
    bool initializeWindow();
    void initializeBackground();
    void initializeObstacle();
    void resetGame();
    void getUserName();
    void Render();

private:
    const int freeSpace = 11,
              characterLimit = 14;
    // isTop is for the two obstacle that start from bottom & isBottom is for the other two
    bool isTop = false,
         isBottom = false,
         isLeft1 = false,
         isLeft2 = false,
         isLeft3 = false,
         isLeft4 = false;
};

class Font
{
public:
    string standardFontBold = "./Fonts/robotoMonoBold.ttf",
           standardFont = "./Fonts/robotoMonoRegular.ttf";
    Uint8 textOpacity = SDL_ALPHA_OPAQUE; // this will be used to create blinking effect
    Font()
    {
        if (TTF_Init() == -1)
        {
            printf("TTF failed to initialize\nError: %s\n",
                   TTF_GetError());
        }
        getInitialDataFromFile();
        // initialize only two fonts for better performance
        fontMultiple = TTF_OpenFont(standardFont.c_str(), 24);
        TTF_SetFontWrappedAlign(fontMultiple, TTF_WRAPPED_ALIGN_CENTER);

        fontSingle = TTF_OpenFont(standardFontBold.c_str(), 32);
        TTF_SetFontWrappedAlign(fontSingle, TTF_WRAPPED_ALIGN_CENTER);
    }
    ~Font()
    {
        TTF_CloseFont(fontMultiple);
        TTF_CloseFont(fontSingle);
        TTF_Quit();
    }

    void getInitialDataFromFile();

    void RenderSingleLine(string message, SDL_Color textColor, SDL_Rect &textRect)
    {
        SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(fontSingle, message.c_str(), textColor, textRect.w);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_DestroyTexture(texture);
    }

    void RenderMultipleText(string message, SDL_Color textColor, SDL_Rect &textRect)
    {
        SDL_Surface *surface = TTF_RenderText_Blended_Wrapped(fontMultiple, message.c_str(), textColor, textRect.w);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        SDL_RenderCopy(renderer, texture, nullptr, &textRect);
        SDL_DestroyTexture(texture);
    }

    void renderAllTimeBestScores(Background &);

    void blinkingEffect()
    {
        if (textOpacity <= 80)
            increaseOpacity = true;
        if (textOpacity >= 250)
            increaseOpacity = false;
        if (increaseOpacity)
            textOpacity += 10;
        else
            textOpacity -= 10;
    }

private:
    TTF_Font *fontMultiple = nullptr,
             *fontSingle = nullptr;
    vector<pair<int, string>> allTimeBestScores, mostSurvivedInSeconds;
    const int totalSavedScores = 15, totalSavedSurvivalTime = 10;
    SDL_Rect
        bestScoresRect = {screenBottomBoxRectangle.x, screenBottomBoxRectangle.y, screenBottomBoxRectangle.w, 60},
        mostTimeSpentRect = {screenBottomLeftBox.x, screenBottomLeftBox.y, screenBottomLeftBox.w, 90};
    SDL_Color newScore = {255, 0, 255, SDL_ALPHA_OPAQUE},
              prevScore = {0, 255, 255, SDL_ALPHA_OPAQUE},
              scoreColor = {0, 255, 255, SDL_ALPHA_OPAQUE},
              timeColor = {0, 255, 255, SDL_ALPHA_OPAQUE};
};

class Collision
{
public:
    bool detectCollision(Background &);
    bool detectCollisionWithFruit(Snake *);
    bool detectCollisionWithBonusFruit();
};

class mainMenu
{
public:
    SDL_Rect menuTextR, menuPlayR, menuScorecardR, menuDifficultyR, menuQuitR,
        modeTextR, modeEasyR, modeHardR, modeImpossibleR;
    SDL_Texture *textureMainMenu[totalMenuTexture];
    mainMenu()
    {
        initializeMainMenu();
    }
    ~mainMenu()
    {
        for (int i = 0; i < totalMenuTexture; i++)
            SDL_DestroyTexture(textureMainMenu[i]);
    }
    void Render(SDL_Rect &menuRect, SDL_Texture &texture)
    {
        SDL_RenderCopy(renderer, &texture, NULL, &menuRect);
    }

    void mouseMovement(Music &);
    void initializeMainMenu();
    void renderMenu();
    void renderModeSelection();
};

void destroyBackground(Fruit *fruit, Snake *snake)
{
    delete fruit;
    delete snake;

    for (int i = 0; i < totalWaterTexture; i++)
        SDL_DestroyTexture(textureWater[i]);
    for (int i = 0; i < totalBoarderTexture; i++)
        SDL_DestroyTexture(textureBoarder[i]);
    for (int i = 0; i < totalFruitTexture; i++)
        SDL_DestroyTexture(textureFruit[i]);
    SDL_DestroyTexture(textureSnakeSkin);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Update(Snake *snake, Collision &collision, Background &background, Music &music, Fruit *fruit, Font &font, mainMenu &menu)
{
    SDL_RenderClear(renderer);
    background.Render();

    // check all the menu activities here
    if (mainMenuActive)
    {
        menu.renderMenu();
        if (mouseButtonPressed)
        {
            menu.mouseMovement(music);
            mouseButtonPressed = false;
        }
        if (showScorecard)
            font.renderAllTimeBestScores(background);
        if (modeSelectionActive)
            menu.renderModeSelection();
    }
    // check whether the snake eat the fruit or not
    if (collision.detectCollisionWithFruit(snake))
    {
        music.playChuck(background.fruitEatingSound[rand() % totalFruitEatingSound], 2, 0, background.fadeInChunck);

        totalScore++;
        // check for bonus fruit
        if (!fruit->bonusFruitTimerFlag && totalScore % 7 == 0)
        {
            startTime = SDL_GetTicks();
            fruit->initializeBonusFruit();
            fruit->bonusFruitTimerFlag = true;
        }

        // increase obstacle moving speed if in impossible mode
        if (gameMode != "Easy")
        {
            if (background.obstacleMovingSpeed < 10 && totalScore % 2 == 0)
                background.obstacleMovingSpeed++;
        }
    }
    if (fruit->bonusFruitTimerFlag)
    {
        if (collision.detectCollisionWithBonusFruit())
        {
            music.playChuck(background.bonusFruitSound, 1, 0, background.fadeInChunck);
            snake->updateSnakeSize();

            totalScore += 10;
            fruit->bonusFruitTimerFlag = false;
        }
        else
        {
            double elapsedTime = (SDL_GetTicks() - startTime) / 1000.0;
            if (elapsedTime <= 4.0)
                fruit->Render(bonusFruitControl, *textureFruit[fruit->bonusFruitIndex]);
            else
                fruit->bonusFruitTimerFlag = false;
        }
    }

    font.blinkingEffect();
    // render regular fruit, score and snake
    fruit->Render(fruitControl, *textureFruit[pickFruit]);
    font.RenderSingleLine(to_string(totalScore), {0, 0, 0, 255}, scoreTextRect);
    for (int i = 0; i < snakeBody.size(); i++)
        snake->Render(snakeBody[i], *textureSnakeSkin);

    // detect collision
    if (collision.detectCollision(background))
    {
        if (gameIsStarted && !collisionDetected)
            endPlaying = SDL_GetTicks();
        font.renderAllTimeBestScores(background);

        // render game over screen
        SDL_Rect gameOverRect = {middleScreenTextRect.x, middleScreenTextRect.y, middleScreenTextRect.w, 80};
        font.RenderSingleLine("Game Over\nFinal Score: " + to_string(totalScore), {0, 255, 255, font.textOpacity}, gameOverRect);
        gameOverRect.y += gameOverRect.h;
        font.RenderSingleLine("Survival Time: " + to_string((int)timePlayed) + " Seconds\nPress Space To Play Again", {255, 255, 0, font.textOpacity}, gameOverRect);

        if (!collisionDetected)
        {
            music.stopMusic();
            music.playChuck(background.collisionSound[rand() % totalCollisionSound], -1, 0, background.fadeInChunck);
        }

        gameIsStarted = false;
        collisionDetected = true;
    }
    else
    { // check what mode to run the game on
        if (gameMode == "Hard")
        {
            background.renderObstacles();
            absoluteSnakeVelocity = true;
        }
        else if (gameMode == "Impossible")
        {
            background.renderObstacles();
            background.renderExtraObstacles();
            absoluteSnakeVelocity = false;
        }
        else
            absoluteSnakeVelocity = false;
    }

    // start the game from here
    if (gameIsStarted)
    {
        // play background music
        if (!Mix_PlayingMusic())
            music.playMusic(background.backgroundMusic[rand() % totalBackgroundSound], 0, background.fadeInMusic);
        if (gameIsPaused)
        {
            if (background.obstacleMovingSpeed)
            {
                background.prevSpeed = background.obstacleMovingSpeed;
                background.obstacleMovingSpeed = 0;
            }
            // render pause screen
            font.RenderSingleLine("Press Space To Resume", {255, 255, 255, font.textOpacity}, middleScreenTextRect);

            if (!pauseScreenSoundPlayed)
            {
                //-1 to play the chunk on the first free channel
                music.playChuck(background.pauseSound, 0, 0, background.fadeInChunck);
                pauseScreenSoundPlayed = true;
            }
        }
        else
        {
            if (!background.obstacleMovingSpeed)
                background.obstacleMovingSpeed = background.prevSpeed;
            // normal gameplay functionalities
            if (keyWasPressed)
            {
                snake->updateSnakeDirection(pressedKey);
                keyWasPressed = false;
            }
            snake->updateSnakePosition(snakeVelocity);

            if (pauseScreenSoundPlayed)
            {
                music.playChuck(background.resumeSound, 0, 0, background.fadeInChunck);
                pauseScreenSoundPlayed = false;
            }
        }

        // increase snake speed up to 15;
        if (!absoluteSnakeVelocity && (totalScore / snake->incraseSpeedAfter) > prevResult &&
            snakeVelocity < 15)
        {
            snakeVelocity++;
            SDL_Log("New Snake Speed: %d->%d", snakeVelocity - 1, snakeVelocity);
            prevResult = (totalScore / snake->incraseSpeedAfter);
        }
    }
    else if (goToGameScreen && !collisionDetected)
    {
        // render start screen
        font.RenderSingleLine("Press Space To Start", {255, 255, 255, font.textOpacity}, middleScreenTextRect);

        startTheGameNow = true;
        startPlaying = SDL_GetTicks();
        totalPausedTime = 0.0;
    }

    // vSync failed to activate so a 20 ms delay is the adjustment
    if (vSyncActive != 0)
    {
        SDL_Log("vSync Failed To Activate.\nUsing A Delay Of 20ms Instead!");
        SDL_Delay(20);
    }
    SDL_RenderPresent(renderer);
}

int main(int argc, char **argv)
{
    // make class objects
    Collision collision;
    Background background;
    Font font;
    Snake *snake = new Snake("./Snake/skin.bmp");
    Fruit *fruit = new Fruit();
    Music music;
    mainMenu menu;

    while (gameIsRunning)
    {
        processInput(music);
        // reset the game
        if (collisionDetected && pressedKey == 'k')
        {
            music.stopMusic();
            delete snake;
            delete fruit;
            background.resetGame();
            snake = new Snake("./Snake/skin.bmp");
            fruit = new Fruit();
        }
        Update(snake, collision, background, music, fruit, font, menu);
    }

    destroyBackground(fruit, snake);
}

// implimentation of all prototype functions
void Font::getInitialDataFromFile()
{
    // populate saved highest scores
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
    in.close();
    sort(allTimeBestScores.rbegin(), allTimeBestScores.rend());

    // populate saved survival times in seconds
    ifstream inTimeSpent;
    inTimeSpent.open("./Files/secondsSurvived.txt");
    for (int i = 0; i < totalSavedSurvivalTime; i++)
    {
        getline(inTimeSpent, getLineFromFile);
        string getSecondsFromLine = "",
               getNameFromLine = "";
        bool startTakingSeconds = false;
        for (int i = 0; i < getLineFromFile.size(); i++)
        {
            if (startTakingSeconds)
                getSecondsFromLine.push_back(getLineFromFile[i]);
            else
            {
                if (getLineFromFile[i] == ':')
                    startTakingSeconds = true;
                getNameFromLine.push_back(getLineFromFile[i]);
            }
        }

        mostSurvivedInSeconds.push_back({stoi(getSecondsFromLine), getNameFromLine});
    }
    inTimeSpent.close();
    sort(mostSurvivedInSeconds.rbegin(), mostSurvivedInSeconds.rend());
}

void Font ::renderAllTimeBestScores(Background &background)
{
    // check if the final score can be placed in the list of all time best scores
    if (totalScore > allTimeBestScores[totalSavedScores - 1].first && !newHighScore)
    {
        newHighScore = true;
        allTimeBestScores[totalSavedScores - 1].first = totalScore;
        allTimeBestScores[totalSavedScores - 1].second = background.userName + ':';
        sort(allTimeBestScores.rbegin(), allTimeBestScores.rend());

        // write to the score file
        ofstream out;
        out.open("./Files/saveHighestScore.txt");
        for (auto &bestScore : allTimeBestScores)
            out << bestScore.second << bestScore.first << endl;
        out.close();
    }

    // render highest score
    int changeBy = 17;
    (newHighScore) ? newScore.g += changeBy, newScore.b -= changeBy : prevScore.r += changeBy, prevScore.b -= changeBy;

    string highestScore = "All Time Best Score: " + to_string(allTimeBestScores[0].first);
    RenderSingleLine(highestScore, ((newHighScore) ? newScore : prevScore), screenTopTextRect);

    // show some best scores
    bestScoresRect.y = screenBottomBoxRectangle.y;
    bestScoresRect.h = 60;
    RenderSingleLine("All Time Best Scores", {255, 0, 255, 255}, bestScoresRect);
    bestScoresRect.y += bestScoresRect.h;
    bestScoresRect.h = 30;
    for (auto &bestScore : allTimeBestScores)
    {
        RenderMultipleText(bestScore.second + ' ' + to_string(bestScore.first), scoreColor, bestScoresRect);
        scoreColor.r += 51;
        scoreColor.g -= 51;
        bestScoresRect.y += bestScoresRect.h;
    }

    // calculate total survival time in seconds here
    timePlayed = ((endPlaying - startPlaying) - totalPausedTime) / 1000.0;
    if (timePlayed > mostSurvivedInSeconds[totalSavedSurvivalTime - 1].first && !newMostSurvived)
    {
        newMostSurvived = true;
        mostSurvivedInSeconds[totalSavedSurvivalTime - 1].first = timePlayed;
        mostSurvivedInSeconds[totalSavedSurvivalTime - 1].second = background.userName + ':';
        sort(mostSurvivedInSeconds.rbegin(), mostSurvivedInSeconds.rend());

        // write to the survuval seconds file
        ofstream outTimeSpent;
        outTimeSpent.open("./Files/secondsSurvived.txt");
        for (auto &mostSurvived : mostSurvivedInSeconds)
            outTimeSpent << mostSurvived.second << mostSurvived.first << endl;
        outTimeSpent.close();
    }

    // show some best survival records in seconds
    mostTimeSpentRect.y = screenBottomLeftBox.y;
    mostTimeSpentRect.h = 120;
    RenderSingleLine("All Time Best Survival Records In Seconds", {255, 0, 255, 255}, mostTimeSpentRect);
    mostTimeSpentRect.y += mostTimeSpentRect.h + 10;
    mostTimeSpentRect.h = 35;
    for (auto &mostSpent : mostSurvivedInSeconds)
    {
        RenderMultipleText(mostSpent.second + ' ' + to_string(mostSpent.first), timeColor, mostTimeSpentRect);
        timeColor.r += 24;
        timeColor.b -= 24;
        mostTimeSpentRect.y += mostTimeSpentRect.h;
    }
}

bool Background ::initializeWindow(void)
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

    textureWater[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Background/water.bmp"));
    textureWater[1] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Background/water.bmp"));

    // initialize boarder
    boarderTopRect.x = 0;
    boarderTopRect.y = -1;
    boarderTopRect.w = SCREEN_WIDTH - scoreTextRect.w;
    boarderTopRect.h = boarderHeight;

    boarderBottomRect.w = SCREEN_WIDTH;
    boarderBottomRect.h = boarderHeight;
    boarderBottomRect.x = 0;
    boarderBottomRect.y = SCREEN_HEIGHT - boarderBottomRect.h + 1;

    boarderLeftRect.x = -1;
    boarderLeftRect.y = boarderHeight - 1;
    boarderLeftRect.w = boarderWidth;
    boarderLeftRect.h = SCREEN_HEIGHT - (2 * boarderLeftRect.y);

    boarderRightRect.w = boarderWidth;
    boarderRightRect.x = SCREEN_WIDTH - boarderRightRect.w + 1;
    boarderRightRect.y = scoreTextRect.h;
    boarderRightRect.h = SCREEN_HEIGHT - scoreTextRect.h - boarderHeight + 1;

    textureBoarder[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Background/boarder.bmp"));

    // intialize middle screen message rectangle
    middleScreenTextRect.w = 600;
    middleScreenTextRect.h = 100;
    middleScreenTextRect.x = (SCREEN_WIDTH - middleScreenTextRect.w) / 2;
    middleScreenTextRect.y = (SCREEN_HEIGHT - middleScreenTextRect.h) / 2;

    // initialize top Screen Text rectangle
    screenTopTextRect.w = 600;
    screenTopTextRect.h = 100;
    screenTopTextRect.x = (SCREEN_WIDTH - screenTopTextRect.w) / 2;
    screenTopTextRect.y = boarderHeight + 30;

    // initialize bottom screen box rectangle
    screenBottomBoxRectangle.w = SCREEN_WIDTH - (boarderWidth + screenTopTextRect.x + screenTopTextRect.w);
    screenBottomBoxRectangle.h = SCREEN_HEIGHT - boarderHeight - scoreTextRect.h;
    screenBottomBoxRectangle.x = (SCREEN_WIDTH - screenBottomBoxRectangle.w) - boarderWidth;
    screenBottomBoxRectangle.y = (SCREEN_HEIGHT - screenBottomBoxRectangle.h) - boarderHeight;

    // inititalize bottom left screen text rectangle
    screenBottomLeftBox.w = (middleScreenTextRect.x - boarderWidth);
    screenBottomLeftBox.h = SCREEN_HEIGHT - 2 * boarderHeight - screenTopTextRect.h;
    screenBottomLeftBox.x = boarderWidth;
    screenBottomLeftBox.y = screenTopTextRect.y + screenTopTextRect.h / 2;

    // initialize whole obstacles
    wholeObstacleOne.w = boarderWidth;
    wholeObstacleOne.h = SCREEN_HEIGHT;
    wholeObstacleOne.x = snakeWidth * freeSpace;
    wholeObstacleOne.y = 0;

    wholeObstacleTwo.w = boarderWidth;
    wholeObstacleTwo.h = SCREEN_HEIGHT;
    wholeObstacleTwo.x = wholeObstacleOne.x + wholeObstacleOne.w + snakeWidth * freeSpace;
    wholeObstacleTwo.y = 0;

    wholeObstacleThree.w = boarderWidth;
    wholeObstacleThree.h = SCREEN_HEIGHT;
    wholeObstacleThree.x = wholeObstacleTwo.x + wholeObstacleTwo.w + snakeWidth * freeSpace;
    wholeObstacleThree.y = 0;

    wholeObstacleFour.w = boarderWidth;
    wholeObstacleFour.h = SCREEN_HEIGHT;
    wholeObstacleFour.x = wholeObstacleThree.x + wholeObstacleThree.w + snakeWidth * freeSpace;
    wholeObstacleFour.y = 0;

    wholeObstacleFive.w = SCREEN_WIDTH;
    wholeObstacleFive.h = boarderHeight;
    wholeObstacleFive.x = 0;
    wholeObstacleFive.y = (freeSpace * snakeHeight - wholeObstacleFive.h) / 2;

    wholeObstacleSix.w = SCREEN_WIDTH;
    wholeObstacleSix.h = boarderHeight;
    wholeObstacleSix.x = 0;
    wholeObstacleSix.y = (SCREEN_HEIGHT - boarderWidth) -
                         (snakeHeight * freeSpace - wholeObstacleSix.h) / 2;

    // initialize fruit eating and background sounds
    for (int i = 0; i < totalBackgroundSound; i++)
        backgroundMusic.push_back("./Sounds/backgroundTrack" + to_string(i + 1) + ".mp3");
    for (int i = 0; i < totalFruitEatingSound; i++)
        fruitEatingSound.push_back("./Sounds/eatFruit" + to_string(i + 1) + ".wav");
    for (int i = 0; i < totalCollisionSound; i++)
        collisionSound.push_back("./Sounds/gameOver" + to_string(i + 1) + ".wav");
    pauseSound = "./Sounds/pauseGame.wav";
    resumeSound = "./Sounds/resumeGame.wav";
    bonusFruitSound = "./Sounds/eatBonusFruit.wav";

    menuButtonSound = "./Sounds/menuButtonClick.wav";
    modeButtonSound = "./Sounds/modeButtonClick.wav";
}

void Background::initializeObstacle()
{
    // intialize obstacles
    leftFirstObstacle.w = boarderWidth;
    leftFirstObstacle.h = boarderHeight;
    leftFirstObstacle.x = snakeWidth * freeSpace;
    leftFirstObstacle.y = boarderBottomRect.y - leftFirstObstacle.w;

    leftSecondObstacle.w = boarderWidth;
    leftSecondObstacle.h = boarderHeight;
    leftSecondObstacle.x = leftFirstObstacle.x + leftFirstObstacle.w + snakeWidth * freeSpace;
    leftSecondObstacle.y = boarderHeight - 1;

    leftThirdObstacle.w = boarderWidth;
    leftThirdObstacle.h = boarderHeight;
    leftThirdObstacle.x = leftSecondObstacle.x + leftSecondObstacle.w + snakeWidth * freeSpace;
    leftThirdObstacle.y = leftFirstObstacle.y;

    leftFourthObstacle.w = boarderWidth;
    leftFourthObstacle.h = boarderHeight;
    leftFourthObstacle.x = leftThirdObstacle.x + leftSecondObstacle.w + snakeWidth * freeSpace;
    leftFourthObstacle.y = leftSecondObstacle.y;

    // initialize impossible mode obstacles
    horizontalFirstObstacle.w = boarderWidth;
    horizontalFirstObstacle.h = boarderHeight;
    horizontalFirstObstacle.y = (freeSpace * snakeHeight - horizontalFirstObstacle.h) / 2;
    horizontalFirstObstacle.x = leftFirstObstacle.x;

    horizontalSecondObstacle.w = boarderWidth;
    horizontalSecondObstacle.h = boarderHeight;
    horizontalSecondObstacle.y = (SCREEN_HEIGHT - boarderWidth) -
                                 (snakeHeight * freeSpace - horizontalSecondObstacle.h) / 2;
    horizontalSecondObstacle.x = leftSecondObstacle.x;

    horizontalThirdObstacle.w = boarderWidth;
    horizontalThirdObstacle.h = boarderHeight;
    horizontalThirdObstacle.y = horizontalFirstObstacle.y;
    horizontalThirdObstacle.x = leftThirdObstacle.x;

    horizontalFourthObstacle.w = boarderWidth;
    horizontalFourthObstacle.h = boarderHeight;
    horizontalFourthObstacle.y = horizontalSecondObstacle.y;
    horizontalFourthObstacle.x = leftFourthObstacle.x;
}

void Background ::resetGame()
{
    gameIsStarted = false;
    collisionDetected = false;
    keyWasPressed = false;
    gameIsPaused = false;
    increaseOpacity = false;
    pauseScreenSoundPlayed = false;
    newHighScore = false;
    newMostSurvived = false;
    mouseButtonPressed = false;
    showScorecard = false;
    modeSelectionActive = false;
    absoluteSnakeVelocity = false;
    totalPausedTime = 0.0;
    startPlaying = 0;
    endPlaying = 0;
    timePlayed = 0.0;
    totalScore = 0;
    prevResult = 0;
    mainMenuActive = true;
    goToGameScreen = false;
    startTheGameNow = false;
    snakeVelocity = snakeInitialVelocity;
    snakeBody.clear();
    snakeCurrentDirection = 'r';
    pressedKey = '/';  // default value;
    gameMode = "Easy"; // default game mode

    initializeObstacle();
    obstacleMovingSpeed = baseSpeed;
}

void Background ::getUserName(void)
{
    string userInput = "";
    cout << "Enter Your Name Here (LIMIT: " + to_string(characterLimit) + " CHARACTERS): ";
    getline(cin, userInput);
    for (int i = 0; i < userInput.size(); i++)
    {
        if (userInput[i] == ':' || i >= characterLimit)
            break;
        userName.push_back(userInput[i]);
    }
}

void Background ::Render(void)
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

void Background ::renderExtraObstacles()
{
    simpleRenderer(horizontalFirstObstacle, *textureBoarder[0]);
    simpleRenderer(horizontalSecondObstacle, *textureBoarder[0]);
    simpleRenderer(horizontalThirdObstacle, *textureBoarder[0]);
    simpleRenderer(horizontalFourthObstacle, *textureBoarder[0]);

    // move Obstacles
    // move first obstacle
    if (isLeft1)
        horizontalFirstObstacle.x += obstacleMovingSpeed;
    else
        horizontalFirstObstacle.x -= obstacleMovingSpeed;
    if (horizontalFirstObstacle.x <= (boarderWidth + (freeSpace / 2) * snakeHeight))
        isLeft1 = true;
    else if (horizontalFirstObstacle.x >= leftFirstObstacle.x + (freeSpace / 2) * snakeHeight)
        isLeft1 = false;

    // move third obstacle
    if (isLeft2)
        horizontalThirdObstacle.x += obstacleMovingSpeed;
    else
        horizontalThirdObstacle.x -= obstacleMovingSpeed;
    if (horizontalThirdObstacle.x <= leftThirdObstacle.x - ((freeSpace / 2) * snakeHeight))
        isLeft2 = true;
    else if (horizontalThirdObstacle.x >= leftThirdObstacle.x + (freeSpace / 2) * snakeHeight)
        isLeft2 = false;

    // move second obstacle
    if (isLeft3)
        horizontalSecondObstacle.x += obstacleMovingSpeed;
    else
        horizontalSecondObstacle.x -= obstacleMovingSpeed;
    if (horizontalSecondObstacle.x <= leftSecondObstacle.x - ((freeSpace / 2) * snakeHeight))
        isLeft3 = true;
    else if (horizontalSecondObstacle.x >= leftSecondObstacle.x + (freeSpace / 2) * snakeHeight)
        isLeft3 = false;

    // move fourth obstacle
    if (isLeft4)
        horizontalFourthObstacle.x += obstacleMovingSpeed;
    else
        horizontalFourthObstacle.x -= obstacleMovingSpeed;
    if (horizontalFourthObstacle.x <= leftFourthObstacle.x - ((freeSpace / 2) * snakeHeight))
        isLeft4 = true;
    else if (horizontalFourthObstacle.x >= leftFourthObstacle.x + (freeSpace / 2) * snakeHeight)
        isLeft4 = false;
}

void Background ::renderObstacles()
{
    // render obstacle
    simpleRenderer(leftFirstObstacle, *textureBoarder[0]);
    simpleRenderer(leftSecondObstacle, *textureBoarder[0]);
    simpleRenderer(leftThirdObstacle, *textureBoarder[0]);
    simpleRenderer(leftFourthObstacle, *textureBoarder[0]);

    // move obstacle
    // move first & third obstacle
    if (leftFirstObstacle.y >= (boarderBottomRect.y - leftFirstObstacle.w))
        isTop = false;
    else if (leftFirstObstacle.y <= boarderHeight + (snakeHeight * freeSpace))
        isTop = true;
    if (isTop)
    {
        leftFirstObstacle.y += obstacleMovingSpeed;
        leftThirdObstacle.y += obstacleMovingSpeed;
    }
    else
    {
        leftFirstObstacle.y -= obstacleMovingSpeed;
        leftThirdObstacle.y -= obstacleMovingSpeed;
    }

    // move second and fourth obstacle
    if (leftSecondObstacle.y <= (boarderHeight - 1))
        isBottom = false;
    else if (leftSecondObstacle.y >= (SCREEN_HEIGHT - boarderHeight - (snakeHeight * freeSpace)))
        isBottom = true;
    if (isBottom)
    {
        leftSecondObstacle.y -= obstacleMovingSpeed;
        leftFourthObstacle.y -= obstacleMovingSpeed;
    }
    else
    {
        leftSecondObstacle.y += obstacleMovingSpeed;
        leftFourthObstacle.y += obstacleMovingSpeed;
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

    while (true)
    {

        fruitControl.x = rand() % SCREEN_WIDTH;
        fruitControl.y = rand() % SCREEN_HEIGHT;
        while (SDL_HasIntersection(&fruitControl, &snakeBody[0]) ||
               SDL_HasIntersection(&fruitControl, &scoreTextRect) ||
               SDL_HasIntersection(&fruitControl, &boarderTopRect) ||
               SDL_HasIntersection(&fruitControl, &boarderBottomRect) ||
               SDL_HasIntersection(&fruitControl, &boarderLeftRect) ||
               SDL_HasIntersection(&fruitControl, &boarderRightRect))
            continue;

        if (gameMode != "Easy")
        {
            if (SDL_HasIntersection(&fruitControl, &wholeObstacleOne) ||
                SDL_HasIntersection(&fruitControl, &wholeObstacleTwo) ||
                SDL_HasIntersection(&fruitControl, &wholeObstacleThree) ||
                SDL_HasIntersection(&fruitControl, &wholeObstacleFour))
                continue;
            if (gameMode == "Impossible")
            {
                if (SDL_HasIntersection(&fruitControl, &wholeObstacleFive) ||
                    SDL_HasIntersection(&fruitControl, &wholeObstacleSix))
                    continue;
            }
        }
        break;
    }

    // intialize bonus fruit
    bonusFruitControl.w = fruitWidth;
    bonusFruitControl.h = fruitHeight;

    // pick A random fruit;
    pickFruit = rand() % (totalFruitTexture - 1);
}

void Fruit ::initializeBonusFruit()
{
    // avoid snake body & regular fruit
    bool fruitInsideSnake = true;
    while (fruitInsideSnake)
    {
        bonusFruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - 2 * boarderWidth));
        bonusFruitControl.y = boarderHeight + (rand() % (SCREEN_HEIGHT - fruitHeight - 2 * boarderHeight));

        // avoid regular fruit & score showing rectangle
        if (SDL_HasIntersection(&bonusFruitControl, &fruitControl) ||
            SDL_HasIntersection(&bonusFruitControl, &scoreTextRect))
            continue;
        // avoid obstacle if exists
        if (gameMode != "Easy")
        {
            if (SDL_HasIntersection(&bonusFruitControl, &wholeObstacleOne) ||
                SDL_HasIntersection(&bonusFruitControl, &wholeObstacleTwo) ||
                SDL_HasIntersection(&bonusFruitControl, &wholeObstacleThree) ||
                SDL_HasIntersection(&bonusFruitControl, &wholeObstacleFour))
                continue;
            if (gameMode == "Impossible")
            {
                if (SDL_HasIntersection(&bonusFruitControl, &wholeObstacleFive) ||
                    SDL_HasIntersection(&bonusFruitControl, &wholeObstacleSix))
                    continue;
            }
        }
        // avoid snake body
        for (int i = 0; i < snakeBody.size(); i++)
        {
            if (SDL_HasIntersection(&snakeBody[i], &bonusFruitControl))
            {
                fruitInsideSnake = true;
                break;
            }
            else
                fruitInsideSnake = false;
        }
    }
}

Snake ::Snake(string filePath)
{
    initialSnake.w = snakeWidth;
    initialSnake.h = snakeHeight;
    initialSnake.x = boarderWidth + 1;
    initialSnake.y = (SCREEN_HEIGHT - initialSnake.h) / 2;

    textureSnakeSkin = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP(filePath.c_str()));

    snakeBody.push_back(initialSnake);

    // speed increase after a certain score
    // use the current system time ass seed to generate pseudo-random values;
    srand((unsigned)time(0));
    incraseSpeedAfter = 10 + rand() % 11;
    SDL_Log("Increase Snake Speed After: %d", incraseSpeedAfter);
}

void Snake ::updateSnakePosition(int snakeVelocity)
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

bool Collision ::detectCollisionWithFruit(Snake *snake)
{
    if (SDL_HasIntersection(&fruitControl, &snakeBody[0]))
    {
        snake->updateSnakeSize();
        // spawn new fruit. i.e., pick a random fruit & also avoid snake body
        pickFruit = rand() % (totalFruitTexture - 1);
        // avoid snake body & bonus fruit
        bool fruitInsideSnake = true;
        while (fruitInsideSnake)
        {
            fruitControl.x = boarderWidth + (rand() % (SCREEN_WIDTH - fruitWidth - 2 * boarderWidth));
            fruitControl.y = boarderHeight + (rand() % (SCREEN_HEIGHT - fruitHeight - 2 * boarderHeight));

            // avoid bonus fruit & score showing rectangle
            if (SDL_HasIntersection(&bonusFruitControl, &fruitControl) ||
                SDL_HasIntersection(&fruitControl, &scoreTextRect))
                continue;
            // avoid obstacle is exists
            if (gameMode != "Easy")
            {
                if (SDL_HasIntersection(&fruitControl, &wholeObstacleOne) ||
                    SDL_HasIntersection(&fruitControl, &wholeObstacleTwo) ||
                    SDL_HasIntersection(&fruitControl, &wholeObstacleThree) ||
                    SDL_HasIntersection(&fruitControl, &wholeObstacleFour))
                    continue;
                if (gameMode == "Impossible")
                {
                    if (SDL_HasIntersection(&fruitControl, &wholeObstacleFive) ||
                        SDL_HasIntersection(&fruitControl, &wholeObstacleSix))
                        continue;
                }
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
        return true;
    }
    return false;
}

bool Collision::detectCollisionWithBonusFruit()
{
    if (SDL_HasIntersection(&snakeBody[0], &bonusFruitControl))
        return true;
    return false;
}

bool Collision::detectCollision(Background &background)
{
    // check collision with Obstacle
    if (gameMode == "Impossible")
    {
        for (int i = 0; i < snakeBody.size(); i++)
        {
            if (SDL_HasIntersection(&snakeBody[i], &background.leftFirstObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.leftSecondObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.leftThirdObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.leftFourthObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.horizontalFirstObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.horizontalSecondObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.horizontalThirdObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.horizontalFourthObstacle))
                return true;
        }
    }
    else if (gameMode == "Hard")
    {
        for (int i = 0; i < snakeBody.size(); i++)
        {
            if (SDL_HasIntersection(&snakeBody[i], &background.leftFirstObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.leftSecondObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.leftThirdObstacle) ||
                SDL_HasIntersection(&snakeBody[i], &background.leftFourthObstacle))
                return true;
        }
    }

    // check collosion with boarder
    if (SDL_HasIntersection(&snakeBody[0], &boarderTopRect) ||
        SDL_HasIntersection(&snakeBody[0], &boarderBottomRect) ||
        SDL_HasIntersection(&snakeBody[0], &boarderLeftRect) ||
        SDL_HasIntersection(&snakeBody[0], &boarderRightRect) ||
        SDL_HasIntersection(&snakeBody[0], &scoreTextRect))
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

void mainMenu::initializeMainMenu()
{
    int height = 60, width = 200, times = -1, space = 15;
    menuTextR.h = 50;
    menuTextR.w = width;
    menuTextR.x = (SCREEN_WIDTH - menuTextR.w) / 2;
    menuTextR.y = (SCREEN_HEIGHT - menuTextR.h) / 2 - (height + space) - (menuTextR.h + space);

    menuPlayR.h = height;
    menuPlayR.w = width;
    menuPlayR.x = (SCREEN_WIDTH - menuPlayR.w) / 2;
    menuPlayR.y = (SCREEN_HEIGHT - menuPlayR.h) / 2 + times++ * (menuPlayR.h + space);

    menuDifficultyR.h = height;
    menuDifficultyR.w = width;
    menuDifficultyR.x = (SCREEN_WIDTH - menuDifficultyR.w) / 2;
    menuDifficultyR.y = (SCREEN_HEIGHT - menuDifficultyR.h) / 2 +
                        times++ * (menuDifficultyR.h + space);

    menuScorecardR.h = height;
    menuScorecardR.w = width;
    menuScorecardR.x = (SCREEN_WIDTH - menuScorecardR.w) / 2;
    menuScorecardR.y = (SCREEN_HEIGHT - menuScorecardR.h) / 2 +
                       times++ * (menuScorecardR.h + space);

    menuQuitR.h = height;
    menuQuitR.w = width;
    menuQuitR.x = (SCREEN_WIDTH - menuQuitR.w) / 2;
    menuQuitR.y = (SCREEN_HEIGHT - menuQuitR.h) / 2 + times++ * (menuQuitR.h + space);

    // initialize mode rectangle
    int spaceBetween = 50;
    modeTextR.h = 50;
    modeTextR.w = width;
    modeTextR.x = menuDifficultyR.x + menuDifficultyR.w + spaceBetween;
    modeTextR.y = menuDifficultyR.y;

    modeEasyR.h = height;
    modeEasyR.w = width;
    modeEasyR.x = modeTextR.x;
    modeEasyR.y = modeTextR.y + modeTextR.h + space;

    modeHardR.h = height;
    modeHardR.w = width;
    modeHardR.x = modeEasyR.x;
    modeHardR.y = modeEasyR.y + modeEasyR.h + space;

    modeImpossibleR.h = height;
    modeImpossibleR.w = width;
    modeImpossibleR.x = modeHardR.x;
    modeImpossibleR.y = modeHardR.y + modeHardR.h + space;

    // initialize texture
    textureMainMenu[0] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/menuText.bmp"));
    textureMainMenu[1] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/menuPlay.bmp"));
    textureMainMenu[2] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/menuDifficulty.bmp"));
    textureMainMenu[3] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/menuScorecard.bmp"));
    textureMainMenu[4] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/menuQuit.bmp"));
    textureMainMenu[5] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/modeText.bmp"));
    textureMainMenu[6] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/modeEasy.bmp"));
    textureMainMenu[7] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/modeHard.bmp"));
    textureMainMenu[8] = SDL_CreateTextureFromSurface(renderer, SDL_LoadBMP("./Other/modeImpossible.bmp"));
}

void mainMenu ::mouseMovement(Music &music)
{
    SDL_Rect mouseRect = {mouseX, mouseY, 1, 1};
    if (SDL_HasIntersection(&mouseRect, &menuPlayR))
    {
        music.playChuck(menuButtonSound, 3, 0, 10);
        goToGameScreen = true;
        modeSelectionActive = false;
        showScorecard = false;
        mainMenuActive = false;
    }
    else if (SDL_HasIntersection(&mouseRect, &menuScorecardR))
    {
        music.playChuck(menuButtonSound, 3, 0, 10);
        if (!showScorecard)
            showScorecard = true;
        else
            showScorecard = false;
    }
    else if (SDL_HasIntersection(&mouseRect, &menuQuitR))
    {
        music.playChuck(menuButtonSound, 3, 0, 10);
        gameIsRunning = false;
    }
    else if (SDL_HasIntersection(&mouseRect, &menuDifficultyR))
    {
        music.playChuck(menuButtonSound, 3, 0, 10);
        if (!modeSelectionActive)
            modeSelectionActive = true;
        else
            modeSelectionActive = false;
    }

    // check for modes
    if (modeSelectionActive)
    {
        if (SDL_HasIntersection(&mouseRect, &modeEasyR))
        {
            music.playChuck(modeButtonSound, 3, 0, 10);
            gameMode = "Easy";
        }
        else if (SDL_HasIntersection(&mouseRect, &modeHardR))
        {
            music.playChuck(modeButtonSound, 3, 0, 10);
            gameMode = "Hard";
        }
        else if (SDL_HasIntersection(&mouseRect, &modeImpossibleR))
        {
            music.playChuck(modeButtonSound, 3, 0, 10);
            gameMode = "Impossible";
        }
    }
}

void mainMenu ::renderMenu()
{
    Render(menuTextR, *textureMainMenu[0]);
    Render(menuPlayR, *textureMainMenu[1]);
    Render(menuDifficultyR, *textureMainMenu[2]);
    Render(menuScorecardR, *textureMainMenu[3]);
    Render(menuQuitR, *textureMainMenu[4]);
}

void mainMenu::renderModeSelection()
{
    Render(modeTextR, *textureMainMenu[5]);
    Render(modeEasyR, *textureMainMenu[6]);
    Render(modeHardR, *textureMainMenu[7]);
    Render(modeImpossibleR, *textureMainMenu[8]);
}
