#include "Co4.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

void runPlCo4() {
    cout << "\n--- PL CO4: Searching, sorting and hashing ---\n";
    struct ChainedLabelIndex {
        vector<vector<pair<string, int>>> buckets;
        size_t collisionCount = 0;

        explicit ChainedLabelIndex(size_t bucketCount)
            : buckets(max<size_t>(bucketCount, 1)) {}

        size_t bucketFor(const string& label) const {
            size_t hash = 0;
            for (unsigned char character : label) {
                hash = hash * 31 + character;
            }
            return hash % buckets.size();
        }

        void insert(const string& label, int position) {
            vector<pair<string, int>>& bucket = buckets[bucketFor(label)];
            if (!bucket.empty()) ++collisionCount;
            for (auto& entry : bucket) {
                if (entry.first == label) {
                    entry.second = position;
                    return;
                }
            }
            bucket.push_back({label, position});
        }

        bool find(const string& label, int& position,
                  size_t& comparisons) const {
            const auto& bucket = buckets[bucketFor(label)];
            for (const auto& entry : bucket) {
                ++comparisons;
                if (entry.first == label) {
                    position = entry.second;
                    return true;
                }
            }
            return false;
        }
    };

    vector<pair<string, int>> cityNodes = {
        {"WH-Main", 0}, {"CS-Downtown", 9}, {"CP-Alpha", 3},
        {"CS-West", 6}, {"CP-Beta", 2}, {"WH-East", 8}
    };
    sort(cityNodes.begin(), cityNodes.end(),
         [](const pair<string, int>& left, const pair<string, int>& right) {
             return left.second < right.second;
         });
    cout << "  Route nodes sorted by grid position:";
    for (const auto& entry : cityNodes) {
        cout << " " << entry.first << "@" << entry.second;
    }
    cout << "\n";

    ChainedLabelIndex collisionDemo(2);
    for (const auto& entry : cityNodes) {
        collisionDemo.insert(entry.first, entry.second);
    }
    int foundPosition = -1;
    size_t hashComparisons = 0;
    if (!collisionDemo.find("CS-West", foundPosition, hashComparisons) ||
        foundPosition != 6 || collisionDemo.collisionCount == 0) {
        throw runtime_error(
            "PL CO4 chained hash lookup or collision handling failed.");
    }
    cout << "  Chained hash lookup found CS-West at position "
         << foundPosition << "; collisions handled by bucket chaining: "
         << collisionDemo.collisionCount << "\n";

    cout << "  Search comparison counts on increasing datasets "
            "(target is the final label):\n"
         << "    Items    Linear    Binary    Hash-chain\n";
    for (const size_t itemCount : {64U, 512U, 4096U}) {
        vector<string> labels;
        labels.reserve(itemCount);
        ChainedLabelIndex index(itemCount / 2);
        for (size_t item = 0; item < itemCount; ++item) {
            ostringstream label;
            label << "NODE-" << setw(5) << setfill('0') << item;
            labels.push_back(label.str());
            index.insert(labels.back(), static_cast<int>(item));
        }

        const string& target = labels.back();
        size_t linearComparisons = 0;
        bool linearFound = false;
        for (const string& label : labels) {
            ++linearComparisons;
            if (label == target) {
                linearFound = true;
                break;
            }
        }

        size_t binaryComparisons = 0;
        size_t low = 0;
        size_t high = labels.size();
        while (low < high) {
            const size_t middle = low + (high - low) / 2;
            ++binaryComparisons;
            if (labels[middle] < target) {
                low = middle + 1;
            } else {
                high = middle;
            }
        }
        const bool binaryFound =
            low < labels.size() && labels[low] == target;

        size_t measuredHashComparisons = 0;
        int hashedPosition = -1;
        const bool hashFound =
            index.find(target, hashedPosition, measuredHashComparisons);
        if (!linearFound || !binaryFound || !hashFound ||
            hashedPosition != static_cast<int>(itemCount - 1)) {
            throw runtime_error(
                "PL CO4 failed a search on an increasing dataset.");
        }
        cout << "    " << setw(5) << itemCount
             << "    " << setw(6) << linearComparisons
             << "    " << setw(6) << binaryComparisons
             << "    " << setw(10) << measuredHashComparisons << "\n";
    }
    cout << "PL CO4 checks passed: sorting, three search strategies, "
            "collision handling and dataset-size comparison.\n";
}
