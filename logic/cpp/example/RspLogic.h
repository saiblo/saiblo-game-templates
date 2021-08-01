#ifndef SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H
#define SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H

#include "../sdk/BaseLogic.h"

class RspLogic : public BaseLogic {
    int turn = 0;

    std::vector<std::string> history;
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
            ss << record << std::endl;
        }
        writeTextToReplay(ss.str());

        std::vector<int> scores;
        scores.push_back(win[0]);
        scores.push_back(win[1]);
        sendGameOverMessage(scores);
    }

    void sendMsgToPlayer() {
        std::string sendContent;
        if (getState() <= 2) {
            sendContent = "ready";
        } else if (turn == 0) {
            sendContent = history[getState() - 2];
        } else {
            sendContent = history[getState() - 4];
        }

        // Following is an example of using `anySend`.
        /* AnyMessages messages;
        messages.emplace_back(turn, sendContent + "\n");
        anySend(messages); */

        // Using `singleSend` in this case is easier.
        singleSend(turn, sendContent + "\n");
    }

    void listenToPlayerResponse() {
        ErrorType errorType;
        int errorPlayer;
        std::string recvContent = getTargetMessage(errorType, errorPlayer);
        if (errorType == NONE) {
            // Error handling is VERY IMPORTANT!!!
            // Even if no error occurs in the process of listening,
            // there can still be errors in the `recvContent` itself!!!
            if (recvContent != "rock" && recvContent != "scissors" && recvContent != "paper") {
                errorType = RE;
                errorPlayer = turn;
            } else {
                history.push_back(recvContent);
            }
        }

        // Terminate game if any error occurs.
        if (errorType != NONE) {
            history.push_back("Player " + std::to_string(errorPlayer) + " error: " + std::to_string(errorType) + " " +
                              recvContent);
            win[1 - errorPlayer] = 1;
            win[errorPlayer] = 0;
            writeReplayAndGameOver();
        }
    }

    void reverseTurn() {
        turn = 1 - turn;  // Change turn.
        if (turn == 0) {
            // Both player 0 and player 1 have made their choices.
            auto choices = std::make_pair(history[getState() - 2], history[getState() - 1]);
            if (choices.first == "rock" && choices.second == "scissors" ||
                choices.first == "scissors" && choices.second == "paper" ||
                choices.first == "paper" && choices.second == "rock") {
                ++win[0];
            } else if (choices.first != choices.second) {
                ++win[1];
            }

            if (win[0] >= 3 || win[1] >= 3) {
                history.push_back(std::to_string(win[0]) + ":" + std::to_string(win[1]));
                writeReplayAndGameOver();
            }
        }
    }

    void handleLogic() override {
        // `handleLogic()` does not necessarily have to be composed of these three phases.
        // You can create your own way of logic handling.
        sendMsgToPlayer();
        listenToPlayerResponse();
        reverseTurn();
    }
};

#endif //SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H
