//---------------------------------------------------------------------------

#ifndef LoggerH
#define LoggerH
#include <stdio.h>
//---------------------------------------------------------------------------

class Logger {

  private:
        int hitCount;
        int missAgeCount;
        int missHashCount;
        int missIndexCount;
        int hashOverwriteCount;
        const char* lastError;
        int expandEven;
        int expandOdd;
        int reserved;

  public:

        int bigParentsCulled1;
        int bigParentsCulled2;
        int bigGrandParentsCulled1;
        int bigGrandParentsCulled2;

        Logger();
        void hit();
        void missAge();
        void missHash();
        void missIndex();
        void expand(int count);
        void hashOverwrite();

        void error(const char *message);

        void printLastError(char *buffer);
};
#endif
