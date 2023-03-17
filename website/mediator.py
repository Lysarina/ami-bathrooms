import paho.mqtt.client as mqtt
import json

CLIENT_NAME = "mediator"
#BROKER_URL = "localhost"
BROKER_URL = "165.22.31.23"
USERNAME = "hallvard"
PASSWORD = "HallvardPass3"
BROKER_PORT = 1883
TOPICS = ["occupancy", "sensors/light", "sensors/temperature", "sensors/humidity", "queue"]

queue = []
latest_data = {}

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    list(map(client.subscribe, TOPICS))
    print("subbed_topics", TOPICS)

def on_message(client, userdata, msg):
    print(f"New message on: {msg.topic}, msg.payload: {msg.payload}")
    data = json.loads(msg.payload)

    if msg.topic == "queue":
        if data["waiting"] == 1:
            queue.append(data["userID"])
            client.publish("queue/size", len(queue))
            print("Added " + data["userID"] + " to queue, queue length is now " + str(len(queue)))
    else:
        id = data["bathroomID"]
        if id not in latest_data.keys():
            latest_data[id] = {
                "occupancy": "No value",
                "light": "No value",
                "temperature": "No value",
                "humidity": "No value"
            }
        if msg.topic == "occupancy":
            latest_data[id]["occupancy"] = data["occupancy"]
            print("Bathroom " + str(id) + ": " + str(data["occupancy"]))
            if data["occupancy"] == 0 and len(queue) > 0:
                userID = queue.pop(0)
                json_dict = {
                    "bathroomID": data["bathroomID"],
                    "userID": userID,
                    "waiting": 0,
                    "queueSize": len(queue)
                }
                client.publish("queue", json.dumps(json_dict))
                client.publish("queue/size", len(queue))
        else:
            latest_data[id][data["sensor"]] = data["data"]     


def init():
    client = mqtt.Client(CLIENT_NAME)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_URL, BROKER_PORT, 60)

    client.loop_forever()

if __name__ == "__main__":
    init()