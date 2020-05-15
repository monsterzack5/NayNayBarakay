#include <pgmspace.h>

const char ROOT_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
    <meta content="text/html;charset=utf-8" http-equiv="Content-Type">
    <meta content="utf-8" http-equiv="encoding">
</head>
<body>
    <h2>Welcome to the super secure NayNayBarakay System!</h2><br>
    <div>
        <h3>The Door Is: <span id="doorState"> Reading state...</span></h3>
        <button onclick="sendPost('/opendoor')">Open The Door</button>
        <br>
        <button onclick="sendPost('/closedoor')">Close the Door</button>
        <br>
        <button onclick="handleStop()">Immediately Stop the Door</button>
        <br>
        <button id="start" onclick="handleStart()">Allow the Door To Move Again</button>
    </div>
    <script>
        // bool isDoorHalted = false;
        // Add handling to the returned JSON So we can still display
        // the start button after a refresh
        // because right now, it would be impossible
        // to start, after pressing stop, then refreshing
        // lmao

        // Make the AllowStart button hidden by default
        document.getElementById("start").style.visibility = "hidden";

        function handleStart() {
            document.getElementById("start").style.visibility = "hidden";
            sendPost("/allowstart");
        }

        function handleStop() {
              document.getElementById("start").style.visibility = "visible";
              sendPost("/stop");
        }

        function sendPost(endpoint) {
            if (endpoint) {
                return fetch(endpoint, {
                    method: "POST",
                });
            }
            console.log("Endpoint not specified!");
        }
        async function updateDoorState() {
            const element = document.getElementById('doorState');
            const getDoorStateRESP = await fetch("/getdoorstate");
            const doorState = await getDoorStateRESP.json();

            if (!doorState.allowedToMove) {
                document.getElementById("start").style.visibility = "visible";
                element.innerText = "Not Allowed To Move!";
                return;
            }

            // cases taken from router.cpp
            switch (doorState.status) {
                case 'DOOROPEN':
                    element.innerText = "Open!";
                    break;

                case 'DOOROPENING':
                    element.innerText = "Opening!";
                    break;

                case 'DOORCLOSED':
                    element.innerText = "Closed!";
                    break;

                case 'DOORCLOSING':
                    element.innerText = "Closing!";
                    break;

                case 'DOORFLOATING':
                    if (!doorState.isMoving) {
                        element.innerText = "Door is Currently Floating"
                    }
                    break;
                case 'ERROR':
                    element.innerText = "Error!";
                    break;
                default:
                    element.innerText = "Unknown State Returned from ESP";
                    break;
            }
        }
        setInterval(() => updateDoorState(), 1000);
    </script>
    <form action='/login' method='POST'><button name='subject' type='submit' value='DISCONNECT'>Logout</button>
    </form>
</body>
</html>
)=====";
