#pragma once
#include <citro2d.h>

extern C3D_RenderTarget *top;
extern C3D_RenderTarget *bottom;

extern int selectedShockerIndex;
extern int intensity;
extern int duration;
extern void saveUser();
extern void resetSave();
extern void printStatus();
extern void action(const char* action);