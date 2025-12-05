#include "objects/TextObject.h"
#include <algorithm>
#include <sstream>
#include <vector>

TextObject::TextObject(TextRenderer* renderer, int fontSize)
    : renderer_(renderer)
    , fontSize_(fontSize)
    , scale_(1.0f)
    , color_(1.0f, 1.0f, 1.0f, 1.0f)
    , alignmentX_(ALIGN_LEFT)
    , alignmentY_(ALIGN_TOP)
    , textAlignment_(TEXT_ALIGN_LEFT)
    , anchorX_(0.0f)
    , anchorY_(0.0f)
    , renderX_(0.0f)
    , renderY_(0.0f)
    , textGap_(0.0f)
    , cachedWidth_(0.0f)
    , cachedHeight_(0.0f)
{
}

TextObject::~TextObject() {
}

void TextObject::setText(const std::string& newText) {
    if (text_ != newText) {
        text_ = newText;
        updateDimensions();
        updatePosition();
    }
}

void TextObject::setPosition(int x, int y) {
    anchorX_ = (float)x;
    anchorY_ = (float)y;
    updatePosition();
}

void TextObject::setColor(glm::vec4 color) {
    color_ = color;
}

void TextObject::setAlignment(TextAlignment alignment) {
    if (textAlignment_ != alignment) {
        textAlignment_ = alignment;
        updateDimensions();
        updatePosition();
    }
}

void TextObject::setXAlignment(TextXAlignment alignment) {
    if (alignmentX_ != alignment) {
        alignmentX_ = alignment;
        updatePosition();
    }
}

void TextObject::setYAlignment(TextYAlignment alignment) {
    if (alignmentY_ != alignment) {
        alignmentY_ = alignment;
        updatePosition();
    }
}

void TextObject::setTextGap(float gap) {
    if (textGap_ != gap) {
        textGap_ = gap;
        updateDimensions();
        updatePosition();
    }
}

float TextObject::getLineCount() const {
    if (text_.empty()) {
        return 0.0f;
    }
    return static_cast<float>(std::count(text_.begin(), text_.end(), '\n') + 1);
}

void TextObject::updateDimensions() {
    if (text_.empty() || !renderer_) {
        cachedWidth_ = 0.0f;
        cachedHeight_ = 0.0f;
        return;
    }

    std::vector<std::string> lines;
    std::stringstream ss(text_);
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    
    if (!text_.empty() && text_.back() == '\n') {
        lines.push_back("");
    }

    float maxWidth = 0.0f;
    for (const std::string& l : lines) {
        if (!l.empty()) {
            glm::vec2 size = renderer_->measureText(l, scale_);
            if (size.x > maxWidth) {
                maxWidth = size.x;
            }
        }
    }

    float lineHeight = renderer_->getLineHeight(scale_);
    float totalHeight = lineHeight * lines.size() + textGap_ * (lines.size() - 1);

    cachedWidth_ = maxWidth;
    cachedHeight_ = totalHeight;
}

void TextObject::updatePosition() {
    switch (alignmentX_) {
        case ALIGN_LEFT:
            renderX_ = anchorX_;
            break;
        case ALIGN_CENTER:
            renderX_ = anchorX_ - (cachedWidth_ / 2.0f);
            break;
        case ALIGN_RIGHT:
            renderX_ = anchorX_ - cachedWidth_;
            break;
    }

    switch (alignmentY_) {
        case ALIGN_TOP:
            renderY_ = anchorY_;
            break;
        case ALIGN_MIDDLE:
            renderY_ = anchorY_ - (cachedHeight_ / 2.0f);
            break;
        case ALIGN_BOTTOM:
            renderY_ = anchorY_ - cachedHeight_;
            break;
    }
}

std::string TextObject::alignTextLines(const std::string& text) {
    if (textAlignment_ == TEXT_ALIGN_LEFT) {
        return text;
    }

    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    
    std::string result;
    for (size_t i = 0; i < lines.size(); ++i) {
        result += lines[i];
        if (i < lines.size() - 1) {
            result += '\n';
        }
    }

    return result;
}

void TextObject::render(float screenWidth, float screenHeight) {
    if (text_.empty() || !renderer_) {
        return;
    }

    std::vector<std::string> lines;
    std::stringstream ss(text_);
    std::string line;
    
    while (std::getline(ss, line, '\n')) {
        lines.push_back(line);
    }
    
    if (!text_.empty() && text_.back() == '\n') {
        lines.push_back("");
    }

    float lineHeight = renderer_->getLineHeight(scale_);
    float currentY = renderY_;
    float maxLineWidth = cachedWidth_;

    for (const std::string& l : lines) {
        float lineX = renderX_;

        if (!l.empty()) {
            if (textAlignment_ == TEXT_ALIGN_CENTER) {
                glm::vec2 lineSize = renderer_->measureText(l, scale_);
                lineX = renderX_ + (maxLineWidth - lineSize.x) / 2.0f;
            } else if (textAlignment_ == TEXT_ALIGN_RIGHT) {
                glm::vec2 lineSize = renderer_->measureText(l, scale_);
                lineX = renderX_ + (maxLineWidth - lineSize.x);
            }

            renderer_->renderText(l, lineX, currentY, scale_, color_, screenWidth, screenHeight);
        }

        currentY += lineHeight + textGap_;
    }
}