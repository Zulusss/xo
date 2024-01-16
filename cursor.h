//---------------------------------------------------------------------------

#ifndef cursorH
#define cursorH
#include "tnode.h"
#include "SimplyNumbers.h"
#include "Hashtable.h"

#define fsize 15  //field size
#define TOTAL_CELLS 225 //fsize*fsize, cells total

#define MAX_ENABLERS 32
//---------------------------------------------------------------------------

class Cursor {

#define MAX_RELATIVES 224 // == (fsize*fsize - 1)

public:
        class RelativeBucket {
                public:
                int count;
                TNode* node[MAX_RELATIVES];
                TMove move[MAX_RELATIVES];
        };

  struct CursorHistory {
    CursorHistory();
    TMove en[MAX_ENABLERS],//"Enabled" moves buffer
          move;
    int enCount;//count of actually "Enabled" moves
    int symmX, symmY, symmXY, symmW, symmXW, symmYW, symmXYW;
    TNode *node;
    RelativeBucket parents;
    int previousKlValue;
//    bool removed;
  };

  int gameMode;//0 = Go - Moku, 1 = 5-in-a-row, 2 = Renjue
  int count;//count of moves made

  inline CursorHistory *current();
  TNode *getFirstNode();
  TNode *lastMove();
  CursorHistory *getMove(int i);
  void restart();
  bool back();

  Logger *logger;

  Hashtable *movesHash;
  CursorHistory history[TOTAL_CELLS];


  SimplyNumbers *simplyGen;
  TMove moves[fsize*fsize]; //ordered history of moves

  TMove kl[fsize*fsize]; // moves on field
  bool isAlllowed(TMove N);
        inline bool allow(int move);
private:

        void enable(CursorHistory *curr, int x, int y, int maxDistance);
protected:

        //bool unique(TMove move);

        Cursor(SimplyNumbers *simplyGen, Hashtable *movesHash);

        bool forward(TMove N);
        bool forward(TMove N, TNode* node);


};


#endif
