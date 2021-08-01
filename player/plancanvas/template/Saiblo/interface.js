const DEBUG = true;

// enum GameMode
const OFFLINE = 0;
const ONLINE = 1;
const SPECTATOR = 2;

let gameMode = OFFLINE;

// Interact with saiblo

function startWebSocket(token, tokenB64, secure) {
    const ws = new WebSocket((secure ? "wss://" : "ws://") + token);

    ws.onopen = () => {
        console.log("Connection open...");
        gameMode = ONLINE;
        ws.send(JSON.stringify({request: "connect", token: tokenB64}));
    };

    ws.onmessage = (event) => {
        const {request, content} = JSON.parse(event.data);
        if (request === "action") {
            parseOnline(content);
        }
    };

    ws.onclose = () => {
        console.log("Connection close");
    };

    return (contentObj) => {
        ws.send(JSON.stringify({request: "action", token: tokenB64, content: JSON.stringify(contentObj)}));
    };
}

let sendMsg = (contentObj) => {
    console.log("WebSocket not open yet.");
};

if (DEBUG) {
    const token = "127.0.0.1:9010/1/Aglove/1";
    sendMsg = startWebSocket(token, btoa(token), false);
}

let notifyCanvasHeight = (height) => {
    if (!DEBUG) {
        console.log('Father website is not ready yet.');
    }
};

function readMsg(event) {
    const msg = event.data;
    notifyCanvasHeight = (height) => {
        event.source.postMessage({message: "resized", height}, event.origin);
    };
    if (msg.message === "init_replay_player") { // 离线播放器
        gameMode = OFFLINE;
        const reader = new FileReader();
        reader.onloadend = () => {
            parseOffline(reader.result, (nof) => {
                event.source.postMessage({message: "init_successfully", number_of_frames: nof}, event.origin);
            });
        };
        reader.readAsText(msg.replay_data);
    } else if (msg.message === "load_frame") {
        loadFrame(msg.index);
    } else if (msg.message === "load_next_frame") {
        loadNextFrame();
    } else if (msg.message === "init_player_player") { // 在线播放器
        gameMode = ONLINE;
        sendMsg = startWebSocket(atob(msg.token), msg.token, true);
    } else if (msg.message === "init_spectator_player") { // 观战模式
        gameMode = SPECTATOR;
    }
}
