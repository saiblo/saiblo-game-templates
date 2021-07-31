#ifndef SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H
#define SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H

#include <fstream>
#include "../jsoncpp/json/json.h"

enum PlayerStatus {
    FAIL = 0,
    AI = 1,
    HUMAN = 2,
};

class BaseLogic {
    std::string replayLocation;

    static Json::Value listen() {
        // Receive message header from judger
        uint8_t head[4];
        scanf("%4c", head);
        uint32_t len = head[3] | (head[2] << 8) | (head[1] << 16) | (head[0] << 24);

        // Receive message body from judger
        std::stringstream msg_stream;
        for (int i = 0; i < len; i++) msg_stream.put((char) getchar());

        // Convert message to Json result
        Json::Value result;
        msg_stream >> result;
        return result;
    }

    void receiveMetadata() {
        Json::Value v = listen();

        // Save player status
        for (const auto &player:v["player_list"]) {
            playerStatus.push_back(static_cast<PlayerStatus>(player.asInt()));
        }

        // Save replay location
        replayLocation = v["replay"].asString();
    }

protected:
    std::vector<PlayerStatus> playerStatus;

    void writeTextToReplay(const std::string &text) {
        std::ofstream replay(replayLocation);
        replay << text;
        replay.close();
    }

    void writeJsonToReplay(const Json::Value &json) {
        std::ofstream replay(replayLocation);
        replay << json;
        replay.close();
    }

public:
    void run() {
        receiveMetadata();
    }
};

#endif //SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H
