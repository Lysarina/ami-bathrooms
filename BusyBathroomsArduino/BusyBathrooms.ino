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
int bathroomID = 000;

// COMMUNICATIONS
char wifi_ssid[] = SECRET_SSID_WIFI;
char wifi_pass[] = SECRET_PASS_WIFI;  // your network password (use for WPA, or use as key for WEP)
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "165.22.31.23";
int port = 1883;
const char topicMotion[] = "sensors/motion";
const char topicTemp[] = "sensors/temperature";
const char topicHumi[] = "sensors/humidity";
const char topicLight[] = "sensors/light";
const char topicBathroom1[] = "queue/bathroom1";
DynamicJsonDocument doc(1024);

//set interval for sending messages (milliseconds)
const long interval = 1000;

// PIR
int pirPin = A5;
int state = LOW;                        // we start with assuming no motion detected
int pirVal = 0;                         // variable for reading the pin status
int minimummSecsLowForInactive = 5000;  // If the sensor reports low for
// more than this time, then assume no activity
long unsigned int timeLow;
boolean takeLowTime;
int calibrationTime = 30;

// VARIABLE DECLARATIONS
int humStickPin = 6;
int humStickVal = 0;
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

// // WEATHER
// const int httpPort = 80;
// String openWeatherMapApiKey = "09e5ac2fcd251ef524531d900c7452bb";
// String city = "Lisbon";
// String countryCode = "PT";
// String jsonBuffer;
// DynamicJsonDocument weatherDoc(1024);
// const char kHostname[] = "https://api.openweathermap.org/";
// byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// // Number of milliseconds to wait without receiving any data before we give up
// const int kNetworkTimeout = 30 * 1000;
// // Number of milliseconds to wait if no data is available before trying again
// const int kNetworkDelay = 1000;

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(humStickPin, INPUT);
  Serial.begin(9600);
  while (!Serial)
    ;
  carrier.begin();
  Serial.println("==========================================");
  // calibrateSensor();
  wifiConnect();
  mqttConnect();
  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);
  mqttClient.subscribe(topicBathroom1);
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

  // publish
  if (abs(temp - tempPrev) >= 0.5) {
    tempPrev = temp;
    pubMQTTJSON(temp, topicTemp, "temperature");
    display_temp_humidity(temp, humi);
  }
  if (abs(humi - humiPrev) >= 0.5) {
    humiPrev = humi;
    pubMQTTJSON(humi, topicHumi, "humidity");
    display_temp_humidity(temp, humi);
  }
  Serial.print(light);
  Serial.print("light");
  Serial.print(lightPrev);
  if (abs(light - lightPrev) >= 500) {
    lightPrev = light;
    pubMQTTJSON(light, topicLight, "light");
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

void pubMQTT(float value, const char topic[]) {
  mqttClient.beginMessage(topic);
  mqttClient.print(value);
  mqttClient.endMessage();
  Serial.println("======================");
  Serial.print("Value: ");
  Serial.println(value);
  Serial.print("published to topic: ");
  Serial.println(topic);
}

void pubMQTT(int value, const char topic[]) {
  mqttClient.beginMessage(topic);
  mqttClient.print(value);
  mqttClient.endMessage();
  Serial.println("======================");
  Serial.print("Value: ");
  Serial.println(value);
  Serial.print("published to topic: ");
  Serial.println(topic);
}

void pubMQTTJSON(float data, const char topic[], const char sensor[]) {
  doc["bathroomID"] = bathroomID;
  // doc["time"] = time();
  doc["data"] = data;
  doc["sensor"] = sensor;
  // JsonArray data = doc.createNestedArray("data");
  // data.add(48.756080);
  // data.add(2.302038);
  char out[128];
  int b = serializeJson(doc, out);
  mqttClient.beginMessage(topic);
  mqttClient.print(out);
  mqttClient.endMessage();
  Serial.print("Published: ");
  Serial.println(out);
}

void pubMQTTJSON(int data, const char topic[]) {
  doc["bathroomID"] = bathroomID;
  // doc["time"] = time();
  doc["data"] = data;
  // JsonArray data = doc.createNestedArray("data");
  // data.add(48.756080);
  // data.add(2.302038);
  char out[128];
  int b = serializeJson(doc, out);
  mqttClient.beginMessage(topic);
  mqttClient.print(out);
  mqttClient.endMessage();
  Serial.println("Stuff has been published.");
  Serial.print("out");
  Serial.println(out);
}

void addToQueue(int n) {
  Serial.print("n recieved in add to queue: ");
  Serial.println(n);
  if (n == 9) {
    // Bathroom is resetting
    bathroomReset();
  } else {
    nInQueue += n;
    if (nInQueue < 6) {
      carrier.Buzzer.sound(200);
      carrier.leds.fill(colorRed, 0, 5);
      carrier.leds.show();
      delay(500);
      carrier.leds.fill(colorBlue, 0, 5);
      carrier.leds.show();
      delay(500);
      carrier.Buzzer.noSound();
    } else {
      // More than 5 people are waiting!
      playCelebrationMelody();  // Playing a melody meaning now it's urgent!
    }
    // Flash with lights
    carrier.leds.fill(colorRed, 0, 5);
    carrier.leds.show();
    delay(500);
    carrier.leds.fill(colorGreen, 0, 5);
    carrier.leds.show();
    delay(500);
    carrier.leds.fill(colorRed, 0, 5);
    carrier.leds.show();
    delay(500);
    carrier.leds.fill(colorGreen, 0, 5);
    carrier.leds.show();
    delay(500);
    carrier.leds.fill(colorRed, 0, nInQueue);
    carrier.leds.show();
  }
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
      pubMQTTJSON(1, topicMotion, "motion");
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
      if ((millis() - timeSinceMotion) >= 20000) { // millis() øker, timeSinceMotion konstant
        Serial.println("MOTION ENDED!------------------------------------");
        delay(50);
        pubMQTTJSON(0, topicMotion, "motion");
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
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  char readChar;
  int readInt = 0;
  while (mqttClient.available()) {
    readChar = ((char)mqttClient.read());
    Serial.print("readChar: ");
    Serial.println(readChar);
    readInt = readChar - '0';  // convert char to int
    Serial.print("readInt: ");
    Serial.println(readInt);
  }
  addToQueue(readInt);
  if(topic == "weather" & weatherNotAnnounced){
    announceWeather();
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


// void willItRain_v3() {
//   if (WiFi.status() == WL_CONNECTED) {
//     // http://api.openweathermap.org/data/2.5/weather?q=Lisbon,PT&APPID=4215e5c8c0b772b0bfa3c4aaaf257893
//     String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

//     httpGETRequest(serverPath.c_str());
//     // Serial.println(jsonBuffer);
//     // deserializeJson(weatherDoc, jsonBuffer);

//     // Serial.print("Temperature: ");
//     // Serial.println(weatherDoc["main"]["temp"]);
//     // Serial.print("Pressure: ");
//     // Serial.println(weatherDoc["main"]["pressure"]);
//     // Serial.print("Humidity: ");
//     // Serial.println(weatherDoc["main"]["humidity"]);
//     // Serial.print("Wind Speed: ");
//     // Serial.println(weatherDoc["wind"]["speed"]);
//   }
// }

// String httpGETRequest(const char* serverpath) {
//   int err = 0;
//   WiFiClient c;
//   HTTPClient http;

//   err = http.begin(kHostname, serverpath);
//   http.GET();

//   if (err == 0) {
//     Serial.println("startedRequest ok");

//     err = http.responseStatusCode();
//     if (err >= 0) {
//       Serial.print("Got status code: ");
//       Serial.println(err);
//       // Usually you'd check that the response code is 200 or a
//       // similar "success" code (200-299) before carrying on,
//       // but we'll print out whatever response we get

//       err = http.skipResponseHeaders();
//       if (err >= 0) {
//         int bodyLen = http.contentLength();
//         Serial.print("Content length is: ");
//         Serial.println(bodyLen);
//         Serial.println();
//         Serial.println("Body returned follows:");

//         // Now we've got to the body, so we can print it out
//         unsigned long timeoutStart = millis();
//         char c;
//         // Whilst we haven't timed out & haven't reached the end of the body
//         while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout)) {
//           if (http.available()) {
//             c = http.read();
//             // Print out this character
//             Serial.print(c);

//             bodyLen--;
//             // We read something, reset the timeout counter
//             timeoutStart = millis();
//           } else {
//             // We haven't got any data, so let's pause to allow some to
//             // arrive
//             delay(kNetworkDelay);
//           }
//         }
//       } else {
//         Serial.print("Failed to skip response headers: ");
//         Serial.println(err);
//       }
//     } else {
//       Serial.print("Getting response failed: ");
//       Serial.println(err);
//     }
//   } else {
//     Serial.print("Connect failed: ");
//     Serial.println(err);
//   }
//   http.stop();
// }


// void willItRain_v2() {
//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.print("Connecting to ");
//     Serial.println(host);
//     if (!wifiClient.connect(host, httpPort)) {
//       Serial.println("Connection failed");
//       return;
//     }
//     String url = "/weatherapi/locationforecast/2.0/compact?lat=38.736789&lon=-9.138877";
//     wifiClient.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: ArduinoWiFi/1.1\r\n" + "Connection: close\r\n\r\n");

//     Serial.println("Request sent.");

//     while (wifiClient.connected()) {
//       String line = wifiClient.readStringUntil('\n');
//       if (line == "\r") {
//         Serial.println("Headers received.");
//         break;
//       }
//     }

//     String response = wifiClient.readString();
//     Serial.println("Response: ");
//     Serial.println(response);
//   }
// }

// void willItRain_v1() {
//   Serial.print("Requesting URL: ");
//   // Serial.println(url);
//   // wifiClient.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "User-Agent: ArduinoWiFi/1.1\r\n" + "Connection: close\r\n\r\n");
//   wifiClient.print(String("GET https://api.met.no/HTTP/1.1?latitude=38.72&longitude=-9.13&start_date=2023-03-13&end_date=2023-03-14&hourly=temperature_2m"));
//   Serial.println("Request sent");
//   while (wifiClient.connected()) {
//     String line = wifiClient.readStringUntil('\n');
//     if (line == "\r") {
//       Serial.println("Headers received");
//       break;
//     }
//     Serial.print("Line:");
//     Serial.println(line);
//   }

//   String response = wifiClient.readString();
//   Serial.print("Response: ");
//   Serial.println(response);

//   if (response.indexOf("\"precipitation\":{\"value\":") != -1) {
//     int start = response.indexOf("\"precipitation\":{\"value\":") + 26;
//     int end = response.indexOf(",", start);
//     String precipitationValue = response.substring(start, end);

//     float precipitation = precipitationValue.toFloat();

//     if (precipitation > 0.3) {
//       Serial.println("It's going to rain today");
//     } else {
//       Serial.println("It's not going to rain today");
//     }
//   }
// }


// void calibrateSensor() {  // NOT USED
//   //give the sensor some time to calibrate
//   Serial.print("Calibrating sensor ");
//   for (int i = 0; i < calibrationTime; i++) {
//     Serial.print(".");
//     delay(1000);
//   }
//   Serial.println(" done");
//   Serial.println("Sensor calibrated.");
//   delay(50);
// }

// void printWIFIData(){ // NOT USED
//   Serial.println("Board Information:");
//   // print your board's IP address:
//   IPAddress ip = WiFi.localIP();
//   Serial.print("IP Address: ");
//   Serial.println(ip);

//   Serial.println();
//   Serial.println("Network Information:");
//   Serial.print("SSID: ");
//   Serial.println(WiFi.SSID());

//   // print the received signal strength:
//   long rssi = WiFi.RSSI();
//   Serial.print("signal strength (RSSI):");
//   Serial.println(rssi);

//   byte encryption = WiFi.encryptionType();
//   Serial.print("Encryption Type:");
//   Serial.println(encryption, HEX);
//   Serial.println();
// }