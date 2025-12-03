#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <iostream>

#include <utils/AudioManager.h>
#include <utils/Variables.h>

#include <BaseState.h>
#include <states/SongSelectState.h>
#include <states/PlayState.h>
#include <objects/FPSCounter.h>

BaseState *state = NULL;
void* statePayload = nullptr;

int curState = -1;
int prevState = -1;

Uint64 lastInputTick = 0;
float currentInputLatencyMs = 0.0f;

Uint64 TARGET_TICK_DURATION = SDL_GetPerformanceFrequency() / FRAMERATE_CAP;
const char* FPS_FONT_PATH = "assets/fonts/GoogleSansCode-Bold.ttf";

BaseState *createState(int stateID)
{
    switch (stateID)
    {
    case STATE_SONG_SELECT:
        return new SongSelectState();
    case STATE_PLAY:
        return new PlayState();
    default:
        return nullptr;
    }
}

void setState(int stateID, void* payload)
{
    if (stateID >= 0 && stateID < STATE_COUNT)
    {
        curState = stateID;
        statePayload = payload;
    }
}

SDL_AppResult SDL_Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Error %s", SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    Logger::getInstance().setLogLevel(LogLevel::GAME_DEBUG);
    GAME_LOG_INFO("Logger initialized.");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        GAME_LOG_ERROR("Failed to initialize SDL: " + std::string(SDL_GetError()));
        return SDL_Fail();
    }

    if (TTF_Init() == -1) {
        GAME_LOG_ERROR("Failed to initialize SDL_ttf: " + std::string(SDL_GetError()));
        return SDL_Fail();
    }

    char *title = (char *)malloc(256);
    snprintf(title, 256, "%s v%s", GAME_NAME, GAME_VERSION);

    SDL_Window *window = SDL_CreateWindow(title, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!window)
    {
        GAME_LOG_ERROR("Failed to create SDL window: " + std::string(SDL_GetError()));
        return SDL_Fail();
    }

    auto *app = (AppContext *)SDL_malloc(sizeof(AppContext));
    *appstate = app;

    app->window = window;
    app->renderer = SDL_CreateRenderer(window, NULL);
    app->fpsCounter = new FPSCounter(app->renderer, FPS_FONT_PATH, 12);

    app->switchState = setState;

    if (!app->renderer)
    {
        GAME_LOG_ERROR("Failed to create SDL renderer: " + std::string(SDL_GetError()));
        return SDL_Fail();
    }

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_ShowWindow(window);

    if (!AudioManager::getInstance().initialize()) {
        GAME_LOG_ERROR("Failed to initialize AudioManager.");
        return SDL_Fail();
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    auto *app = (AppContext *)appstate;

    switch (event->type)
    {
    case SDL_EVENT_QUIT:
    {
        app->appQuit = SDL_APP_SUCCESS;
        break;
    }
    default:
    {
        if (state != nullptr)
        {
            state->handleEvent(*event);
        }
    }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto *app = (AppContext *)appstate;

    if (curState < 0)
        curState = STATE_SONG_SELECT;

    if (curState != prevState)
    {
        if (state)
        {
            state->destroy();
            delete state;
            state = nullptr;
        }

        state = createState(curState);
        prevState = curState;

        if (state != nullptr)
        {
            if (app->renderer == nullptr)
            {
                GAME_LOG_ERROR("Renderer is null during state initialization.");
                return SDL_APP_FAILURE;
            }

            state->init(app, statePayload);
            statePayload = nullptr;
        }
    }

    Uint64 frameStartTime = SDL_GetPerformanceCounter();
    
    if (app->renderer != nullptr)
    {
        AudioManager::getInstance().updateStream();

        if (app->fpsCounter) {
            app->fpsCounter->update();
        }

        SDL_RenderClear(app->renderer);

        if (state != nullptr)
        {
            state->update();
            state->postBuffer();
        }

        if (app->fpsCounter) {
            app->fpsCounter->render();
        }

        SDL_RenderPresent(app->renderer);

        Uint64 frameEndTime = SDL_GetPerformanceCounter();
        Uint64 frameDurationTicks = frameEndTime - frameStartTime;

        if (frameDurationTicks < TARGET_TICK_DURATION)
        {
            Uint64 waitTicks = TARGET_TICK_DURATION - frameDurationTicks;
            
            float waitTimeMs = (float)waitTicks * 1000.0f / (float)SDL_GetPerformanceFrequency();
            float pollingMarginMs = 0.5f; 

            if (waitTimeMs > pollingMarginMs) {
                SDL_Delay((Uint32)(waitTimeMs - pollingMarginMs));
            }

            Uint64 finalWaitEnd = frameStartTime + TARGET_TICK_DURATION;
            while (SDL_GetPerformanceCounter() < finalWaitEnd) { } 
        }
    }

    return app->appQuit;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (state != nullptr)
    {
        state->destroy();
        delete state;
        state = nullptr;
    }

    Logger::getInstance().shutdown();

    auto *app = (AppContext *)appstate;
    if (app)
    {
        if (app->fpsCounter) {
            delete app->fpsCounter;
            app->fpsCounter = nullptr;
        }
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }

    TTF_Quit();
    SDL_Quit();
}