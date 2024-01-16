//---------------------------------------------------------------------------

#ifndef GameBoardH
#define GameBoardH
//---------------------------------------------------------------------------

#include "Builder.h"

//can flip board
class GameBoard : public Builder {

public:
        bool *swapX, *swapY, *swapW, *swapXYW;

        GameBoard(SimplyNumbers *simplyGen, Hashtable *movesHash, 
                 bool *swapX, bool *swapY, bool *swapW, bool *swapXYW, int gameMode);

        bool put(TMove N);//can flip board
        bool forceForward(TMove N);
        int move();

};

#endif
 