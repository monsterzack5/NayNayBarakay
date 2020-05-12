#include <pgmspace.h>

const char ROOT_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>

<head>
</head>
<body>
    <h2>Welcome to the super secure NayNayBarakay System!</h2><br>
    <div>
        <h3>The Door Is Currently: <span id="doorState"> Reading state...</span></h3>
        <button onclick="sendPost('/opendoor')">Open The Door</button>
        <br>
        <button onclick="sendPost('/closedoor')">Close the Door</button>
    </div>
    <script>
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

            // if we are passed a new state, ignore whatever was returned by JSON
            // if (newState && newState === 'open') {
            //     doorState.status = "DOOROPENING";
            //     sendPost("/opendoor");
            // }
            // if (newState && newState === 'close') {
            //     doorState.status = "DOORCLOSING";
            //     sendPost("/closedoor");
            // }
            // if (doorState.status.endsWith("ING")) {
            //     // we're here if its in motion
            //     console.log("Door should be in motion");
            //     return;
            // }
            // cases taken from router.cpp
            switch (doorState.status) {
                case 'DOOROPEN':
                    element.innerText = "Open!";
                    break;

                case 'DOOROPENING':
                    element.innerText = "The Door is Opening!";
                    break;

                case 'DOORCLOSED':
                    element.innerText = "Closed!";
                    break;

                case 'DOORCLOSING':
                    element.innerText = "The Door is Closing!";
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
                    element.innerText = "Shits fucked!";
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
