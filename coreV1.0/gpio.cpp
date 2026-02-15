#include "gpio.h"
#include <Arduino.h>

void handleGpioMode(AsyncWebServerRequest *request) {
    if (!request->hasParam("pin") || !request->hasParam("mode")) {
        request->send(400, "text/plain", "Missing parameters");
        return;
    }

    int pin  = request->getParam("pin")->value().toInt();
    int mode = INPUT;
    String m = request->getParam("mode")->value();

    if (m == "output" || m == "1") mode = OUTPUT;
    else if (m == "input_pullup" || m == "2") mode = INPUT_PULLUP;

    pinMode(pin, mode);
    request->send(200, "text/plain", "OK");
}

void handleGpioWrite(AsyncWebServerRequest *request) {
    if (!request->hasParam("pin") || !request->hasParam("value")) {
        request->send(400);
        return;
    }
    int pin   = request->getParam("pin")->value().toInt();
    int value = request->getParam("value")->value().toInt() ? HIGH : LOW;
    digitalWrite(pin, value);
    request->send(200, "text/plain", "OK");
}

void registerGPIOHandlers(AsyncWebServer &server) {
    server.on("/api/gpio/mode",  HTTP_POST, handleGpioMode);
    server.on("/api/gpio/write", HTTP_POST, handleGpioWrite);
    // read, toggle — по аналогии
}