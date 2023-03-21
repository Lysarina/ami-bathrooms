const host = "165.22.31.23";   
const port = "8080";
const userId  = "ella";  
const passwordId = "EllaPass3";

const noOfBathrooms = 2;

var userID;
var inQueue;
var queueSize;

var freeBathrooms;

function onConnect(){
    document.getElementById("connectionWarning").innerHTML = "";
    // subscribe to topics
    var topics = ["occupancy", "sensors/light", "sensors/temperature", "sensors/humidity", "queue"]
    for (let i = 0; i < topics.length; i++){
        client.subscribe(topics[i])
        console.log("Subscribed to topic " + topics[i])
    }

    // call for latest data; out: 1 signifies this comes from user
    client.subscribe("getLatest")
    msg = new Paho.MQTT.Message("{\"out\": 1}");
    msg.destinationName = "getLatest";
    client.send(msg)
}

function onConnectionLost(responseObject){
    console.log("ERROR: Connection is lost.")
    if(responseObject !=0){
        console.log("ERROR:"+ responseObject.errorMessage);
    }
    document.getElementById("connectionWarning").innerHTML = "Connection to server lost. Please refresh the page.";
}

function onMessageArrived(message){
    try {
        const topic = message.destinationName;
        console.log("Received a message from " + topic + ": " + message.payloadString);
        const data = JSON.parse(message.payloadString);
        
        if (topic === "queue") {
            // queue handling
            if (inQueue) {
                if (data.waiting == 0){
                    // if a bathroom has become free
                    queueSize = data.queueSize;
                    if (data.userID === userID) {
                        alert("Bathroom " + data.bathroomID + " is now free!");
                        document.getElementById("notifyMsg").innerHTML = "";
                        document.getElementById("queueWarning").innerHTML = "";
                        document.getElementById("alreadyInQueue").innerHTML = "";
                        document.getElementById("cancelBtn").style.display = "none";
                        inQueue = false;
                    } else {
                        document.getElementById("queueWarning").innerHTML = "There are people before you in the queue."
                    }
                } else if (data.waiting == 1){
                    // if someone added themselves to queue
                    queueSize += 1;
                } else {
                    // if someone cancelled their queue spot
                    queueSize -= 1;
                }
            } else if (data.waiting == 0) {
                // if a bathroom has become free
                queueSize = data.queueSize;
                if (data.queueSize > 0) {
                    document.getElementById("queueWarning").innerHTML = "There are " + queueSize + " people in queue.";
                } else {
                    document.getElementById("queueWarning").innerHTML = "";
                }
            } else if (data.waiting == 1) {
                // if someone added themselves to queue
                queueSize += 1;
                document.getElementById("queueWarning").innerHTML = "There are " + queueSize + " people in queue.";
            } else {
                // if someone cancelled their queue spot
                queueSize -= 1;
                if (queueSize > 0) {
                    document.getElementById("queueWarning").innerHTML = "There are " + queueSize + " people in queue.";
                } else {
                    document.getElementById("queueWarning").innerHTML = "";
                }                
            }
        } else if (topic === "occupancy") {
            processMotion(data.occupancy, data.bathroomID);
        } else if (topic === "sensors/light") {
            processLight(data.data, data.bathroomID);
        } else if (topic === "getLatest") {
            if (data.out == 0) {
                // out = 0 means this comes from mediator
                queueSize = data.queueSize;
                if (data.queueSize > 0) document.getElementById("queueWarning").innerHTML = "There are " + queueSize + " people in queue.";
                for (let i = 1; i <= noOfBathrooms; i++){
                    var bathroom = data[i.toString()]
                    for (var sensor in bathroom) {
                        if (sensor === "occupancy") {
                            processMotion(bathroom[sensor], i);
                        } else if (sensor === "light") {
                            processLight(bathroom[sensor], i);
                        } else {
                            var sign = "";
                            if (sensor === "temperature") sign = " &#8451;";
                            else if (sensor === "humidity") sign = "%";
                            document.getElementById(sensor + i).innerHTML = Math.round(bathroom[sensor]) + sign;
                        }
                    }
                }
                // after initial data has been received, we get info in real time
                client.unsubscribe("getLatest")
            }             
        } else {
            // temperature or humidity data
            var sign = "";
            if (data.sensor === "temperature") sign = " &#8451;";
            else if (data.sensor === "humidity") sign = "%";
            document.getElementById(data.sensor + data.bathroomID).innerHTML = Math.round(data.data) + sign;
        }
    } catch (error) {
        console.log("Error receiving message: " + error);
    }
}

