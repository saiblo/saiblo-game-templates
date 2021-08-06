#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#define DEBUG(msg) { debug(std::string(__FILE__) + "(" + std::to_string(__LINE__) + "):" + msg); }

class Client {
    static void sendMsg(const std::string &msg) {
        uint32_t len = msg.length();
        char head[5]{
                static_cast<char>(len),
                static_cast<char>(len >> 8),
                static_cast<char>(len >> 16),
                static_cast<char>(len >> 24),
        };
        puts(head);
        puts(msg.c_str());
    }

    static std::string readMsg() {
        char head[9];
        scanf("%8c", head);
        int len = std::stoi(head);
        std::stringstream msg_stream;
        for (int i = 0; i < len; i++) msg_stream.put((char) getchar());
        return msg_stream.str();
    }

protected:
    std::vector<std::string> myHistory;
    std::vector<std::string> enemyHistory;

    static void debug(const std::string &msg) {
        std::cerr << msg << std::endl;
    }

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
            if (enemyChoice != "ready") {
                enemyHistory.push_back(enemyChoice);
            } else {
                enemyChoice = "";
            }

            auto myChoice = strategy(enemyChoice);
            myHistory.push_back(myChoice);
            sendMsg(myChoice);
        }
    }
};