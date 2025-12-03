#ifndef PLAY_STATE_H
#define PLAY_STATE_H

#include <BaseState.h>

class PlayState : public BaseState
{
public:
    void init(AppContext* appContext, void* payload) override;
    void destroy() override;
private:
    
};

#endif