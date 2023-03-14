var noOfBathrooms;
var userID;

function onConnect(){
    topic =  "sensors/motion";
    console.log("Subscribing to topics")
    client.subscribe(topic);
    client.subscribe("notify")
}

function onConnectionLost(responseObject){
    console.log("ERROR: Connection is lost.")
    if(responseObject !=0){
        console.log("ERROR:"+ responseObject.errorMessage);
    }
}

function onMessageArrived(message){
    console.log("OnMessageArrived: "+message.payloadString);
    const data = JSON.parse(message.payloadString);
    const topic = message.destinationName;
    
    if (topic === "notify") {
        // expects: user id, notify off
        // maybe: bathroom that is free?
        if (message.payloadString === "off") {
            alert("Free bathroom!")
        }
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

function publishNotifyMessage(){
    // publish json with  user ID and on message
    msgJSON = "{"+
        + "userID: " + userID +
        + "notify: 1" +
    "}";
    msg = "on";
    topic = "notify";
    Message = new Paho.MQTT.Message(msgJSON);
    Message.destinationName = topic;
    client.send(Message);
    console.log("Message to topic "+topic+" is sent")
    document.getElementById("notifymsg").innerHTML = "You will be notified when a bathroom is available. Do not refresh the page!";
}

function startConnect(){
    clientID = "Webapp";
    host = "165.22.31.23";   
    port = "8080";
    userId  = "ella";  
    passwordId = "EllaPass3";  
    console.log("Connecting to " + host + "on port " +port);
    console.log("Using the client ID " + clientID);
    client = new Paho.MQTT.Client(host,Number(port),clientID);
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
//     fr.onload = () => {
//         const bathrooms = fr.result.split(/\r?\n/);
//         console.log(bathrooms);
//     };
//     fr.readAsText(this.files[0]);
// }

window.onload = function () {
    // if (!('hasCodeRunBefore' in localStorage)){
        startConnect();
        // readTextFile("/bathrooms.txt");
        noOfBathrooms = 2;
        for (let i = 1; i <= noOfBathrooms; i++){
        document.getElementById("bathrooms").innerHTML += "<tr><td>Bathroom " + i + "</td><td id=\"motion" + i + "\">Occupied?</td><td id=\"temperature" + i + "\">Temperature</td><td id=\"humidity" + i + "\">Humidity</td></tr>"
        }
        // generate random user id
    //     localStorage.setItem("hasCodeRunBefore", true);
    // }
}