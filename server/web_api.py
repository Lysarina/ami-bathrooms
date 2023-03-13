from flask import Flask, render_template
from flask_mqtt import Mqtt

app = Flask(__name__)
# app.config['MQTT_BROKER_URL'] = 'localhost'
app.config['MQTT_BROKER_URL'] = '165.22.31.23'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_REFRESH_TIME'] = 1.0
app.config['MQTT_USERNAME'] = 'ella'
app.config['MQTT_PASSWORD'] = 'EllaPass3' 

mqtt = Mqtt(app)

NO_OF_BATHROOMS = 3
bathrooms = ["Bathroom "] * NO_OF_BATHROOMS

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    mqtt.subscribe('sensors/motion')
    mqtt.subscribe('notify')

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    if (message.topic == "sensors/motion"):
        process_motion(message.payload.decode())
    elif (message.topic == "notify"):
        print(message.payload.decode())

@app.route("/", methods = ['GET'])
def index():
    global bathrooms
    return render_template('index.html', rooms = bathrooms)

@app.route("/", methods = ['POST'])
def notify():
    mqtt.publish("notify", "on")
    return render_template('index.html', rooms = bathrooms)

def process_motion(msg):
    # msg should contain data + some identifier for the bathroom
    global bathrooms
    bathrooms[0] = "Bathroom " + msg
    print(bathrooms)

