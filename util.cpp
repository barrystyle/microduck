#include <util.h>

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{

    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
        return 0;

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

bool ReturnValueFromJsonKey(char* buf, int bsz, char* key, int ksz, char* val)
{
    int cnt = 0;
    while (++cnt < bsz) {
        if (memcmp(buf + cnt, key, ksz) == 0 && memcmp(buf + cnt + ksz + 1, ":", 1) == 0) {
            int icnt = 1;
            while (memcmp(buf + cnt + ksz + 1 + icnt, ",", 1) != 0 && memcmp(buf + cnt + ksz + 1 + icnt, "}", 1) != 0) {
                if (cnt + ksz + 1 + icnt >= bsz)
                    return false;
                val[icnt - 1] = buf[cnt + ksz + 1 + icnt];
                ++icnt;
            }
            val[icnt] = 0;
            return true;
        }
    }
    return false;
}

std::string ErrorLookup(int& errorRet)
{
    if (errorRet == CURLERR)
        return std::string("curl-error");
    else if (errorRet == EMPTYREPLY)
        return std::string("empty-reply");
    else if (errorRet == CLOUDFLARE)
        return std::string("cloud-flare");
    return std::string("unknown");
}

static std::string BuildApiUrl()
{
    std::string url;
    url = "https://duckdice.io/api/play?api_key=" + API_KEY;
    return url;
}

bool PlaceBet(Bet& current, std::string& currency, int& resultRet, int& errorRet, uint64_t& elapsed, char *jsonReturn)
{

    ////////////////////
    Timer betTimer;
    betTimer.start();
    ////////////////////

    CURL* curl;
    CURLcode res;
    struct MemoryStruct chunk;
    chunk.memory = (char*)malloc(1);
    chunk.size = 0;

    char jsonbuf[256];
    sprintf(jsonbuf, "{"
                     "   \"symbol\": \"%s\","
                     "   \"chance\": \"%.8f\","
                     "   \"isHigh\": %s,"
                     "   \"amount\": \"%.8f\","
                     "   \"faucet\": %s"
                     "}\n",
        currency.c_str(), current.percent, current.type ? "true" : "false", current.amount, "false");

    std::string url = BuildApiUrl();

    struct curl_slist* hs;
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
        curl_easy_cleanup(curl);
        if (res != CURLE_OK) {
            errorRet = CURLERR;
            return false;
        } else if (chunk.size == 0) {
            errorRet = EMPTYREPLY;
            return false;
        } else if (chunk.size >= 12288) {
            errorRet = CLOUDFLARE;
            return false;
        }
    }

    // return full json if buffer provided
    if (jsonReturn) {
        memcpy(jsonReturn, chunk.memory, chunk.size);
    }

    // extract result
    char jsonret[128];
    memset(jsonret, 0, sizeof(jsonret));
    char jsonkey[] = "result";
    if (!ReturnValueFromJsonKey(chunk.memory, chunk.size, &jsonkey[0], 6, &jsonret[0])) {
        return false;
    }

    // match
    resultRet = 0;
    if (memcmp(jsonret, "true", 4) == 0) {
        resultRet = 1;
    }

    free(chunk.memory);

    /////////////////////////////////////////////
    betTimer.stop();
    elapsed = betTimer.between_milliseconds();
    /////////////////////////////////////////////

    return true;
}