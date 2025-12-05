#ifndef TEXTOBJECT_H
#define TEXTOBJECT_H

#include <string>
#include <glm/glm.hpp>
#include "TextRenderer.h"

enum TextXAlignment {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT
};

enum TextYAlignment {
    ALIGN_TOP,
    ALIGN_MIDDLE,
    ALIGN_BOTTOM
};

enum TextAlignment {
    TEXT_ALIGN_LEFT,
    TEXT_ALIGN_CENTER,
    TEXT_ALIGN_RIGHT
};

class TextObject {
public:
    TextObject(TextRenderer* renderer, int fontSize);
    ~TextObject();

    void setText(const std::string& newText);
    void setPosition(int x, int y);
    void setColor(glm::vec4 color);
    void render(float screenWidth, float screenHeight);

    void getPosition(float& x, float& y) const {
        x = renderX_;
        y = renderY_;
    }

    float getTextGap() const { return textGap_; }
    float getLineCount() const;

    void setTextGap(float gap);
    void setAlignment(TextAlignment alignment);
    void setXAlignment(TextXAlignment alignment);
    void setYAlignment(TextYAlignment alignment);

    float getRenderedWidth() const { return cachedWidth_; }
    float getRenderedHeight() const { return cachedHeight_; }

private:
    TextRenderer* renderer_;
    int fontSize_;
    float scale_;

    std::string text_;
    glm::vec4 color_;

    TextXAlignment alignmentX_;
    TextYAlignment alignmentY_;
    TextAlignment textAlignment_;

    float anchorX_;
    float anchorY_;
    float renderX_;
    float renderY_;
    float textGap_;

    float cachedWidth_;
    float cachedHeight_;

    void updateDimensions();
    void updatePosition();
    std::string alignTextLines(const std::string& text);
};

#endif