#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H
#include "ui_screen.h"


class MainScreen : public UIScreen {
public:
    MainScreen();

    void drawTopScreen() override;
    void drawBottomScreen() override;

    void updateScreen(touchPosition touchPosition, bool touchdown) override;
};


#endif //MAIN_SCREEN_H
