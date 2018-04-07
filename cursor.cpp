//---------------------------------------------------------------------------


//#pragma hdrstop

#include <mem.h>
#include "cursor.h"


//---------------------------------------------------------------------------

//#pragma package(smart_init)

Cursor::CursorHistory::CursorHistory() {
        setmem((void*)en, 0, sizeof(CursorHistory));
};

//-------------------------------  CURSOR -----------------------------------

Cursor::Cursor(SimplyNumbers *simplyGen, Hashtable *movesHash) {
        this->simplyGen = simplyGen;
        this->movesHash = movesHash;
        memset(history,0,sizeof(CursorHistory)*TOTAL_CELLS);
        count = 0;
};

//=============================================================================

inline Cursor::CursorHistory *Cursor::current() {
        return &(history[count-1]);
}

Cursor::CursorHistory *Cursor::getMove(int i) {
        return &(history[i]);
}

TNode *Cursor::getFirstNode() {
        return history[0].node;
}

TNode *Cursor::lastMove() {
        return current()->node;
}

void Cursor::restart() {
  while(count > 1) {
     back();
  }
  //lastmove = first->node;
}



//=============================================================================
//Increment the Cursor to next move N. Returns true on success, otherwise - false.
bool Cursor::forward(TMove N) {

  TNode *node = current()->node;

  THash hashCodeX = node->hashCodeO;
  THash hashCodeO = node->hashCodeX * simplyGen->getHash(N);

  TNode *nextNode = movesHash->get(hashCodeX, hashCodeO, node->age + 1);
  if (nextNode == NULL) {
        return false;//no entry, return error
  }

  return forward(N, nextNode);
}

//=============================================================================
bool Cursor::forward(TMove N, TNode* node) {

//  if (kl[N] > 3) {//TODO
//    return (bool)N / 0;
//  }

  int x = N % 15, y = N / 15;


  //begin: forward cursor
  kl[N] = ((count&1)+1)<<2;
  ++count;
  moves [count] = N;

  CursorHistory *curr = current(), *prev = &(history[count <= 1? 0 : count-2]);
  curr->node = node;
  curr->move = N;

  //begin: update simmetries history
        int d = 0,i;
        d = x==7 ? 0 : (kl[y*15+(14-x)] == kl[N]) ? -1 : 1;
        curr->symmX = prev->symmX + d;

        d = y==7 ? 0 : (kl[(14-y)*15+x] == kl[N]) ? -1 : 1;
        curr->symmY = prev->symmY + d;

        i = (14-y)*15+(14-x);
        d = N==i? 0 : (kl[i] == kl[N]) ? -1 : 1;
        curr->symmXY = prev->symmXY + d;

        d = x==y ? 0 : (kl[x*15+y] == kl[N]) ? -1 : 1;
        curr->symmW = prev->symmW + d;

        i=(14-x)*15+y;
        d = N==i? 0 : (kl[i] == kl[N]) ? -1 : 1;
        curr->symmXW = prev->symmXW + d;

        i=x*15+14-y;
        d = N==i? 0 : (kl[i] == kl[N]) ? -1 : 1;
        curr->symmYW = prev->symmYW + d;

        i=(14-x)*15+14-y;
        d = N==i? 0 : (kl[i] == kl[N]) ? -1 : 1;
        curr->symmXYW = prev->symmXYW + d;
  //end: update symmetries history

  //begin: update "enablers" history
    curr->enCount = 0;

    if (gameMode == 1 &&  lastMove()->age == 1) {
        for (int x1=4; x1<=10; x1++)
            for (int y1=4; y1<=10; y1++)
                if (x1==4||x1==10||y1==4||y1==10) {
                        TMove move = y1*fsize+x1;
                        if (kl[move] == 0 && isAlllowed(move)) {
                            if (curr->enCount >= MAX_ENABLERS) {
                                curr->enCount = curr->enCount; //TODO
                            } else {
                                kl[move] = 1;
                                curr->en[curr->enCount++] = move;
                            }
                        }
                }
    } else if (gameMode == 1 &&  lastMove()->age == 2) {
        enable(curr, x, y, 1);
        enable(curr, 7, 7, 1);
        enable(curr, history[1].move%15, history[1].move/15, 1);
    } else {
        int max = count > 1 ? 2 : 1;
        enable(curr, x, y, max);
    }
  //end: update "enablers" history

  if (current()->node == NULL) {
        return false;
  }

  return true;
};

//=============================================================================

void Cursor::enable(CursorHistory *curr, int x, int y, int maxDistance) {
        for (int d=1; d<=maxDistance; d++)
            for (int i=-1; i<=1; i++)
              for (int j=-1; j<=1; j++)
                if ((!i)&&(!j)) continue;
                else {
                    int x1 = x+i*d, y1 = y+j*d;
                    TMove move = y1*fsize+x1;
                    if (x1>=0 && y1>=0 && x1<fsize && y1<fsize && kl[move] == 0 && isAlllowed(move)) {
                          if (curr->enCount >= MAX_ENABLERS) {
                              curr->enCount = curr->enCount; //TODO
                          } else {
                              kl[move] = 1;
                              curr->en[curr->enCount++] = move;
                        }
                    }
                }

}

//set cursor to one move back
bool Cursor::back() {
  if (count == 0) {
        return false;
  }
  CursorHistory *curr = current();
  kl[curr->move] = 1;
  for(int t=0; t<curr->enCount; t++)
        kl[curr->en[t]] = 0;
  --count;
  if (current()->node == NULL) {
        return false;
  }
  return true;
}

//=============================================================================

bool Cursor::isAlllowed(TMove N) {
    int x = N%fsize - 7;
    int y = N/fsize - 7;
    if ((  history[count-1].symmX  == 0) && x < 0) {
      return false;
    }
    if ((  history[count-1].symmY  == 0 || history[count-1].symmXY  == 0) && y < 0) {
      return false;                }
    if ((  history[count-1].symmW  == 0) && x < y) {
      return false;
    }
    if ((  history[count-1].symmXYW  == 0) && x < -y) {
      return false;
    }
    return allow(N);
};


//------------------------------------------------------------------------------

//bool Cursor::unique(TMove move) {
//        if (history[count-1].symmX && history[count-1].symmY && history[count-1].symmXY && history[count-1].symmW
//                //&& history[count-1].symmXW && history[count-1].symmYW && history[count-1].symmXYW
//                ) {
//                return childCount;
//        }

//                int x = move%fsize - 7;
//                int y = move/fsize - 7;

//                return (history[count-1].symmX  != 0 || x >= 0)
//                              && (history[count-1].symmY  != 0 || y >= 0)
//                              //&& (history[count-1].symmXY != 0 || x + y >= 0)
//                              && (history[count-1].symmW != 0 || x >= y)
//                              //&& (history[count-1].symmXW  != 0 || ((x>=0 || y<=0) && (x<=0 || y>=)0))
//                              //&& (history[count-1].symmYW != 0 || ((x>=0 || y<=0) && (x<=0 || y>=0)))
//                              && (history[count-1].symmXYW != 0 || x + y  >= 0)
//                              && (allow(move));
//}

//------------------------------------------------------------------------------

//initial implementation for supporting of prohibited moves for
//balanced games, currently supported 1st player
//to make 2nd move in central 5x5 square
inline bool Cursor::allow(int move) {
  if (gameMode == 1 && this->lastMove()->age == 1) {
        int x = move % 15 - 7, y = move / 15 - 7;
        return ! (x < 3 && x > -3 && y < 3 && y > -3);
  }
  return true;
}



