#include "ui_components.h"

#include <utility>
#include "../network.hpp"

u32 clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
u32 clrGreen = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
u32 clrBlue = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);

bool BaseComponent::isTouched(const touchPosition touch_position) const {
    return static_cast<float>(touch_position.px) >= x && static_cast<float>(touch_position.py) >= y && static_cast<
               float>(touch_position.px) <= x + width &&
           static_cast<float>(touch_position.py) <= y
           + height;
}

Button::Button(const float x, const float y, const float width, const float height, const char *label,
               const ButtonPressedCallback buttonPressedCallback):
    BaseComponent(x, y, width, height) {
    this->label = label;
    this->labelTextBuf = C2D_TextBufNew(strlen(label) + 1);
    this->buttonPressedCallback = buttonPressedCallback;
    C2D_TextParse(&this->labelText, this->labelTextBuf, label);
    C2D_TextOptimize(&this->labelText);
    C2D_TextGetDimensions(&this->labelText, 0.75f, 0.75f, &this->textWidth, &this->textHeight);
}

void Button::draw() { // todo: add hover effect
    C2D_DrawRectangle(x, y, 0.0f, width, height, clrGreen, clrRed, clrBlue, clrWhite);
    C2D_DrawText(&labelText, 0, x + width / 2.0f - textWidth / 2.0f, y + height / 2.0f - textHeight / 2.0f, 0.0f, 0.75f,
                 0.75f);
}

void Button::update(const touchPosition touchPosition, const bool touchdown) {
    if (isTouched(touchPosition) && touchdown) {
        if (buttonPressedCallback) {
            buttonPressedCallback();
        }
    }
}


Slider::Slider(const float x, const float y, const float width, const float height, const char *label,
               const float value, const float min,
               const float max, const SliderUpdateCallback valueUpdateCallback):
    BaseComponent(x, y, width, height) {
    this->value = value;
    this->min = min;
    this->max = max;
    this->label = label;
    this->labelTextBuf = C2D_TextBufNew(strlen(label) + 1);
    this->valueUpdateCallback = valueUpdateCallback;
    C2D_TextParse(&this->labelText, this->labelTextBuf, label);
    C2D_TextOptimize(&this->labelText);
    C2D_TextGetDimensions(&this->labelText, 0.75f, 0.75f, &this->textWidth, &this->textHeight);
}

void Slider::draw() {
    C2D_DrawRectangle(x, y + height / 4.0f, 0.0f, width, height / 2.0f, clrGreen, clrRed, clrBlue, clrWhite);

    const float knobX = x + (value - min) / (max - min) * width;

    C2D_DrawRectangle(x, y + height / 4.0f, 0.0f, knobX - x, height / 2.0f, clrBlue, clrBlue, clrBlue, clrBlue);

    C2D_DrawCircleSolid(knobX, y + height / 2.0f, 0.0f, height / 2.5f, clrWhite);

    C2D_DrawText(&labelText, 0, x + width / 2.0f - textWidth / 2.0f, y + height / 2.0f - textHeight / 2.0f, 0.0f, 0.75f, 0.75f);
}

void Slider::update(const touchPosition touchPosition, bool touchdown) {
    if (isTouched(touchPosition) && touchdown) {
        dragging = true;
    }
    if (touchPosition.px == 0 && touchPosition.py == 0) {
        dragging = false;
    }

    if (dragging) {
        float newValue = min + (static_cast<float>(touchPosition.px) - x) / width * (max - min);
        newValue = fclamp(newValue, min, max);
        if (newValue != value) {
            value = newValue;
            if (valueUpdateCallback) {
                valueUpdateCallback(value);
            }
        }
    }
}

Checkbox::Checkbox(const float x, const float y, const float width, const float height, const char *label,
                   const bool checked, const CheckboxUpdateEvent checkboxUpdateEvent):
    BaseComponent(x, y, width, height) {
    this->checked = checked;
    this->checkboxUpdateEvent = checkboxUpdateEvent;

    this->label = label;
    this->labelTextBuf = C2D_TextBufNew(strlen(label) + 1);
    C2D_TextParse(&this->labelText, this->labelTextBuf, label);
    C2D_TextOptimize(&this->labelText);
    C2D_TextGetDimensions(&this->labelText, 0.75f, 0.75f, &this->textWidth, &this->textHeight);
}

void Checkbox::draw() {
    const float checkboxSize = height * 0.8f;
    const float innerCheckboxSize = checkboxSize * 0.8f;
    C2D_DrawRectangle(x, y, 0.0f, checkboxSize, checkboxSize, clrRed, clrRed, clrRed, clrRed);
    if (checked) {
        C2D_DrawRectangle(x + (checkboxSize - innerCheckboxSize) / 2.0f, y + (checkboxSize - innerCheckboxSize) / 2.0f,
                          0.0f, innerCheckboxSize, innerCheckboxSize, clrGreen, clrGreen, clrGreen, clrGreen);
    }

    // Draw the label text on the right of the checkbox
    const float textX = x + checkboxSize + 5.0f; // Add some padding between the checkbox and the text
    C2D_DrawText(&labelText, 0, textX, y + height / 2.0f - textHeight / 2.0f, 0.0f, 0.75f, 0.75f);
}

void Checkbox::update(const touchPosition touchPosition, bool touchdown) {
    if (isTouched(touchPosition) && touchdown) {
        checked = !checked;
        if (checkboxUpdateEvent) {
            checkboxUpdateEvent(checked);
        }
    }
}
