#ifndef BASE_STATE_H
#define BASE_STATE_H

#include <string>
#include "utils/Variables.h"
#include <SDL3/SDL.h>

class BaseState
{
public:
    virtual ~BaseState(); 

    virtual void init(AppContext* appContext, void* payload = nullptr) {
        this->appContext = appContext;
        this->renderer = appContext->renderer;
    };
    virtual void handleEvent(const SDL_Event& e); 
    virtual void update();
    virtual void destroy();
    virtual void postBuffer();
    
    virtual std::string getName() const { return "BaseState"; }
    void requestStateSwitch(int stateID, void* payload = nullptr) {
        if (appContext && appContext->switchState) {
            appContext->switchState(stateID, payload);
        } else {
            GAME_LOG_ERROR("State switch requested but switchState function is null.");
        }
    }

protected:
    AppContext* appContext = nullptr;
    SDL_Renderer* renderer = nullptr;
    AudioManager& audioManager = AudioManager::getInstance();
};

#endif