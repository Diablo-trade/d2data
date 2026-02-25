/**
 * Written for linux g++ and c++17 std lib.
 *
 * You probably can compile this with other compilers on other platforms, but I haven't tested it.
 */

#include <unordered_map>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <atomic>
#include <random>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <filesystem>

struct Entry {
    std::string name;
    int prob = 0;
};

struct TC {
    std::string name;
    int group = 0;
    int level = 0;
    int picks = 0;
    int unique = 0;
    int set = 0;
    int rare = 0;
    int magic = 0;
    int nodrop = 0;

    Entry items[10];

    int total = 0;
};

struct AtomicTC {
    std::vector<Entry> items;

    int total = 0;
};

std::unordered_map<std::string, AtomicTC> atomic;
std::unordered_map<std::string, TC> treasureClasses;
int playermod = 1;
std::random_device rd;
std::mt19937 gen(rd());

// Helper function to split string by tab delimiter, keeping empty strings between tabs
std::vector<std::string> splitByChar(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    long start = 0;
    long end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    // Add the last token (after the final tab or remaining text)
    tokens.push_back(str.substr(start));

    return tokens;
}

void pickAtomic(std::string tcname, std::vector<std::string> &drops) {
    if (atomic.find(tcname) == atomic.end()) {
        drops.push_back(tcname);
        return;
    }

    AtomicTC &atc = atomic[tcname];

    if (atc.total == 0) {
        return;
    }

    std::uniform_int_distribution<> dis(0, atc.total - 1);
    int picknum = dis(gen);

    for (auto item : atc.items) {
        if (picknum < item.prob) {
            drops.push_back(item.name);
            return;
        }

        picknum -= item.prob;
    }
}

long calcNodrop(long e, long nd, long d) {
    double _e = (double) e, _nd = (double) nd, _d = (double) d;
    if (nd < 1) {
        return 0;
    }

    if (d < 1) {
        return nd;
    }

    return (long)(_d / (pow((_nd + _d) / nd, _e) - 1));
}

void pick(std::string tcname, std::vector<std::string> &drops) {
    if (drops.size() >= 6) {
        return;
    }

    if (treasureClasses.find(tcname) == treasureClasses.end()) {
        pickAtomic(tcname, drops);
        return;
    }

    TC &tc = treasureClasses[tcname];

    if (tc.picks == 0) {
        return;
    }

    if (tc.total == 0) {
        return;
    }

    int picks = tc.picks > 0 ? tc.picks : -tc.picks;
    long nodrop = calcNodrop(playermod, tc.nodrop, tc.total);

    std::uniform_int_distribution<long> dis(0, nodrop + tc.total - 1);

    if (tc.picks > 0) {
        for (int i = 0; i < picks; i++) {
            if (drops.size() >= 6) {
                return;
            }

            long picknum = dis(gen) - nodrop;
            
            if (picknum < 0) {
                continue;
            }

            for (auto item : tc.items) {
                if (item.prob) {
                    if (picknum < item.prob) {
                        pick(item.name, drops);
                        break;
                    }
    
                    picknum -= item.prob;
                }
            }
        }
    }
    else {
        for (auto item : tc.items) {
            for (int i = 0; i < item.prob; i++) {
                if (picks <= 0 || drops.size() >= 6) {
                    return;
                }

                pick(item.name, drops);
                picks--;
            }
        }
    }
}

std::string realpath(std::string path) {
    return std::filesystem::canonical(std::filesystem::absolute(path));
}

