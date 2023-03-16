#include <ArduinoMqttClient.h>
#include "Arduino_MKRIoTCarrier.h"
#include <WiFiNINA.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <HTTPClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include "pitches.h"
MKRIoTCarrier carrier;

// SET BATHROOM ID
int bathroomID = 1;

// COMMUNICATIONS
char wifi_ssid[] = SECRET_SSID_WIFI;
char wifi_pass[] = SECRET_PASS_WIFI;  // your network password (use for WPA, or use as key for WEP)
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "165.22.31.23";
int port = 1883;
const char topicOccupation[] = "occupation";
const char topicTemp[] = "sensors/temperature";
const char topicHumi[] = "sensors/humidity";
const char topicLight[] = "sensors/light";
const char topicQueueSize[] = "queue/size";
DynamicJsonDocument doc(1024);

//set interval for sending messages (milliseconds)
const long interval = 1000;

// PIR
int pirPin = A5;
int state = LOW;  // we start with assuming no motion detected
int pirVal = 0;   // variable for reading the pin status
int calibrationTime = 30;
int motionStillAllowance = 10000;

// VARIABLE DECLARATIONS
int light = 0;
int lightPrev = 0;
float temp = 0;
float tempPrev = 0;
float humi = 0;
float humiPrev = 0;
uint32_t colorRed = carrier.leds.Color(200, 0, 0);
uint32_t colorGreen = carrier.leds.Color(0, 200, 0);
uint32_t colorBlue = carrier.leds.Color(0, 0, 200);
uint32_t colorNone = carrier.leds.Color(0, 0, 0);
int nInQueue = 0;
int prevNInQueue = 0;
int timeSinceMotion = 0;

// MUSIC
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};
int finalMelody[] = {
  NOTE_E6, NOTE_G6, NOTE_E7, NOTE_C7, NOTE_D7, NOTE_G7, NOTE_E7, NOTE_G6
};
int celebrationMelody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// FUNCTION DECLARATION
void wifiConnect();
void mqttConnect();
void display_temp_humidity(float temp, float hum);
void addToQueue();
void pubMQTT(float value, const char topic[]);
void pubMQTT(int value, const char topic[]);
void playCelebrationMelody();
void calibrateSensor();
void pubMQTTJSON(float value, const char topic[], const char sensor[]);

void setup() {
  pinMode(pirPin, INPUT);
  Serial.begin(9600);
  while (!Serial);
  carrier.begin();
  Serial.println("==========================================");
  // calibrateSensor();
  wifiConnect();
  mqttConnect();
  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(topicQueueSize);
  carrier.leds.fill(colorGreen, 2, 1);
  carrier.leds.show();
}

void loop() {
  // avoid disconnect
  mqttClient.poll();

  // sensor readings
  int none = 0;
  if (carrier.Light.colorAvailable()) {
    carrier.Light.readColor(none, none, none, light);
  }
  pirVal = digitalRead(pirPin);
  temp = carrier.Env.readTemperature();  //reads temperature
  humi = carrier.Env.readHumidity();     //reads humidity
  humi = carrier.Env.readHumidity();     //reads airQuality
  
  if (abs(temp - tempPrev) >= 0.5) {
    tempPrev = temp;
    pubSensorData(temp, topicTemp, "temperature");
    display_temp_humidity(temp, humi);
  }
  if (abs(humi - humiPrev) >= 0.5) {
    humiPrev = humi;
    pubSensorData(humi, topicHumi, "humidity");
    display_temp_humidity(temp, humi);
  }
  if (abs(light - lightPrev) >= 100 || lightPrev == 0 & light != 0) {
    lightPrev = light;
    pubSensorData(light, topicLight, "light");
  }
  motionDetection(pirVal);
  delay(interval);
}

void display_temp_humidity(float temp, float hum) {
  carrier.display.fillScreen(ST77XX_BLACK);    //black background
  carrier.display.setTextColor(ST77XX_WHITE);  //white text
  carrier.display.setTextSize(3);              //medium sized text

  carrier.display.setCursor(30, 60);     //sets position for printing (x and y)
  carrier.display.println("Humidity:");  //prints text
  carrier.display.setCursor(60, 90);     //sets position for printing (x and y)
  carrier.display.print(humi);           //prints a variable
  carrier.display.println("%");          //prints text

  carrier.display.setCursor(20, 130);       //sets position for printing (x and y)
  carrier.display.println("Temperature:");  //prints text
  carrier.display.setCursor(60, 160);       //sets position for printing (x and y)
  carrier.display.print(temp);              //prints a variable
  carrier.display.println("C");             //prints text
}

void pubSensorData(float data, const char topic[], const char sensor[]) {
  doc["bathroomID"] = bathroomID;
  doc["data"] = data;
  doc["sensor"] = sensor;
  char out[128];
  serializeJson(doc, out);
  mqttClient.beginMessage(topic);
  mqttClient.print(out);
  mqttClient.endMessage();
  Serial.print("Published: ");
  Serial.println(out);
}

