#include <fstream>

// <len><char> -- wash, rinse, repeat.
void rleNaive() {
    std::ifstream inFile("test.wav", std::ios::binary | std::ios::in);
    std::ofstream outFile("rletest1.wav", std::ios::binary | std::ios::out);

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
void rleMagic00() {
    std::ifstream inFile("test.wav", std::ios::binary | std::ios::in);
    std::ofstream outFile("rletest2.wav", std::ios::binary | std::ios::out);

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
            outFile.put(ch);
        }
    }
    inFile.close();
    outFile.close();
}

// Assume 0x00 is a magic sentinel value.
// Assume 0x00 <something other than 0x00> is just a regular pair of bytes.
// Assume 0x00 0x00 is followed by an RLE length for inserting zeros.
void rleZerosOnly() {
    std::ifstream inFile("test.wav", std::ios::binary | std::ios::in);
    std::ofstream outFile("rletest3.wav", std::ios::binary | std::ios::out);

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

int main(int argc, char **argv) {
    rleNaive();
    rleMagic00();
    rleZerosOnly();
    return 0;
}
