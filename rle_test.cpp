#include <fstream>
#include <iostream>

// <len><char> -- wash, rinse, repeat.
void rleNaive(const char *inputFilename, const char *outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::ofstream outFile(outputFilename, std::ios::binary | std::ios::out);

    // Copy header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));
    outFile.write(buffer, sizeof(buffer));

    // Attempt RLE
    while (inFile.good()) {
        char rle = 0;
        char ch = 0;
        inFile.get(rle);
        inFile.get(ch);
        for (int i = 0; i < (unsigned char) rle; i++) {
            outFile.put(ch);
        }
    }
    inFile.close();
    outFile.close();
}

// Assume 0x00 is a magic sentinel value.
// Assume 0x00 <len> <char> is an RLE directive.
// Assume 0x00 0x00 is just a plain 0x00.
void rleMagic00(const char *inputFilename, const char *outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::ofstream outFile(outputFilename, std::ios::binary | std::ios::out);

    // Copy header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));
    outFile.write(buffer, sizeof(buffer));

    // Attempt RLE
    while (inFile.good()) {
        char ch = 0;
        inFile.get(ch);
        if (ch == 0) {
            inFile.get(ch);
            if (ch == 0) {
                outFile.put(0);
            } else {
                char rle = ch;
                inFile.get(ch);
                for (int i = 0; i < (unsigned char) rle; i++) {
                    outFile.put(ch);
                }
            }
        } else {
            outFile.put(ch);
        }
    }
    inFile.close();
    outFile.close();
}

// Assume 0x00 is a magic sentinel value.
// Assume 0x00 <char> <len> is an RLE directive.
// Assume 0x00 0x00 is just a plain 0x00.
void rleMagic00Alt(const char *inputFilename, const char *outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::ofstream outFile(outputFilename, std::ios::binary | std::ios::out);

    // Copy header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));
    outFile.write(buffer, sizeof(buffer));

    // Attempt RLE
    while (inFile.good()) {
        char ch = 0;
        inFile.get(ch);
        if (ch == 0) {
            inFile.get(ch);
            if (ch == 0) {
                outFile.put(0);
            } else {
                char rle = 0;
                inFile.get(rle);
                for (int i = 0; i < (unsigned char) rle; i++) {
                    outFile.put(ch);
                }
            }
        } else {
            outFile.put(ch);
        }
    }
    inFile.close();
    outFile.close();
}

// Assume 0x00 is a magic sentinel value.
// Assume 0x00 0x00 is just a plain 0x00. (But this seems weird because it'd be the same as 0x00 0x01.)
// Assume 0x00 <len> is an RLE directive to insert that many zeros.
void rleOnly00(const char *inputFilename, const char *outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::ofstream outFile(outputFilename, std::ios::binary | std::ios::out);

    // Copy header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));
    outFile.write(buffer, sizeof(buffer));

    // Attempt RLE
    while (inFile.good()) {
        char ch = 0;
        inFile.get(ch);
        if (ch == 0) {
            inFile.get(ch);
            if (ch == 0) {
                outFile.put(0);
            } else {
                for (int i = 0; i < (unsigned char) ch; i++) {
                    outFile.put(0);
                }
            }
        } else {
            outFile.put(ch);
        }
    }
    inFile.close();
    outFile.close();
}

// Assume 0x00 is a magic sentinel value.
// Assume 0x00 <something other than 0x00> is just a regular pair of bytes.
// Assume 0x00 0x00 is followed by an RLE length for inserting zeros.
void rleZerosOnly(const char *inputFilename, const char *outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::ofstream outFile(outputFilename, std::ios::binary | std::ios::out);

    // Copy header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));
    outFile.write(buffer, sizeof(buffer));

    // Attempt RLE
    while (inFile.good()) {
        char ch = 0;
        inFile.get(ch);
        if (ch == 0) {
            inFile.get(ch);
            if (ch != 0) {
                outFile.put(0);
                outFile.put(ch);
            } else {
                char rle = 0;
                inFile.get(rle);
                for (int i = 0; i < (unsigned char) rle; i++) {
                    outFile.put(0);
                }
            }
        } else {
            outFile.put(ch);
        }
    }
    inFile.close();
    outFile.close();
}

// Assume each 8-bit sample is really two 4-bit samples. Separate them and double each one to get the full 8-bit swing.
void fourBitToEightBit(const char *inputFilename, const char *outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::ofstream outFile(outputFilename, std::ios::binary | std::ios::out);

    // Copy header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));
    outFile.write(buffer, sizeof(buffer));

    while (inFile.good()) {
        char ch = 0;
        inFile.get(ch);
        outFile.put((ch >> 4) * 2);
        outFile.put((ch & 0x0F) * 2);
    }
    inFile.close();
    outFile.close();
}

void fourBitToEightBitRle(const char *inputFilename, const char *outputFilename) {
    std::ifstream inFile(inputFilename, std::ios::binary | std::ios::in);
    std::ofstream outFile(outputFilename, std::ios::binary | std::ios::out);

    // Copy header
    char buffer[44] = {0};
    inFile.read(buffer, sizeof(buffer));
    outFile.write(buffer, sizeof(buffer));

    // Attempt RLE
    while (inFile.good()) {
        char ch = 0;
        inFile.get(ch);
        if (ch == 0) {
            inFile.get(ch);
            if (ch == 0) {
                outFile.put(0);
            } else {
                char rle = 0;
                inFile.get(rle);
                inFile.get(ch);
                for (int i = 0; i < (unsigned char) rle; i++) {
                    outFile.put(ch);
                }
            }
        } else {
            outFile.put((ch >> 4) * 2);
            outFile.put((ch & 0x0F) * 2);
        }
    }
    inFile.close();
    outFile.close();
}

int main(int argc, char **argv) {
    //const char *inputFilename = "output/file-000003-54252.wav";
    const char *inputFilename = "test.wav";
    rleNaive(inputFilename, "rletest1.wav");
    rleMagic00(inputFilename, "rletest2.wav");
    rleMagic00Alt(inputFilename, "rletest3.wav");
    rleOnly00(inputFilename, "rletest4.wav");
    rleZerosOnly(inputFilename, "rletest5.wav");
    fourBitToEightBit(inputFilename, "rletest6.wav");
    fourBitToEightBitRle(inputFilename, "rletest7.wav");
    return 0;
}
