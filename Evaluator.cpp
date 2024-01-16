//---------------------------------------------------------------------------


#pragma hdrstop

#include "mem.h"
#include "Evaluator.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

#define linec 54
unsigned char line [2][linec][9]=
{  "XXXXX","88888",//0,1
   "_XXXX_","x_XXX_x","xx_XX_xx","xxx_X_xxx",//2
//  3: close opened 3
   "_8888_","Xooo__","__oooX","To_ooT","Too_oT","oToToTo",
   "oTTooTo","oTooTTo","oooTTTooo","*_ooo_*",
//  4: build closed 4
   "XXXX_","_XXXX","XXX_X","X_XXX","XX_XX",
//  5: build opened 3
   "_XXX__","__XXX_","_XX_X_","_X_XX_","X_X_X_X",
   "X_XX__X","X__XX_X","XX_X__XX","XX__X_XX",
//  9:  close 2 or closed 3
   "TTooo","oooTT","?oToo?","?ooTo?","oToTo","oTToo","ooTTo",
   "_TooT_","TTToo_+","+_ooTTT","ToToT_","_ToToT","_oTTo_",
//  10: build 2  or closed 3
   "_TxTT_","_TTxT_","_X__X_",
   "XXX__","__XXX","_XXX_","X_XX_","_XX_X","XX_X_","_X_XX",
//11: ToT  (for the 2nd move)
   "ToT",
};

Evaluator::Evaluator(SimplyNumbers *simplyGen, Hashtable *movesHash)
        : Cursor (simplyGen, movesHash) {
  for(int i=0; i<linec*9; i++)
    switch (line[0][0][i])
    { case '_': line[0][0][i] = 2; break;//space
      case 'x': line[0][0][i] = 4; break;
      case 'o': line[0][0][i] = 8; break;
      case 'T': line[0][0][i] =34; break;//X or space
      case 'X': line[0][0][i] =36; break;//X or x
      case '8': line[0][0][i] =40; break;//X or o
      case '+': line[0][0][i] =20; break;//x or #
      case '*': line[0][0][i] =52; break;//X or x or #
      case '?': line[0][0][i] =54; break;//X or x or # or space
    };
  memcpy(line[1][0],line[0][0],linec*9);
  for(int i=0; i<linec*9; i++)
    if ((line[1][0][i]&12) != 12)
      if (line[1][0][i]&4) line[1][0][i] += 4;
      else if (line[1][0][i]&8) line[1][0][i] -= 4;
}

//==============================================================================

inline bool Evaluator::comp(int x, int y, unsigned char c)
{ if ((x<0)||(y<0)||(x>=fsize)||(y>=fsize))
    return (c&16);
  char n = kl[x+y*fsize];
  if (n<2) n = 2;
  return (n&c);
}

//==============================================================================

bool __fastcall Evaluator::scanlines(int BlNo, int &lines, int N)
{ static const int bl[18] = {0,1,2,6,16,21,30,6,21,30,43,53,54,9,11,30,53},
     vec[4][2] = {{1,1},{1,-1},{1,0},{0,1}}, p2[4] = {1,2,4,8};
  int nvec,sdv,nline,c, x = N%fsize, y = N/fsize,
        id = count & 1;//todo check if negation is needed here

  for (nvec=0; nvec<4; nvec++) if (lines&(p2[nvec]))
    for (nline = bl[BlNo]; nline<bl[BlNo+1]; nline++)
      for (sdv=0; sdv<9; sdv++) if (line[id][nline][sdv]&32)
        for (c = 0; c<9; c++)
          if (c==sdv) goto skip; else if (!line[id][nline][c]) goto skip;
          else if (comp(x+vec[nvec][0]*(c-sdv),y+vec[nvec][1]*(c-sdv),line[id][nline][c]))
skip:     { if (c==8)
            { lines -= p2[nvec];
              return true;
          } } else break;
  return false;
};

//==============================================================================

