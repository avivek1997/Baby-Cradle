#include <Arduino.h>
#include <Messages.h>
#include <Configs.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Pins.h>
#include <DHT.h>
#include <Servo.h>
#include "ThingSpeak.h"
WiFiClient  client;

Servo myservo;  

#define DHTTYPE DHT11
DHT dht(Pins.TemperatureSensor, DHTTYPE);
BlynkTimer timer;

bool manualFanControl=0;
void HandleTemperatureSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(VirtualPins.Humidity, h);
  Blynk.virtualWrite(VirtualPins.Temperature, t);
  ThingSpeak.setField(1, t);

  int x = ThingSpeak.writeFields(Configs.ThingsSpeak_SECRET_CH_ID, Configs.ThingsSpeak_APIKEY);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  if(int(t) >= Configs.TemperatureThreshold && !manualFanControl)
  {
    Blynk.virtualWrite(VirtualPins.Fan, HIGH);
    Serial.println(Messages.FanOnAuto);
    manualFanControl=1;
    Blynk.syncVirtual(VirtualPins.Fan);
  }
  else if((int(t) < (Configs.TemperatureThreshold-1)) && manualFanControl){
    Blynk.virtualWrite(VirtualPins.Fan, LOW);
    Serial.println(Messages.FanOffAuto);
    manualFanControl = 0;
    Blynk.syncVirtual(VirtualPins.Fan);
  }
}


bool isBabyCryNotificationSent=0;
int currentSamplingSecond=Configs.SoundSamplingPeriod;
int babyCryCount=0;
void HandleBabyCry()
{
  pinMode(Pins.SoundSensor,INPUT);

  int sound = analogRead(Pins.SoundSensor);
  //Serial.println("sound : "+String(sound)+" babyCryCount : "+String(babyCryCount));
  if(currentSamplingSecond--)
  {
       if(sound > Configs.SoundThreshold)
        babyCryCount++;
  }
  else
  {
    currentSamplingSecond = Configs.SoundSamplingPeriod;
    //Serial.println("sound : "+String(sound)+" babyCryCount : "+String(babyCryCount)+" babyNoCryCount : "+String(babyNoCryCount));
    if(babyCryCount>Configs.BabyCryThresholdPerSamplingPeriod)
    {
      Serial.println(Messages.BabyCry);
      if(!isBabyCryNotificationSent)
      {
        Blynk.notify(Messages.BabyCry);
        isBabyCryNotificationSent = 1;
      }
    }
    babyCryCount=0;
  }
  
}

bool isWetBedNotificationSent=0;

void HandleMoistureSensor()
{
  int currentValue = digitalRead(Pins.MoistureSensor);
  
  if(!currentValue && !isWetBedNotificationSent)
  {
    Blynk.notify(Messages.BabyWetBed);
    isWetBedNotificationSent=1;
  }
  else if(currentValue){
    isWetBedNotificationSent=0;
  }
  //Serial.println("Moisture : "+String(currentValue)+"isWetBedNotificationSent : "+String(isWetBedNotificationSent));
}
void ResetBabyCryNotificationSent()
{
  isBabyCryNotificationSent = 0;
}

void SwingCradle()
{
  
  if(isBabyCryNotificationSent)
  {
    digitalWrite(Pins.Buzzer,HIGH);
        int pos = 0;
        myservo.attach(Pins.ServoMotor);
        for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
          // in steps of 1 degree
          myservo.write(pos);              // tell servo to go to position in variable 'pos'
          delay(15);                       // waits 15 ms for the servo to reach the position
         }
        for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
          myservo.write(pos);              // tell servo to go to position in variable 'pos'
          delay(15);                       // waits 15 ms for the servo to reach the position
        }
        myservo.detach();
    digitalWrite(Pins.Buzzer,LOW);
  }
}
BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); 
  digitalWrite(Pins.Fan,!pinValue);
}
BLYNK_WRITE(V4)
{
  int pinValue = param.asInt(); 

  Configs.TemperatureThreshold = pinValue;
  Serial.println(Configs.TemperatureThreshold);
}


void setup() {
  Serial.begin(9600);
  Blynk.begin(Configs.BlynkAuth, Configs.Wifi_SSID, Configs.WIFI_Password);
  dht.begin();
  pinMode(Pins.Fan ,OUTPUT);
  pinMode(Pins.SoundSensor,OUTPUT);
  pinMode(Pins.Buzzer,OUTPUT);
  pinMode(Pins.MoistureSensor, INPUT);
  Blynk.syncAll();
  ThingSpeak.begin(client);
  timer.setInterval(1000L, HandleTemperatureSensor);
  timer.setInterval(3000L,HandleMoistureSensor);
  timer.setInterval(50L, HandleBabyCry);
  timer.setInterval(20000L, ResetBabyCryNotificationSent);
  timer.setInterval(6000L, SwingCradle);
}

void loop() {
   Blynk.run();
   timer.run();
}