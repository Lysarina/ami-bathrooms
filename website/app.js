var noOfBathrooms;
var userID;
var inQueue;

function onConnect(){
    var topics = ["sensors/motion", "sensors/light", "sensors/temperature", "sensors/humidity", "queue"]
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
    const topic = message.destinationName;
    console.log("Received a message from " + topic + ": " + message.payloadString);
    const data = JSON.parse(message.payloadString);
    // expected fields for sensor data: "sensor" (sensor type), "data" (sensor value), "bathroomID"
    
    if (topic === "queue") {
        if (data.waiting === 0 && inQueue){
        // only relevant to process queue message if user is in queue + bathroom becomes free
        // expects: user id, 0, bathroomID
            if (data.userID === userID) {
                alert("Bathroom " + data.bathroomID + " is now free!");
                document.getElementById("notifyMsg").innerHTML = "";
                document.getElementById("queueWarning").innerHTML = "";
                document.getElementById("alreadyInQueue").innerHTML = "";
                inQueue = false;
            } else {
                document.getElementById("queueWarning").innerHTML = "There are still people before you in line, please wait.";
            }
        }  
    } else if (topic === "sensors/light") {
        var lights;
        if (data.data>0){
            lights = "On"
        } else {
            lights = "Off"
        }
        document.getElementById(data.sensor + data.bathroomID).innerHTML = lights;
    } else {
        document.getElementById(data.sensor + data.bathroomID).innerHTML = data.data;
    }
    
    // if (topic === "sensors/motion") {
    //     // expects: bathroom id, motion
    //     document.getElementById(data.sensor + data.bathroomID).innerHTML = data.data;
    // } else if (topic === "sensors/temperature") {
    //     // expects: bathroom id, temp
    //     document.getElementById("temp" + data.bathroomID).innerHTML = data.data;
    // } else if (topic === "sensors/humidity") {
    //     // expects: bathroom id, humidity
    //     document.getElementById("humidity" + data.bathroomID).innerHTML = data.data;
    // } else if (topic === "notify") {
    //     // expects: user id, notify off
    //     // maybe: bathroom that is free?
    //     if (message.payloadString === "off") {
    //         alert("Free bathroom!")
    //     }
    // }
}

function startDisconnect(){
    client.disconnect();
    console.log("Disconnected from broker");
}

function publishQueueMessage(){
    if (!inQueue) {
        // publish json with  user ID and on message
        msgJSON = "{" +
            "\"userID\": \"" + userID + "\", " +
            "\"waiting\": 1," +
            "\"bathroomID\": 0" +
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
    host = "165.22.31.23";   
    port = "8080";
    userId  = "ella";  
    passwordId = "EllaPass3";  
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

// function readTextFile(file)
// {
//     const fr = new FileReader();
//     fr.onload = () => {True
//         const bathrooms = fr.result.split(/\r?\n/);
//         console.log(bathrooms);
//     };
//     fr.readAsText(this.files[0]);
// }

window.onload = function () {
    // generate random user ID
    userID = "user" + Math.floor(Math.random() * 500);
    console.log("UserID " + userID);
    inQueue = false;

    startConnect();
    // readTextFile("/bathrooms.txt");
    noOfBathrooms = 2;
    for (let i = 1; i <= noOfBathrooms; i++){
        document.getElementById("bathrooms").innerHTML += "<tr><td>Bathroom " + i + "</td>" + 
            "<td id=\"motion" + i + "\">Occupied?</td>" + 
            "<td id=\"temperature" + i + "\">Temperature</td>" + 
            "<td id=\"humidity" + i + "\">Humidity</td>" + 
            "<td id=\"light" + i + "\">Lights</td>" +
            "</tr>"
    }
}