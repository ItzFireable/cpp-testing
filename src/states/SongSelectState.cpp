#include <states/SongSelectState.h>
#include <rhythm/ChartManager.h>
#include <rhythm/DifficultyCalculator.h>
#include <utils/Utils.h>
#include "utils/Logger.h"
#include <utils/AudioManager.h>
#include <objects/TextObject.h>
#include <SDL3/SDL.h>
#include <iostream>
#include <iomanip>
#include <sstream>

static const std::string fontPath = "assets/fonts/GoogleSansCode.ttf";
static const float TITLE_X_POS = 16.0f;
static const float SCROLL_SPEED_PX_S = 200.0f;
static const float CROSSFADE_DURATION = 1.5f;

void SongSelectState::playAudioFile(std::string filePath, float startTime)
{
    if (filePath.empty()) {
        return;
    }

    GAME_LOG_INFO("Playing audio file: " + filePath + " from start time: " + std::to_string(startTime));
    
    if (!AudioManager::getInstance().switchMusicStream(filePath, CROSSFADE_DURATION, startTime)) {
        GAME_LOG_ERROR("Failed to switch music stream to: " + filePath);
        
        if (!AudioManager::getInstance().switchMusicStream(filePath, 0.0f, startTime)) {
            GAME_LOG_ERROR("Instant switch also failed!");
        }
    }
}

float SongSelectState::getAudioStartPos(const ChartData& chartData)
{
    auto it = chartData.metadata.find("previewTime");
    if (it != chartData.metadata.end() && !it->second.empty()) {
        std::string startPosStr = it->second;
        try {
            float startPos = std::stof(startPosStr);
            if (startPos >= 0.0f) {
                return startPos;
            } else {
                GAME_LOG_ERROR("Invalid start position in metadata: " + startPosStr);
            }
        } catch (const std::invalid_argument& e) {
            GAME_LOG_ERROR("Invalid start position format: " + std::string(e.what()));
        } catch (const std::out_of_range& e) {
            GAME_LOG_ERROR("Start position out of range: " + std::string(e.what()));
        }
    }
    
    return 0.0f;
}

std::string SongSelectState::getAudioPath(const ChartData& chartData)
{
    auto it = chartData.metadata.find("audio");
    if (it != chartData.metadata.end() && !it->second.empty()) {
        std::string audioPath = it->second;

        if (audioPath[0] != '/' && audioPath.find("assets/") != 0) {
            std::string chartDir = chartData.filename;
            size_t lastSlash = chartDir.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                chartDir = chartDir.substr(0, lastSlash + 1);
                audioPath = chartDir + audioPath;
            }
        }
        
        return audioPath;
    }
    
    std::string chartFile = "audio.mp3";
    std::string filePath = chartData.filePath;
    size_t lastSlash = filePath.find_last_of("/\\");

    if (lastSlash != std::string::npos) {
        std::string mp3Path = filePath + "/" + chartFile;
        return mp3Path;
    }
    
    return "";
}

std::string SongSelectState::getChartTitle(ChartData chartData) {
    auto it = chartData.metadata.find("title");
    if (it != chartData.metadata.end() && !it->second.empty()) {
        return it->second;
    }
    
    std::string filename = chartData.filename;
    size_t lastSlash = filename.find_last_of("/\\");
    std::string base = (lastSlash == std::string::npos) ? filename : filename.substr(lastSlash + 1);
    
    size_t lastDot = base.find_last_of('.');
    return (lastDot == std::string::npos) ? base : base.substr(0, lastDot);
}

void SongSelectState::updateChartPositions()
{
    if (this->chartTitles_.empty()) return;
    float baseListY = this->listCenterY_ + this->listYOffset_;

    for (int i = 0; i < this->chartTitles_.size(); ++i)
    {
        TextObject* currentTitle = this->chartTitles_[i];
        
        float targetY = baseListY + ((float)i * (float)this->lineSkip_);

        currentTitle->setAlignment(TEXT_ALIGN_RIGHT);
        currentTitle->setXAlignment(ALIGN_RIGHT);
        currentTitle->setYAlignment(ALIGN_MIDDLE);
        currentTitle->setPosition(screenWidth_ - TITLE_X_POS, targetY);
    }
}

