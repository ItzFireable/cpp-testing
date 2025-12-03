#ifndef SONG_SELECT_STATE_H
#define SONG_SELECT_STATE_H

#include <BaseState.h>
#include <rhythm/ChartManager.h>
#include <rhythm/DifficultyCalculator.h>
#include <objects/TextObject.h>
#include <SDL3/SDL.h>
#include <vector>
#include <string>
#include <map>

class SongSelectState : public BaseState
{
public:
    void init(AppContext* appContext, void* payload) override;
    void handleEvent(const SDL_Event& e) override;
    void update() override;
    void destroy() override;

private:
    std::vector<ChartData> charts_;
    std::vector<TextObject*> chartTitles_;
    int selectedIndex_ = 0;
    float selectedRate = 1.0f;
    
    TextObject* diffTextObject_ = nullptr;
    DifficultyCalculator calculator;
    std::map<std::string, FinalResult> difficultyCache_;
    
    int screenWidth_ = 0;
    int screenHeight_ = 0;
    float listCenterY_ = 0.0f;
    float listYOffset_ = 0.0f;
    float targetYOffset_ = 0.0f;
    int lineSkip_ = 32;
    Uint64 lastUpdateTick_ = 0;
    
    void playAudioFile(std::string filePath, float startTime = 0.0f);
    std::string getAudioPath(const ChartData& chartData);
    std::string getChartTitle(ChartData chartData);
    float getAudioStartPos(const ChartData& chartData);
    void updateChartPositions();
    void updateDifficultyDisplay(ChartData chartData);
};

#endif