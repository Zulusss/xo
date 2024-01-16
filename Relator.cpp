//---------------------------------------------------------------------------


#pragma hdrstop

#include "Relator.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

Relator::Relator(SimplyNumbers* sn, Hashtable* ht) : Evaluator(sn, ht){};

TNode* Relator::getChild(TNode *parent, TMove childMove) {
   THash hashCodeX = parent->hashCodeO;
   THash hashCodeO = parent->hashCodeX * simplyGen->getHash(childMove);
   return movesHash->get(hashCodeX, hashCodeO, parent->age + 1);
}

void Relator::calculateChilds() {
    childs.count = 0;
    TNode *node = current()->node;
    bool mode1 = gameMode == 1 &&  count == 2;
    for (TMove i = 0; i < TOTAL_CELLS; ++i) {
        if (mode1 ? kl[i]<=1 && isPerspectiveChildMode1(i) : isPerspectiveChild(i)) {
            TNode* n = getChild(node, i);
            if (n != NULL) {
                childs.move[childs.count] = i;
                childs.node[childs.count++] = n;
            }
        }
    }
    /*
    if (childs.count == 0) {

    for (int i = 0; i < TOTAL_CELLS; ++i) {
        if (kl[i]==1) {
            TNode* n = getChild(node, i);
            if (n != NULL) {
                childs.move[childs.count] = i;
                childs.node[childs.count++] = n;
            }
        }
    }
      return;
    }*/
};



//=============================================================================
void Relator::updateParents(int addedChilds) {
    updateParents(current()->node, 0, 0, true, true, count-1, addedChilds);
};

//=============================================================================
//Private recursive method, used to update parent nodes of "node" by stages:
//"depth" parameter sets which parent level to update (0-parents, 1-grandparents and so on)
//"removed" parameter sets a number of moves that was virually removed when calculating parent
//"max" - maximum removable index
void Relator::updateParents(TNode *node, int removed, int removedFromEnd,
                bool onlyLastRemoved, bool updateRating, int max, int addedChilds) {
    for(int i = max; i > 0; i -= 2) {
        TNode *parent = getParent(node, history[i].move);
        if (parent != NULL) {
            updateRating = updateNode(parent, node, updateRating, addedChilds, removedFromEnd);
            updateParents(parent, removed+1,
                onlyLastRemoved&& i == max
                    ? removedFromEnd+1
                    : removedFromEnd,
                onlyLastRemoved&& i == max
                    ? true
                    : false,
                updateRating, i-1, addedChilds);
//        } else {
//                if (i == max && onlyLastRemoved) {
//                        logger->error("no parent found");
//                        getParent(node, history[i].move);
//                }
        }
    }
};

bool Relator::updateNode(TNode *node, TNode *from, bool updateRating, int addedChilds, int removedFromEnd) {
    bool ratingUpdated = false;
//    if (!node->fixedRating && (updateRating || addedChilds == 0)) {
    if ((updateRating || addedChilds == 0)) {
        short int max_rating = -32600;

        bool fromFound = false;
        if (gameMode == 1 && node->age == 1) {
                for (TMove i = 0; i < TOTAL_CELLS; ++i) {
                    if (isPerspectiveChildMode1(i)) {
                        TNode *child = getChild(node, i);
                        if (child == from) {
                                fromFound = true;
                        }
                        if (child != NULL && child->rating > max_rating) {
                            max_rating = child->rating;
                        }
                    }
                }

        } else {/*
                for(int i = 0; i<count-removedFromEnd; ++i)
                    for(int j = 0; j<history[i].enCount; ++j) {
                        TNode *child = getChild(node, history[i].en[j]);
                        if (child == from) {
                                fromFound = true;
                        }
                        if (child != NULL && child->rating > max_rating) {
                            max_rating = child->rating;
                        }
                }*/
                for (TMove i = 0; i < TOTAL_CELLS; ++i) {
                    if (kl[i]!=0) {
                        TNode *child = getChild(node, i);
                        if (child == from) {
                                fromFound = true;
                        }
                        if (child != NULL && child->rating > max_rating) {
                            max_rating = child->rating;
                        }
                    }
                }
        }


        if (fromFound==false) {
                logger->error("from not found");
                if (from->rating > max_rating) {
                    max_rating = from->rating;
                }
                //updateNode(node, from, updateRating, addedChilds, removedFromEnd);
        }

        if (max_rating > 30000) --max_rating;
        else if (max_rating < -30000) ++max_rating;

        if (node->rating != -max_rating) {
            TRating absRatingOld = node->rating;
            TRating absRating = node->rating = -max_rating;
            if (node->age == 0) {
                movesHash->medRating = absRating;
            }

            if (node->totalChilds >= BIG_PARENT) {
                if (absRatingOld < 0) absRatingOld = -absRatingOld;
                if (absRating < 0) absRating = -absRating;

                if (absRatingOld < CULL_RATING1 && absRating >= CULL_RATING1) {
                        ++logger->bigParentsCulled1;
                        if (node->totalChilds >= BIG_GRAND_PARENT) {
                                ++logger->bigGrandParentsCulled1;
                        }
                } else if (absRatingOld < CULL_RATING2 && absRating >= CULL_RATING2) {
                        ++logger->bigParentsCulled2;
                        if (node->totalChilds >= BIG_GRAND_PARENT) {
                                ++logger->bigGrandParentsCulled2;
                        }
                }
            }
            ratingUpdated = true;
        }
    }
    node->totalChilds += addedChilds;
    return ratingUpdated;
}

//---------------------------------------------------------------------------

TNode* Relator::getParent(TNode *node, TMove move) {

    unsigned long multiplier = simplyGen->getExistingHash(move);
    THash prevPos = node->hashCodeO / multiplier;
    THash remainder = node->hashCodeO % multiplier;

    if (remainder==0) {
        TNode* n = movesHash->get(prevPos, node->hashCodeX, node->age - 1);
        if (n != NULL || multiplier != 2) {
                return n;
        }
    }
    TNode* ret = NULL;
    unsigned long k = 1, n, t, r3;
    unsigned long m2 = THASH_MAX / multiplier;
    unsigned long r1 = (THASH_MAX % multiplier) + 1;

    for(;k<multiplier;++k) {
        t = r1*k + remainder;
        n = t/multiplier;
        r3 = t%multiplier;
        if (0 == r3) {
            ret = movesHash->get(k*m2 + prevPos + n, node->hashCodeX, node->age - 1);
            break;
        }
    }
    return ret;
};

//----------------------------------------------------------------------------

void Relator::findMovesToExpand() {//TODO use single iteration
    newChilds.count = 0;
    bool mode1 = gameMode == 1 &&  count == 2;
    for (TMove i = 0; i < TOTAL_CELLS; ++i) {
        if ((mode1 ? kl[i]<=1 && isPerspectiveChildMode1(i) : isPerspectiveChild(i))  && isAlllowed(i)) {
                newChilds.move[newChilds.count++] = i;
        }
    }
};

//----------------------------------------------------------------------------
bool Relator::isPerspectiveChild(TMove move) {
    return kl[move]==1;
}

//----------------------------------------------------------------------------
bool Relator::isPerspectiveChildMode1(TMove move) {
        int y = move/15 - 7;
        int x = move%15 - 7;
        if (y<0) y =-y;
        if (x<0) x =-x;
        return (x==3 && y<=3 || y==3 && x<=3);
}

