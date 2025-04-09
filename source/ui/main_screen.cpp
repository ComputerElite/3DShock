//
// Created by user on 08.04.25.
//

#include "main_screen.h"

#include "../main.h"

MainScreen::MainScreen() {
    addBottomComponent(new Button(10, 5, 100, 25, "Settings", []() {
        activeUiScreen = settingsScreen;
    }));

    addBottomComponent(new Button(10, 50, SCREEN_WIDTH_BOTTOM / 2 - 5, 25, "Previous", []() {
        printf("Prev.\n");
        selectedShockerIndex--;
        printStatus();
    }));
    addBottomComponent(new Button(SCREEN_WIDTH_BOTTOM / 2 + 15, 50, SCREEN_WIDTH_BOTTOM / 2 - 15, 25, "Next", []() {
        printf("Next\n");
        selectedShockerIndex++;
        printStatus();
    }));

    addBottomComponent(new Slider(10, 80, SCREEN_WIDTH_BOTTOM - 20, 30, "Intensity", 20.0f, 0.0f, 100.0f,
                                  [](const float value) {
                                      printf("Intensity: %f\n", value);
                                      intensity = static_cast<int>(value);
                                      printStatus();
                                  }));

    addBottomComponent(new Slider(10, 115, SCREEN_WIDTH_BOTTOM - 20, 30, "Duration", 300.0f, 300.0f, 30000.0f,
                                  [](const float value) {
                                      printf("Duration: %f\n", value);
                                      duration = static_cast<int>(value);
                                      printStatus();
                                  }));

    addBottomComponent(new Button(10, 150, SCREEN_WIDTH_BOTTOM / 3 - 20, 25, "Shock", []() {
        printf("Shock triggered\n");
        action("Shock");
        printStatus();
    }));
    addBottomComponent(new Button(SCREEN_WIDTH_BOTTOM / 3 + 7.5, 150, SCREEN_WIDTH_BOTTOM / 3 - 20, 25, "Vibrate", []() {
        printf("Vibration triggered\n");
        action("Vibrate");
        printStatus();
    }));
    addBottomComponent(new Button(2 * SCREEN_WIDTH_BOTTOM / 3 + 5, 150, SCREEN_WIDTH_BOTTOM / 3 - 20, 25, "Beep", []() {
        printf("Beep triggered\n");
        action("Sound");
        printStatus();
    }));
}

void MainScreen::drawTopScreen() {
    UIScreen::drawTopScreen();
}

void MainScreen::drawBottomScreen() {
    UIScreen::drawBottomScreen();
}

void MainScreen::updateScreen(const touchPosition touchPosition, const bool touchdown) {
    UIScreen::updateScreen(touchPosition, touchdown);
}
