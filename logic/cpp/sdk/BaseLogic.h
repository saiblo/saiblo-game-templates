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

    int timeLimit{3};
    int lengthLimit{1024};

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

    static void send(uint32_t target, const std::string &msg) {
        uint32_t len = msg.length();
        char head[8]{
                static_cast<char>(len),
                static_cast<char>(len >> 8),
                static_cast<char>(len >> 16),
                static_cast<char>(len >> 24),
                static_cast<char>(target),
                static_cast<char>(target >> 8),
                static_cast<char>(target >> 16),
                static_cast<char>(target >> 24),
        };
        puts(head);
        puts(msg.c_str());
    }

    void updateLimits() const {
        Json::Value v;
        v["state"] = 0;
        v["time"] = timeLimit;
        v["length"] = lengthLimit;
        send(-1, Json::FastWriter().write(v));
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

    static void singleSend(int target, const std::string &msg) {
        if (target < 0) return;
        send(target, msg);
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
     * You can also update the timeLimit and lengthLimit if you like.
     *
     * @param timeLimit    reference to the timeLimit
     * @param lengthLimit  reference to the lengthLimit
     * @return player ID that you will be listening to, or -1 if none.
     */
    virtual int setListenTarget(int &timeLimit, int &lengthLimit) = 0;

    /**
     * Handle game logic as well as sending and receiving messages from players.
     *
     * Note that timing of the target player starts as soon as the first time `anySend()` is called.
     */
    virtual void handleLogic() = 0;

public:
    void run() {
        receiveMetadata();
        prepare();
        while (!gameOver) {
            // Set listen target and update limits if necessary.
            int lastTimeLimit = timeLimit;
            int lastLengthLimit = lengthLimit;
            listenTarget = setListenTarget(timeLimit, lengthLimit);
            if (lastTimeLimit != timeLimit || lastLengthLimit != lengthLimit) {
                updateLimits();
            }

            // Handle game logic
            handleLogic();
            ++state;
        }
    }
};

#endif //SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H
