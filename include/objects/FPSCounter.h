#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <SDL3/SDL.h>
#include <string>
#include "TextObject.h" // Assuming TextObject.h is in the same directory

class FPSCounter {
public:
    /**
     * @brief Initializes the FPS counter, creating the underlying TextObject.
     */
    FPSCounter(SDL_Renderer* renderer, const std::string& fontPath, int fontSize);
    
    ~FPSCounter();

    /**
     * @brief Updates the internal timing logic and, if needed, recalculates the FPS
     * and updates the TextObject's text.
     */
    void update();

    /**
     * @brief Renders the FPS text to the screen.
     */
    void render();

private:
    TextObject* textObject_ = nullptr;
    
    // Timing variables for FPS calculation
    Uint64 lastTick_ = 0;
    Uint64 perfFrequency_ = 0;
    
    Uint32 frameCount_ = 0;
    float frameAccumulator_ = 0.0f; // Accumulates time in seconds

    float latestFrameTimeMs_ = 0.0f;
    long memoryUsageKb_ = 0;

    long getAppMemoryUsageKb();
};

#endif // FPS_COUNTER_H