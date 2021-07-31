#ifndef SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H
#define SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H

#include <fstream>
#include <random>
#include "../jsoncpp/json/json.h"

enum PlayerStatus {
    FAIL = 0,
    AI = 1,
    HUMAN = 2,
};

class BaseLogic {
    std::default_random_engine rng{std::random_device()()};

    std::string replayLocation;

    int state{1};

    int listenTarget{-1};

    bool gameOver{};

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

    /**
     * Executed after receiving metadata and before entering the major loop.
     *
     * You can send messages to arbitrary players BUT MAY NOT LISTEN to any one.
     */
    virtual void prepare() = 0;

    /**
     * Executed before `handleLogic()`.
     *
     * This determines the player you listen to during execution of `handleLogic()`.
     *
     * @return player ID that you will be listening to, or -1 if none.
     */
    virtual int setListenTarget() = 0;

    /**
     * Handle game logic as well as sending and receiving messages from players.
     *
     * Note that timing of the target player starts as soon as the first message is sent.
     */
    virtual void handleLogic() = 0;

public:
    void run() {
        receiveMetadata();
        prepare();
        while (!gameOver) {
            listenTarget = setListenTarget();
            handleLogic();
            ++state;
        }
    }
};

#endif //SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H
