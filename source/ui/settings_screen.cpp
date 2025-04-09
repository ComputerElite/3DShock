#include "settings_screen.h"

#include <cstdlib>
#include "../camera.hpp"
#include "../network.hpp"
#include "../main.h"

static SwkbdState swkbd;
static SwkbdStatusData swkbdStatus;

SettingsScreen::SettingsScreen() {
    addBottomComponent(new Button(50, 20, SCREEN_WIDTH_BOTTOM - 50 - 50, 25, "Login with Token", []() {
        char *apiToken = init_qr();
        if (apiToken != nullptr) {
            user.token = strdup(apiToken);
            free(apiToken);
            printf(
                    "API token read. Next press API URL to enter your OpenShock server address WITH A TRAILING SLASH!!!\n");
        } else {
            printf("Failed to get API token.\n");
        }
    }));
    addBottomComponent(new Button(50, 50, SCREEN_WIDTH_BOTTOM - 50 - 50, 25, "API URL", []() {
        char *apiServerUrl = strdup("https://api.openshock.app/");
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 3, -1);
        swkbdSetInitialText(&swkbd, strdup("https://api.openshock.app/"));
        swkbdSetHintText(&swkbd, "OpenShock api url with trailing slash");
        static bool reload = false;
        swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
        reload = true;
        swkbdInputText(&swkbd, apiServerUrl, sizeof(apiServerUrl));
        printf("API server url: %s\n", apiServerUrl);
        user.server = strdup(apiServerUrl);
        free(apiServerUrl);
        printf("Press save if you added your token and api url\n");
    }));
    addBottomComponent(new Button(50, 80, SCREEN_WIDTH_BOTTOM - 50 - 50, 25, "Back", []() {
        activeUiScreen = mainScreen;
    }));
    addBottomComponent(new Button(50, 110, SCREEN_WIDTH_BOTTOM - 50 - 50, 25, "Save", []() {
        user.loggedIn = true;
        saveUser();
        printf("Reopen the app now\n");
    }));
    addBottomComponent(new Button(50, 140, SCREEN_WIDTH_BOTTOM - 50 - 50, 25, "Reset", []() {
        user.loggedIn = false;
        resetSave();
        printf("Reopen the app now\n");
    }));
}

void SettingsScreen::drawTopScreen() {
    UIScreen::drawTopScreen();
}

void SettingsScreen::drawBottomScreen() {
    UIScreen::drawBottomScreen();
}

void SettingsScreen::updateScreen(const touchPosition touchPosition, const bool touchdown) {
    UIScreen::updateScreen(touchPosition, touchdown);
}
