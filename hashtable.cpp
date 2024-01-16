#include <mem.h>
#include "hashtable.h"

//------------------------------------------------------------------
Hashtable::Hashtable(Logger *logger) {
    table = new TNode*[hashTableSizeX*hashTableSizeO];
    memset(table, 0, sizeof(TNode**)*hashTableSizeX*hashTableSizeO);
    this->logger = logger;
    medRating = 0;
    lastMoveRatingDiff = 0;
}
/*
//------------------------------------------------------------------
void Hashtable::put(TNode *node) {

    THash hashX = node->hashCodeX;
    unsigned long h1 = hashX / hashTableSizeZ;
    unsigned long zIndex = hashX % hashTableSizeZ;
    h1 = h1 % hashTableSizeO;
    unsigned long h2 = node->hashCodeO % hashTableSizeO;
    unsigned long index = h1*hashTableSizeO + h2;

    TNode *array = table[index];
    if (array == NULL) {
        table[index] = array = new TNode[hashTableSizeZ];
        memset(array, 0, sizeof(TNode)*hashTableSizeZ);
    }

    TNode *curr = &(array[zIndex]);

    while (curr->hashCodeX != 0) {
//        if (array[0]->hashCodeX == node->hashCodeX && array[0]->hashCodeO == node->hashCodeO) {
//          return;
//        }
        curr = &(curr[0]->next);
    }
    curr[0] = node;
}
*/

//------------------------------------------------------------------

void Hashtable::setLastMoveRating(TRating r) {
    lastMoveRatingDiff = medRating - r;
    if (lastMoveRatingDiff < 0) lastMoveRatingDiff = -lastMoveRatingDiff;
}

//------------------------------------------------------------------
TNode *Hashtable::getOrCreate(THash hX, THash hO, int age, bool &created) {
    THash t1 = hO - 1;
    THash t2 = hX - 1;
    unsigned long h1 = t1 / hashTableSizeZ;
    unsigned long zIndex = t1 % hashTableSizeZ;
    h1 = h1 % hashTableSizeX;
    unsigned long h2 = t2 % hashTableSizeO;
    unsigned long index = h1*hashTableSizeO + h2;

    TNode *array = table[index];
    bool m2 = false;
    bool m3 = false;

//    if (hX ==0 || hO ==0) {
//        logger->error("zero hashcode");
//    }

//    midRating


    if (array == NULL) {
        table[index] = array = new TNode[hashTableSizeZ];
        memset(array, 0, sizeof(TNode)*hashTableSizeZ);
    }

    TNode *curr = &(array[zIndex]);
    TNode *choosen = 0;
    int maxDeviate = 0;

    while (curr->hashCodeX != 0) {
        if (curr->hashCodeX == hX && curr->hashCodeO == hO) {
          if (age == curr->age) {
                logger->hit();
                created = false;
                return curr;
          } else {
                m3 = true;
                logger->error("age collision");
          }
        } else {
                m2 = true;
                int expected = curr->age % 2 ? -medRating : medRating;
                int deviate = expected - curr->rating;

                if (deviate > maxDeviate && deviate > RATING_OVERWRITE_DELTA && curr->totalChilds == 0 && lastMoveRatingDiff <= SUPPRESS_OVERWRITE_DELTA){// && curr->age>=11 && curr->age>=age) {
                        maxDeviate = deviate;
                        choosen = curr;
                }

        }

        if (curr->next == NULL) {
                if (choosen != 0) {
                        curr = choosen;
                        logger->hashOverwrite();
                } else {
                        curr->next = new TNode();
                        curr = curr->next;
                }
                break;
        } else {
                curr = curr->next;
        }
    }

    curr->hashCodeX = hX;
    curr->hashCodeO = hO;
    curr->age=age;
    created = true;

    if (m3) {
        logger->missAge();
    } else if (m2) {
        logger->missHash();
    } else {
        logger->missIndex();
    }
    return curr;
};
//------------------------------------------------------------------
TNode *Hashtable::get(THash hX, THash hO, int age) {
    THash t1 = hO - 1;
    THash t2 = hX - 1;
    unsigned long h1 = t1 / hashTableSizeZ;
    unsigned long zIndex = t1 % hashTableSizeZ;
    h1 = h1 % hashTableSizeX;
    unsigned long h2 = t2 % hashTableSizeO;
    unsigned long index = h1*hashTableSizeO + h2;

    TNode *array = table[index];
    bool m2 = false;
    bool m3 = false;

    if (hX ==0 || hO ==0) {
        logger->error("zero hashcode");
    }

    if (array != NULL) {
      TNode *curr = &(array[zIndex]);
      while (curr != NULL) {
        if (curr->hashCodeX == hX && curr->hashCodeO == hO) {
          if (age == curr->age) {
                        logger->hit();
                        return curr;
          } else {
                m3 = true;
          }
        } else {
                m2 = true;
        }
        curr = curr->next;
      }
    }

    if (m3) {
        logger->missAge();
    } else if (m2) {
        logger->missHash();
    } else {
        logger->missIndex();
    }
    return NULL;
};



