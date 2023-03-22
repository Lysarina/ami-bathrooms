# Busy Bathrooms

The project has three main components: the Arduino, that reads and publishes sensor data, the server, which hosts the Mosquitto broker as well as a Python server (which serves the website), and the website.

The solution does not need  

Python 3.8.10

## Mosquitto broker


## Server

The server is a DigitalOcean droplet with IP 165.22.31.23.

## Website

The website is running on 165.22.31.23:8000.

The website can also be run locally. This is done by running

```python3 -m http.server 8000```

and

```python3 mediator.py```

in the website folder, and then accessing the website via localhost:8000.

### Testing

To try out the functionality of the website, if the arduino isn't running, run

```python3 dummy_publisher.py```

in the test folder.

The test folder also contains two bash files for freeing and occupying bathroom 2, that can be run with

```bash freebathroom2.sh```

and

```bash occupybathroom2.sh```

respectively.