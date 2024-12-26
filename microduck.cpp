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
    double betpercent = 0.02;
    std::string currency = "TON";

    while (true) {

        bool valid;
        int errorRet;
        int ishigh = rand() % 10;
        betpercent = 30;
        printf("%f %d\n", betpercent, ishigh);

        //if (replay == -1)
        //    ishigh = replay;

        int resultRet;
        Bet current(LOW, 0.0, 50);
        if (ishigh >= 5) {
            current = Bet(HIGH, 0.25, betpercent);
            valid = PlaceBet(current, currency, resultRet, errorRet, elapsed);
        } else {
            current = Bet(LOW, 0.25, betpercent);
            valid = PlaceBet(current, currency, resultRet, errorRet, elapsed);
        }

        printf("bet %d %s and %s\n", 32, currency.c_str(), resultRet ? "won" : "lost");

        if (valid)
            printf("bet in %dms\n", elapsed);
        else
            printf("error: %s\n", ErrorLookup(errorRet).c_str());

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
