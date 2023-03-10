# import eventlet
from flask import Flask, render_template
from flask_mqtt import Mqtt
# from flask_socketio import SocketIO

#eventlet.monkey_patch()

app = Flask(__name__)
app.config['MQTT_BROKER_URL'] = 'localhost'
app.config['MQTT_BROKER_PORT'] = 1883
app.config['MQTT_REFRESH_TIME'] = 1.0

mqtt = Mqtt(app)
#socketio = SocketIO(app)

bathrooms = ["", "", ""]

@mqtt.on_connect()
def handle_connect(client, userdata, flags, rc):
    print("owo")
    mqtt.subscribe('sensors/motion')

@mqtt.on_message()
def handle_mqtt_message(client, userdata, message):
    global bathrooms
    #print(message.payload.decode())
    data = dict(
        topic=message.topic,
        payload=message.payload.decode()
    )
    bathrooms[0] = "Bathroom " + message.payload.decode()
    print(bathrooms)
    # return render_template('index.html', msg=data.payload)
    # emit a mqtt_message event to the socket containing the message data
    #socketio.emit('mqtt_message', data=data)

@app.route("/")
def hello_world():
    #print(latest_msg)
    return render_template('index.html', rooms = bathrooms)

# @app.route("/data")
# def data():
#     return render_template('graphs.html')

#socketio.run(app, host='localhost', port=5000, use_reloader=True, debug=True)

