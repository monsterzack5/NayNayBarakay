// Needed For ESPAsyncWebServer
#define ESP32 1

/* Enabling and Disabling of logging output is done in Log.h */

/* Comment this out to disable the ability to update via OTA */
// #define OTA_UPLOAD 1
#ifdef OTA_UPLOAD
#    include <ArduinoOTA.h>
#endif

/* Comment this line out if outsideESPWakeup is a button */
#define OUTSIDEESPWAKEUPISTOUCH 1

#include <Arduino.h>
#include <Config.h>
#include <ESPAsyncWebServer.h>
#include <Esp.h>
#include <Indicator.h>
#include <Log.h>
#include <MotorController.h>
#include <Router.h>
#include <Sleeper.h>
#include <WiFiHelper.h>
#include <esp_sleep.h>

/* __Config__ */
const char* SSID = "";                                                    // Wifi network name
const char* PASSWORD = "";                                                // Wifi network password
const char* ESP_LOCAL_AP_SSID = "";                                       // Self hosted AP name
const char* ESP_LOCAL_AP_PASSWORD = "";                                   // Self hosted AP password
const char* AUTH_LOGIN_USERNAME = "";                                     // Web UI Username
const char* AUTH_LOGIN_PASSWORD = "";                                     // Web UI Password
const char* AUTH_COOKIE_NAME = "";                                        // The name of the stored session cookie (ideally random letters)
const IPAddress ESP_LOCAL_IP(192, 168, 1, 1);                             // IP to connect to in AP mode
const bool ESP_SHOULD_LOCAL_WIFI_BE_HIDDEN = false;                       // Should the self hosted AP be hidden
const uint8_t ESP_LOCAL_NETWORK_CHANNEL_ID = 8;                           // Wifi channel the ESP AP will use
const uint8_t WiFi_TRY_TO_CONNECT_THEN_SLEEP_LIMIT = 3;                   // How many times to sleep then check Wifi
const uint16_t SLEEPER_SHORT_DEEP_SLEEP_SECONDS = 60;                     // Seconds to wait inbetween Wifi connect retries
const uint16_t SLEEPER_LONG_DEEP_SLEEP_SECONDS = 3600;                    // If Wifi doesn't connect after X tries, wait this long before retrying
const uint16_t WIFI_HOW_LONG_TO_HOST_OWN_WIFI_SECONDS = 180;              // Seconds to host own AP from outsideESPWakeup
const uint16_t WIFI_HOW_OFTEN_TO_CHECK_WIFI_STILL_RUNNING_SECONDS = 120;  // How often the Wifi watchdog checks if the wifi is disconnected
const uint16_t INDICATOR_SHORT_BLINK_TIME_MS = 200;                       // How long the LED should be lit up for A short blink
const uint16_t INDICATOR_LONG_BLINK_TIME_MS = 1000;                       // How long the LED should be lit up for A long blink
const uint16_t INDICATOR_WAIT_BETWEEN_NONIMPORTANT_BLINK_CODES_MS = 5000; // Wait time between nonimportant codes being blinked (Normal Operation)
const uint16_t INDICATOR_WAIT_BETWEEN_IMPORTANT_BLINK_CODES_MS = 750;     // Wait time between important codes being blinked (Error's)

#ifdef OTA_UPLOAD
const uint16_t ARDUINO_OTA_PORT = 3232; // Arduino OTA Port (3232 is the standard default)
const char* ARDUINO_OTA_PASSWORD = "";  // Arduino OTA Password
const char* ARDUINO_OTA_HOSTNAME = "";  // Arduino OTA Hostname
#endif

#ifdef OUTSIDEESPWAKEUPISTOUCH
const uint8_t OUTSIDE_WAKE_BUTTON_TOUCH_THRESHOLD = 20;
const uint64_t interruptPinsBitmask = pow(2, insideDoorOpen);
#else
const uint64_t interruptPinsBitmask = (pow(2, insideDoorOpen) + pow(2, outsideESPWakeup));
#endif

AsyncWebServer server(80);

RTC_DATA_ATTR uint8_t RTC_WiFiRetriesCount = 0;
RTC_DATA_ATTR bool RTC_ShouldHostOwnNetwork = false;

// Outputs
const uint8_t hBridgeOpen = 32;
const uint8_t hBridgeClose = 33;
const uint8_t indicatorPin = 21;

// Inputs
/*
    Don't Forget Interupts Need To Be On RTC_GPIO Pins
    And outsideESPWakeup needs to be on a Touch GPIO Pin
    if You're using it as a touch button.
 */
const uint8_t insideDoorOpen = 34;
const uint8_t outsideESPWakeup = 13;
const uint8_t limitSwitchOpen = 22;
const uint8_t limitSwitchClose = 23;
const uint8_t doorReedSwitch = 14;

