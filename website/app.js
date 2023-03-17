const host = "165.22.31.23";   
const port = "8080";
const userId  = "ella";  
const passwordId = "EllaPass3";

var noOfBathrooms;
var userID;
var inQueue;
var queueSize;

function onConnect(){
    var topics = ["occupancy", "sensors/light", "sensors/temperature", "sensors/humidity", "queue"]
    for (let i = 0; i < topics.length; i++){
        client.subscribe(topics[i])
        console.log("Subscribed to topic " + topics[i])
    }
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
            if (inQueue && data.waiting == 0) {
                if (data.userID === userID) {
                    alert("Bathroom " + data.bathroomID + " is now free!");
                    document.getElementById("notifyMsg").innerHTML = "";
                    document.getElementById("queueWarning").innerHTML = "";
                    document.getElementById("alreadyInQueue").innerHTML = "";
                    inQueue = false;
                } else {
                    document.getElementById("queueWarning").innerHTML = "There are still people before you in the queue."
                }
            } else if (data.waiting == 0) {
                console.log("Queue size " + data.queueSize);
                queueSize = data.queueSize;
                if (data.queueSize > 0) {
                    document.getElementById("queueWarning").innerHTML = "There are " + queueSize + " people in queue.";
                } else {
                    document.getElementById("queueWarning").innerHTML = "";
                }
            } else {
                if (queueSize == 0){
                    document.getElementById("queueWarning").innerHTML = "There are people in queue.";
                } else {
                    document.getElementById("queueWarning").innerHTML = "There are " + queueSize + " people in queue.";
                }
            }
        } else if (topic === "occupancy") {
            if (data.occupancy == 1){
                document.getElementById("occupancy" + data.bathroomID).innerHTML = "Occupied";
                document.getElementById("occupancy" + data.bathroomID).style.color = "red";
            } else {
                document.getElementById("occupancy" + data.bathroomID).innerHTML = "Free";
                document.getElementById("occupancy" + data.bathroomID).style.color = "green";
            }
        } else if (topic === "sensors/light") {
            var lights;
            if (data.data>700){
                lights = "On"
            } else {
                lights = "Off"
            }
            document.getElementById(data.sensor + data.bathroomID).innerHTML = lights;
        } else {
            document.getElementById(data.sensor + data.bathroomID).innerHTML = data.data;
        }
    } catch (error) {
        console.log("Error receiving message: " + error);
    }
}

function startDisconnect(){
    client.disconnect();
    console.log("Disconnected from broker");
}

function publishQueueMessage(){
    if (!inQueue) {
        // publish json with user ID and on message
        msgJSON = "{" +
            "\"userID\": \"" + userID + "\", " +
            "\"waiting\": 1," +
            "\"bathroomID\": 0," +
            "\"queueSize\": 0" +
        "}";
        topic = "queue";
        Message = new Paho.MQTT.Message(msgJSON);
        Message.destinationName = topic;
        client.send(Message);
        console.log("Message to topic "+topic+" is sent");
        inQueue = true;
        document.getElementById("notifyMsg").innerHTML = "You will be notified when a bathroom is available. Do not refresh the page!";
    } else {
        document.getElementById("alreadyInQueue").innerHTML = "You are already in the queue. Please wait.";
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

    startConnect();
    noOfBathrooms = 2;
    for (let i = 1; i <= noOfBathrooms; i++){
        document.getElementById("bathrooms").innerHTML += "<tr><td>Bathroom " + i + "</td>" + 
            "<td id=\"occupancy" + i + "\">No value</td>" + 
            "<td id=\"temperature" + i + "\">No value</td>" + 
            "<td id=\"humidity" + i + "\">No value</td>" + 
            "<td id=\"light" + i + "\">No value</td>" +
            "</tr>"
    }
}