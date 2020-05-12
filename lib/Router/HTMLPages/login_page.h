#include <pgmspace.h>

const char LOGIN_PAGE[] PROGMEM = R"=====(
<html>
<body>
    <form action='/login' method='POST'>Please Sign In First<br>
        "User:<input type='text' name='USERNAME' placeholder='username'><br>
        "Password:<input type='password' name='PASSWORD' placeholder='password'><br>
        <input type='submit' name='SUBMIT' value='Submit'></form>
</body>
</html>
)=====";
