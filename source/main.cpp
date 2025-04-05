// Simple citro2d untextured shape example
#include <citro2d.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
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


typedef struct {
    u32 bufferSize;
    u64* contentLength;
    void* userData;
    Result (*callback)(void* userData, void* buffer, size_t size);

    void* buf;
    u32 pos;

    Result res;
} http_curl_data;
#define HTTP_CONTENT_LENGTH_HEADER "Content-Length"
#define HTTP_TIMEOUT_SEC 5

static size_t http_curl_header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    http_curl_data* curlData = (http_curl_data*) userdata;

    size_t bytes = size * nitems;
    size_t headerNameLen = strlen(HTTP_CONTENT_LENGTH_HEADER);

    if(bytes >= headerNameLen && strncmp(buffer, HTTP_CONTENT_LENGTH_HEADER, headerNameLen) == 0) {
        char* separator = strstr(buffer, ": ");
        if(separator != NULL) {
            char* valueStart = separator + 2;

            char value[32];
            memset(value, '\0', sizeof(value));
            strncpy(value, valueStart, bytes - (valueStart - buffer));

            if(curlData->contentLength != NULL) {
                *(curlData->contentLength) = (u64) atoll(value);
            }
        }
    }

    return size * nitems;
}

static size_t http_curl_write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    http_curl_data* curlData = (http_curl_data*) userdata;

    size_t available = size * nmemb;
    while(available > 0) {
        size_t remaining = curlData->bufferSize - curlData->pos;
        size_t copySize = available < remaining ? available : remaining;

        memcpy((u8*) curlData->buf + curlData->pos, ptr, copySize);
        curlData->pos += copySize;
        available -= copySize;

        if(curlData->pos == curlData->bufferSize) {
            curlData->res = curlData->callback(curlData->userData, curlData->buf, curlData->bufferSize);
            curlData->pos = 0;
        }
    }

    return R_SUCCEEDED(curlData->res) ? size * nmemb : 0;
}

void cleanup_services() {
    if(soc_buffer != NULL) {
        free(soc_buffer);
        soc_buffer = NULL;
    }
}

Result init_services() {
    Result res = 0;

    soc_buffer = (u32*)memalign(0x1000, 0x100000);
    if(soc_buffer != NULL) {
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
//---------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    init_services();
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

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Main loop
    while (aptMainLoop()) {
        hidScanInput();
        touchPosition touch;

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_B) {
            // getShockers();
        }
        // break in order to return to hbmenu
        //printf("\x1b[1;1HSimple citro2d shapes example");
        //printf("\x1b[3;1HCPU:     %6.2f%%\x1b[K", C3D_GetProcessingTime() * 6.0f);
        //printf("\x1b[4;1HGPU:     %6.2f%%\x1b[K", C3D_GetDrawingTime() * 6.0f);
        //printf("\x1b[5;1HCmdBuf:  %6.2f%%\x1b[K", C3D_GetCmdBufUsage() * 100.0f);

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

        if (kDown & KEY_B) {
            CURL *curl = curl_easy_init();
            if (curl) {
                const char *url = "https://openshock.app/";
                const int bufferSize = 4096;
                u64* contentLength = 0;
                void* userData = nullptr;
                Result (*callback)(void* userData, void* buffer, size_t size) = nullptr;
                char buffer[bufferSize] = ""; // Buffer to hold the HTTP response
                http_curl_data curlData = {bufferSize, contentLength, userData, callback, buffer, 0, 0};

                // Set up curl options
                curl_easy_setopt(curl, CURLOPT_URL, url);
                curl_easy_setopt(curl, CURLOPT_USERAGENT, "Test");
                curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, bufferSize);
                curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT_SEC);
                curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_curl_write_callback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &curlData);
                curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, http_curl_header_callback);
                curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void*) &curlData);

                // Perform the request
                CURLcode res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                } else {
                    printf("HTTP Response:\n%s\n", buffer);
                }

                // Cleanup
                curl_easy_cleanup(curl);
            } else {
                printf("Failed to initialize libcurl.\n");
            }
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

    // Deinit libs
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
