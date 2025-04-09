#include "ui_screen.h"

UIScreen *activeUiScreen = nullptr;
UIScreen *mainScreen = nullptr;
UIScreen *settingsScreen = nullptr;

void UIScreen::addTopComponent(BaseComponent *component) {
    topComponents.push_back(component);
}

void UIScreen::addBottomComponent(BaseComponent *component) {
    bottomComponents.push_back(component);
}

UIScreen::~UIScreen() {
    for (const auto component : topComponents) {
        delete component;
    }
    for (const auto component : bottomComponents) {
        delete component;
    }
}

void UIScreen::drawTopScreen() {
    for (const auto &component : topComponents) {
        component->draw();
    }
}

void UIScreen::drawBottomScreen() {
    for (const auto &component : bottomComponents) {
        component->draw();
    }
}

void UIScreen::updateScreen(const touchPosition touchPosition, const bool touchdown) {
    for (const auto &component : topComponents) { // todo:? maybe create a input struct or something
        component->update({0,0}, false);
    }
    for (const auto &component : bottomComponents) {
        component->update(touchPosition, touchdown);
    }
}
