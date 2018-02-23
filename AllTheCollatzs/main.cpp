#include <iostream>
#include <string>
#include <limits>
#include <algorithm>
#include <inttypes.h>

#include "channel.hpp"

namespace collatz {
    uint64_t steps(uint64_t i) {
        if (i <= 1) return 0;

        uint64_t count = 0;

        // Need to do much more testing comparing this implementation to others to find which is truly best
        while (i != 1) {
            while (i & 1) {
                i = ((3 * i) + 1) >> 1;
                count += 2;
            }

            while (!(i & 1)) {
                i >>= 1;
                count++;
            }
        }

        return count;
    }

    struct work {
        uint64_t start;
        uint64_t end;
    };

    void run(channel<work>& req, channel<uint64_t>& resp) {
        while (!req.is_closed()) {
            work w;
            if (req.get(w)) {
                uint64_t sum = 0;
                for (auto i = w.start; i <= w.end; i++) sum += steps(i);
                resp.put(sum);
            }
        }
    }

    void run_until(uint64_t limit, uint64_t work_size) {
        auto cpus = std::thread::hardware_concurrency();

        std::cout << "Running with work size " << work_size << ", " << cpus << " threads, until " << limit << "\n" << std::flush; 
        
        channel<uint64_t> resps;

        auto reqs = new channel<work>[cpus];
        auto threads = new std::thread[cpus];

        for (auto i = 0u; i < cpus; i++) threads[i] = std::thread(run, std::ref(reqs[i]), std::ref(resps));
        
        uint64_t current = 0;
        uint64_t sum = 0;

        while (current < limit) {
            auto reqs_make = 0;

            for (auto i = 0u; i < cpus; i++) {
                auto next = std::min(current + work_size, limit);
                if (current == next) break;

                auto w = work{ current, next };
                current = next;

                reqs[i].put(w);
                reqs_make++;
            }

            for (auto i = 0u; i < reqs_make; i++) {
                uint64_t res;
                if(!resps.get(res)) throw std::exception("someone done goofed");
                sum += res;
            }

            std::cout << current << " | " << sum << "\n" << std::flush;
        }

        for (auto i = 0u; i < cpus; i++) {
            reqs[i].close();
            threads[i].join();
        }

        delete[] reqs;
        delete[] threads;
    }
}

int main(int argc, char **argv) {
    if (argc != 1 && argc != 2 && argc != 3) {
        std::cerr << "Usage: collatz [limit] [work-size]\n";
        return 1;
    }

    uint64_t limit = 0;
    uint64_t work_size = 0;

    if (argc >= 2) {
        limit = std::stoull(argv[1]);
        if (argc == 3) work_size = std::stoull(argv[2]);
    } 

    if (limit == 0)     limit     = std::numeric_limits<uint64_t>::max();
    if (work_size == 0) work_size = 100000;

    collatz::run_until(limit, work_size);

    return 0;
}