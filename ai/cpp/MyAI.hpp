#include <iostream>
#include <string>
#include <vector>
#include "sdk/Client.hpp"

class MyAI : public Client {
    std::string strategy(const std::string &enemyChoice) override {
        // Copy what the enemy chose
        return enemyChoice;
    }
};