void Evaluator::rate(TNode *src, TNode *destNode, TMove move) { //fills {totalRating,x3,x4,o3,o4} of dest;
  static const int vec[4][2] = {{1,1},{1,-1},{1,0},{0,1}};

  destNode->hashCodeX = src->hashCodeO;
  destNode->hashCodeO = src->hashCodeX * simplyGen->getHash(move);

  destNode->o2 = src->x2;
  destNode->o3 = src->x3;
  destNode->o4 = src->x4;
  destNode->x2 = src->o2 > 1 ? src->o2 - 1 : src->o2;
  destNode->x3 = src->o3;
  destNode->x4 = src->o4;

  int t = 15; // 'lines'

  if (destNode->o4 > 0)// построить 5
    if (scanlines(0, t, move)) {
      destNode->rating = 32600;
      return;
    } 
  int ret = 0;
  bool c3my = false;
  if (destNode->x4 > 0)//закрыть 4
    if (scanlines(1, t, move)) {
        --destNode->x4;
        c3my = 1;
      }

  if (destNode->o3 > 0) {//постр открытую 4
    if (scanlines(2, t, move)) {
      destNode->o4 += 2;
      --destNode->o3;
    }
  }
  bool c3 = true;
  while (destNode->x3&&c3) {//закрыть 3
    int t0 = t;
    if (0 != (c3 = scanlines(3, t, move))) {
        --destNode->x3;
        int tdiff = t0-t;
        int nvec = tdiff == 1 ? 0
                : tdiff == 2 ? 1
                : tdiff == 4 ? 2
                : tdiff == 8 ? 3
                : -1;//should not happen
        if (nvec < 0) {//Die!
                nvec = nvec / 0;
        }
        int x = move % 15, y = move / 15;
        bool inside = kl[x + vec[nvec][0] + 15*(y + vec[nvec][1])]
                == kl[x - vec[nvec][0] + 15*(y - vec[nvec][1])];
        if (!inside) {
                //closed 3 is built
                destNode->x2 += 12;
        }
        ret = -600;//this is helpful for some reason
      }
  }


//  if (count == 1) {
//    if (scanlines(11, t, move)) {//второй ход
//        destNode->rating = 200;
//    } else {
//        destNode->rating = 0;
//    }
//    return;
//  }

  //t = 15;
  while (scanlines(4, t, move)) {//построить закрытую 4
    ++destNode->o4;
    destNode->o2 -= destNode->o2 >= 12 ? 12 : destNode->o2;
    if (c3my) c3my = 2;
  }

  while (scanlines(5, t, move)) {//постр 3
    destNode->o3 += 1;
    destNode->o2 -= destNode->o2 >= 10 ? 10 : destNode->o2;
  }

  while (destNode->x2 > 0 && scanlines(9, t, move)) {//закрыл 2
    destNode->x2 -= destNode->x2 >= 9 ? 8 : (destNode->x2+1)/2; //>= 2 ? 2 : destNode->x2 >= 1 ? 1 : 0;
  }

  while (scanlines(10, t, move)) {//постр 2
    destNode->o2 += 10;
  }

        double k2 = destNode->age > 12 ? 0.1
                : destNode->age > 10 ? 0.2
                : destNode->age > 7 ? 0.4
                : destNode->age > 4 ? 0.7
                : destNode->age > 2 ? 0.9
                : 1;

        if (t > 0) {
            double k1 = destNode->age > 14 ? 0.1
                : destNode->age > 12 ? 0.3
                : destNode->age > 10 ? 0.5
                : destNode->age > 8 ? 0.7
                : destNode->age > 6 ? 0.8
                : destNode->age > 4 ? 0.9
                : 1;

            while (scanlines(11, t, move)) {//ToT
                if (0 == (count&1)) {//TODO recheck this
                        ret += (int)(220*k1);
                }
                else {ret -= (int)(110*k1);}
            }
        }
        ret -= src->rating*k2;


//  //hz hz
//  if ((c3my==2)&&(dest->o3)) {
//    int i=0, n15 = 15;
//    for (int N = 0; N<fsize*fsize; ++N)
//      if (kl[N]&3) {
//         while (scanlines(2, n15, N)) ++i;
//         n15 = 15;
//      }
//    dest->o3 = i;
//  }

exit:

//begin check special cases
//  if (destNode->age==4 && destNode->o2 >= 25 && destNode->x2 <= 5) {
//        ret = 6026;
        //destNode->fixedRating = true;
  //} else
  if (destNode->age==5) {//todo recheck
        int m1 = history[1].move;
        int m2 = history[3].move;
        int m3 = move;
        if (m1/15 == m2/15 && m1/15 == m3/15 || m1%15 == m2%15 && m1%15 == m3%15) {
          ret = -6026;
          destNode->fixedRating = true;
        }
  }
//end check special cases

  if (destNode->fixedRating) {}
  else if (destNode->x4) {
    ret = -32200;
  } else if (destNode->x3 > 0 && destNode->o4 == 0) {
    ret = -30000;
  } else {
    ret +=destNode->o4*4200
        + destNode->o3*3840
        + destNode->o2*84
        - destNode->x3*6000
        - destNode->x2*150
      /*- src->o4*42
        - src->o3*30
        - src->o2*2
        + src->x3*60
        + src->x2*3*/;

//    if (destNode->o4 > 0 && destNode->o3 > 0) {
//            ret += 1000;
//    }
  }
#undef c3my


  destNode->rating = ret;


};