void updateDifficultyText(TextObject* textObject, const FinalResult& result)
{
    if (!textObject) return;

    std::stringstream ss;

    ss << std::fixed;
    ss << std::setprecision(3);
    
    ss << "Raw Diff (MSD): " << result.rawDiff << " (" << result.playbackRate << "x)\n";
    ss << "Total Rice Skill: " << result.riceTotal << "\n";
    ss << "Total LN Skill: " << result.lnTotal << "\n";

    ss << "\n+ Skill Breakdown (Rice)\n";
    ss << "Stream: " << result.skills.stream << "\n";
    ss << "Jumpstream: " << result.skills.jumpstream << "\n";
    ss << "Handstream: " << result.skills.handstream << "\n";
    ss << "Jack: " << result.skills.jack << "\n";
    ss << "Chordjack: " << result.skills.chordjack << "\n";
    ss << "Technical: " << result.skills.technical << "\n";
    ss << "Stamina: " << result.skills.stamina << "\n";

    ss << "\n+ Skill Breakdown (Long Notes)\n";
    ss << "LN Density: " << result.skills.density << "\n";
    ss << "LN Speed: " << result.skills.speed << "\n";
    ss << "LN Shields: " << result.skills.shields << "\n";
    ss << "LN Complexity: " << result.skills.complexity;

    textObject->setText(ss.str());
}

void SongSelectState::updateDifficultyDisplay(ChartData chartData)
{
    FinalResult result;
    const std::string& key = chartData.filename + "@" + std::to_string(this->selectedRate);

    if (this->difficultyCache_.count(key)) {
        result = this->difficultyCache_[key];
    } else {
        result = this->calculator.calculate(chartData, this->selectedRate);
        this->difficultyCache_[key] = result;
    }

    updateDifficultyText(this->diffTextObject_, result);
}

void SongSelectState::init(AppContext* appContext, void* payload)
{
    BaseState::init(appContext);
    this->calculator = DifficultyCalculator();

    int screenWidth, screenHeight;
    SDL_GetCurrentRenderOutputSize(this->renderer, &screenWidth, &screenHeight);
    this->screenWidth_ = screenWidth;
    this->screenHeight_ = screenHeight;
    this->listCenterY_ = screenHeight / 2;

    this->listYOffset_ = 0.0f;
    this->targetYOffset_ = 0.0f; 
    this->lastUpdateTick_ = SDL_GetPerformanceCounter();

    std::vector<std::string> chartList = Utils::getChartList();
    
    for (const auto& chartPath : chartList)
    {
        std::string chartFile = Utils::getChartFile(chartPath);

        std::string fileContent = Utils::readFile(chartFile);
        ChartData chartData = ChartManager::parseChart(chartFile, fileContent);

        chartData.filePath = chartPath;
        this->charts_.push_back(chartData);

        TextObject* titleText = new TextObject(this->renderer, fontPath, 20);
        titleText->setText(getChartTitle(chartData));
        this->chartTitles_.push_back(titleText);

        GAME_LOG_INFO("Loaded chart data from: " + chartFile);
    }
    
    this->diffTextObject_ = new TextObject(this->renderer, fontPath, 16);
    this->diffTextObject_->setAlignment(TEXT_ALIGN_LEFT);
    this->diffTextObject_->setXAlignment(ALIGN_LEFT);
    this->diffTextObject_->setYAlignment(ALIGN_MIDDLE);
    this->diffTextObject_->setPosition(16.0f, listCenterY_);
    this->diffTextObject_->setColor({255, 255, 255, 255});
    
    if (!this->charts_.empty()) {
        this->selectedIndex_ = 0;

        this->targetYOffset_ = - (float)this->selectedIndex_ * (float)this->lineSkip_;
        this->listYOffset_ = this->targetYOffset_;

        this->updateChartPositions();
        this->updateDifficultyDisplay(this->charts_[this->selectedIndex_]);
        
        std::string audioPath = getAudioPath(this->charts_[this->selectedIndex_]);
        float startPos = getAudioStartPos(this->charts_[this->selectedIndex_]);
        if (!audioPath.empty()) {
            playAudioFile(audioPath, startPos);
        }
    } else {
        this->diffTextObject_->setText("No charts found.");
    }
}

