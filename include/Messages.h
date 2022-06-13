#include <Arduino.h>
struct Messages
{
    const char* BabyWetBed = "Baby has wet the bed, please change it.";
    const char* BabyCry = "Baby is Crying";
    const char* FanOnAuto = "Fan Turned ON Due to High Temperature";   
    const char* FanOffAuto = "Fan Turned OFF Due to LOW Temperature";
    const char* TemperatureAlert = "Alert!! Room Temperature is HIGH";
}Messages;