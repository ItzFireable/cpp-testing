#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

#include <utils/AudioManager.h>
#include <utils/Variables.h>
#include <BaseState.h>
#include <states/SongSelectState.h>
#include <states/PlayState.h>
#include <states/ResultsState.h>
#include <objects/FPSCounter.h>

#include "SpriteBatch.h"
#include "TextRenderer.h"

using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;
using Duration = std::chrono::duration<double>;

TimePoint lastFrameTime;
TimePoint lastInputTime;
double currentInputLatencyMs = 0.0;

const double TARGET_FRAME_TIME = 1.0 / 999.0; // 999 FPS cap

BaseState* state = nullptr;
void* statePayload = nullptr;

int curState = -1;
int prevState = -1;

int WINDOW_WIDTH = 1600;
int WINDOW_HEIGHT = 900;

// Forward declarations
BaseState* createState(int stateID);
void setState(AppContext* app, int stateID, void* payload);
bool setRenderResolution(AppContext* app, float width, float height);

// GLFW Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    AppContext* app = static_cast<AppContext*>(glfwGetWindowUserPointer(window));
    
    lastInputTime = Clock::now();
    
    // Create SDL-like event for compatibility with existing code
    SDL_Event event;
    if (action == GLFW_PRESS) {
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.key = key; // Map GLFW keys to SDL keys
        event.key.repeat = false;
    } else if (action == GLFW_RELEASE) {
        event.type = SDL_EVENT_KEY_UP;
        event.key.key = key;
    } else if (action == GLFW_REPEAT) {
        event.type = SDL_EVENT_KEY_DOWN;
        event.key.repeat = true;
    }
    
    if (state) {
        state->handleEvent(event);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    SDL_Event event;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    
    if (action == GLFW_PRESS) {
        event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    } else {
        event.type = SDL_EVENT_MOUSE_BUTTON_UP;
    }
    
    event.button.button = button;
    event.button.x = static_cast<float>(xpos);
    event.button.y = static_cast<float>(ypos);
    
    if (state) {
        state->handleEvent(event);
    }
}

BaseState* createState(int stateID) {
    switch (stateID) {
        case STATE_SONG_SELECT:
            return new SongSelectState();
        case STATE_PLAY:
            return new PlayState();
        case STATE_RESULTS:
            return new ResultsState();
        default:
            return nullptr;
    }
}

void setState(AppContext* app, int stateID, void* payload) {
    if (stateID >= 0 && stateID < STATE_COUNT) {
        if (!app->isTransitioning) {
            app->isTransitioning = true;
            app->transitioningOut = true;
            app->transitionProgress = 0.0f;
            app->nextState = stateID;
            app->nextStatePayload = payload;
        }
    }
}

