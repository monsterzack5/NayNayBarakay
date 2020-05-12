#include <Button.h>
#include <Config.h>
#include <ESPAsyncWebServer.h>
#include <IPAddress.h>
#include <Indicator.h>
#include <IndicatorState.h>
#include <Log.h>
#include <MotorController.h>
#include <WiFiHelper.h>
#include <esp_wifi.h>

bool WiFiHelper::CanFindOurNetwork() {
    // if there's more than 65,535 networks available, then something's _very_ wrong
    uint16_t numOfFoundNetworks = WiFi.scanNetworks();
    for (uint16_t i = 0; i < numOfFoundNetworks; i++) {
        if (WiFi.SSID(i) == SSID) {
            return true;
        }
    }
    return false;
}

bool WiFiHelper::StartLanServer() {
    DEBUG_PRINTLN("Trying to Connect to Lan Server");
    uint8_t wifiConnectionAttemps = 0;
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        wifiConnectionAttemps += 1;
        delay(500);
        DEBUG_PRINT(".");
        // Hardcoding 20 here as it doesn't make much sense to modify this value.
        if (wifiConnectionAttemps > 20) {
            WiFi.disconnect();
            return false;
        }
    }

    // Sometimes, the ESP will report WL_CONNECTED, and spit out the IPAddress 0.0.0.0
    // which means it didnt actually connect, and will won't respond to its address
    if (WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
        DEBUG_PRINTLN("ESP Connected but returned IP 0.0.0.0, Restarting");
        esp_restart();
    }
    DEBUG_PRINT("Connected to ");
    DEBUG_PRINTLN(SSID);
    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    startLanWiFiWatchdog();
    Indicator::indicate(IndicatorState::NORMALOPERATION);
    return true;
}

bool WiFiHelper::StartLocalServer() {
    DEBUG_PRINTLN("Trying to Create Local Server");
    startLocalWiFiResetTimer();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ESP_LOCAL_AP_SSID, ESP_LOCAL_AP_PASSWORD,
                ESP_LOCAL_NETWORK_CHANNEL_ID, ESP_SHOULD_LOCAL_WIFI_BE_HIDDEN);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    //                local_ip (set in main.ino)  Gateway                    Subnet
    WiFi.softAPConfig(ESP_LOCAL_IP, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    DEBUG_PRINT("Local Server SSID: ");
    DEBUG_PRINTLN(ESP_LOCAL_AP_SSID);
    DEBUG_PRINT("Local Server IP address: ");
    DEBUG_PRINTLN(WiFi.softAPIP());
    Indicator::indicate(IndicatorState::HOSTINGOWNWIFI);
    return true;
}

void WiFiHelper::StopLocalServer(void* arg) {
    if (MotorController::isDoorShutWithReed()) {
        DEBUG_PRINTLN("Gonna Stop hosting our own network and sleep now, nighty night");
        RTC_ShouldHostOwnNetwork = false;
        ESP.deepSleep(100);
        return;  // this line is never reached
    }
    DEBUG_PRINTLN("Tried to turn off local wifi and go to sleep but the door is still open");
}

void WiFiHelper::reconnectToWifi_task(void* arg) {
    DEBUG_PRINTLN("Restarting ESP in reconnectToWiFi_task");
    esp_restart();
}

void WiFiHelper::reconnectToWifi(void* arg) {
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("Wifi Watchdog reported wifi disconnected, rebooting");
        xTaskCreatePinnedToCore(reconnectToWifi_task, "reconnectToWifi_task", 1000, NULL, 1, NULL, 1);
    }
}

void WiFiHelper::startLanWiFiWatchdog() {
    const esp_timer_create_args_t wifiWatchdogTimer_args = {
        .callback = &reconnectToWifi,
    };
    esp_timer_handle_t wifiWatchdogTimer;
    esp_timer_create(&wifiWatchdogTimer_args, &wifiWatchdogTimer);
    esp_timer_start_periodic(wifiWatchdogTimer, WIFI_HOW_OFTEN_TO_CHECK_WIFI_STILL_RUNNING_SECONDS * S_TO_uSECONDS);
}

void WiFiHelper::startLocalWiFiResetTimer() {
    const esp_timer_create_args_t disableLocalWiFiTimer_args = {
        .callback = &StopLocalServer,
    };
    esp_timer_handle_t disableLocalWiFiTimer;
    esp_timer_create(&disableLocalWiFiTimer_args, &disableLocalWiFiTimer);
    esp_timer_start_periodic(disableLocalWiFiTimer, WIFI_HOW_LONG_TO_HOST_OWN_WIFI_SECONDS * S_TO_uSECONDS);
}