void setup()
{
    delay(100);

    pinMode(hBridgeOpen, OUTPUT);
    pinMode(hBridgeClose, OUTPUT);
    pinMode(indicatorPin, OUTPUT);

    // Just to be safe
    digitalWrite(hBridgeOpen, LOW);
    digitalWrite(hBridgeClose, LOW);

    pinMode(insideDoorOpen, INPUT_PULLDOWN);
    pinMode(limitSwitchClose, INPUT_PULLUP);
    pinMode(limitSwitchOpen, INPUT_PULLUP);
    pinMode(doorReedSwitch, INPUT_PULLUP);

#ifdef OUTSIDEESPWAKEUPISTOUCH
    esp_sleep_enable_touchpad_wakeup();
    touchAttachInterrupt(outsideESPWakeup, {}, OUTSIDE_WAKE_BUTTON_TOUCH_THRESHOLD);
#endif

#ifdef DEBUG
    Serial.begin(115200);
    delay(500);
    DEBUG_PRINTLN("DEBUG Printing Enabled");
#endif

    esp_sleep_enable_ext1_wakeup(interruptPinsBitmask, ESP_EXT1_WAKEUP_ANY_HIGH);

    Indicator::init();

    // If woken up from an interrupt, do something based on that interrupt
    switch (Sleeper::interruptWakeupPin()) {
    case insideDoorOpen:
        MotorController::changeDoorStateAndWaitForDoor();
        RTC_WiFiRetriesCount = 0;
        ESP.deepSleep(100);
        break;
    case outsideESPWakeup:
        RTC_ShouldHostOwnNetwork = true;
        break;
    case 0:
        break;
    default:
        DEBUG_PRINTLN("interruptWakeupPin Returning unknown state");
        break;
    }

    // Check if the ESP can see our network via scanning
    if (WiFiHelper::CanFindOurNetwork()) {
        if (!WiFiHelper::StartLanServer()) {
            DEBUG_PRINTLN("Found Network Via Scanning But Failed to Connect, Rebooting!");
            esp_restart();
        }
    } else {
        // This gets set when pressing the outdoorESPWake interrupt button
        if (RTC_ShouldHostOwnNetwork) {
            WiFiHelper::StartLocalServer();
        } else {
            Sleeper::checkWiFiLoop();
        }
    }

    server.on("/", HTTP_GET, Router::handleRoot);
    server.on("/login", HTTP_POST, Router::handleLoginPost);
    server.on("/login", HTTP_GET, Router::handleLoginGet);
    server.on("/opendoor", HTTP_GET | HTTP_POST, Router::handleOpenDoor);
    server.on("/closedoor", HTTP_GET | HTTP_POST, Router::handleCloseDoor);
    server.on("/getdoorstate", HTTP_GET, Router::getDoorState);
    server.on("/allowstart", HTTP_GET | HTTP_POST, Router::handleAllowStart);
    server.on("/stop", HTTP_GET | HTTP_POST, Router::handleStop);
    server.on("/openthenshut", HTTP_GET | HTTP_POST, Router::handleOpenThenShut);
    server.on("/manifest.webmanifest", HTTP_GET, Router::handleGetManifest);
    server.onNotFound(Router::handleNotFound);
    server.begin();

#ifdef OTA_UPLOAD
    ArduinoOTA.setPort(ARDUINO_OTA_PORT);
    ArduinoOTA.setPassword(ARDUINO_OTA_PASSWORD);
    ArduinoOTA.setHostname(ARDUINO_OTA_HOSTNAME);

    ArduinoOTA
        .onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            DEBUG_PRINTLN("Start updating " + type);
        })
        .onEnd([]() {
            DEBUG_PRINTLN("\nEnd");
        })
        .onProgress([](unsigned int progress, unsigned int total) {
            DEBUG_PRINTF("Progress: %u%%\r", (progress / (total / 100)));
        })
        .onError([](ota_error_t error) {
            DEBUG_PRINTF("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR)
                DEBUG_PRINTLN("Auth Failed");
            else if (error == OTA_BEGIN_ERROR)
                DEBUG_PRINTLN("Begin Failed");
            else if (error == OTA_CONNECT_ERROR)
                DEBUG_PRINTLN("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR)
                DEBUG_PRINTLN("Receive Failed");
            else if (error == OTA_END_ERROR)
                DEBUG_PRINTLN("End Failed");
        });

    ArduinoOTA.begin();
#endif

    MotorController::init();
}

void loop()
{
    MotorController::loop();
#ifdef OTA_UPLOAD
    ArduinoOTA.handle();
#endif
}
