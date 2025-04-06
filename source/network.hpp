#pragma once
#include <3ds.h>
#include <list>

extern const char* userAgent;
struct User {
    char *token;
    char *server;
    bool loggedIn;
};

struct ShockerLimits {
    int intensity;
    int duration;
};

struct ShockerPermissions {
    bool shock;
    bool vibrate;
    bool sound;
    bool live;
};

struct Shocker {
    char* name;
    char* id;
    char* model;
    bool isPaused;
    ShockerLimits limits;
    ShockerPermissions permissions;
};

extern User user;

extern int clamp(int value, int min, int max);

extern bool sendAction(const char* action, int intensity, int duration, Shocker s);
extern std::list<Shocker> getShockers() ;