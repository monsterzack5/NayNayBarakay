#pragma once

class WiFiHelper {
   public:
    static bool CanFindOurNetwork();
    static bool StartLanServer();
    static bool StartLocalServer();

   private:
    static void startLanWiFiWatchdog();
    static void startLocalWiFiResetTimer();
    static void StopLocalServer(void*);
    static void reconnectToWifi(void*);
    static void reconnectToWifi_task(void*);
};
