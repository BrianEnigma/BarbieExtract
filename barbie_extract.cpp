#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#define DEBUG_SEEK false
#define DEBUG_COPY true

const std::string INPUT_FILE = "pinames.hug";

/// Find the next wav file header, starting at the given offset.
/// If no more are found, return -1.
int findNextHeader(std::ifstream &inFile, int currentOffset) {
    while (inFile.good() && currentOffset >= 0) {
        char buffer[1024] = {0};
        if (DEBUG_SEEK)
            std::cout << "CO:" << currentOffset << std::endl;
        inFile.seekg(currentOffset);
#if 0
        // This isn't working on my Mac...?
        size_t readCount = inFile.readsome(&buffer[0], sizeof(buffer));
#else
        size_t readCount = 1024;
        inFile.read(&buffer[0], sizeof(buffer));
#endif
        if (DEBUG_SEEK)
            std::cout << "RC: " << readCount << std::endl;
        int firstLetterMatch = 0;
        while (firstLetterMatch < sizeof(buffer)) {
            if (DEBUG_SEEK)
                std::cout << "FLM:" << firstLetterMatch << std::endl;
            char *ptr = (char *) std::memchr(buffer + firstLetterMatch, 'R', sizeof(buffer));
            // Not found. Move to next block.
            if (ptr == nullptr) {
                if (DEBUG_SEEK)
                    std::cout << "not found" << std::endl;
                currentOffset += sizeof(buffer) - 4; // minus four in case the buffer lands in the middle of a header string
                break;
            }
            //std::cout << "ptr:" << ptr << std::endl;
            // Don't look outside the buffer. If it's toward the end of the buffer, we'll find it next loop.
            if (ptr - &buffer[0] < sizeof(buffer) - 4) {
                if (*(ptr + 1) == 'I' && *(ptr + 2) == 'F' && *(ptr + 3) == 'F') {
                    return currentOffset + ptr - &buffer[0];
                } else {
                    firstLetterMatch = ptr - &buffer[0] + 1; // Look for next match.
                    continue;
                }
            } else {
                if (DEBUG_SEEK)
                    std::cout << "end of buffer1" << std::endl;
                break;
            }
        }
        currentOffset += sizeof(buffer) - 4; // minus four in case the buffer lands in the middle of a header string
        if (DEBUG_SEEK)
            std::cout << "end of buffer2" << std::endl;
    }
    return -1;
}

void extractFile(std::ifstream &inFile, const char *filename, size_t offset, size_t length) {
    if (DEBUG_COPY)
        std::cout << "Copying from 0x" << std::hex << offset << std::dec << ", length " << length << std::endl;
    std::ofstream outFile(filename, std::ios::binary | std::ios::out);
    inFile.seekg(offset);
    while (length > 0) {
        char buffer[1024] = {0};
        int readSize = std::min(sizeof(buffer), length);
        inFile.read(buffer, readSize);
        outFile.write(buffer, readSize);
        length -= readSize;
    }
    outFile.close();
}

void extractFiles(std::ifstream &inFile) {
    mkdir("./output", 0755);
    int fileIndex = 0;
    int previousHeaderOffset = -1;
    int headerOffset = 0;
    for (int i = 0; i < 20; i++) {
        headerOffset = findNextHeader(inFile, headerOffset);
        std::cout << std::hex << headerOffset << std::dec << std::endl;
        if (headerOffset != -1 && previousHeaderOffset != -1) {
            std::ostringstream outFilename;
            outFilename << "./output/file-" << std::setfill('0') << std::setw(6) << fileIndex << "-" << headerOffset << ".wav";
            extractFile(inFile, outFilename.str().c_str(), previousHeaderOffset, headerOffset - previousHeaderOffset - 1);
        }
        previousHeaderOffset = headerOffset;
        headerOffset++;
        fileIndex++;
    }


}

int main(int argc, char **argv) {
    std::ifstream inFile(INPUT_FILE, std::ios::binary | std::ios::in);
    extractFiles(inFile);
    return 0;
}