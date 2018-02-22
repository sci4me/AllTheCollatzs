#include <iostream>
#include <string>
#include <limits>
#include <inttypes.h>

namespace collatz {
    uint64_t steps(uint64_t i) {
        if (i <= 1) return 0;

        uint64_t count = 0;

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

    void run_until(uint64_t limit) {
        uint64_t current = 0;
        uint64_t sum = 0;

        while (current < limit) {
            sum += steps(current);
            std::cout << current << " | " << sum << "\n";
            current++;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 1 && argc != 2) {
        std::cerr << "Usage: collatz [limit]\n";
        return 1;
    }

    if (argc == 1) {
        // This probably causes a bug? To do with the while loop checking that current < limit
        // Not sure; needs investigation
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