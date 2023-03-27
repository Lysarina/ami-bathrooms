import paho.mqtt.client as mqtt
import json

# Class to handle the queue, as well as save latest sensor data to send to newly opened clients

CLIENT_NAME = "mediator"
BROKER_URL = "165.22.31.23"
USERNAME = "hallvard"
PASSWORD = "HallvardPass3"
BROKER_PORT = 1883
TOPICS = ["occupancy", "sensors/light", "sensors/temperature", "sensors/humidity", "queue", "getLatest"]
# also publishes to queue/size, queue/update

queue = []
latest_data = {"out": 0, "queueSize": 0}

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    list(map(client.subscribe, TOPICS))
    print("subbed_topics", TOPICS)

def on_message(client, userdata, msg):
    try:
        print(f"New message on: {msg.topic}, msg.payload: {msg.payload}")
        data = json.loads(msg.payload)
        if msg.topic == "getLatest":
            if data["out"] == 1:
                # if msg comes from a user (out = 1), send saved data 
                print(json.dumps(latest_data))
                client.publish("getLatest", json.dumps(latest_data))
        else:
            id = data["bathroomID"]

            if id not in latest_data.keys():
                latest_data[id] = {
                    "occupancy": "No value",
                    "light": "No value",
                    "temperature": "No value",
                    "humidity": "No value"
                }
            if msg.topic == "queue":
                if data["waiting"] == 1:
                    # someone wants to be in queue
                    queue.append(data["userID"])
                    client.publish("queue/size", len(queue))
                    latest_data["queueSize"] = len(queue)
                    print("Added " + data["userID"] + " to queue, queue length is now " + str(len(queue)))  
                elif data["waiting"] == 2:
                    # someone wants to be removed from queue
                    queue.remove(data["userID"])
                    client.publish("queue/size", len(queue))
                    latest_data["queueSize"] = len(queue)
                    print("Removed " + data["userID"] + " from queue, queue length is now " + str(len(queue)))
                client.publish("queue/update", json.dumps(queue))        
            else:
                if msg.topic == "occupancy":
                    latest_data[id]["occupancy"] = data["occupancy"]
                    if data["occupancy"] == 0 and len(queue) > 0:
                        # bathroom is free and people in queue; send notif to first in queue
                        userID = queue.pop(0)
                        json_dict = {
                            "bathroomID": data["bathroomID"],
                            "userID": userID,
                            "waiting": 0
                        }
                        client.publish("queue", json.dumps(json_dict))
                        client.publish("queue/size", len(queue))
                        client.publish("queue/update", json.dumps(queue))
                        latest_data["queueSize"] = len(queue)
                        print("Bathroom " + str(id) + " is now free; sent notification to user " + userID)
                else:
                    # save sensor data to latest data
                    latest_data[id][data["sensor"]] = data["data"]
    except Exception as e:
        print(e)    


def init():
    client = mqtt.Client(CLIENT_NAME)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_URL, BROKER_PORT, 60)

    client.loop_forever()

if __name__ == "__main__":
    init()