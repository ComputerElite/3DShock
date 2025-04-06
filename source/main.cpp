// Simple citro2d untextured shape example
#include <citro2d.h>

#include <curl/curl.h>
#include <sys/iosupport.h>
#include <malloc.h>

#include <3ds.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "camera.hpp"

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




void action(const char* action) {
    printf("Sending action: %s\n", action);
    selectedShockerIndex = selectedShockerIndex % shockers.size();
    auto selectedShocker = shockers.begin();
    std::advance(selectedShocker, selectedShockerIndex);
    sendAction(action, intensity, duration, *selectedShocker);
}
C3D_RenderTarget * top;
C3D_RenderTarget * bottom;

void waitForKey(int key) {
    while (!(hidKeysDown() & key)) {
        hidScanInput();
    }
}

#define USER_FILE "OpenShockUser.json"

void saveUser() {
    FILE *file = fopen(USER_FILE, "wb");
    if (file == NULL) {
        return;
    }

    json userJson;
    userJson["token"] = user.token;
    userJson["server"] = user.server;
    userJson["loggedIn"] = user.loggedIn;

    char *jsonString = strdup(userJson.dump().c_str());
    fwrite(jsonString, sizeof(char), strlen(jsonString), file);
    fclose(file);
    free(jsonString);
    printf("User saved\n");
}

User readUser() {
    FILE *file = fopen(USER_FILE, "rb");
    User readUser{nullptr, nullptr, false};
    if (file == nullptr) {
        return readUser;
    }

    // seek to end of file
    fseek(file,0,SEEK_END);

    // file pointer tells us the size
    off_t size = ftell(file);

    // seek back to start
    fseek(file,0,SEEK_SET);

    // read the file
    char *buffer = (char *)malloc(size + 1);
    fread(buffer, sizeof(char), size, file);
    fclose(file);
    buffer[size] = '\0';
    json userJson = json::parse(buffer);
    free(buffer);
    if (userJson["token"] != nullptr) readUser.token = strdup(userJson["token"].get<std::string>().c_str());
    if (userJson["server"] != nullptr) readUser.server = strdup(userJson["server"].get<std::string>().c_str());
    if (userJson["loggedIn"] != nullptr) readUser.loggedIn = userJson["loggedIn"].get<bool>();
    printf("User read\n");
    return readUser;
}

static SwkbdState swkbd;
static SwkbdStatusData swkbdStatus;
void updateUser() {
    consoleClear();
    printf("Please create an api token in ShockAlarm. Once you have the QR code press A.\n");
    waitForKey(KEY_A);
    printf("Now scan the QR code with your 3DS\n");
    char *apiToken = init_qr();
    if (apiToken != nullptr) {
        user.token = strdup(apiToken);
        free(apiToken);
    } else {
        printf("Failed to get API token.\n");
        return;
    }
    printf("API token read. Next enter your OpenShock server address WITH A TRAILING SLASH!!!\n");
    printf("Press A to continue\n");
    waitForKey(KEY_A);
    char* apiServerUrl = strdup("https://api.openshock.app/");
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
    user.loggedIn = true;
    saveUser();
}


void printStatus() {
    consoleClear();
    if (!user.loggedIn) {
        printf("You are not logged in yet. You'll now be walked through the login");
        updateUser();
        return;
    }
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
    printf("\x1b[3;1HDuration: %d\n\x1b[K", duration);;
    printf("\x1b[5;1HL/R select shocker\n\x1b[K");
    printf("\x1b[6;1HY - vibrate, X - sound, B - shock\n\x1b[K");
    printf("\x1b[7;1HSelect - reload shockers, Start - exit\n\x1b[K");
    printf("\x1b[8;1HDPad up/down - intensity\n\x1b[K");
    printf("\x1b[9;1HDPad left/right - Duration\n\x1b[K");
    printf("\x1b[10;1HZR - Switch user\n\x1b[K");
}

//---------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    init_services();
    PrintConsole topConsole;
    // Init libs

    gfxInitDefault();
    consoleInit(GFX_TOP, &topConsole);

    // Init citro2d
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    // load user from sd
    user = readUser();

    // Create screens
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);


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
    if (user.loggedIn) shockers = getShockers();
    printStatus();

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


        if (kDown & KEY_SELECT) {
            shockers = getShockers();
            printStatus();
        }

        if (kDown & KEY_R) {
            selectedShockerIndex++;
            printStatus();
        }
        if (kDown & KEY_L) {
            selectedShockerIndex--;
            printStatus();
        }
        if (kDown & KEY_DDOWN) {
            intensity -= 5;
            printStatus();
        }
        if (kDown & KEY_DUP) {
            intensity += 5;
            printStatus();
        }
        if (kDown & KEY_DLEFT) {
            duration -= 200;
            printStatus();
        }
        if (kDown & KEY_DRIGHT) {
            duration += 200;
            printStatus();
        }
        if (kDown & KEY_Y) {
            action("Vibrate");
            printStatus();
        }
        if (kDown & KEY_X) {
            action("Sound");
            printStatus();
        }
        if (kDown & KEY_ZR) {
            updateUser();
            shockers = getShockers();
            printStatus();
        }
        if (kDown & KEY_B) {
            action("Shock");
            printStatus();
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
