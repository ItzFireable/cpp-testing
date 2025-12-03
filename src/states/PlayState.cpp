#include <states/PlayState.h>

void PlayState::init(AppContext* appContext, void* payload)
{
    PlayStateData* data = static_cast<PlayStateData*>(payload);
    if (data) {
        GAME_LOG_INFO("Initializing PlayState with song: " + data->songPath + ", chart: " + data->chartFile + ", rate: " + std::to_string(data->playbackRate) + "x");
    }
}

void PlayState::destroy()
{
    
}