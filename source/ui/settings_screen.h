#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "ui_screen.h"

class SettingsScreen : public UIScreen {
public:
    SettingsScreen();

    void drawTopScreen() override;
    void drawBottomScreen() override;

    void updateScreen(touchPosition touchPosition, bool touchdown) override;
};



#endif //SETTINGS_SCREEN_H
