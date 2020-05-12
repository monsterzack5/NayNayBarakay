#include <Config.h>
#include <Log.h>
#include <MotorController.h>
#include <Router.h>

#include "HTMLPages/login_page.h"
#include "HTMLPages/root.h"

bool Router::isAuthenticated(AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("Enter isAuthenticated");

    if (request->hasHeader("Cookie")) {
        String cookie = request->header("Cookie");
        if (cookie.indexOf(AUTH_COOKIE_NAME) != -1) {
            return true;
        }
    }
    DEBUG_PRINTLN("User is not currently Authenticated");
    AsyncWebServerResponse *response = request->beginResponse(301);
    response->addHeader("Location", "/login");
    response->addHeader("Cache-Control", "no-cache");
    request->send(response);
    return false;
}

void Router::handleRoot(AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("Enter handleRoot");

    if (!isAuthenticated(request)) return;

    String content = ROOT_PAGE;
    request->send(200, "text/html", content);
}

void Router::handleNotFound(AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("Enter handleNotFound");

    if (!isAuthenticated(request)) return;

    String page = "Page Not Found\n\n";
    page += "URI: ";
    page += request->url();
    page += "\nMethod: ";
    page += request->methodToString();
    page += "\nArgs: ";
    page += request->args();
    request->send(404, "text/plain", page);
}

void Router::handleLoginPost(AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("Enter handleLoginPost");

    AsyncWebServerResponse *response = request->beginResponse(200);

    // Check the args to see if this request is a Disconnect request
    /* I tried using hasArg() for this, but I have NO IDEA why it doesnt work*/
    for (uint8_t i = 0; i < request->args(); i++) {
        if (request->arg(i) == "DISCONNECT") {
            DEBUG_PRINTLN("handleLoginPost: Disconnection");
            response->addHeader("Location", "/login");
            response->addHeader("Cache-Control", "no-cache");
            response->addHeader("Set-Cookie", "NULLANDVOID");
            response->setCode(301);
            request->send(response);
            return;
        }
    }

    // If we get a Post Request with a login try
    if (request->hasArg("USERNAME") && request->hasArg("PASSWORD")) {
        DEBUG_PRINT("User trying to login with username: ");
        DEBUG_PRINTLN(request->arg("USERNAME"));
        DEBUG_PRINT("and password: ");
        DEBUG_PRINTLN(request->arg("PASSWORD"));

        if (request->arg("USERNAME") == AUTH_LOGIN_USERNAME && request->arg("PASSWORD") == AUTH_LOGIN_PASSWORD) {
            DEBUG_PRINTLN("handleLoginPost: User Entered the Correct AuthString");
            AsyncWebServerResponse *response = request->beginResponse(301);
            response->addHeader("Location", "/");
            response->addHeader("Cache-Control", "no-cache");
            response->addHeader("Set-Cookie", AUTH_COOKIE_NAME);
            request->send(response);
            DEBUG_PRINTLN("handleLoginPost: Log in Successful");
            return;
        }

        DEBUG_PRINTLN("handleLoginPost: User Entered Incorrect AuthString");
    }
}

void Router::handleLoginGet(AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("Enter handleLoginGet");

    request->send(200, "text/html", LOGIN_PAGE);
}

void Router::taskOpen(void *param) {
    MotorController::openDoor();
    vTaskDelete(NULL);
}

void Router::taskClose(void *param) {
    MotorController::closeDoor();
    vTaskDelete(NULL);
}

void Router::handleOpenDoor(AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("Enter handleDoorOpen");

    if (!isAuthenticated(request)) return;

    if (MotorController::areWebCommandsAllowed()) {
        request->send(200);
        xTaskCreatePinnedToCore(taskOpen, "taskOpen", 4096, NULL, 1, NULL, 1);
        return;
    }
    DEBUG_PRINTLN("handleDoorOpen Failed as webcommands currently are not allowed");
    return request->send(500);
}

void Router::handleCloseDoor(AsyncWebServerRequest *request) {
    DEBUG_PRINTLN("Entering handleDoorClose");

    if (!isAuthenticated(request)) return;

    if (MotorController::areWebCommandsAllowed()) {
        xTaskCreatePinnedToCore(taskClose, "taskClose", 4096, NULL, 1, NULL, 1);
        request->send(200);
        return;
    }
    DEBUG_PRINTLN("handleDoorClose Failed as webcommands currently are not allowed");
    return request->send(500);
}

void Router::handleStop(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) return;

    MotorController::setAllowedToMove(false);
}

void Router::handleAllowStart(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) return;
    //  FIXME: Buttons for stopping then starting again
    MotorController::setAllowedToMove(true);
}

void Router::getDoorState(AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) return;

    DoorStatus State = MotorController::getDoorState();
    String content = "";
    /**
     * All Possible return states:
     * ::DOOROPEN
     * ::DOOROPENING
     * ::DOORCLOSED
     * ::DOORCLOSING
     * ::DOORFLOATING
     * ::DOORERROR
     * 
     * Example end result string:
     *  { "status": "DOOROPEN", "isMoving": true }
     */

    content += "{\"status\": \"";
    if (State == DoorStatus::DOOROPEN) {
        content += "DOOROPEN";
    }
    if (State == DoorStatus::DOOROPENING) {
        content += "DOOROPENING";
    }
    if (State == DoorStatus::DOORCLOSED) {
        content += "DOORCLOSED";
    }
    if (State == DoorStatus::DOORCLOSING) {
        content += "DOORCLOSING";
    }
    if (State == DoorStatus::DOORFLOATING) {
        content += "DOORFLOATING";
    }
    if (State == DoorStatus::ERROR) {
        content += "DOORERROR";
    }

    content += "\",\"isMoving\": ";
    content += MotorController::isMoving() ? "true}" : "false}";

    request->send(200, "application/json", content);
}