#pragma once

#include <ESPAsyncWebServer.h>

class Router {
   public:
    static void handleRoot(AsyncWebServerRequest*);
    static void handleNotFound(AsyncWebServerRequest*);
    static void handleLoginPost(AsyncWebServerRequest*);
    static void handleLoginGet(AsyncWebServerRequest*);
    static void handleOpenDoor(AsyncWebServerRequest*);
    static void handleCloseDoor(AsyncWebServerRequest*);
    static void handleStop(AsyncWebServerRequest*);
    static void handleAllowStart(AsyncWebServerRequest*);
    static void handleOpenThenShut(AsyncWebServerRequest*);
    static void handleGetManifest(AsyncWebServerRequest*);
    static void getDoorState(AsyncWebServerRequest*);

   private:
    static void taskOpen(void*);
    static void taskClose(void*);
    static void taskOpenThenClose(void*);
    static bool isAuthenticated(AsyncWebServerRequest*);
};
