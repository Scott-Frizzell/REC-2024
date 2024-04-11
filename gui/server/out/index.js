import WebSocket, { WebSocketServer } from "ws";
const buttons = [
    {
        name: "mastercaution",
        state: 0,
        color: "#ffb300"
    },
    {
        name: "lowbattery",
        state: 0,
        color: "#ffb300"
    },
    {
        name: "missedping",
        state: 0,
        color: "#ffb300"
    },
    {
        name: "overspeed",
        state: 0,
        color: "#ffb300"
    },
    {
        name: "brakeoverride",
        state: 0,
        color: "#ffb300"
    },
    {
        name: "missedcheckpt",
        state: 0,
        color: "#ffb300"
    },
    {
        name: "masterwarn",
        state: 0,
        color: "#ff2f00"
    },
    {
        name: "deadbattery",
        state: 0,
        color: "#ff2f00"
    },
    {
        name: "disconnect",
        state: 0,
        color: "#ff2f00"
    },
    {
        name: "stall",
        state: 0,
        color: "#ff2f00"
    },
    {
        name: "brakefailure",
        state: 0,
        color: "#ff2f00"
    },
    {
        name: "emergencystop",
        state: 0,
        color: "#ff2f00"
    }
];
const vehicles = [
    {
        id: 2,
        location: 1,
        nickname: "Cube",
        status: "Online"
    },
    {
        id: 1,
        location: 0,
        nickname: "Circle",
        status: "Offline"
    }
];
const sections = [
    {
        id: 1,
        vehicle: 2,
        brake: {
            status: "open",
            top: 21.3,
            left: 30.2
        }
    },
    {
        id: 2,
        vehicle: 0,
        brake: {
            status: "closed",
            top: 49,
            left: 43
        }
    },
    {
        id: 3,
        vehicle: 0,
        brake: {
            status: "closed",
            top: 61.3,
            left: 15.6
        }
    },
    {
        id: 4,
        vehicle: 0,
        brake: {
            status: "closed",
            top: 62.2,
            left: 7.8
        }
    }
];
let launchesThisHour = 0;
let launchesLastHour = 0;
let timeSinceLastLaunch = 0;
const wss = new WebSocketServer({
    port: 8080
});
wss.on("connection", (ws) => {
    ws.on('error', _errorHandler);
    ws.on('message', _messageHandler);
});
function _errorHandler(err) {
    console.error(err);
}
function _messageHandler(data) {
    let parsed = JSON.parse(data.toString());
    switch (parsed.type) {
        case "GET_DATA":
            sendData();
            break;
    }
}
function sendData() {
    wss.clients.forEach((client) => {
        if (client.readyState !== WebSocket.OPEN)
            return;
        client.send(JSON.stringify({ type: "UPDATE_BUTTONS", data: { buttons } }));
        client.send(JSON.stringify({ type: "UPDATE_VEHICLES", data: { vehicles } }));
        client.send(JSON.stringify({ type: "UPDATE_SECTIONS", data: { sections } }));
        client.send(JSON.stringify({ type: "UPDATE_LAUNCH", data: { launchesThisHour, launchesLastHour, timeSinceLastLaunch } }));
    });
}
function sendButtons() {
    wss.clients.forEach((client) => {
        if (client.readyState !== WebSocket.OPEN)
            return;
        client.send(JSON.stringify({ type: "UPDATE_BUTTONS", data: { buttons } }));
    });
}
function sendLaunch() {
    wss.clients.forEach((client) => {
        if (client.readyState !== WebSocket.OPEN)
            return;
        client.send(JSON.stringify({ type: "UPDATE_LAUNCH", data: { launchesThisHour, launchesLastHour, timeSinceLastLaunch } }));
    });
}
setInterval(() => {
    buttons.forEach((val, idx, arr) => {
        arr[idx].state = 0;
    });
    let rand = Math.floor(Math.random() * 12);
    buttons[rand].state = 1;
    sendButtons();
}, 2000);
setInterval(() => {
    timeSinceLastLaunch++;
    sendLaunch();
}, 1000);