void pubSensorData(int data, const char topic[], const char sensor[]) {
  doc["bathroomID"] = bathroomID;
  doc["data"] = data;
  doc["sensor"] = sensor;
  char out[128];
  serializeJson(doc, out);
  mqttClient.beginMessage(topic);
  mqttClient.print(out);
  mqttClient.endMessage();
  Serial.print("Published: ");
  Serial.println(out);
}

void pubOccupiedStatus(int occupied){
  // 0 = free, 1 = occupied
  doc["bathroomID"] = bathroomID;
  doc["occupied"] = occupied;
  char out[128];
  serializeJson(doc, out);
  mqttClient.beginMessage(topicOccupation);
  mqttClient.print(out);
  mqttClient.endMessage();
  Serial.print("Published: ");
  Serial.println(out);
}

void updateQueueSize(int n) {
  if (n > 4) {
    n = 5;
    carrier.Buzzer.sound(200);
    carrier.leds.fill(colorRed, 0, 5);
    carrier.leds.show();
    delay(500);
    carrier.leds.fill(colorBlue, 0, 5);
    carrier.leds.show();
    delay(500);
    carrier.Buzzer.noSound();
  }
  // Flash with lights
  carrier.leds.fill(colorRed, 0, n);
  carrier.leds.show();
  delay(500);
  carrier.leds.fill(colorGreen, 0, n);
  carrier.leds.show();
  delay(500);
  carrier.leds.fill(colorRed, 0, n);
  carrier.leds.show();
  delay(500);
  carrier.leds.fill(colorGreen, 0, n);
  carrier.leds.show();
  delay(500);
  carrier.leds.fill(colorRed, 0, n);
  carrier.leds.show();
}

void wifiConnect() {
  Serial.print("Connecting to WIFI (SSID): ");
  Serial.print(wifi_ssid);
  while (WiFi.begin(wifi_ssid, wifi_pass) != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("WIFI connected.");
}

void mqttConnect() {
  Serial.print("Connecting to MQTT broker: ");
  Serial.println(broker);
  mqttClient.setUsernamePassword(MQTT_USERNAME, MQTT_PASSWORD);
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1)
      ;  // Stop the program from doing anything useful...
  }
  Serial.println("MQTT broker connected.");
}

void motionDetection(int pirVal) {
  if (pirVal == HIGH) {  // There is motion
    timeSinceMotion = 0;
    if (state == LOW) {  // Used to be no motion
      Serial.println("MOTION DETECTED!!--------------------------------");
      delay(50);
      pubOccupiedStatus(1);
      carrier.Buzzer.sound(100);
      delay(1000);
      carrier.Buzzer.noSound();
      state = HIGH;  // For the next time instant, motion has already been detected
    }
  } else {                // There is no motion
    if (state == HIGH) {  // Used to be motion
      if (timeSinceMotion == 0) {
        timeSinceMotion = millis();
        delay(500);
      }
      if ((millis() - timeSinceMotion) >= motionStillAllowance - 6000) {  // +6 seconds for the PIR hardware
        Serial.println("MOTION ENDED!------------------------------------");
        delay(50);
        pubOccupiedStatus(0);
        carrier.Buzzer.sound(200);
        delay(2000);
        carrier.Buzzer.noSound();
        state = LOW;
        timeSinceMotion = 0;
      }
    }
  }
}

void onMqttMessage(int messageSize) {
  char readChar;
  int readInt = 0;
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  // if topIC IS QUEUE...
  if(mqttClient.messageTopic() == topicQueueSize){
    while (mqttClient.available()) {
      readChar = ((char)mqttClient.read());
      Serial.print("readChar: ");
      Serial.println(readChar);
      readInt = readChar - '0';  // convert char to int
      Serial.print("readInt: ");
      Serial.println(readInt);
    }
    updateQueueSize(readInt);
  }
}

void bathroomReset() {
  nInQueue = 0;
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    carrier.Buzzer.sound(finalMelody[thisNote]);
    carrier.leds.fill(colorBlue, 0, 5);
    carrier.leds.show();
    delay(noteDuration / 3);
    carrier.leds.fill(colorGreen, 0, 5);
    carrier.leds.show();
    delay(noteDuration / 3);
    carrier.leds.fill(colorBlue, 0, 5);
    int pauseBetweenNotes = noteDuration * 1.0;
    delay(noteDuration / 3);
    carrier.leds.fill(colorGreen, 0, 5);
    carrier.leds.show();
    delay(pauseBetweenNotes);
    carrier.leds.show();
    carrier.Buzzer.noSound();
  }
  carrier.leds.fill(colorGreen, 2, 1);
  carrier.leds.show();
}

void playCelebrationMelody() {
  // Playing a song
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    int noteDuration = 1000 / noteDurations[thisNote];
    carrier.Buzzer.sound(celebrationMelody[thisNote]);
    delay(noteDuration);
    int pauseBetweenNotes = noteDuration * 1.0;
    delay(pauseBetweenNotes);
    carrier.Buzzer.noSound();
  }
}
