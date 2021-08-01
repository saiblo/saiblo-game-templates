#include <iostream>
#include <string>
#include <vector>

class Client {
    void sendMsg(const std::string &msg) {
        uint32_t len = msg.length();
        char head[4]{
                static_cast<char>(len),
                static_cast<char>(len >> 8),
                static_cast<char>(len >> 16),
                static_cast<char>(len >> 24),
        };
        puts(head);
        puts(msg.c_str());
        myHistory.push_back(msg);
    }

    std::string readMsg() {
        std::string msg;
        std::getline(std::cin, msg);
        if (msg != "ready") {
            enemyHistory.push_back(msg);
        } else {
            msg = "";
        }
        return msg;
    }

protected:
    std::vector<std::string> myHistory;
    std::vector<std::string> enemyHistory;

    static bool greater(const std::string &l, const std::string &r) {
        return l == "rock" && r == "scissors" ||
               l == "scissors" && r == "paper" ||
               l == "paper" && r == "rock";
    }

    virtual std::string strategy(const std::string &enemyChoice) = 0;

public:
    [[noreturn]] void run() {
        while (true) {
            auto enemyChoice = readMsg();
            auto myChoice = strategy(enemyChoice);
            sendMsg(myChoice);
        }
    }
};