function onConnect(){
    topic =  "sensors/motion";
    // document.getElementById("messages").innerHTML += "<span> Subscribing to topic "+topic + "</span><br>";
    client.subscribe(topic);
}

function onConnectionLost(responseObject){
    document.getElementById("messages").innerHTML += "<span> ERROR: Connection is lost.</span><br>";
    if(responseObject !=0){
        // document.getElementById("messages").innerHTML += "<span> ERROR:"+ responseObject.errorMessage +"</span><br>";
    }
}

function onMessageArrived(message){
    console.log("OnMessageArrived: "+message.payloadString);
    // document.getElementById("messages").innerHTML += "<span> Topic:"+message.destinationName+"| Message : "+message.payloadString + "</span><br>";
    if (message.destinationName === "sensors/motion") {
        document.getElementById("bathroom1").innerHTML = "Bathroom 1: " + message.payloadString;
    }
}

function startDisconnect(){
    client.disconnect();
    document.getElementById("messages").innerHTML += "<span> Disconnected. </span><br>";
}

function publishMessage(){
    msg = "on";
    topic = "notify";
    Message = new Paho.MQTT.Message(msg);
    Message.destinationName = topic;
    client.send(Message);
    document.getElementById("messages").innerHTML += "<span> Message to topic "+topic+" is sent </span><br>";
}

function startConnect(){
    clientID = "Webapp";
    host = "165.22.31.23";   
    port = "8080";
    userId  = "ella";  
    passwordId = "EllaPass3";  
    document.getElementById("messages").innerHTML += "<span> Connecting to " + host + "on port " +port+"</span><br>";
    document.getElementById("messages").innerHTML += "<span> Using the client Id " + clientID +" </span><br>";
    client = new Paho.MQTT.Client(host,Number(port),clientID);
    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;
    client.connect({
        onSuccess: onConnect,
        userName: userId,
        password: passwordId
    });
}