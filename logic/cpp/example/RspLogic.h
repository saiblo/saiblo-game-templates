#ifndef SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H
#define SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H

#include "../sdk/BaseLogic.h"

class RspLogic : public BaseLogic {
    int round = 0;  // BaseLogic has no responsibility for maintaining round information.
    int turn = 0;

    std::vector<std::pair<std::string, std::string>> history;
    int win[2]{};

    void prepare() override {
        // Do nothing...
    }

    int setListenTarget(int &timeLimit, int &lengthLimit) override {
        // Time and length limits remain default and do not need updating.
        return turn;
    }

    void writeReplayAndGameOver() {
        std::stringstream ss;
        for (const auto &record:history) {
            ss << record.first << ", " << record.second << std::endl;
        }
        writeTextToReplay(ss.str());

        std::vector<int> scores;
        scores.push_back(win[0]);
        scores.push_back(win[1]);
        sendGameOverMessage(scores);
    }

    void handleLogic() override {
        // Send
        AnyMessages messages;
        std::string sendContent;
        if (round == 0) {
            sendContent = "ready";
        } else if (turn == 0) {
            sendContent = history[round - 1].second;
        } else {
            sendContent = history[round - 1].first;
        }
        messages.emplace_back(turn, sendContent);
        anySend(messages);  // `singleSend` shouldn't be used as it will not reset timing.

        // Listen
        ErrorType errorType;
        int errorPlayer;
        std::string recvContent = getTargetMessage(errorType, errorPlayer);
        if (errorType == NONE) {
            // Error handling is VERY IMPORTANT!!!
            if (recvContent != "rock" && recvContent != "scissors" && recvContent != "paper") {
                errorType = RE;
                errorPlayer = turn;
            } else {
                if (turn == 0) {
                    history.emplace_back(recvContent, "");
                } else {
                    history[round].second = recvContent;
                }
            }
        }

        // Terminate game if any error occurs.
        if (errorType != NONE) {
            history.emplace_back("Player " + std::to_string(errorPlayer) + " error: " + std::to_string(errorType),
                                 recvContent);
            win[1 - errorPlayer] = 1;
            win[errorPlayer] = 0;
            writeReplayAndGameOver();
        }

        turn = 1 - turn;  // Reverse turn.
        if (turn == 0) {
            // Both player 0 and player 1 have made their choices.
            auto choices = history[round];
            if (choices.first == "rock" && choices.second == "scissors" ||
                choices.first == "scissors" && choices.second == "paper" ||
                choices.first == "paper" && choices.second == "rock") {
                ++win[0];
            } else if (choices.first != choices.second) {
                ++win[1];
            }

            if (win[0] >= 3 || win[1] >= 3) {
                history.emplace_back(std::to_string(win[0]), std::to_string(win[1]));
                writeReplayAndGameOver();
            }

            // Increase round counter by 1.
            ++round;
        }
    }
};

#endif //SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H
