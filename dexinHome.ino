#define REQ_CODE  "/****?****=****"
#define REQ_OFF   "/****?****=****"
#define REQ_ON    "/****?****=****"
#define HOST_IP   "**.**.**.**"

#define DEBUG_HTTP  false
#define noneReadSwithMs 600
#define pBreath   5
#define pState    9
#define pError    10
#define pRelay    12
#define Lightness A0
#define swUser    4
#define swRelay   13

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;

unsigned long timeBgNoReadSw = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.flush();

  pinMode(pBreath, OUTPUT);
  pinMode(pState, OUTPUT);
  pinMode(pError, OUTPUT);
  pinMode(pRelay, OUTPUT);

  pinMode(swUser, INPUT);
  pinMode(swRelay, INPUT);

  digitalWrite(pBreath, LOW);
  digitalWrite(pState, LOW);
  digitalWrite(pError, LOW);

  WiFiMulti.addAP("**********", "**********");
}

bool thisUserSwState = false;
bool lastUserSwState = true;
void loop() {
  if (millis() % 300 == 0) {
    if ((WiFiMulti.run() == WL_CONNECTED)) {
      HTTPClient http;
      http.begin(HOST_IP, 7001, REQ_CODE);
      int httpCode = http.GET();
      if (httpCode) {
        if (httpCode == 200) {
          String httpPayload = http.getString();
          DynamicJsonBuffer jsonBuffer;
          JsonObject& root = jsonBuffer.parseObject(httpPayload);
          int value = root[String("val")];

          Serial.print("Get value:");
          Serial.println(value);
          digitalWrite(pError, LOW);

          if (value >= 125)
            digitalWrite(pRelay, HIGH);
          else
            digitalWrite(pRelay, LOW);
        }
      }
      else
        digitalWrite(pError, HIGH);
    }
    else
      digitalWrite(pError, HIGH);
  }

  if (millis() % 10 == 0) {
    if (digitalRead(swRelay) == 0)
      digitalWrite(pState, HIGH);
    else
      digitalWrite(pState, LOW);

    thisUserSwState = digitalRead(swUser);
    //Serial.print(thisUserSwState);

    if (millis() - timeBgNoReadSw > noneReadSwithMs) {
      if (lastUserSwState != thisUserSwState)
      {
        if (thisUserSwState == false) {
          //Serial.println(",1");
          timeBgNoReadSw = millis();
          if ((WiFiMulti.run() == WL_CONNECTED)) {
            HTTPClient http;
            if (digitalRead(swRelay))
              http.begin(HOST_IP, 7001, REQ_ON);
            else
              http.begin(HOST_IP, 7001, REQ_OFF);
            int httpCode = http.GET();
            if (httpCode) {
              if (httpCode == 200) {
                String httpPayload = http.getString();
                Serial.println(httpPayload);
              }
            }
          }
          digitalWrite(pRelay, digitalRead(swRelay));//protect from unlink wi-fi
        }
      }
    }

    lastUserSwState = thisUserSwState;
    delay(1);
  }

  analogWrite(pBreath, 125 * sin(0.002 * millis()) + 130);
}
