#ifndef UI_SCREEN_H
#define UI_SCREEN_H
#include "ui_components.h"
#include <vector>

constexpr float SCREEN_WIDTH_BOTTOM = 320;
constexpr float SCREEN_HEIGHT_BOTTOM = 240;


class UIScreen {
    std::vector<BaseComponent *> topComponents;
    std::vector<BaseComponent *> bottomComponents;

protected:
    void addTopComponent(BaseComponent *component);
    void addBottomComponent(BaseComponent *component);

public:
    virtual ~UIScreen();

    virtual void drawTopScreen();

    virtual void drawBottomScreen();

    virtual void updateScreen(touchPosition touchPosition, bool touchdown);
};

extern UIScreen *activeUiScreen;
extern UIScreen *mainScreen;
extern UIScreen *settingsScreen;

#endif //UI_SCREEN_H
