#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <config.h>
#include <timer.h>
#include <util.h>

#include <string>

// microduck/1.0
// barrystyle 2024
//

int replay = -1;
bool doubleUp = false;

int main()
{
    curl_global_init(CURL_GLOBAL_ALL);

    uint64_t elapsed;
    srand(time(NULL));
    std::string currency = "LTC";

    while (true) {

        bool valid;
        int errorRet;
        int ishigh = rand() % 10;

        double betpercent = 10.0 / (rand() % 100 + 1);

        //if (replay == -1)
        //    ishigh = replay;

        int resultRet;
        double amount;
        double percent;
        Bet current(LOW, 0.0, 50);
        if (ishigh >= 5) {
            current = Bet(HIGH, 0.000125, betpercent);
            valid = PlaceBet(current, currency, resultRet, errorRet, elapsed);
            amount = current.amount;
            percent = current.percent;
        } else {
            current = Bet(LOW, 0.000125, betpercent);
            valid = PlaceBet(current, currency, resultRet, errorRet, elapsed);
            amount = current.amount;
            percent = current.percent;
        }

        printf("bet %0.8f %s (chance: %0.2f%) and %s (%dms)\n", amount, percent, currency.c_str(), resultRet ? "won" : "lost", valid ? elapsed : -1);

        if (!valid) {
            printf("error: %s\n", ErrorLookup(errorRet).c_str());
        }

        if (resultRet)
            doubleUp = true;
        else
            doubleUp = false;

        if (!valid)
            replay = ishigh;
        else
            replay = -1;
    }

    curl_global_cleanup();

    return 1;
}
