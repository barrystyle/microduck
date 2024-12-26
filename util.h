#ifndef UTIL_H
#define UTIL_H

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>

#include <config.h>
#include <timer.h>

enum {
    LOW,
    HIGH
};

enum {
    CURLERR,
    EMPTYREPLY,
    CLOUDFLARE
};

struct Bet {
    int type;
    double amount;
    double percent;
    double result;
    Bet(int type_, double amount_, double percent_)
    {
        type = type_;
        amount = amount_;
        percent = percent_;
    }
};

struct MemoryStruct {
    char* memory;
    size_t size;
};

std::string ErrorLookup(int& errorRet);
bool ReturnValueFromJsonKey(char* buf, int bsz, char* key, int ksz, char* val);
bool PlaceBet(Bet& current, std::string& currency, int& resultRet, int& errorRet, uint64_t& elapsed, char *jsonReturn = NULL);

#endif // TIMER_H