void SongSelectState::handleEvent(const SDL_Event& e)
{
    if (e.type == SDL_EVENT_KEY_DOWN)
    {
        const SDL_KeyboardEvent& keyEvent = e.key;
        int oldIndex = this->selectedIndex_;
        float oldRate = this->selectedRate;
        
        if (keyEvent.key == SDLK_DOWN)
        {
            this->selectedIndex_ = (this->selectedIndex_ + 1) % this->charts_.size();
        } 
        else if (keyEvent.key == SDLK_UP)
        {
            this->selectedIndex_ = (this->selectedIndex_ - 1 + this->charts_.size()) % this->charts_.size();
        }
        else if (keyEvent.key == SDLK_PAGEDOWN)
        {
            this->selectedIndex_ += 5;
            if (this->selectedIndex_ >= this->charts_.size()) {
                this->selectedIndex_ = this->charts_.size() - 1;
            }
        }
        else if (keyEvent.key == SDLK_PAGEUP)
        {
            this->selectedIndex_ -= 5;
            if (this->selectedIndex_ < 0) {
                this->selectedIndex_ = 0;
            }
        }
        else if (keyEvent.key == SDLK_LEFT)
        {
            this->selectedRate -= 0.1f;
            if (this->selectedRate < 0.1f) this->selectedRate = 0.1f;

            AudioManager::getInstance().setMusicPlaybackRate(this->selectedRate);
            GAME_LOG_INFO("Selected rate: " + std::to_string(this->selectedRate) + "x");
        }
        else if (keyEvent.key == SDLK_RIGHT)
        {
            this->selectedRate += 0.1f;
            if (this->selectedRate > 3.0f) this->selectedRate = 3.0f;

            AudioManager::getInstance().setMusicPlaybackRate(this->selectedRate);
            GAME_LOG_INFO("Selected rate: " + std::to_string(this->selectedRate) + "x");
        }
        else if (keyEvent.key == SDLK_RETURN)
        {
            PlayStateData* data = new PlayStateData();
            ChartData chart = this->charts_[this->selectedIndex_];

            data->chartFile = chart.filename;
            data->songPath = chart.filePath;
            data->playbackRate = this->selectedRate;

            requestStateSwitch(STATE_PLAY, data);
            return;
        }

        if (this->selectedIndex_ != oldIndex) {
            this->targetYOffset_ = - (float)this->selectedIndex_ * (float)this->lineSkip_;
            this->updateDifficultyDisplay(this->charts_[this->selectedIndex_]);
            
            std::string audioPath = getAudioPath(this->charts_[this->selectedIndex_]);
            float startPos = getAudioStartPos(this->charts_[this->selectedIndex_]);
            if (!audioPath.empty()) {
                playAudioFile(audioPath, startPos);
            }
        }

        if (this->selectedRate != oldRate) {
            this->updateDifficultyDisplay(this->charts_[this->selectedIndex_]);
        }
    }
}

void SongSelectState::update()
{
    Uint64 currentTick = SDL_GetPerformanceCounter();
    float deltaTime = (float)(currentTick - this->lastUpdateTick_) / (float)SDL_GetPerformanceFrequency(); 
    this->lastUpdateTick_ = currentTick;
    
    float distance = this->targetYOffset_ - this->listYOffset_;

    if (std::abs(distance) < 1.0f) 
    {
        this->listYOffset_ = this->targetYOffset_;
    } 
    else 
    {
        float moveAmount = SCROLL_SPEED_PX_S * deltaTime;
        
        if (distance > 0) {
            this->listYOffset_ += std::min(moveAmount, distance);
        } else {
            this->listYOffset_ -= std::min(moveAmount, -distance); 
        }
    }
    
    this->updateChartPositions();
    for (int i = 0; i < this->chartTitles_.size(); ++i)
    {
        if (i == this->selectedIndex_) {
            this->chartTitles_[i]->setColor({255, 255, 0, 255});
        } else {
            this->chartTitles_[i]->setColor({255, 255, 255, 255});
        }
        this->chartTitles_[i]->render();
    }
    
    this->diffTextObject_->render();
}

void SongSelectState::destroy()
{
    GAME_LOG_INFO("Destroying SongSelectState");
    AudioManager::getInstance().stopMusicStream();
    
    for (TextObject* obj : this->chartTitles_)
    {
        delete obj;
    }
    this->chartTitles_.clear();
    
    delete this->diffTextObject_;
    this->diffTextObject_ = nullptr;
    
    this->charts_.clear();
}