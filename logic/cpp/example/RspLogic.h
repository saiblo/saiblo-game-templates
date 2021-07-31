#ifndef SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H
#define SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H

#include "../sdk/BaseLogic.h"

class RspLogic : public BaseLogic {
    int round = 0;  // BaseLogic has no responsibility for maintaining round information.
    int turn = 0;

    void prepare() override {
        // Do nothing...
    }

    int setListenTarget(int &timeLimit, int &lengthLimit) override {
        // Time and length limits remain default and do not need updating.
        return turn;
    }

    void handleLogic() override {

        turn = 1 - turn;  // Reverse turn.
        if (turn == 0) {
            // Both player 0 and player 1 have given their choices.
            // Increase round counter by 1.
            ++round;
        }
    }
};

#endif //SAIBLO_LOGIC_CPP_TEMPLATE_RSPLOGIC_H
