#include "WifiConnect.h"
#include <HTTPClient.h>
//#include <ArduinoJson.h>

int ledPin = 21;
int sensorPin1 = 22;
int sensorPin2 = 23;

int ledState = LOW;
unsigned long previousMillis = 0;
long interval = 20;
unsigned long timeSinceConnecting = 0;
unsigned long timeSinceLastCommand = 0;

int pin1Val;
int pin2Val;
int currentNum = 0;

int person_count = 0;

unsigned long first_trigger = 0;
unsigned long last_trigger = 0;
int trigger_count = 0;

int first = -1, second = -1;
boolean recentDetection = false;

int in_vector[] = {sensorPin2,sensorPin1};
int out_vector[] = {sensorPin1,sensorPin2};

//23(pin2),22(pin1) is in
//22(pin1),23(pin2) is out
//just a 1 or 2 trigger can be ignored (no change)

//need to implement a person count

WebServer* server;

//WiFiClient client;

const char* location = "office";
//String serverName = "http://192.168.1.100:8081"; //desktop
String serverName = "http://192.168.1.26:8081"; //pi 

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  
  Serial.begin(115200);

  server = SetupOTAServer("PasswordIsPassword", "getthisitisactuallypassword");
  timeSinceConnecting = millis();
}

void loop()
{
  currentNum = (currentNum + 1) % 1000;
  if(currentNum == 0)
    server->handleClient();

  toggleLight();
  
  pin1Val = digitalRead(sensorPin1);
  pin2Val = digitalRead(sensorPin2);

  if((pin2Val == 0 || pin1Val == 0) && ledState == HIGH) {
    int quick_check = millis();
    if(quick_check - last_trigger > 100) {
      first_trigger = quick_check;
      if(pin2Val == 0 && pin1Val == 1) {
        first = sensorPin2;
        second = sensorPin1;
      } else if(pin2Val == 1 && pin1Val == 0) {
        first = sensorPin1;
        second = sensorPin2;
      } else {
        first = -1;
        second = -1;
      }
    }
    
    last_trigger = millis();
    recentDetection = true;

    trigger_count++;
  }

  if(validDetection())
  {
      
    Serial.printf("first: %d, second: %d\n", first, second);
    sendLightCommand(first, second);
    recentDetection = false;
    trigger_count = 0;
    
  }

  reset_detections();
  
}

void sendLightCommand(int first, int second)
{
  int direction_vector[] = {first, second};

  HTTPClient http;

  int on_or_off = onOrOff(direction_vector);
  if(on_or_off == 0) {
    person_count--;
    if(person_count > 0) {
      on_or_off = 1; //subtract a person, but don't turn off
    } else {
      person_count = 0; //just to make sure -- shit's buggy
    }
  } else {
    person_count++;
  }
  
  http.begin(serverName);
  
  char json[128];
  char length_number[4]; //4 bytes, that translates to no number higher than (10^4) - 1, so this should be pretty safe
  //might need to change out "code" in the future if you want to do something besides change light on/off
  sprintf(json, "{\"command\":{\"location\":\"%s\",\"code\":1,\"value\":%s, \"person_count\":%d}}", location, numberToTrueFalse(on_or_off), person_count); 
  
  String data_out(json);
  itoa(data_out.length(), length_number, 10);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Connection", "close");
  http.addHeader("Content-Length", length_number);
  
  int httpCode = http.POST(data_out);
  timeSinceLastCommand = millis();

  Serial.printf("httpCode = %d, person_count = %d", httpCode, person_count);
}

void toggleLight()
{
  unsigned long currentMillis = micros();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(ledPin, ledState);
  }
}

void reset_detections() 
{
  if(millis() - last_trigger > 250) {
    first = -1;
    second = -1;
    trigger_count = 0;
  }
}

//yes this is gross, but it works
boolean validDetection()
{
  return recentDetection && //
         trigger_count > 100 && // actual human detection causes quite a bit more than 100 zeros, but random IR signals generally come in super slow
         millis() - last_trigger > 50 && // leave 50 ms just in case any other signals come
         first_trigger - last_trigger > 100 && //it takes longer than 100ms for all the zero (0) signals to be read from the sensors
         millis() - timeSinceConnecting > 5000 && //sometimes there are erratic IR signals on startup, best to ignore them
         (first != -1 || second != -1) &&  // any negatives are just errors
         millis() - timeSinceLastCommand > 1000; // only every second
}

const char* numberToTrueFalse(int num)
{
  return num >= 1 ? "true" : "false";
}

//AS ABOVE
//int in_vector[] = {sensorPin2,sensorPin1};
//int out_vector[] = {sensorPin1,sensorPin2};
//23(pin2),22(pin1) is in
//22(pin1),23(pin2) is out

//return 1 for going in, 0 for going out
int onOrOff(int* direction_vector)
{
  //just compare against in, if it's not equal, then they must be going out
  for(int i = 0; i < 2; i++) {
    if(direction_vector[i] != in_vector[i]){
      return 0;
    }
  }
  return 1;
}
