//---------------------------------------------------------------------------
#ifndef hashtableH
#define hashtableH
//---------------------------------------------------------------------------
#include "hashtable.h"
#include "tnode.h"
#include "Logger.h"

//103x105x331 ->62 M
//53x1009x331 ->64 M

#define hashTableSizeX 53
#define hashTableSizeO 1009
#define hashTableSizeZ 331

#define RATING_OVERWRITE_DELTA 8000

class Hashtable {
        private:
        TNode **table;
        Logger *logger;

        public:
        TRating medRating;
        Hashtable(Logger *logger);
//        void put(TNode *node);
        TNode *get(THash hX, THash hO, int age);
        TNode *getOrCreate(THash hX, THash hO, int age, bool &created);
};

//---------------------------------------------------------------------------
#endif
