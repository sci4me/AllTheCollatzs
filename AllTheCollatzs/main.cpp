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

    void run(channel<work> *req, channel<uint64_t> *resp) {
        while (!req->is_closed()) {
            work w;
            if (req->get(w)) {
                uint64_t sum = 0;
                for (auto i = w.start; i < w.end; i++) sum += steps(i);
                resp->put(sum);
            }
        }
    }

    void run_until(uint64_t limit) {
        const uint64_t WORK_SIZE = 10000;
        auto cpus = std::thread::hardware_concurrency();

        auto reqs = new channel<work>[cpus];
        auto resps = new channel<uint64_t>[cpus];
        auto threads = new std::thread[cpus];

        for (auto i = 0u; i < cpus; i++) threads[i] = std::thread(run, &reqs[i], &resps[i]);
        
        uint64_t current = 0;
        uint64_t sum = 0;

        while (current < limit) {
            for (auto i = 0u; i < cpus; i++) {
                auto next = std::min(current + WORK_SIZE, limit);
                auto w = work{ current, next };
                current = next;

                reqs[i].put(w);
            }

            for (auto i = 0u; i < cpus; i++) {
                uint64_t res;
                if(!resps[i].get(res)) throw std::exception("someone done goofed");
                sum += res;
            }

            std::cout << current << " | " << sum << "\n" << std::flush;
        }

        for (auto i = 0u; i < cpus; i++) {
            reqs[i].close();
            resps[i].close();
            threads[i].join();
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 1 && argc != 2) {
        std::cerr << "Usage: collatz [limit]\n";
        return 1;
    }

    if (argc == 1) {
        collatz::run_until(std::numeric_limits<uint64_t>::max());
    } else {
        try {
            auto limit = std::stoull(argv[1]);
            collatz::run_until(limit);
        } catch (std::invalid_argument e) {
            std::cerr << "'" << argv[1] << "' is not an unsigned integer\n";
            return 1;
        }
    }

    return 0;
}