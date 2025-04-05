// Simple citro2d untextured shape example
#include <citro2d.h>

#include <curl/curl.h>
#include <sys/iosupport.h>
#include <malloc.h>

#include <3ds.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "network.hpp"
#include "json.hpp"

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

static u32 *soc_buffer = nullptr;

using json = nlohmann::json;

float lastX = -1, lastY = -1;

void cleanup_services() {
    if(soc_buffer != nullptr) {
        free(soc_buffer);
        soc_buffer = nullptr;
    }
}

Result init_services() {
    Result res = 0;

    soc_buffer = static_cast<u32 *>(memalign(0x1000, 0x100000));
    if(soc_buffer != nullptr) {
        Handle tempAM = 0;
        if(R_SUCCEEDED(res = srvGetServiceHandle(&tempAM, "am:net"))) {
            svcCloseHandle(tempAM);
            socInit(soc_buffer, 0x100000);
        }
    }

    if(R_FAILED(res)) {
        cleanup_services();
    }

    return res;
}

int selectedShockerIndex = 0;
std::list<Shocker> shockers;
int intensity = 25;
int duration = 300;

int clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void printStatus() {
    if (shockers.empty()) {
        printf("\x1b[1;1HNo shockers found.\n\x1b[K");
    } else {
        selectedShockerIndex = selectedShockerIndex % shockers.size();
        auto selectedShocker = shockers.begin();

        std::advance(selectedShocker, selectedShockerIndex);
        printf("\x1b[1;1HSelected Shocker: %s (I%d, D%d)\n\x1b[K", selectedShocker->name, selectedShocker->limits.intensity, selectedShocker->limits.duration);
    }
    intensity = clamp(intensity, 0, 100);
    duration = clamp(duration, 300, 30000);
    printf("\x1b[2;1HIntensity: %d\n\x1b[K", intensity);
    printf("\x1b[3;1HDuration: %d\n\x1b[K", duration);
}

void action(const char* action) {
    // ToDo: POST request to server with action
}

//---------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    init_services();
    // Init libs

    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    consoleInit(GFX_TOP, nullptr);

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

    u32 clrTri1 = C2D_Color32(0xFF, 0x15, 0x00, 0xFF);
    u32 clrTri2 = C2D_Color32(0x27, 0x69, 0xE5, 0xFF);

    u32 clrClear = C2D_Color32(0xFF, 0xD8, 0xB0, 0x68);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    shockers = getShockers();
    consoleClear();


    // Main loop
    while (aptMainLoop()) {
        hidScanInput();
        touchPosition touch;

        // Respond to user input
        u32 kDown = hidKeysDown();
        // break in order to return to hbmenu

        hidTouchRead(&touch);

        // Render the scene

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        printStatus();

        if (kDown & KEY_B) {
            shockers = getShockers();
            printf("Memory: %d %d %d\n", mallinfo().arena, mallinfo().uordblks, mallinfo().fordblks);
        }

        if (kDown & KEY_X) {
            selectedShockerIndex++;
        }
        if (kDown & KEY_DDOWN) {
            intensity -= 5;
        }
        if (kDown & KEY_DUP) {
            intensity += 5;
        }
        if (kDown & KEY_DLEFT) {
            duration -= 200;
        }
        if (kDown & KEY_DRIGHT) {
            duration += 200;
        }
        if (kDown & KEY_A) {
            action("Vibrate");
        }

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

        if (kDown & KEY_START)
            break;
    }

    curl_global_cleanup();
    //cleanup_services();

    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
