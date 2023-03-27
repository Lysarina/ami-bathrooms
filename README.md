# Busy Bathrooms

The project has three main components: the Arduino, which reads and publishes sensor data and displays queue size information to the bathroom user, the server, which hosts the Mosquitto broker, a mediator service and a Python server (which serves the website), and the website clients.

If working properly, the project should not need any installations. However, the Python server serving the website is unstable and the website may need to be served locally; in this case Python 3.8.10 should be installed.

## Arduino

The hardware consists of an MKR IoT Carrier (version 1) with luminance, temperature and humidity sensors (amongst others which were not used) for environment readings, a buzzer for catching the attention when there's a long queue (>5) for the bathroom, five LEDs displaying the number of people currently in the queue, and a passive infrared (PIR) sensor for detecting if bathroom is in use or not. A MKR Wif1010 board is connected to the IoT carrier, enabling the board to communicate over wifi. 

The Arduino needs to connect to the internet. To do this, modify the "SECRET_SSID_WIFI" and "SECRET_PASS_WIFI" in the arduino_secrets.h file. A username and password for data transmission to the MQTT broker has been kept in the file since the application is for educational purposes and the data transmitted on the topics are not considered sensitive. 

There should be one arduino-device (as described above) deployed in each bathroom of the application. These will all communicate over the same topics to the broker. This is why in the code of each arduino there will be set a bathroom id. There are also the two settable parameteres: motionStillAllowance, specifying the time [ms] the sensor should detect no movement until it should deem the bathroom free, and silentMode, quieting the buzzer.

## Server

The server is a DigitalOcean droplet with IP address 165.22.31.23.

### Mosquitto broker

The Mosquitto broker is running on the droplet. It listens to port 8080 (used by website clients) and port 1883 (used by mediator and arduinos).

### Mediator

A systemd service running on the droplet which handles the queue and saves the latest data from the sensors to send to newly opened clients.

If not running, can be ran locally in the test folder:

```python3 mediator.py```

This requires installing paho-mqtt through

```pip instal paho-mqtt```

### Python server

A systemd service running on the droplet which serves the website at port 8000. However, not very stable; see next section.

## Website

The website can be accessed through 165.22.31.23:8000.

If the website cannot be accessed through this IP, the Python server service is most likely down. In this case, the website can also be run locally through running

```python3 -m http.server 8000```

in the website folder, and then accessing the website via localhost:8000.

### Testing

To try out the functionality of the website, if the arduino isn't running, run

```python3 dummy_publisher.py```

in the test folder.

The test folder also contains two bash files for freeing and occupying bathroom 2, that can be run with

```bash freebathroom2.sh```

and

```bash occupybathroom2.sh```

respectively. These can be used to try out the queue functionality.