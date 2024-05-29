//Checks the temperature at update time, and registers if any sound has been detected over the 5 second interval
#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

// the a number of analog readings to take an average of
#define CYCLES 50

const int tempPin = A0;
const int soundPin = 13;

const char ssid[] = SECRET_SSID; // network SSID 
const char pass[] = SECRET_PASS; // network password
WiFiClient  client;

const unsigned long myChannelNumber = CHANNEL_ID;
const char myWriteAPIKey[] = WRITE_KEY;

// Initialize our variables and index
float tmp = 0.0;
bool soundDetected = false;

int i = 0;
void setup() {
  Serial.begin(115200); // Initialize serial, needs a high baud for analog
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak 
  attachInterrupt(digitalPinToInterrupt(soundPin), updateSound, RISING);
}

void loop() {
  
  connectWifi();

  if (i < CYCLES) { //collect analog values to average
    tmp += checkTemp();
    i += 1;
  } else { // send data to thingspeak 
    float avgTmp = tmp/(float)CYCLES;
    // reset counter/value for next time
    i = 0;
    tmp = 0.0;
        
    updateThingspeak(avgTmp, soundDetected);
    soundDetected = false;
    delay(5000); // Wait 5 seconds to update the channel again
  }
}

void connectWifi() {
  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
}
 
void updateThingspeak(float tmpReading, int soundReading) {
  // set the fields with the values
  ThingSpeak.setField(1, tmpReading);
  ThingSpeak.setField(2, soundReading);
  
  // write to the ThingSpeak channel 
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

float checkTemp() {
  int reading = analogRead(tempPin);  
  // temperature reading is proportional to voltage applied
  float voltage = reading * 3.3;
  // in celsius
  float temp = (voltage/1024 - 0.5)*100;
  return temp; 
}


void updateSound() {
  soundDetected = true;
}