// Copyright (c) 2017 SIGE developer
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bench.h"
#include "perf.h"
#include "crypto/ctaes/ctaes.h"

#include <iostream>
#include <iomanip>

benchmark::BenchRunner::BenchmarkMap &benchmark::BenchRunner::benchmarks() {
    static std::map<std::string, benchmark::BenchFunction> benchmarks_map;
    return benchmarks_map;
}

#ifdef _MSC_VER
#include <Winsock2.h>

int gettimeofday(struct timeval* tp, void* ptimezone)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    const unsigned long long EPOCH = 116444736000000000ULL;

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    unsigned long long time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((unsigned long long)file_time.dwLowDateTime);
    time += ((unsigned long long)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

#else
#include <sys/time.h>
#endif

static double gettimedouble(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_usec * 0.000001 + tv.tv_sec;
}

benchmark::BenchRunner::BenchRunner(std::string name, benchmark::BenchFunction func)
{
    benchmarks().insert(std::make_pair(name, func));
}

void
benchmark::BenchRunner::RunAll(double elapsedTimeForOne)
{
    perf_init();
    std::cout << "#Benchmark" << "," << "count" << "," << "min" << "," << "max" << "," << "average" << ","
              << "min_cycles" << "," << "max_cycles" << "," << "average_cycles" << "\n";

    for (const auto &p: benchmarks()) {
        State state(p.first, elapsedTimeForOne);
        p.second(state);
    }
    perf_fini();
}

// precompilation bug with <algorithm> macros and numeric_limits::max

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

bool benchmark::State::KeepRunning()
{
    if (count & countMask) {
      ++count;
      return true;
    }
    double now;
    uint64_t nowCycles;
    if (count == 0) {
        lastTime = beginTime = now = gettimedouble();
        lastCycles = beginCycles = nowCycles = perf_cpucycles();
    }
    else {
        now = gettimedouble();
        double elapsed = now - lastTime;
        double elapsedOne = elapsed * countMaskInv;
        if (elapsedOne < minTime) minTime = elapsedOne;
        if (elapsedOne > maxTime) maxTime = elapsedOne;

        // We only use relative values, so don't have to handle 64-bit wrap-around specially
        nowCycles = perf_cpucycles();
        uint64_t elapsedOneCycles = (nowCycles - lastCycles) * countMaskInv;
        if (elapsedOneCycles < minCycles) minCycles = elapsedOneCycles;
        if (elapsedOneCycles > maxCycles) maxCycles = elapsedOneCycles;

        if (elapsed*128 < maxElapsed) {
          // If the execution was much too fast (1/128th of maxElapsed), increase the count mask by 8x and restart timing.
          // The restart avoids including the overhead of this code in the measurement.
          countMask = ((countMask<<3)|7) & ((1LL<<60)-1);
          countMaskInv = 1./(countMask+1);
          count = 0;
          minTime = std::numeric_limits<double>::max();
          maxTime = std::numeric_limits<double>::min();
          minCycles = std::numeric_limits<uint64_t>::max();
          maxCycles = std::numeric_limits<uint64_t>::min();
          return true;
        }
        if (elapsed*16 < maxElapsed) {
          uint64_t newCountMask = ((countMask<<1)|1) & ((1LL<<60)-1);
          if ((count & newCountMask)==0) {
              countMask = newCountMask;
              countMaskInv = 1./(countMask+1);
          }
        }
    }
    lastTime = now;
    lastCycles = nowCycles;
    ++count;

    if (now - beginTime < maxElapsed) return true; // Keep going

    --count;

    // Output results
    double average = (now-beginTime)/count;
    int64_t averageCycles = (nowCycles-beginCycles)/count;
    std::cout << std::fixed << std::setprecision(15) << name << "," << count << "," << minTime << "," << maxTime << "," << average << ","
              << minCycles << "," << maxCycles << "," << averageCycles << "\n";

    return false;
}
