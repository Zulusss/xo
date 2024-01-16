//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <shellapi.h>
#include <stdio.h>
#include <windows.h>
#include "xomain.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "CSPIN"
#pragma resource "*.dfm"
TMainForm *MainForm;

#define MAX_CHILDS_WIZARD  40000000

#define MAX_CHILD_PER_MOVE_ZONE0  12000000
#define MAX_CHILD_PER_MOVE_ZONE1  6000000

#define MAX_CHILD_DECREASE_FACTOR 6
#define MAX_CHILD_DECREASE_SINCE 50000000
#define MAX_CHILD_DECREASE_TILL 90000000

#define ZONE01_RATING 5700
#define ZONE12_RATING 32500


DWORD WINAPI __stdcall grow(LPVOID lpParam) {
  static int count;
  TMainForm *f =(TMainForm*)lpParam;
  int wizardMode = 1000;
  short int flowRating = 0;

   if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL)) {
        wizardMode = false;
            Application->MessageBox("Error setting THREAD_PRIORITY_BELOW_NORMAL", 0);
   } else {
            //Application->MessageBox("success",0);
   }

  int childs0 = 0;
  unsigned long beginTime = GetTickCount();
  while (!f->exitRequested) {

        if (f->xo == NULL) {
          Sleep(100);
          continue;
        }

        //********* STEP 1   initialize ***************
        if (f->ComboBoxMode->ItemIndex < 2) {
            wizardMode = 0;
        } else if (wizardMode == 0 && f->ComboBoxMode->ItemIndex >= 2) {
            wizardMode = 1000;
        }

        bool changed = true;

        TNode *firstNode = f->xo->getFirstNode();
        TNode *lastMove = f->xo->lastMove();

        TRating currRating = lastMove->rating;
        TRating currXRating = f->xo->count%2
                        ? currRating
                        : -currRating;

        int delta = currXRating - firstNode->rating;
        if (delta < 0) {
                delta = -delta;
        }
        bool medRating = (delta < 500);
        unsigned int totalChilds = firstNode->totalChilds;

        bool mediumicPlay = medRating;
/*              && (f->xo->count <  4
                || (f->xo->count <  5 && totalChilds >  500000)
                || (f->xo->count <  6 && totalChilds >  600000)
                || (f->xo->count <  7 && totalChilds >  700000)
                || (f->xo->count <  8 && totalChilds >  800000)
                || (f->xo->count <  9 && totalChilds > 1200000)
                || (f->xo->count < 11 && totalChilds > 1400000)
                || (f->xo->count < 13 && totalChilds > 2000000)
                || (f->xo->count < 15 && totalChilds > 3000000)
                || (f->xo->count < 17 && totalChilds > 4000000)
                || (f->xo->count < 18 && totalChilds > 7000000)
                || (f->xo->count < 19 && totalChilds >18000000)
                || (f->xo->count < 20 && totalChilds >20000000)
                || (f->xo->count < 21 && totalChilds >23000000)
                );*/

        int childPerMove = f->CSpinEditChilds->Value*1000;
        if (wizardMode && childs0 && childs0 >= childPerMove && childs0 < childPerMove*2) {
                childPerMove = childs0 + 50000;
        }

        double mcdf = totalChilds > MAX_CHILD_DECREASE_SINCE
                ? totalChilds > MAX_CHILD_DECREASE_TILL
                        ? MAX_CHILD_DECREASE_FACTOR
                        : MAX_CHILD_DECREASE_FACTOR
                                * ((totalChilds - MAX_CHILD_DECREASE_SINCE)/1000)
                                / (double)((MAX_CHILD_DECREASE_TILL - MAX_CHILD_DECREASE_SINCE)/1000)
                : 1;
        if (mcdf < 1) mcdf=1;


        int maxChilds = currRating < ZONE01_RATING && currRating > -ZONE01_RATING
                ? MAX_CHILD_PER_MOVE_ZONE0 / mcdf
                : currRating < ZONE12_RATING && currRating > -ZONE12_RATING
                        ? MAX_CHILD_PER_MOVE_ZONE1 / mcdf
                        :childPerMove + 1;
        if (maxChilds < childPerMove) {
                maxChilds = childPerMove;
        }

        //********* STEP 2   process requested actions ***************
        if (f->restartRequested) {
            f->restartRequested = false;
            f->xo->restart();
        } else if (f->userMoveRequested != 255) {
                int i;
                bool res = f->xo->put(f->userMoveRequested);
                if (res) {
                        int totl = lastMove->totalChilds;
                        f->resultRecieved = lastMove->rating;
                        if ((res < 32600)&&(f->ComboBoxMode->ItemIndex == 1)) {
                            f->moveRequested = true;
                        }
                }
                f->userMoveRequested = 255;

        } else if (f->moveRequested &&
                (wizardMode || lastMove->totalChilds >= childPerMove
                        || lastMove->rating < -20000
                        || lastMove->rating > 20000
                        || lastMove->totalDirectChilds == 1
                        || lastMove->o4 > 0)) {

          f->moveRequested = false;
          flowRating = currRating;
          f->resultRecieved = f->xo->move();
          if (f->resultRecieved == -32600)
                f->restartRequested = true;
          childs0 = lastMove->totalChilds;
          currRating = lastMove->rating;
        } else if (f->takeBackRequested) {
            f->takeBackRequested = false;

            if (wizardMode>0) {
                if (totalChilds > MAX_CHILDS_WIZARD) {
                        wizardMode = 0;
                } else {
                        --wizardMode;
                }
                if (wizardMode ==0) {
                // drop to human vs comp from Comp vs comp modes if "takeback" pressed
                        if (f->ComboBoxMode->ItemIndex == 2) {
                                f->restartRequested = true;
                                f->ComboBoxMode->ItemIndex = 1;
                        }
                }
            }

            if (f->xo->count > 1) {
                f->xo->back();
//                if (f->ComboBoxMode->ItemIndex ==1 && f->xo->count > 1) {
//                        f->xo->takeback();
//                }
                //f->xo->lastmove = f->xo->cursorLink->node; //TODO is it needed?
            }

            currRating = lastMove->rating;
        }

        //********* STEP 3   autoplay stuff ***************

        if (f->ComboBoxMode->ItemIndex == 3 && wizardMode > 0) {//Show debuts
                if (!medRating) {
                        if (count%5 == 0 || count%7 == 0) {
                                f->restartRequested = true;
                        } else {
                                f->takeBackRequested = true;
                        }
                        ++count;
                        continue;
                }
        }

        unsigned int lastCount = lastMove->totalChilds;
        if (wizardMode >= 0 && (f->ComboBoxMode->ItemIndex > 1)
                && (lastCount >= (mediumicPlay
                                        ? f->CSpinEditChilds->Value*1000
                                        : 20000))) {
                f->moveRequested = true;
                continue;
        }

        //********* STEP 4   tree grow ***************

        if (lastCount <  maxChilds
                        && currRating < 32500
                        && currRating > -32500
                        && f->CheckBoxGrow->Checked) {
          f->xo->buildTree();
          changed = false;
          ++count;
        } else {
          if (wizardMode && f->CheckBoxGrow->Checked) {

                  if (mediumicPlay || f->ComboBoxMode->ItemIndex == 2) {//Comp vs Comp
                                f->moveRequested = true;
                                continue;
                  } else {
                          if (!medRating) {
                                        //bad path
                                        if (wizardMode)
                                        f->takeBackRequested = true;

                                } else {
                                        //just too long path
                                        f->takeBackRequested = true;
                                }
                                continue;
                  }
          }
          Sleep(300);
        }

        //********* STEP 5   stat outputs ***************

        if (f->xo!=NULL && (changed || !(count%100))) {
              int min = 1000000000;
              int max = 0;
              int decr;
              int i=0;

              //begin hints calculation
              f->xo->calculateChilds();
              for(i=0; i < f->xo->childs.count; ++i) {
                int r = f->xo->childs.node[i]->rating;
                if (r<min) min = r;
                if (r>max) max = r;
              }

              memset(f->dkl, 0, fsize*fsize);
              for(i=0; i<f->xo->childs.count; ++i) {

                int r = f->xo->childs.node[i]->rating;
                decr = min-1;
                f->dkl[f->xo->childs.move[i]] = 30+(r-decr)*225/(max-decr);
              }
              //end hints calculation

              f->movesCount = f->xo->count;
              int i1 = firstNode->totalChilds;
              int i2 = lastMove->totalChilds;

              sprintf(f->msg1, "Childs count: %d%c / %d%c",
                            i1 / (i1 > 2000000 ? 1000000 : i1 > 2000 ? 1000 : 1), (i1 > 2000000 ? 'M' : i1 > 2000 ? 'K' : ' '),
                            i2 / (i2 > 2000000 ? 1000000 : i2 > 2000 ? 1000 : 1), (i2 > 2000000 ? 'M' : i2 > 2000 ? 'K' : ' '));

              sprintf(f->msg2, "Rating: %d / %d",
                        f->xo->getFirstNode()->rating,
                        lastMove->rating);

              sprintf(f->msg3, "Max path length: %d",
                        f->xo->max_count);

              sprintf(f->msgStatus, wizardMode
                        ? "Please either switch playing mode or just wait.."
                        : f->restartRequested || f->moveRequested || f->takeBackRequested || f->userMoveRequested != 255
                              ? "Please wait.."
                              : "Please make your move. Or, you can give it to me, by pressing Move button.");

              f->xo->logger->printLastError(f->msg4);
//              sprintf(f->msg4, f->xo->movesHash->miss3 > 0 || f->xo->movesHash->miss4 > 0
//                        ? "Hash collisions %d / %d" : "\0\0", f->xo->movesHash->miss3, f->xo->movesHash->miss4);

/*
              double updateFreq = TNode::updatesCount
                        ? 100 * TNode::updatesCount / (double)(TNode::updatesCount + TNode::skippedCount)
                        : 0;
              sprintf(f->msg5, "Dev: %.3f%% : %d / %d",
                        updateFreq,
                        (int)TNode::avgDiff,
                        (int)TNode::avgSquareDiff);
*/

/* TODO
              if (f->xo != NULL) {
                unsigned long time = GetTickCount() - beginTime;
                sprintf(f->msg5, "RTime: %.3f%%",
                        100 * f->xo->rateTime / (float)time,
                        (int)TNode::avgDiff,
                        (int)TNode::avgSquareDiff);
              }
*/

              f->xRating = f->xo->count%2
                ? lastMove->rating
                : -lastMove->rating;

              f->Invalidate();
              f->grid->Invalidate();
        }
  }
  long ret;
  return ret;
}



