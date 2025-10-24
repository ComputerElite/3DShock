#include <citro2d.h>

#include <curl/curl.h>
#include <sys/iosupport.h>
#include <malloc.h>

#include <3ds.h>
#include <arpa/inet.h>

#include "network.hpp"
#include "json.hpp"
#include "ui/main_screen.h"
#include "ui/settings_screen.h"
#include "ui/ui_screen.h"

#define SCREEN_WIDTH  400
#define SCREEN_HEIGHT 240

static u32 *soc_buffer = nullptr;
using json = nlohmann::json;

float lastX = -1, lastY = -1;
C3D_RenderTarget *top;
C3D_RenderTarget *bottom;

int selectedShockerIndex = 0;
std::list<Shocker> shockers;
int intensity = 25;
int duration = 300;

void cleanup_services() {
    if (soc_buffer != nullptr) {
        free(soc_buffer);
        soc_buffer = nullptr;
    }
}

Result init_services() {
    Result res = 0;

    soc_buffer = static_cast<u32 *>(memalign(0x1000, 0x100000));
    if (soc_buffer != nullptr) {
        Handle tempAM = 0;
        if (R_SUCCEEDED(res = srvGetServiceHandle(&tempAM, "am:net"))) {
            svcCloseHandle(tempAM);
            socInit(soc_buffer, 0x100000);
        }
    }

    if (R_FAILED(res)) {
        cleanup_services();
    }

    return res;
}

#define USER_FILE "OpenShockUser.json"
#define NUM_THREADS 3
#define STACKSIZE (16 * 1024)

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

void resetSave() {
    if (remove(USER_FILE) != 0) {
        printf("Error deleting file\n");
    } else {
        printf("File successfully deleted\n");
    }
}

User readUser() {
    FILE *file = fopen(USER_FILE, "rb");
    User readUser{nullptr, nullptr, false};
    if (file == nullptr) {
        return readUser;
    }

    // seek to end of file
    fseek(file, 0,SEEK_END);

    // file pointer tells us the size
    off_t size = ftell(file);

    // seek back to start
    fseek(file, 0,SEEK_SET);

    // read the file
    char *buffer = (char *) malloc(size + 1);
    fread(buffer, sizeof(char), size, file);
    fclose(file);
    buffer[size] = '\0';
    json userJson = json::parse(buffer);
    free(buffer);
    if (userJson["token"] != nullptr)
        readUser.token = strdup(userJson["token"].get<std::string>().c_str());
    if (userJson["server"] != nullptr)
        readUser.server = strdup(userJson["server"].get<std::string>().c_str());
    if (userJson["loggedIn"] != nullptr)
        readUser.loggedIn = userJson["loggedIn"].get<bool>();
    printf("User read\n");
    return readUser;
}

bool runThreads = true;
s32 prio = 0;
struct Action {
    int intensity;
    int duration;
    const char *action;
    Shocker shocker;
};

void sendActionThread(void *action) {
    //SslCurlWrapper wrapper;
    auto actionData = static_cast<Action *>(action);
    sendAction(actionData->action, actionData->intensity, actionData->duration, actionData->shocker);
}


void action(const char* action) {
    printf("Sending action: %s\n", action);
    selectedShockerIndex = selectedShockerIndex % shockers.size();
    auto selectedShocker = shockers.begin();
    std::advance(selectedShocker, selectedShockerIndex);
    auto actionData = new Action();
    actionData->intensity = intensity;
    actionData->duration = duration;
    actionData->action = action;
    actionData->shocker = *selectedShocker;
    threadCreate(sendActionThread, actionData, STACKSIZE, prio-1, 0, false);
    usleep(100);
}

void printStatus() {
    consoleClear();
    if (!user.loggedIn) {
        printf("You are not logged in yet. You'll now be walked through the login");
        printf("Please create an api token in ShockAlarm. Once you have the QR code go to settings and press login.\n");
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
    //printf("\x1b[10;1HZR - Switch user\n\x1b[K");
}

bool touched = false;

//---------------------------------------------------------------------------------
int main() {
    init_services();
    // Init libs
    PrintConsole topConsole;

    gfxInitDefault();
    consoleInit(GFX_TOP, &topConsole);

    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    // consoleInit(GFX_TOP, nullptr);

    user = readUser();

    // Create colors
    const u32 clrRed = C2D_Color32(0xFF, 0x00, 0x00, 0xFF);

    if (user.loggedIn)
        shockers = getShockers();
    printStatus();

    mainScreen = new MainScreen();
    settingsScreen = new SettingsScreen();
    activeUiScreen = mainScreen;

    // Create screens
    top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    curl_global_init(CURL_GLOBAL_ALL);

    constexpr u32 clrClear = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
    svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

    // Main loop
    while (aptMainLoop()) {
        hidScanInput();
        touchPosition touch;

        u32 kDown = hidKeysDown();

        hidTouchRead(&touch);

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
        if (kDown & KEY_B) {
            action("Shock");
            printStatus();
        }

        // Render the scene

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);


        // todo: maybe replace top console with this
        // C2D_TargetClear(top, clrClear);
        // C2D_SceneBegin(top);
        // activeUiScreen->drawTopScreen();

        // C2D_DrawRectangle(50, 0, 0, 50, 50, clrRec1, clrRec2, clrRec3, clrRec4);


        C2D_TargetClear(bottom, clrClear);
        C2D_SceneBegin(bottom);
        activeUiScreen->drawBottomScreen();
        // C2D_DrawRectangle(50, 0, 0, 50, 50, clrRec1, clrRec2, clrRec3, clrRec4);

        if (touch.px != 0 && touch.py != 0) {
            if (lastX != -1 && lastY != -1) {
                C2D_DrawLine(lastX, lastY, clrRed, touch.px, touch.py, clrRed, 3, 0);
            }

            lastX = touch.px;
            lastY = touch.py;
        } else {
            touched = false;
            lastX = -1;
            lastY = -1;
        }

        // touchdown is true if last frame was not touched and this frame is touched
        const bool touchdown = !touched && touch.px != 0 && touch.py != 0;

        activeUiScreen->updateScreen(touch, touchdown);

        if (touchdown) {
            touched = true;
        }


        C3D_FrameEnd(0);

        if (kDown & KEY_START)
            break;
    }


    runThreads = false;

    curl_global_cleanup();

    //cleanup_services();

    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
