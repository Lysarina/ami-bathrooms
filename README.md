# Busy Bathrooms

The project has three main components: the Arduino, that reads and publishes sensor data, the server, which hosts the Mosquitto broker, a mediator service and a Python server (which serves the website), and the website clients.

If working properly, the project should not need any installations. However, the Python server serving the website is unstable and the website may need to be served locally; in this case Python 3.8.10 should be installed.

## Server

The server is a DigitalOcean droplet with IP 165.22.31.23.

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