#include <Arduino.h>
#include <BlynkSimpleEsp8266.h>
struct Pins
{
    static const uint8_t TemperatureSensor = D5;
    static const uint8_t Fan = D0;
    static const uint8_t MoistureSensor = D2;
    static const uint8_t SoundSensor = A0;
    static const uint8_t ServoMotor = D3;
    static const uint8_t Buzzer = D1;
}Pins;
struct VirtualPins
{
    int Fan = V1;
    int TemperatureThreshold = V4;
    int Humidity = V5;
    int Temperature = V6;
}VirtualPins;
