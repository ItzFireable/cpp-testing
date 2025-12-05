#pragma once

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <chrono>
#include <TextureLoader.h>
#include <SpriteBatch.h>

#include <utils/Logger.h>
#include <utils/AudioManager.h>
#include <objects/FPSCounter.h>
#include <objects/DebugInfo.h>
#include <rhythm/ChartManager.h>

#define GAME_NAME "?"
#define GAME_VERSION "?.?.?"

enum AppStateID
{
    STATE_SONG_SELECT = 1,
    STATE_PLAY = 2,
    STATE_RESULTS = 3,
    STATE_COUNT
};

static constexpr int FRAMERATE_CAP = 999;
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

inline const std::string MAIN_FONT_PATH = "assets/fonts/GoogleSansCode-Bold.ttf";

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

extern TimePoint lastInputTime;
extern TimePoint lastFrameTime;
extern double currentInputLatencyMs;

inline int glfwKeyToSDL(int glfwKey) {
    switch (glfwKey) {
        case GLFW_KEY_Z: return SDLK_Z;
        case GLFW_KEY_X: return SDLK_X;
        case GLFW_KEY_COMMA: return SDLK_COMMA;
        case GLFW_KEY_PERIOD: return SDLK_PERIOD;
        case GLFW_KEY_ESCAPE: return SDLK_ESCAPE;
        case GLFW_KEY_ENTER: return SDLK_RETURN;
        case GLFW_KEY_SPACE: return SDLK_SPACE;
        case GLFW_KEY_LEFT: return SDLK_LEFT;
        case GLFW_KEY_RIGHT: return SDLK_RIGHT;
        case GLFW_KEY_UP: return SDLK_UP;
        case GLFW_KEY_DOWN: return SDLK_DOWN;
        default: return glfwKey;
    }
}

#define KEYBIND_STRUM_LEFT GLFW_KEY_Z
#define KEYBIND_STRUM_DOWN GLFW_KEY_X
#define KEYBIND_STRUM_UP GLFW_KEY_COMMA
#define KEYBIND_STRUM_RIGHT GLFW_KEY_PERIOD

class FPSCounter; 
class DebugInfo;
class BaseState;

using StateSwitcher = void (*)(AppContext*, int, void*);

struct PlayStateData
{
    std::string songPath;
    std::string chartFile;
    ChartData chartData;
    float playbackRate;
};

struct AppContext
{
    GLFWwindow* window;
    GLuint renderTarget;
    GLuint renderTexture;
    bool appQuit = false;

    AudioManager& audioManager = AudioManager::getInstance();
    FPSCounter* fpsCounter = nullptr;
    DebugInfo* debugInfo = nullptr;

    StateSwitcher switchState = nullptr;
    BaseState* currentState = nullptr;

    SpriteBatch* spriteBatch = nullptr;
    
    float renderWidth = 1920.0f;
    float renderHeight = 1080.0f;

    bool isTransitioning = false;
    float transitionProgress = 0.0f;
    float transitionDuration = 0.3f;
    int nextState = -1;
    void* nextStatePayload = nullptr;
    bool transitioningOut = true;
};