bool setRenderResolution(AppContext* app, float width, float height) {
    if (app->renderTarget) {
        glDeleteFramebuffers(1, &app->renderTarget);
        glDeleteTextures(1, &app->renderTexture);
    }

    glGenFramebuffers(1, &app->renderTarget);
    glBindFramebuffer(GL_FRAMEBUFFER, app->renderTarget);

    glGenTextures(1, &app->renderTexture);
    glBindTexture(GL_TEXTURE_2D, app->renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, (int)width, (int)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, app->renderTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        GAME_LOG_ERROR("Failed to create framebuffer");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    app->renderWidth = width;
    app->renderHeight = height;

    return true;
}

int main(int argc, char* argv[]) {
    Logger::getInstance().setLogLevel(LogLevel::GAME_DEBUG);
    if (!glfwInit()) {
        GAME_LOG_ERROR("Failed to initialize GLFW");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    char title[256];
    snprintf(title, 256, "%s v%s", GAME_NAME, GAME_VERSION);
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title, nullptr, nullptr);
    if (!window) {
        GAME_LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        GAME_LOG_ERROR("Failed to initialize GLEW");
        return -1;
    }

    glfwSwapInterval(0);

    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    TextureLoader::init();

    auto* app = new AppContext();
    app->window = window;
    app->switchState = setState;
    
    glfwSetWindowUserPointer(window, app);

    app->fpsCounter = new FPSCounter(nullptr, MAIN_FONT_PATH, 16, 7.0f);
    app->fpsCounter->setAppContext(app);
    app->fpsCounter->update();

    app->debugInfo = new DebugInfo(nullptr, MAIN_FONT_PATH, 16, 30.0f);
    app->debugInfo->setAppContext(app);
    app->debugInfo->setYPosition(app->fpsCounter->getYPosition() + app->fpsCounter->getHeight() + 5.0f);
    app->debugInfo->update();

    setRenderResolution(app, app->renderWidth, app->renderHeight);

    SpriteBatch* spriteBatch = new SpriteBatch();
    spriteBatch->init(app->renderWidth, app->renderHeight);

    app->spriteBatch = spriteBatch;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (!AudioManager::getInstance().initialize()) {
        GAME_LOG_ERROR("Failed to initialize AudioManager");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    lastFrameTime = Clock::now();
    lastInputTime = Clock::now();

    curState = STATE_SONG_SELECT;

    while (!glfwWindowShouldClose(window)) {
        TimePoint frameStartTime = Clock::now();
        Duration deltaTimeDuration = frameStartTime - lastFrameTime;
        float deltaTime = static_cast<float>(deltaTimeDuration.count());
        lastFrameTime = frameStartTime;

        Duration inputLatency = frameStartTime - lastInputTime;
        currentInputLatencyMs = inputLatency.count() * 1000.0;

        glfwPollEvents();

        if (app->isTransitioning) {
            app->transitionProgress += deltaTime / app->transitionDuration;

            if (app->transitioningOut && app->transitionProgress >= 1.0f) {
                if (state) {
                    state->destroy();
                    delete state;
                    state = nullptr;
                }

                curState = app->nextState;
                state = createState(curState);
                prevState = curState;

                if (state) {
                    state->init(app, app->nextStatePayload);
                    app->nextStatePayload = nullptr;
                    app->currentState = state;
                }

                app->transitioningOut = false;
                app->transitionProgress = 0.0f;
                lastFrameTime = Clock::now();
            } else if (!app->transitioningOut && app->transitionProgress >= 1.0f) {
                app->isTransitioning = false;
                app->transitionProgress = 0.0f;
            }
        }

        if (!app->isTransitioning && curState != prevState) {
            if (state) {
                state->destroy();
                delete state;
                state = nullptr;
            }

            state = createState(curState);
            prevState = curState;

            if (state) {
                state->init(app, statePayload);
                statePayload = nullptr;
                app->currentState = state;
            }
        }

        glfwGetFramebufferSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
        AudioManager::getInstance().updateStream();

        if (app->fpsCounter) {
            app->fpsCounter->update();
        }

        if (app->debugInfo) {
            FPSCounter* fpsCounter = app->fpsCounter;
            app->debugInfo->setYPosition(fpsCounter->getYPosition() + fpsCounter->getHeight() + 5.0f);
            app->debugInfo->update();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, app->renderTarget);
        glViewport(0, 0, (int)app->renderWidth, (int)app->renderHeight);
        glClear(GL_COLOR_BUFFER_BIT);

        if (state) {
            state->update(deltaTime);
            state->render();
        }

        if (app->fpsCounter) {
            app->fpsCounter->render(nullptr);
        }

        if (app->debugInfo) {
            app->debugInfo->render(nullptr);
        }

        if (app->isTransitioning) {
            float fadeAlpha;
            if (app->transitioningOut) {
                fadeAlpha = app->transitionProgress;
            } else {
                fadeAlpha = 1.0f - app->transitionProgress;
            }

            fadeAlpha = std::clamp(fadeAlpha, 0.0f, 1.0f);
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            glColor4f(0.0f, 0.0f, 0.0f, fadeAlpha);
            glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(app->renderWidth, 0.0f);
            glVertex2f(app->renderWidth, app->renderHeight);
            glVertex2f(0.0f, app->renderHeight);
            glEnd();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT);

        float scaleX = (float)WINDOW_WIDTH / app->renderWidth;
        float scaleY = (float)WINDOW_HEIGHT / app->renderHeight;
        float scale = (scaleX < scaleY) ? scaleX : scaleY;

        int scaledWidth = (int)(app->renderWidth * scale);
        int scaledHeight = (int)(app->renderHeight * scale);
        int offsetX = (WINDOW_WIDTH - scaledWidth) / 2;
        int offsetY = (WINDOW_HEIGHT - scaledHeight) / 2;

        glBindTexture(GL_TEXTURE_2D, app->renderTexture);
        glEnable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f((float)offsetX, (float)offsetY);
        glTexCoord2f(1.0f, 1.0f); glVertex2f((float)(offsetX + scaledWidth), (float)offsetY);
        glTexCoord2f(1.0f, 0.0f); glVertex2f((float)(offsetX + scaledWidth), (float)(offsetY + scaledHeight));
        glTexCoord2f(0.0f, 0.0f); glVertex2f((float)offsetX, (float)(offsetY + scaledHeight));
        glEnd();

        glfwSwapBuffers(window);

        if (state) {
            state->postBuffer();
        }

        TimePoint frameEndTime = Clock::now();
        Duration frameDuration = frameEndTime - frameStartTime;
        double frameTimeSeconds = frameDuration.count();

        if (frameTimeSeconds < TARGET_FRAME_TIME) {
            double waitTime = TARGET_FRAME_TIME - frameTimeSeconds;
            const double spinThreshold = 0.0005;
            
            if (waitTime > spinThreshold) {
                auto sleepDuration = std::chrono::duration<double>(waitTime - spinThreshold);
                std::this_thread::sleep_for(sleepDuration);
            }

            while ((Clock::now() - frameStartTime).count() < TARGET_FRAME_TIME) {
                // busy wait cause im lazy
            }
        }
    }

    if (state) {
        state->destroy();
        delete state;
        state = nullptr;
    }

    if (app->fpsCounter) {
        delete app->fpsCounter;
        app->fpsCounter = nullptr;
    }

    if (app->debugInfo) {
        delete app->debugInfo;
        app->debugInfo = nullptr;
    }

    TextureLoader::shutdown();

    AudioManager::getInstance().shutdown();
    Logger::getInstance().shutdown();

    glDeleteFramebuffers(1, &app->renderTarget);
    glDeleteTextures(1, &app->renderTexture);

    delete app;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}