//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner) : TForm(Owner) {

  ComboBoxMode->ItemIndex = 1;
#ifdef DEBUG_VER
#else
        this->Button1->Visible = false;
        this->LabelAverages->Visible = false;
#endif
  resultRecieved = 0;
  restartRequested = false;
  takeBackRequested = false;
  exitRequested = false;
  moveRequested = false;
  userMoveRequested = 255;
  viewmode = 0;

  lang = 1;
  time_lev = 1;
  setoptions();
  CheckBoxGrow->Checked = false;
//  CheckBoxGrow->Enabled = false;

  DWORD   dwThreadIdArray;

  HANDLE h  = CreateThread(
      NULL,                   // default security attributes
      0,                      // use default stack size
      grow,       // thread function name
      this,          // argument to thread function
      0,                      // use default creation flags
      &dwThreadIdArray);   // returns the thread identifier

}

int TMainForm::transform(int move) {
  int x = move % 15, y = move / 15;
  if (swapX) {
        x = 14-x;
  }
  if (swapY) {
        y = 14-y;
  }
  if (swapW) {
        int t = x;
        x = y;
        y = t;
  }
//  if (swapXYW) {
//        int t = x;
//        x = 14-y;
//        y = 14-t;
//  }

  return y*15+x;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormResize(TObject *Sender)
{ int min = (((ClientWidth-Panel1->Width<ClientHeight - StatusBar->Height) ?
               ClientWidth-Panel1->Width:ClientHeight - StatusBar->Height))/15;
  grid->Width=min*15;
  grid->Height=min*15;
  grid->DefaultColWidth = min-1;
  grid->DefaultRowHeight= min-1;
  Panel1->Left = (grid->Width + ClientWidth - Panel1->Width)/2;
  Panel1->Top  = (ClientHeight - Panel1->Height - StatusBar->Height)/2;
  if (viewmode) viewmode = 0;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::gridDrawCell(TObject *Sender, int Col, int Row,
      TRect &Rect, TGridDrawState State) {

      
  Rect = grid->CellRect(Col,Row);
  int N = transform(Row * grid->ColCount + Col);
  bool found = false;
  bool foundLast;
  for(int i = 0; i < movesCount; ++i) {
      if (xo->getMove(i)->move == N) {
          found = true;
          foundLast = i == movesCount - 1;
          break;
      }
  }

  switch (viewmode)
  { case 0: {
    grid->Canvas->FillRect(Rect);
    if (grid->Width>440) {
      Rect.Right--; Rect.Bottom--;
      Rect.Left++; Rect.Top++;
    }
    float penW = 3.0;
    if (grid->Width > 450) {
        penW = (grid->Width + 148)/150;
    }
    grid->Canvas->Pen->Width = penW;
    if (found && xo->kl[N]&8) {//draw 'o'
      grid->Canvas->Pen->Color = foundLast ? (Graphics::TColor) RGB(255, 160, 160) : clRed;
      grid->Canvas->Ellipse(Rect.Left+2,Rect.Top+2,Rect.Right-2,Rect.Bottom-2);
    }
    if (found && xo->kl[N]&4) {//draw 'x'
      grid->Canvas->Pen->Color = foundLast ? (Graphics::TColor) RGB(168, 168, 255) : clBlue;
      grid->Canvas->MoveTo(Rect.Left+2,Rect.Top+2);
      grid->Canvas->LineTo(Rect.Right-3,Rect.Bottom-3);
      grid->Canvas->MoveTo(Rect.Right-3,Rect.Top+2);
      grid->Canvas->LineTo(Rect.Left+2,Rect.Bottom-3);
    }
    if (CheckBoxSH->Checked) {//draw hint
      grid->Canvas->Pen->Width = dkl[N] > 250 ? penW / 1.1 : penW / 1.5;
      if (dkl[N]>=1  && xo->isAlllowed(N)) {
        int d = 255 - dkl[N];
        grid->Canvas->Pen->Color = (Graphics::TColor) RGB(d, 255, d);
        float zf = penW*2;
        if (movesCount%2) {
                grid->Canvas->Ellipse(Rect.Left + zf, Rect.Top + zf,
                        Rect.Right - zf, Rect.Bottom - zf);
        } else {
                grid->Canvas->MoveTo(Rect.Left + zf, Rect.Top + zf);
                grid->Canvas->LineTo(Rect.Right- zf, Rect.Bottom - zf);
                grid->Canvas->MoveTo(Rect.Right- zf, Rect.Top + zf);
                grid->Canvas->LineTo(Rect.Left + zf, Rect.Bottom - zf);
        }
      }
    }
    }

    break;
    case 1:
      grid->Canvas->CopyMode = cmSrcCopy;
      grid->Canvas->CopyRect(Rect,about->Canvas,Rect);
 }
};
//---------------------------------------------------------------------------
void __fastcall TMainForm::gridClick  (TObject *Sender)
{ if (viewmode)
  { if ((grid->Row == 12)&&(grid->Col<9)&&(grid->Col>2))
       ShellExecute(NULL,NULL,"http:\\\\xogame.chat.ru",NULL,NULL,SW_SHOWDEFAULT);
    else ButtonAClick(NULL);
    return;
  }
  userMoveRequested = transform(grid->Row * grid->ColCount + grid->Col);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ButtonTBClick(TObject *Sender) {
  takeBackRequested = true;/*
if (xo->count)
  { xo->takeback();
    int i;
    TGridDrawState State;
    TRect Rect;
    i = GetTickCount();
    while(GetTickCount()-i < 100);
    i = xo->history[xo->count].nm;
    setkaDrawCell(NULL, i%15, i/15, Rect, State);
    i = GetTickCount();
    while(GetTickCount()-i < 100);
    if ((CheckBoxAM->Checked)&&(xo->count)) xo->takeback();
    for (i = 0; i<225; dkl[i++]=0);
    if (xo->count)
    dkl[xo->history[xo->count-1].nm] = 4;
    setka->Invalidate();
  }*/
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ButtonMClick(TObject *Sender) {
  moveRequested = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::aftermove()
{
  int res = resultRecieved;
  resultRecieved = 0;
  char f[] = "o.wav\0         \0";
  if (xo->count%2) f[0] = 'x';
//  Application->ProcessMessages();
  PlaySound(f,NULL,SND_FILENAME|SND_ASYNC);
  StatusBar->Panels->Items[0]->Text = (AnsiString)(xo->count%2 ? 'O':'X')
                                  + DBank->Items->Strings[21 + lang];
  if (res >= 32600)
  { PlaySound("gameover.wav",NULL,SND_FILENAME|SND_ASYNC);
    Application->MessageBox("GAME OVER !!!","Message",0);
  }

  int i=1;
  char pr = ' ';
  AnsiString s = DBank->Items->Strings[25 + lang];
//  itoa((xo->ntime+50)/1000,s.c_str()+i++,10);
  for(; i< 12; i++)
    if (*(s.c_str()+i) == 0) break;
  *(s.c_str()+i++) = '.';
//  itoa((xo->ntime - ((xo->ntime+50)/1000)*1000 + 50)/100,s.c_str()+i++,10);
//  if (xo->maxdepth != xo->_maxdepth) pr = '!';
  for(; i< 15; i++)
    if (*(s.c_str()+i) == 0) *(s.c_str()+i) = pr;
//  itoa(totl, s.c_str()+30, 10);
  for(i = 31; i< 40; i++)
    if (*(s.c_str()+i) == 0) *(s.c_str()+i) = ' ';
  *(s.c_str()+47) = '??';
  if (res != -1) itoa(res,s.c_str()+47,10);
  StatusBar->Panels->Items[1]->Text = s;
  grid->Invalidate();
};

//---------------------------------------------------------------------------

void __fastcall TMainForm::ButtonRClick(TObject *Sender) {
  restartRequested = true;
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::setoptions() {
  ButtonM->Caption = DBank->Items->Strings[3 + lang];
  ButtonM->Hint = DBank->Items->Strings[5 + lang];
  ButtonTB->Caption = DBank->Items->Strings[7 + lang];
  ButtonR->Caption = DBank->Items->Strings[9 + lang];
  ButtonA->Caption = DBank->Items->Strings[27 + lang];
}

//---------------------------------------------------------------------------

void __fastcall TMainForm::RadioButton1Click(TObject *Sender)
{ lang = 2;
  setoptions();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RadioButton2Click(TObject *Sender)
{ lang = 1;
  setoptions();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ButtonAClick(TObject *Sender)
{ if (viewmode)
  { viewmode = !viewmode;
    delete about;
    grid->Invalidate();
  }
  else
  { int x;
    about = new Graphics::TBitmap;
    about->Height = x = grid->Height;
    about->Width = x;
    TCanvas *c = about->Canvas;
    c->Font = Font;
    c->Font->Height = x*0.09;
    c->TextOut(x*0.21,x*0.125,"&");
    c->Pen->Width = x/104;
    c->Pen->Color = clBlue;
    c->MoveTo(x*0.12,x*0.075);
    c->LineTo(x*0.18,x*0.192);
    c->MoveTo(x*0.18,x*0.075);
    c->LineTo(x*0.12,x*0.192);
    c->Pen->Color = clRed;
    c->Ellipse(x*0.28,x*0.075,x*0.35,x*0.193);
    c->TextOut(x*0.425 - x*x*0.00003,x*0.125,"v e r   2.1");
    c->Font->Height = x*0.06;
    c->TextOut(x*0.15,x*0.544,"R  O  M  A  N      M  A  K  A  R  O  V");
    c->Font->Height = x*0.05;
    c->TextOut(x*0.608,x*0.223,"  a u g u s t  2 0 1 6");
    c->TextOut(x*0.075,x*0.484,"w r i t t e n   b y");
    c->TextOut(x*0.2,x*0.888,"e-mail:   partorgg@gmail.com");
    c->Font->Color = RGB(80,0,150);
    c->TextOut(x*0.2,x*0.82,"http:\\\\xogame.chat.ru");
    TRect Rect;
    TGridDrawState State;
    viewmode = !viewmode;
    for(int i = 0; i<225; i++)
    { gridDrawCell(NULL, i%15, i/15, Rect, State);
      x = GetTickCount();
      while(GetTickCount()-x < 10);
    }
  }
}
//---------------------------------------------------------------------------


void __fastcall TMainForm::gridMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y) {
  int sx,sy;
  grid->MouseToCell(X, Y, sx, sy);

  if (viewmode && (sy==12) && (sx<9) && (sx>2)) {
        grid->Cursor = crHandPoint;
  } else if (SetupForm->balance && xo->lastMove()->age==1 && sx>4&&sx<10&&sy>4&&sy<10) {
        grid->Cursor = crNo;
  } else {
     grid->Cursor = crDefault;
  }
}


//---------------------------------------------------------------------------

void __fastcall TMainForm::Button1Click(TObject *Sender)
{
        DebugForm->Show();
}

//---------------------------------------------------------------------------


void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
        exitRequested = true;
        Sleep(50);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
        exitRequested = true;
        Application->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
        exitRequested = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormPaint(TObject *Sender) {

        if (resultRecieved != 0) {
                aftermove();
        }

        Label1->Caption = msg1;
        Label2->Caption = msg2;
        Label3->Caption = msg3;
        Label8->Caption = msg4;
        LabelAverages->Caption = msg5;
        StatusBar->Panels->Items[1]->Text = msgStatus;


        int r=0, b=0;

        if (cbVisualRating->Checked) {
                if (this->xRating > 0) {
                        r = 90*this->xRating/32600;
                } else {
                        b = -90*this->xRating/32600;
                }
        }

        Graphics::TColor color = (Graphics::TColor) RGB(255-r, 255, 255-b);
        grid->Color = color;
        Panel1->Color = color;
        grid->Canvas->Brush->Color = color;
        Color = color;
}
//---------------------------------------------------------------------------





void __fastcall TMainForm::FormShow(TObject *Sender)
{
  if (CheckBoxGrow->Checked) return;
  CheckBoxGrow->Checked  = true;

  //xo = new Al_ext(SetupForm->balance);

  Logger *logger = new Logger();
  SimplyNumbers *sn = new SimplyNumbers();
  Hashtable *ht = new Hashtable(logger);

  xo = new GameBoard(sn, ht,
          &swapX, &swapY, &swapW, &swapXYW,
          (SetupForm->balance ? 1 : 0));
  xo->logger = logger;
  movesCount = xo->count;
//  xo->mindepth = 1;
  
}
//---------------------------------------------------------------------------


