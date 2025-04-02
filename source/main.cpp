// Simple citro2d untextured shape example
#include <citro2d.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <3ds.h>

#include "network.hpp"


#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

float lastX = -1, lastY = -1;

//---------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    //---------------------------------------------------------------------------------
    // Init libs
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    consoleInit(GFX_TOP, NULL);

    // Create screens
    //C3D_RenderTarget *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    // Create colors
    u32 clrWhite = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    u32 clrGreen = C2D_Color32(0x00, 0xFF, 0x00, 0xFF);
    u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);
    u32 clrBlue = C2D_Color32(0x00, 0x00, 0xFF, 0xFF);

    u32 clrCircle1 = C2D_Color32(0xFF, 0x00, 0xFF, 0xFF);
    u32 clrCircle2 = C2D_Color32(0xFF, 0xFF, 0x00, 0xFF);
    u32 clrCircle3 = C2D_Color32(0x00, 0xFF, 0xFF, 0xFF);

    u32 clrSolidCircle = C2D_Color32(0x68, 0xB0, 0xD8, 0xFF);

    u32 clrTri1 = C2D_Color32(0xFF, 0x15, 0x00, 0xFF);
    u32 clrTri2 = C2D_Color32(0x27, 0x69, 0xE5, 0xFF);

    u32 clrRec1 = C2D_Color32(0x9A, 0x6C, 0xB9, 0xFF);
    u32 clrRec2 = C2D_Color32(0xFF, 0xFF, 0x2C, 0xFF);
    u32 clrRec3 = C2D_Color32(0xD8, 0xF6, 0x0F, 0xFF);
    u32 clrRec4 = C2D_Color32(0x40, 0xEA, 0x87, 0xFF);

    u32 clrClear = C2D_Color32(0xFF, 0xD8, 0xB0, 0x68);

    // Main loop
    while (aptMainLoop()) {
        hidScanInput();
        touchPosition touch;

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu
        printf("\x1b[1;1HSimple citro2d shapes example");
        printf("\x1b[2;1HCPU:     %6.2f%%\x1b[K", C3D_GetProcessingTime() * 6.0f);
        printf("\x1b[3;1HGPU:     %6.2f%%\x1b[K", C3D_GetDrawingTime() * 6.0f);
        printf("\x1b[4;1HCmdBuf:  %6.2f%%\x1b[K", C3D_GetCmdBufUsage() * 100.0f);

        hidTouchRead(&touch);

        // Render the scene

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

/*
        C2D_TargetClear(top, clrClear);
        //C2D_SceneBegin(top);

        C2D_DrawTriangle(50 / 2, SCREEN_HEIGHT - 50, clrWhite,
                         0, SCREEN_HEIGHT, clrTri1,
                         50, SCREEN_HEIGHT, clrTri2, 0);
        C2D_DrawRectangle(SCREEN_WIDTH - 50, 0, 0, 50, 50, clrRec1, clrRec2, clrRec3, clrRec4);

        // Circles require a state change (an expensive operation) within citro2d's internals, so draw them last.
        // Although it is possible to draw them in the middle of drawing non-circular objects
        // (sprites, images, triangles, rectangles, etc.) this is not recommended. They should either
        // be drawn before all non-circular objects, or afterwards.
        C2D_DrawEllipse(0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, clrCircle1, clrCircle2, clrCircle3, clrWhite);
        C2D_DrawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 0, 50, clrCircle3, clrWhite, clrCircle1, clrCircle2);
        C2D_DrawCircle(25, 25, 0, 25,
                       clrRed, clrBlue, clrGreen, clrWhite);
        C2D_DrawCircleSolid(SCREEN_WIDTH - 25, SCREEN_HEIGHT - 25, 0, 25, clrSolidCircle);
        */

        if (kDown & KEY_A) {
            C2D_TargetClear(bottom, clrClear);
            C2D_SceneBegin(bottom);
            // Draw some shapes on the bottom screen
            C2D_DrawRectangle(10, 10, 0, 100, 50, clrGreen, clrRed, clrBlue, clrWhite); // A rectangle
            C2D_DrawCircle(200, 120, 0, 50, clrCircle1, clrCircle2, clrCircle3, clrWhite); // A circle
            C2D_DrawTriangle(50, 200, clrWhite, 100, 200, clrTri1, 75, 150, clrTri2, 0); // A triangle
        } else {
            C2D_SceneBegin(bottom);
        }

        if (touch.px != 0 && touch.py != 0) {
            if (lastX != -1 && lastY != -1) {
                C2D_DrawLine(lastX, lastY, clrRed, touch.px, touch.py, clrRed, 3, 0);
            }

            lastX = touch.px;
            lastY = touch.py;
        } else {
            lastX = -1;
            lastY = -1;
        }

        C3D_FrameEnd(0);
    }

    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
