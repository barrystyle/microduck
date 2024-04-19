#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <config.h>
#include <timer.h>

#include <string>

// microduck/1.0
// barrystyle 2024
//

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
    Bet(int type_, double amount_, double percent_) {
        type = type_;
        amount = amount_;
        percent = percent_;
    }
};

struct MemoryStruct {
    char* memory;
    size_t size;
};

static std::string ErrorLookup(int& errorRet) {
    if (errorRet == CURLERR) return std::string("curl-error");
    else if (errorRet == EMPTYREPLY) return std::string("empty-reply");
    else if (errorRet == CLOUDFLARE) return std::string("cloud-flare");
    return std::string("unknown");
}

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp) {

    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = (char*) realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
        return 0;

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

static std::string BuildApiUrl() {
    std::string url;
    url = "https://duckdice.io/api/play?api_key=" + API_KEY;
    return url;
}

bool PlaceBet(Bet& current, std::string& currency, int& errorRet, uint64_t& elapsed) {

    ////////////////////
    Timer betTimer;
    betTimer.start();
    ////////////////////

    CURL* curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = (char*) malloc(1);
    chunk.size = 0;

    char jsonbuf[256];
    sprintf(jsonbuf, "{"
                     "   \"symbol\": \"%s\","
                     "   \"chance\": \"%.8f\","
                     "   \"isHigh\": %s,"
                     "   \"amount\": \"%.8f\","
                     "   \"faucet\": %s"
                     "}\n", currency.c_str(), current.percent, current.type ? "true" : "false", current.amount, "false");

    std::string url = BuildApiUrl();

    struct curl_slist *hs;
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "MicroDuck/1.0");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonbuf);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(jsonbuf));
        hs = curl_slist_append(hs, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hs);
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            errorRet = CURLERR;
            return false;
        } else if (chunk.size == 0) {
            errorRet = EMPTYREPLY;
            return false;
        } else if (chunk.size >= 16384) {
            errorRet = CLOUDFLARE;
            return false;
        }
        curl_easy_cleanup(curl);
    }

    printf("\n%s\n", chunk.memory);

    free(chunk.memory);

    /////////////////////////////////////////////
    betTimer.stop();
    elapsed = betTimer.between_milliseconds();
    /////////////////////////////////////////////

    return true;
}

int replay = -1;

int main()
{
    curl_global_init(CURL_GLOBAL_ALL);

    uint64_t elapsed;
    srand (time(NULL));
    std::string currency = "LTC";

    while (true) {

       bool valid;
       int errorRet;
       int ishigh = rand() % 10 + 1;

       if (replay == -1)
           ishigh = replay;

       if (ishigh > 5) {
           Bet current(HIGH, 0.125, 40);
           valid = PlaceBet(current, currency, errorRet, elapsed);
       } else {
           Bet current(LOW, 0.125, 40);
           valid = PlaceBet(current, currency, errorRet, elapsed);
       }

       if (valid)
           printf("bet in %dms\n", elapsed);
       else
           printf("error: %s\n", ErrorLookup(errorRet).c_str());

       if (!valid)
           replay = ishigh;
       else
           replay = -1;

    }

    curl_global_cleanup();

    return 1;
}
