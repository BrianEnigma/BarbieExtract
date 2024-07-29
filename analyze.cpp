#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

void frequencyAnalysis(const char *inputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::map<unsigned char, unsigned int> frequencies;
    for (int i=0; i<=255; i++)
        frequencies[i] = 0;

    // Skip header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));

    while (inFile.good()) {
        char ch = 0;
        inFile.get(ch);
        frequencies[(unsigned char) ch]++;
    }
    inFile.close();
    std::cout << frequencies.size() << std::endl;

    // Reverse the map, keying off of count.
    std::map<unsigned int, std::vector<unsigned char> > frequenciesByCount;
    for (unsigned int ch = 0; ch <= 255; ch++) {
        unsigned int count = frequencies[ch];
        frequenciesByCount[count].push_back(ch);
    }
    // Taking advantage that this isn't an unsorted_map, iterate through the counts and print.
    std::cout << "FREQUENCY ANALYSIS" << std::endl;
    std::cout << "~~~~~~~~~ ~~~~~~~~" << std::endl;
    for (const auto &thePair : frequenciesByCount) {
        unsigned int count = thePair.first;
        std::vector<unsigned char> values = thePair.second;
        std::cout << count << "\t[";
        bool first = true;
        for (unsigned char value : values) {
            if (!first)
                std::cout << ", ";
            std::cout << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int) value << std::dec;
            first = false;
        }
        std::cout << "]" << std::endl;
    }
    std::cout << std::endl << std::endl;
}

void zeroAnalysis(const char *inputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);

    // Skip header
    char header[44] = {0};
    inFile.read(header, sizeof(header));

    char buffer[7] = {0};
    const unsigned int middleIndex = std::floor(sizeof(buffer) / 2);
    inFile.read(buffer, sizeof(buffer));
    std::cout << "ZERO NEIGHBOR ANALYSIS" << std::endl;
    std::cout << "~~~~ ~~~~~~~~ ~~~~~~~~" << std::endl;
    while (inFile.good()) {
        // If the middle byte of the buffer is zero...
        if (buffer[middleIndex] == 0) {
            std::cout << "[";
            bool first = true;
            for (int i = 0; i < sizeof(buffer); i++) {
                if (!first)
                    std::cout << ", ";
                std::cout << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int) ((unsigned char) buffer[i]) << std::dec;
                first = false;
            }
            std::cout << "]" << std::endl;
        }

        // Slide the buffer over and read a new character
        for (int i = 0; i < sizeof(buffer) - 1; i++)
            buffer[i] = buffer[i + 1];
        char ch = 0;
        inFile.get(ch);
        buffer[sizeof(buffer) - 1] = ch;
    }
    std::cout << std::endl << std::endl;
}

int main(int argc, char **argv) {
    frequencyAnalysis("test.wav");
    zeroAnalysis("test.wav");
    return 0;
}