// Main takes first parameter as treasure class name
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <treasure_class_name> <player_mod> <drop_cycles>\n";
        return 1;
    }

    std::vector<std::string> pathParts = splitByChar(argv[0], '/');
    pathParts.pop_back();
    std::string path = "";

    for (const auto& part : pathParts) {
        path += part + "/";
    }

    path = realpath(path) + "/";

    std::string tcname = argv[1];
    int dropcycles = 5000;

    if (argc >= 3) {
        playermod = atoi(argv[2]);
    }

    playermod = std::max(1, playermod);
    playermod = std::min(8, playermod);

    
    if (argc >= 4) {
        dropcycles = atoi(argv[3]);
    }
    
    dropcycles = std::max(1, dropcycles);
    dropcycles = std::min(1000000, dropcycles);
    
    std::cout << "TC: " << tcname << std::endl;
    std::cout << "Player mod: " << playermod << std::endl;
    std::cout << "Drop cycles per set: " << dropcycles << std::endl;

    // Open the treasure class file at: txt/treasureclassex.txt
    FILE* tex = fopen((path + "txt/treasureclassex.txt").c_str(), "r");
    if (!tex) {
        std::cout << "Error: Could not open treasure class file\n";
        return 1;
    }

    // Treasure class file is tab delimited and has a header row, so skip the first line
    char line[1024];
    fgets(line, sizeof(line), tex); // Skip header

    while (fgets(line, sizeof(line), tex)) {
        // remove newline if it exists
        line[strcspn(line, "\r\n")] = 0;

        // Split by tab delimiter, preserving empty strings
        std::string lineStr(line);
        std::vector<std::string> tokens = splitByChar(lineStr, '\t');

        if (tokens.empty()) continue;

        TC tc;
        tc.name = tokens[0];

        if (tokens.size() > 1) {
            tc.group = atoi(tokens[1].c_str());
        }

        if (tokens.size() > 2) {
            tc.level = atoi(tokens[2].c_str());
        }

        if (tokens.size() > 3) {
            tc.picks = atoi(tokens[3].c_str());
        }

        if (tokens.size() > 4) {
            tc.unique = atoi(tokens[4].c_str());
        }

        if (tokens.size() > 5) {
            tc.set = atoi(tokens[5].c_str());
        }

        if (tokens.size() > 6) {
            tc.rare = atoi(tokens[6].c_str());
        }

        if (tokens.size() > 7) {
            tc.magic = atoi(tokens[7].c_str());
        }

        if (tokens.size() > 8) {
            tc.nodrop = atoi(tokens[8].c_str());
        }

        // Process items
        for (int i = 0; i < 10; i++) {
            int itemIdx = 9 + (i * 2);
            int probIdx = itemIdx + 1;

            if (itemIdx >= 0 && probIdx < tokens.size() && tokens[itemIdx][0] != '\0' && tokens[probIdx][0] != '\0') {
                tc.items[i].name = tokens[itemIdx];
                tc.items[i].prob = atoi(tokens[probIdx].c_str());
                tc.total += tc.items[i].prob;
            }

        }

        treasureClasses[tc.name] = tc;
    }

    fclose(tex);

    // Open the file at: atomic.txt
    FILE* atomicbase = fopen((path + "atomic.txt").c_str(), "r");

    if (!atomicbase) {
        std::cerr << "Error: Could not open atomic file\n";
        return 1;
    }

    while (fgets(line, sizeof(line), atomicbase)) {
        // remove newline if it exists
        line[strcspn(line, "\r\n")] = 0;

        // Split by tab delimiter, preserving empty strings
        std::string lineStr(line);
        std::vector<std::string> tokens = splitByChar(lineStr, '\t');

        if (tokens.empty()) continue;

        AtomicTC atc;
        std::string tcBaseName = tokens[0];
        atomic[tcBaseName] = atc;

        for (long i = 1; i < tokens.size() - 1; i += 2) {
            std::string itemName = tokens[i];
            int itemProb = atoi(tokens[i + 1].c_str());

            atomic[tcBaseName].items.push_back({itemName, itemProb});
            atomic[tcBaseName].total += itemProb;
        }
    }

    fclose(atomicbase);

    if (treasureClasses.find(tcname) == treasureClasses.end()) {
        std::cerr << "Error: Treasure class not found: " << tcname << "\n";
        return 1;
    }

    // Get CPU count.
    int thread_count = std::thread::hardware_concurrency();

    if (thread_count == 0) {
        thread_count = 1; // Fallback to 1 if hardware_concurrency cannot determine.
    }
    else {
        thread_count -= 2; // Leave 2 cores free to avoid system overload
        if (thread_count < 1) {
            thread_count = 1; // Ensure at least one thread is used
        }
    }

    std::cout << "Using " << thread_count << " threads\n";
    std::cout << "Output files periodically written to: " << (path + "simulations/") << std::endl;

    std::vector<std::thread> threads;

    for (int i = 0; i < thread_count; i++) {
        threads.emplace_back([i, &tcname, dropcycles, &path]() {
            long runs = 0;
            long picks = 0;
            std::unordered_map<std::string, long> drops;

            while (true) {
                for (int j = 0; j < dropcycles; j++) {
                    std::vector<std::string> rundrops;
                    pick(tcname, rundrops);
    
                    for (const auto& drop : rundrops) {
                        drops[drop]++;
                        picks++;
                    }
    
                    runs++;
                }

                // Write results to file as json with name: results-{timestamp}_{i}.json
                std::string filename = path + "simulations/" + tcname + " [" + std::to_string(playermod) + "][" + std::to_string(i) + "].json";
                std::ofstream out(filename);
    
                out << "{\n";
                out << "  \"tc\": \"" << tcname << "\",\n";
                out << "  \"runs\": " << runs << ",\n";
                out << "  \"picks\": " << picks << ",\n";
                out << "  \"playermod\": " << playermod << ",\n";
                out << "  \"avgpicks\": " << std::fixed << std::setprecision(6) << (double)picks / runs << ",\n";
                out << "  \"drops\": {\n";
                long count = 0;
                for (const auto& drop : drops) {
                    std::string escapedDrop = drop.first;
                    // Escape backslashes and double quotes in the drop name
                    long pos = 0;
                    while ((pos = escapedDrop.find('\\', pos)) != std::string::npos) {
                        escapedDrop.insert(pos, "\\");
                        pos += 2; // Move past the escaped backslash
                    }
                    pos = 0;
                    while ((pos = escapedDrop.find('\"', pos)) != std::string::npos) {
                        escapedDrop.insert(pos, "\\");
                        pos += 2; // Move past the escaped quote
                    }
                    out << "    \"" << escapedDrop << "\": " << drop.second;
                    if (++count < drops.size()) {
                        out << ",";
                    }
                    out << "\n";
                }
                out << "  }\n";
                out << "}\n";
            }
        });
    }

    for (int i = 0; i < thread_count; i++) {
        threads[i].join();
    }

    return 0;
}
