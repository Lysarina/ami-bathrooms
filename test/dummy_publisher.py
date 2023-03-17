import paho.mqtt.client as mqtt
import json
import random
import time

# Class for trying out the functionality of the website
# Publishes random value for randomly chosen bathroom every 10 seconds

CLIENT_NAME = "publisher"
BROKER_URL = "165.22.31.23"
USERNAME = "hallvard"
PASSWORD = "HallvardPass3"
BROKER_PORT = 1883

no_of_bathrooms = 2

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

def publish(client):
    while True:
        bID = random.randint(1, no_of_bathrooms)
        
        # occupancy
        occupied = random.choice([1, 0])
        data = {
            "occupancy": occupied,
            "bathroomID": bID
        }

        client.publish("occupancy", json.dumps(data))
        print("Published " + json.dumps(data))

        # light
        value = random.random()*1400
        data = {
            "bathroomID": bID,
            "sensor": "light",
            "data": value
        }

        client.publish("sensors/light", json.dumps(data))
        print("Published " + json.dumps(data))

        # temp
        value = random.random()*100
        data = {
            "bathroomID": bID,
            "sensor": "temperature",
            "data": value
        }

        client.publish("sensors/temperature", json.dumps(data))
        print("Published " + json.dumps(data))

        # humidity
        value = random.random()*100
        data = {
            "bathroomID": bID,
            "sensor": "humidity",
            "data": value
        }

        client.publish("sensors/humidity", json.dumps(data))
        print("Published " + json.dumps(data))

        time.sleep(10)

def on_message(client, userdata, msg):
    print(f"New message on: {msg.topic}, msg.payload: {msg.payload}")
    data = json.loads(msg)     

def init():
    client = mqtt.Client(CLIENT_NAME)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_URL, BROKER_PORT, 60)

    publish(client)
    client.loop_forever()

    

if __name__ == "__main__":
    init()