function processMotion(motion, bID) {
    if (motion == 1){
        document.getElementById("occupancy" + bID).innerHTML = "Occupied";
        document.getElementById("occupancy" + bID).style.color = "red";
        if (freeBathrooms.includes("Bathroom " + bID)) {
            freeBathrooms.splice(indexOf("Bathroom " + bID), 1);
        }
    } else {
        document.getElementById("occupancy" + bID).innerHTML = "Free";
        document.getElementById("occupancy" + bID).style.color = "green";
        if (!freeBathrooms.includes("Bathroom " + bID)) {
            freeBathrooms.push("Bathroom " + bID);
        }
    }
}

function processLight(light, bID) {
    var lights;
    if (light>700){
        lights = "On"
    } else {
        lights = "Off"
    }
    document.getElementById("light" + bID).innerHTML = lights;
}

function startDisconnect(){
    client.disconnect();
    console.log("Disconnected from broker");
}

function publishQueueMessage() {
    if (!inQueue && freeBathrooms.length == 0) {
        // publish json with user ID and waiting 1 (add user to queue)
        msgJSON = "{" +
            "\"userID\": \"" + userID + "\", " +
            "\"waiting\": 1," +
            "\"bathroomID\": 0," +
            "\"queueSize\": 0" +
        "}";
        topic = "queue";
        msg = new Paho.MQTT.Message(msgJSON);
        msg.destinationName = topic;
        client.send(msg);
        console.log("Message to topic "+topic+" is sent");

        inQueue = true;
        document.getElementById("cancelBtn").style.display = "inline";
        document.getElementById("notifyMsg").innerHTML = "You will be notified when a bathroom is available. Do not refresh the page!";
        if (queueSize > 0) document.getElementById("queueWarning").innerHTML = "There are people before you in the queue.";
    } else if (!inQueue && freeBathrooms.length > 0) {
        alert("There are free bathrooms: " + freeBathrooms.toString());
    } else {
        document.getElementById("alreadyInQueue").innerHTML = "You are already in the queue. Please wait.";
    }
}

function publishCancelMessage() {
    if (inQueue) { 
        // publish json with user ID and waiting 2 (remove user from queue)
        msgJSON = "{" +
            "\"userID\": \"" + userID + "\", " +
            "\"waiting\": 2," +
            "\"bathroomID\": 0," +
            "\"queueSize\": 0" +
        "}";
        topic = "queue";
        msg = new Paho.MQTT.Message(msgJSON);
        msg.destinationName = topic;
        client.send(msg);
        console.log("Message to topic " + topic + " is sent");
        
        document.getElementById("cancelBtn").style.display = "none";
        document.getElementById("notifyMsg").innerHTML = "";
        document.getElementById("queueWarning").innerHTML = "";
        document.getElementById("alreadyInQueue").innerHTML = "";
        inQueue = false;
    }
}

function startConnect(){  
    console.log("Connecting to " + host + " on port " +port);
    client = new Paho.MQTT.Client(host, Number(port), userID);
    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;
    client.connect({
        onSuccess: onConnect,
        userName: userId,
        password: passwordId
    });
}

window.onload = function () {
    // generate random user ID
    userID = "user" + Math.floor(Math.random() * 500);
    console.log("UserID " + userID);
    inQueue = false;
    queueSize = 0;
    freeBathrooms = [];

    startConnect();
    // init table with amount of bathrooms
    for (let i = 1; i <= noOfBathrooms; i++){
        document.getElementById("bathrooms").innerHTML += "<tr><td>Bathroom " + i + "</td>" + 
            "<td id=\"occupancy" + i + "\">No value</td>" + 
            "<td id=\"temperature" + i + "\">No value</td>" + 
            "<td id=\"humidity" + i + "\">No value</td>" + 
            "<td id=\"light" + i + "\">No value</td>" +
            "</tr>"
    }
}

window.onunload = function () {
    if (inQueue){
        msgJSON = "{" +
            "\"userID\": \"" + userID + "\", " +
            "\"waiting\": 2," +
            "\"bathroomID\": 0," +
            "\"queueSize\": 0" +
        "}";
        topic = "queue";
        msg = new Paho.MQTT.Message(msgJSON);
        msg.destinationName = topic;
        client.send(msg);
        console.log("Message to topic " + topic + " is sent");
    }
 }

 // send cancel queue AFTER page refresh