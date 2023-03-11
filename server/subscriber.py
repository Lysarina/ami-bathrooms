import paho.mqtt.client as mqtt

CLIENT_NAME = "subscriber"
#BROKER_URL = "localhost"
BROKER_URL = "165.22.31.23"
USERNAME = "hallvard"
PASSWORD = "HallvardPass3"
BROKER_PORT = 1883
TOPICS = ["sensors/motion", "sensors/temperature", "sensors/humidity", "sensors/light", "queue/bathroom1"]




def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    list(map(client.subscribe, TOPICS))
    print("subbed_topics", TOPICS);

def on_message(client, userdata, msg):
    print(f"New message on: {msg.topic}, msg.payload: {msg.payload}")

def init():
    client = mqtt.Client(CLIENT_NAME)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_URL, BROKER_PORT, 60)

    client.loop_forever()

if __name__ == "__main__":
    init()