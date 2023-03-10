import paho.mqtt.client as mqtt

CLIENT_NAME = "subscriber"
#BROKER_URL = "localhost"
BROKER_URL = "165.232.73.30"
USERNAME = "hallvard"
PASSWORD = "MQTTPass3"
BROKER_PORT = 1883

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("sensors/motion")
    client.subscribe("ScentStick/a/b")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    if msg.topic == "sensors/motion":
        print("motion")

def init():
    client = mqtt.Client(CLIENT_NAME)
    client.username_pw_set(USERNAME, PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_URL, BROKER_PORT, 60)

    client.loop_forever()

if __name__ == "__main__":
    init()