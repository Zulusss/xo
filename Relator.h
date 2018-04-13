//---------------------------------------------------------------------------

#ifndef RelatorH
#define RelatorH
//---------------------------------------------------------------------------
#include "Evaluator.h"
#include "TNode.h"

class Relator : public Evaluator {

        public:
        void calculateChilds();
        RelativeBucket childs;

        void updateParents(int addedChilds);        
//-------------------------------------
        protected:

        class MovesBucket {
                public:
                int count;
                TMove move[MAX_RELATIVES];
        };

        Relator(SimplyNumbers*, Hashtable* );

        MovesBucket newChilds;
        TNode* getChild(TNode *parent, TMove childMove);
        void findMovesToExpand();
        Cursor::RelativeBucket getParents(TNode *node);


//-------------------------------------
        private:

        //these are used fro parent calculation
//        void updateParents(int childsAdded);
        void updateParents(TNode *node, int removed, int removedFromEnd,
                bool onlyLastRemoved, bool updateRating, int max, int addedChilds);

        bool updateNode(TNode *node, bool updateRating, int addedChilds, int removedFromEnd);
        TNode* getParent(TNode *node, TMove move);
        int minRemovedEven;
        int minRemovedOdd;

};

#endif
