
//   if(topic == "weather" & weatherNotAnnounced){
//     announceWeather();
//   }

// void pubMQTT(float value, const char topic[]) {
//   mqttClient.beginMessage(topic);
//   mqttClient.print(value);
//   mqttClient.endMessage();
//   Serial.println("======================");
//   Serial.print("Value: ");
//   Serial.println(value);
//   Serial.print("published to topic: ");
//   Serial.println(topic);
// }

// void pubMQTT(int value, const char topic[]) {
//   mqttClient.beginMessage(topic);
//   mqttClient.print(value);
//   mqttClient.endMessage();
//   Serial.println("======================");
//   Serial.print("Value: ");
//   Serial.println(value);
//   Serial.print("published to topic: ");
//   Serial.println(topic);
// }

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


// void updateQueueSize(int n) {
//   if (n > 9) {
//     // Bathroom is resetting
//     bathroomReset();
//   } else {
//     nInQueue += n;
//     if (nInQueue < 6) {
//       carrier.Buzzer.sound(200);
//       carrier.leds.fill(colorRed, 0, 5);
//       carrier.leds.show();
//       delay(500);
//       carrier.leds.fill(colorBlue, 0, 5);
//       carrier.leds.show();
//       delay(500);
//       carrier.Buzzer.noSound();
//     } 
//     else {
//       // More than 5 people are waiting!
//       playCelebrationMelody();  // Playing a melody meaning now it's urgent!
//     }
//     // Flash with lights
//     carrier.leds.fill(colorRed, 0, n);
//     carrier.leds.show();
//     delay(500);
//     carrier.leds.fill(colorGreen, 0, 5);
//     carrier.leds.show();
//     delay(500);
//     carrier.leds.fill(colorRed, 0, 5);
//     carrier.leds.show();
//     delay(500);
//     carrier.leds.fill(colorGreen, 0, 5);
//     carrier.leds.show();
//     delay(500);
//     carrier.leds.fill(colorRed, 0, nInQueue);
//     carrier.leds.show();
//   }
// }