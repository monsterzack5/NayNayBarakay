#include <pgmspace.h>

const char LOGIN_PAGE[] PROGMEM = R"=====(
<html>
<head>
    <meta content="text/html;charset=utf-8" http-equiv="Content-Type">
    <meta content="utf-8" http-equiv="encoding">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="manifest" href="/manifest.json">
</head>
<body>
    <form action='/login' method='POST'>Please Sign In First<br>
        User:<input type='text' name='USERNAME' placeholder='username'><br>
        Password:<input type='password' name='PASSWORD' placeholder='password'><br>
        <input type='submit' name='SUBMIT' value='Submit'></form>
</body>
</html>
)=====";
