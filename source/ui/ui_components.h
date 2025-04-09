#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H
#include <3ds.h>
#include <citro2d.h>
#include <functional>


class BaseComponent {
protected:
    float x, y, width, height;
    bool active = false;

public:
    virtual ~BaseComponent() = default;

    float getX() const { return x; }
    float getY() const { return y; }
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    BaseComponent(const float x, const float y, const float width, const float height)
        : x(x),
          y(y),
          width(width),
          height(height) {
    }

    bool isTouched(touchPosition touchPosition) const;

    virtual void draw();

    virtual void update(touchPosition touchPosition, bool touchdown);
};


class Button final : public BaseComponent {
    typedef void (*ButtonPressedCallback)();

    const char *label;
    C2D_TextBuf labelTextBuf;
    C2D_Text labelText{};
    float textWidth = 0, textHeight = 0;
    ButtonPressedCallback buttonPressedCallback;

public:
    Button(float x, float y, float width, float height, const char *label,
           ButtonPressedCallback buttonPressedCallback = nullptr);

    void draw() override;

    void update(touchPosition touchPosition, bool touchdown) override;
};

class Slider final : public BaseComponent {
    typedef void (*SliderUpdateCallback)(float value);

    const char *label;
    C2D_TextBuf labelTextBuf;
    C2D_Text labelText{};
    float textWidth = 0, textHeight = 0;
    bool dragging = false;
    SliderUpdateCallback valueUpdateCallback;

public:
    float value, min, max;

    Slider(float x, float y, float width, float height, const char *label, float value = 50.0f, float min = 0.0f,
           float max = 100.0f, SliderUpdateCallback valueUpdateCallback = nullptr);

    void draw() override;

    void update(touchPosition touchPosition, bool touchdown) override;
};

class Checkbox final : public BaseComponent {
    typedef void (*CheckboxUpdateEvent)(bool checked);

    const char *label;
    C2D_TextBuf labelTextBuf;
    C2D_Text labelText{};
    float textWidth = 0, textHeight = 0;
    CheckboxUpdateEvent checkboxUpdateEvent;
    bool checked = false;

public:
    Checkbox(float x, float y, float width, float height, const char *label, bool checked = false,
             CheckboxUpdateEvent checkboxUpdateEvent = nullptr);

    void draw() override;

    void update(touchPosition touchPosition, bool touchdown) override;
};


#endif //UI_COMPONENTS_H
