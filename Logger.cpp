//---------------------------------------------------------------------------


#pragma hdrstop

#include "Logger.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

Logger::Logger() {
        hitCount = 0;
        missAgeCount = 0;
        missHashCount = 0;
        missIndexCount = 0;
        lastError = NULL;
        expandEven = 0;
        expandOdd = 0;
        bigParentsCulled1 = 0;
        bigParentsCulled2 = 0;
        bigGrandParentsCulled1 = 0;
        bigGrandParentsCulled2 = 0;
        hashOverwriteCount = 0;
}

void Logger::hit() {
        ++hitCount;
}

void Logger::missAge() {
        ++missAgeCount;
}

void Logger::missHash() {
        ++missHashCount;
}

void Logger::missIndex() {
        ++missIndexCount;
}
void Logger::hashOverwrite() {
        ++hashOverwriteCount;
}

void Logger::error(const char* message) {
        this->lastError = message;
}

void Logger::expand(int count) {
        if (count%2) {
                ++expandOdd;
        } else {
                ++expandEven;
        }
}

void Logger::printLastError(char *buffer) {

        //sprintf(buffer, "%d ", (expandEven*100 / (1+expandOdd + expandEven)));

        if (lastError == NULL) {
                sprintf(buffer, "over = %dK", hashOverwriteCount/1000);
                //sprintf(buffer, "%d/%d/%d/%d", bigParentsCulled1, bigParentsCulled2, bigGrandParentsCulled1, bigGrandParentsCulled2);
        } else {

        sprintf(buffer,
                lastError != NULL
                        ? lastError : missAgeCount > 0
                        ? "Hash collisions %d"
                        : "\0", missAgeCount);
        }
}
