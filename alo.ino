#define BLYNK_TEMPLATE_ID "TMPL6z1Ybp9q8"
#define BLYNK_TEMPLATE_NAME "Assignment 2"
#define BLYNK_AUTH_TOKEN "WiRJ2ntJXJ2edHSg9bXlWjDqnrB5vSHK"
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <SimpleKalmanFilter.h>
#include <BlynkSimpleEsp8266.h>

SimpleKalmanFilter bo_loc(2, 2, 0.001);

char auth[] = BLYNK_AUTH_TOKEN;

int mq2Pin = A0;
int buzzerPin = D5;
int ledWarningPin = D6;

int gasThreshold = 20;
int gasWarningState = 1 ;

BlynkTimer timer;

// Set these to run example.
#define FIREBASE_HOST "assignment-2-f5b99-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyB3iJEFNDFIJXFgY2g9LCMD8K6Xv7LnCOQ"
#define WIFI_SSID "BB"
#define WIFI_PASSWORD "15121967"

void setup() {
  Serial.begin(9600);

  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Blynk.begin(auth, WIFI_SSID, WIFI_PASSWORD);

  pinMode(mq2Pin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledWarningPin, OUTPUT);

  Blynk.virtualWrite(V3, gasWarningState);

  timer.setInterval(2000, checkGas);
}

void loop() {
  Blynk.run();
  timer.run();
}

void checkGas() {
  int gas = analogRead(mq2Pin);
  gas = bo_loc.updateEstimate(gas);

  Serial.print("Gas: ");
  Serial.println(gas);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set gas value to the "Gas" node in your Firebase database
  Firebase.setInt("Gas", gas);

  if (gas > gasThreshold && gasWarningState) {
    digitalWrite(buzzerPin, HIGH);
    digitalWrite(ledWarningPin, HIGH);
    Blynk.virtualWrite(V1, 0);
    Blynk.virtualWrite(V2, 255);
  } else if (gas <= gasThreshold) {
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledWarningPin, LOW);
    Blynk.virtualWrite(V1, 255);
    Blynk.virtualWrite(V2, 0);
  }                                                                    
  Blynk.virtualWrite(V0, gas);
}

BLYNK_WRITE(V4) {
  int pinValue = param.asInt();
  if (pinValue == 0) {
    digitalWrite(buzzerPin, LOW);  // Turn OFF the buzzer
    gasWarningState = 0;
  } else {
    digitalWrite(buzzerPin, HIGH);  // Turn ON the buzzer
    gasWarningState = 1;
  }
}
