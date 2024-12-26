#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <functional>

#include <config.h>
#include <timer.h>
#include <util.h>

#include <string>

// microduck/1.0
// barrystyle 2024
//

void place_random_bet(int thr_id)
{
    int replay;
    uint64_t elapsed;
    srand(time(NULL));
    std::string currency = "LTC";

    while (true) {

        bool valid;
        int errorRet;
        int ishigh = rand() % 10;
        if (replay != -1) {
            ishigh = replay;
        }

        double betpercent = 10.0 / (rand() % 250 + 1);

        int resultRet;
        double amount;
        double percent;
        Bet current(LOW, 0.0, 50);
        if (ishigh >= 5) {
            current = Bet(HIGH, 0.00001545, betpercent);
            valid = PlaceBet(current, currency, resultRet, errorRet, elapsed);
            amount = current.amount;
            percent = current.percent;
        } else {
            current = Bet(LOW, 0.00001545, betpercent);
            valid = PlaceBet(current, currency, resultRet, errorRet, elapsed);
            amount = current.amount;
            percent = current.percent;
        }

        printf("thread%d - bet %0.8f %s (chance: %0.2f%) and %s (%dms)\n", thr_id, amount, percent, currency.c_str(), resultRet ? "won" : "lost", valid ? elapsed : -1);

        if (!valid) {
            printf("error: %s\n", ErrorLookup(errorRet).c_str());
        }

        // save value
        if (!valid)
            replay = ishigh;
        else
            replay = -1;

        // break if not required
        if (replay == -1)
            break;
    }
}

int max_thr = 4;

int main()
{
    std::vector<std::thread> threads;

    curl_global_init(CURL_GLOBAL_ALL);

    while (true) {

        int i=0;
        while (i < max_thr) {
            threads.push_back(std::thread(std::bind(&place_random_bet, std::ref(i))));
            ++i;
        }

        i=0;
        while (i < max_thr) {
            if (threads[i].joinable()) {
                threads[i].join();
            }
            ++i;
        }

        sleep(1);
    }

    curl_global_cleanup();

    return 1;
}
