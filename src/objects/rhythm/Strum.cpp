#include <objects/rhythm/Strum.h>
#include <rhythm/Playfield.h>

Strum::Strum(float x, float y, float width, float height, int column)
    : x_(x), y_(y), width_(width), height_(height), strumTexture_(nullptr), column_(column) {}

Strum::~Strum() {
    for (const auto& pair : ownedTextures_) {
        if (pair.second && pair.first) {
            SDL_DestroyTexture(pair.first);
        }
    }
}

void Strum::setRenderTexture(SDL_Texture* texture, bool ownedByStrum) {
    for (const auto& pair : ownedTextures_) {
        if (pair.second && pair.first) {
            SDL_DestroyTexture(pair.first);
        }
    }
    
    strumTexture_ = texture;
    ownedTextures_[texture] = ownedByStrum;
}

void Strum::loadTexture(SDL_Renderer* renderer) {
    for (const auto& pair : ownedTextures_) {
        if (pair.second && pair.first) {
            SDL_DestroyTexture(pair.first);
        }
    }

    Playfield* playfield = getPlayfield();
    if (!playfield) return;
    
    SkinUtils* skinUtils = playfield->getSkinUtils();
    if (!skinUtils) return;

    std::string filePath = skinUtils->getFilePathForSkinElement("notes/strum");
    std::string pressedFilePath = skinUtils->getFilePathForSkinElement("notes/strumPress");
    
    strumTexture_ = IMG_LoadTexture(renderer, filePath.c_str());
    ownedTextures_[strumTexture_] = true;
    if (!strumTexture_) {
        GAME_LOG_ERROR("Failed to load strum texture from " + filePath);
    }

    strumPressedTexture_ = IMG_LoadTexture(renderer, pressedFilePath.c_str());
    ownedTextures_[strumPressedTexture_] = true;

    if (!strumPressedTexture_) {
        GAME_LOG_ERROR("Failed to load strum pressed texture from " + pressedFilePath);
    }
}

void Strum::setPosition(float x, float y) {
    x_ = x;
    y_ = y;
}

void Strum::setSize(float width, float height) {
    width_ = width;
    height_ = height;
}

void Strum::setPadding(float top, float bottom, float left, float right) {
    padding_top_ = top;
    padding_bottom_ = bottom;
    padding_left_ = left;
    padding_right_ = right;
}

void Strum::update(float deltaTime) {}

void Strum::render(SDL_Renderer* renderer) {
    SDL_Texture* textureToRender = isPressed_ && strumPressedTexture_ ? strumPressedTexture_ : strumTexture_;

    SDL_FRect strumRect = {
        x_ + padding_left_,
        y_ + padding_top_,
        width_ - padding_left_ - padding_right_,
        height_ - padding_top_ - padding_bottom_
    };
    
    if (textureToRender) {
        SDL_RenderTexture(renderer, textureToRender, NULL, &strumRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderRect(renderer, &strumRect);
    }
}