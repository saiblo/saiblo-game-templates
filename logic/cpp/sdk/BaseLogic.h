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

enum ErrorType {
    NONE = -1,
    RE = 0,
    TLE = 1,
    OLE = 2,
};

typedef std::vector<std::pair<int, std::string>> AnyMessages;

class BaseLogic {
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
    std::default_random_engine rng{std::random_device()()};

    /**
     * Indicate players' status, ordered by the players' ID.
     */
    std::vector<PlayerStatus> playerStatus;

    /**
     * Get current state of the logic.
     *
     * @return  value of `state`
     */
    int getState() const {
        return state;
    }

    /**
     * Write text value to the replay file.
     *
     * Note that the replay file is opened with flag `w` rather than `a`.
     *
     * @param text  the text value to write
     */
    void writeTextToReplay(const std::string &text) {
        std::ofstream replay(replayLocation);
        replay << text;
        replay.close();
    }

    /**
     * Write JSON object to the replay file.
     *
     * Note that the replay file is opened with flag `w` rather than `a`.
     *
     * @param json  the JSON object to write
     */
    void writeJsonToReplay(const Json::Value &json) {
        std::ofstream replay(replayLocation);
        replay << json;
        replay.close();
    }

    /**
     * Send message to a target player.
     *
     * NOTE: THIS DOES NOT RESET TIMING!
     *
     * @param target  the target player ID
     * @param msg     the message to send
     */
    static void singleSend(int target, const std::string &msg) {
        if (target < 0) return;
        send(target, msg);
    }

    /**
     * Send messages to arbitrary players, request judger to listen to
     * the `listenTarget` and reset timing if state increases.
     *
     * @param messages  vector of int-string pairs, indicating the messages
     *                  and their corresponding targets
     */
    void anySend(const AnyMessages &messages) const {
        Json::Value v;
        v["state"] = state;
        v["listen"].resize(0);
        if (listenTarget >= 0) v["listen"].append(listenTarget);
        v["player"].resize(0);
        v["content"].resize(0);
        for (const auto &message:messages) {
            v["player"].append(message.first);
            v["content"].append(message.second);
        }
        send(-1, Json::FastWriter().write(v));
    }

    /**
     * Get one message from the listen target.
     *
     * @param errorType    type of error if any occurs, or `NONE` if none
     * @param errorPlayer  the player that caused the error if any occurs, or -1 if none
     * @return message from the listen target, or error message if any error occurs
     */
    std::string getTargetMessage(ErrorType &errorType, int &errorPlayer) const {
        while (true) {
            Json::Value v = listen();
            if (v["player"].asInt() >= 0) {
                errorType = NONE;
                errorPlayer = -1;
                return v["content"].asString();
            } else {
                Json::Value errorContent;
                Json::Reader().parse(v["content"].asString(), errorContent);
                if (errorContent["state"].asInt() != state) continue; // Special case for TLE
                errorType = static_cast<ErrorType>(errorContent["error"].asInt());
                errorPlayer = errorContent["player"].asInt();
                return errorContent["error_log"].asString();
            }
        }
    }

    /**
     * Send game-over message to judger.
     *
     * As logic will terminate as soon as the message is sent,
     * please make sure that replay data has been written to the replay file before this method is called.
     *
     * @param scores  players' scores, ordered by their indices
     */
    void sendGameOverMessage(const std::vector<int> &scores) {
        Json::Value endInfo;
        for (int i = 0; i < scores.size(); ++i) {
            endInfo[std::to_string(i)] = scores[i];
        }
        Json::Value v;
        v["state"] = -1;
        v["end_info"] = Json::FastWriter().write(endInfo);
        send(-1, Json::FastWriter().write(v));
        gameOver = true;
        exit(0);
    }

    /**
     * Executed after receiving metadata and before entering the major loop.
     *
     * IT IS STRICTLY FORBIDDEN TO CALL `anySend()` OR `getTargetMessage()` IN THIS METHOD!!!
     */
    virtual void prepare() = 0;

    /**
     * Executed before `handleLogic()`.
     *
     * This determines the player you listen to during execution of `handleLogic()`.
     *
     * You can also update the timeLimit and lengthLimit if you like.
     *
     * Note that timing of the target player starts IMMEDIATELY AFTER this method is called.
     *
     * IT IS STRICTLY FORBIDDEN TO CALL `anySend()` OR `getTargetMessage()` IN THIS METHOD!!!
     *
     * @param timeLimit    reference to the timeLimit
     * @param lengthLimit  reference to the lengthLimit
     * @return player ID that you will be listening to, or -1 if none.
     */
    virtual int setListenTarget(int &timeLimit, int &lengthLimit) = 0;

    /**
     * Handle game logic as well as sending and receiving messages from players.
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
            if (listenTarget >= 0) {
                anySend(AnyMessages()); // Start timing of the listenTarget
            }

            // Handle game logic
            handleLogic();
            ++state;
        }
    }
};

#endif //SAIBLO_LOGIC_CPP_TEMPLATE_BASELOGIC_H
