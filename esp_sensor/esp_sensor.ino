#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>

// Fingerprint for demo URL, expires on June 2, 2019, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x5A, 0xCF, 0xFE, 0xF0, 0xF1, 0xA6, 0xF4, 0x5F, 0xD2, 0x11, 0x11, 0xC6, 0x1D, 0x2F, 0x0E, 0xBC, 0x39, 0x8D, 0x50, 0xE0};

ESP8266WiFiMulti WiFiMulti;


// defines pins numbers

const int trigPin = 5;  //D1
const int echoPin = 4;  //D2
const int led_pass = 2;
const int pass_time = 10;

const char* host="img.securegg.com";

const char* url="http://img.securegg.com/api/index.php?id=test";

// defines variables
long duration;
int distance;
long loop_nb;
long last_see;
long nb_pass;
int passing;

void setup() {
  pinMode(led_pass, OUTPUT); // Sets the trigPin as an Output
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(9600); // Starts the serial communication

  digitalWrite(led_pass, LOW);

  loop_nb = 0;
  last_see = 0;
  nb_pass = 0;
  passing = 0;

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("aitivity", "aitivity");

}

void send_data(int nb_pass){
  Serial.println("Send data");
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;
    
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, url)) {  // HTTP

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");    //Specify content-type header

      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      String post = "nb_pass=";
      post += nb_pass;
      int httpCode = http.POST(post); 

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP] Unable to connect\n");
    }
  }
  Serial.println("Send data end");
}

void loop() {

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);

    // Calculating the distance
    distance= duration*0.034/2;
    // Prints the distance on the Serial Monitor

    // Nothing in front
    if (distance > 50){
      if (passing){
        if(loop_nb - last_see > pass_time){
          passing = 0;
          digitalWrite(led_pass, LOW);
        }
      }
    }
    //Already see
    else if(passing){
      last_see = loop_nb;
    }
    //not see from long liem
    else{
      nb_pass++;
      passing = 1;
      digitalWrite(led_pass, HIGH);
      last_see = loop_nb;  
      send_data(nb_pass);
    }
    Serial.print(loop_nb);
    Serial.print(" ");
    Serial.print(nb_pass);
    Serial.print(" ");
    Serial.print(passing);
    Serial.print(" ");
    Serial.print("Distance: ");
    Serial.println(distance);
    delay(100);
    loop_nb++;
  }
  else{
    digitalWrite(led_pass, HIGH);
    delay(200);
    digitalWrite(led_pass, LOW);
    delay(200);
  }
}
