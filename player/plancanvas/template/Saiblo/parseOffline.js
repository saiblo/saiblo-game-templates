let replay = [];

let currFrame = 0;

// 注意data是字符串
function parseOffline(data, numberOfFrameIs) { // 离线播放器处理回放文件
    replay = JSON.parse(data);
    numberOfFrameIs(replay.length);
    currFrame = 0;
    loadFrame(0);
}

function loadNextFrame() {
    ++currFrame;
    // drawNextFrame();
}

function loadFrame(frameId) {
    currFrame = frameId;
    // drawFrame(frameId);
}