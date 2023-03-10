import paho.mqtt as mqtt

CLIENT_NAME = "subscriber"
BROKER_URL = "mqtt.eclipseprojects.io"
BROKER_PORT = 1883

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("/motion")

def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    if msg.topic == "/motion":
        print("motion")

def init():
    client = mqtt.client.Client(CLIENT_NAME)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER_URL, BROKER_PORT, 60)

    client.loop_forever()

if __name__ == "__main__":
    init()