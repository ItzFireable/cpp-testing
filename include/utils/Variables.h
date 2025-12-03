#pragma once

#include <SDL3/SDL.h>
#include <utils/Logger.h>
#include <utils/AudioManager.h>
#include <objects/FPSCounter.h>

#define GAME_NAME "?"
#define GAME_VERSION "?.?.?"

enum AppStateID
{
    STATE_SONG_SELECT = 1,
    STATE_PLAY = 2,
    STATE_COUNT
};

static constexpr int FRAMERATE_CAP = 240;
static constexpr int WINDOW_WIDTH = 1280;
static constexpr int WINDOW_HEIGHT = 720;

extern Uint64 lastInputTick;
extern float currentInputLatencyMs;
extern Uint64 TARGET_TICK_DURATION;

using StateSwitcher = void (*)(int, void*);

struct PlayStateData
{
    std::string songPath;
    std::string chartFile;
    float playbackRate;
};

struct AppContext
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_AppResult appQuit = SDL_APP_CONTINUE;

    AudioManager& audioManager = AudioManager::getInstance();
    FPSCounter* fpsCounter = nullptr;

    StateSwitcher switchState = nullptr;
};