//---------------------------------------------------------------------------


#pragma hdrstop

#include "GameBoard.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)


GameBoard::GameBoard(SimplyNumbers *simplyGen, Hashtable *movesHash, 
                 bool *swapXp, bool *swapYp, bool *swapWp, bool *swapXYWp, int gameMode) : Builder(simplyGen, movesHash, gameMode) {

    swapX = swapXp;
    swapY = swapYp;
    swapW = swapWp;
    swapXYW = swapXYWp;

    bool isCreated;
    TNode *node = movesHash->getOrCreate(1, 1, 0, isCreated);

    forward(112, node);
    expand();
};

bool GameBoard::put(TMove N) {
                int x = N%fsize - 7;
                int y = N/fsize - 7;
                bool symmW = history[count-1].symmW == 0;
                bool swapped = false;

                if (history[count-1].symmX  == 0) {
                    if (x < 0) {
                        x = -x;
                        if (*swapW) {
                                *swapY = !*swapY;
                        } else {
                                *swapX = !*swapX;
                        }
                    }
                }
                if ((  history[count-1].symmY  == 0 || history[count].symmXY  == 0) && y < 0) {
                        y = -y;
                        if (*swapW) {
                                *swapX = !*swapX;
                        } else {
                                *swapY = !*swapY;
                        }
                }

                if ( symmW && !swapped == x < y && x!= y) {
                        int t = x;
                        x = y;
                        y = t;
                        *swapW = !*swapW;
                }

                return forceForward((y+7)*15+(x+7));
};

bool GameBoard::forceForward(TMove N) {
  if (kl[N] > 1) return false;
  if (!allow(N)) return false;
  TNode *node = current()->node;

  THash hashCodeX = node->hashCodeO;
  THash hashCodeO = node->hashCodeX * simplyGen->getHash(N);

  bool created = false;
  TNode *nextNode = movesHash->getOrCreate(hashCodeX, hashCodeO, node->age + 1, created);
  if (created) {
        rate(node, nextNode, N);
  }
  if (forward(N, nextNode)) {
        if (created || 0 == nextNode->totalDirectChilds) {
                expand();
        }
        movesHash->setLastMoveRating(nextNode->rating);
        return true;
  }
  return false;
}


int GameBoard::move() {

  TNode *choosen = 0;

  TNode *node = current()->node;
  int totl = node->totalDirectChilds;
  if (totl == 0) {
        //logger->error("no childs");
        expand();
        //return -32600;
  }


    int choosenMove;
    int rating;
    bool mode1 = gameMode == 1 &&  count == 2;
    for (int i = 0; i < TOTAL_CELLS; ++i) {
        if (mode1 ? kl[i]<=1 && isPerspectiveChildMode1(i) : isPerspectiveChild(i)) {
            TNode* n = getChild(node, i);
            if (n != NULL && (choosen == NULL || rating < n->rating)) {
                choosenMove = i;
                choosen = n;
                rating = choosen->rating;
            }
        }
    }

  if (choosen == NULL) {
         //TODO 
        logger->error("no move");

//        node->rating = -
        updateParents(0);

        return -32600;
  }

  if (node->rating!=-rating) {
         //TODO
         // logger->error("parents updated");
        updateNode(node,choosen,true,0,0);
        updateParents(0);
  }

  forward(choosenMove, choosen);
  movesHash->setLastMoveRating(choosen->rating);
  return rating;
};
