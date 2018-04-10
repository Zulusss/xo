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
        const char* lastError;
        int expandEven;
        int expandOdd;
        int reserved;        

  public:
        Logger();
        void hit();
        void missAge();
        void missHash();
        void missIndex();
        void expand(int count);

        void error(const char *message);

        void printLastError(char *buffer);
};
#endif
