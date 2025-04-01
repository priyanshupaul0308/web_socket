const WebSocket = require("ws");
const net = require("net");

const wss = new WebSocket.Server({ port: 3000 });
const tcpClient = new net.Socket();

tcpClient.connect(12345, "127.0.0.1", () => {
    console.log("Connected to C++ TCP server");
});

let clients = {}; // Track connected WebSocket users

wss.on("connection", (ws) => {
    let userName = "";

    ws.on("message", (message) => {
        const data = JSON.parse(message);

        if (data.type === "join") {
            userName = data.username;
            clients[userName] = ws;
            console.log(`User ${userName} joined the chat`);
            broadcastUserList();
            return;
        }

        if (data.type === "chat") {
            console.log("Message from Web UI:", data.message);
            broadcastMessage(userName, data.message);
            tcpClient.write(`${userName}: ${data.message}\n`); // Send message to C++ TCP server
        }
    });

    ws.on("close", () => {
        delete clients[userName];
        console.log(`User ${userName} disconnected`);
        broadcastUserList();
    });
});

tcpClient.on("data", (data) => {
    console.log("Received from C++ Server:", data.toString());
    broadcastMessage("Server", data.toString().trim());
});

function broadcastUserList() {
    const userList = JSON.stringify({ type: "userList", users: Object.keys(clients) });
    for (let client in clients) {
        clients[client].send(userList);
    }
}

function broadcastMessage(sender, message) {
    const chatMessage = JSON.stringify({ type: "chat", username: sender, message: message });
    for (let client in clients) {
        // Send the message to all clients except the sender
        if (clients[client] !== sender) {
            clients[client].send(chatMessage);
        }
    }
}

