// LZ77 Algorithm Implementation

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

struct Token {
    int offset;
    int length;
    char nextChar;
};

void compress(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cout << "Error opening files." << std::endl;
        return;
    }

    std::vector<Token> tokens;
    char ch;
    int searchPos, longestMatch, bufferPos;

    while (inputFile.get(ch)) {
        searchPos = static_cast<int>(tokens.size()) - 1;
        longestMatch = 0;
        bufferPos = 0;

        while (searchPos >= 0 && tokens[searchPos].nextChar != ch && bufferPos < 256) {
            searchPos--;
            bufferPos++;
        }

        while (searchPos >= 0 && tokens[searchPos].nextChar == ch && bufferPos < 256) {
            int length = 1;
            int startPos = searchPos - 1;

            while (startPos >= 0 && tokens[startPos].nextChar == tokens[searchPos + length].nextChar && length < 256) {
                startPos--;
                length++;
            }

            if (length > longestMatch) {
                longestMatch = length;
                bufferPos = 256 - length;
            }

            searchPos--;
            bufferPos++;
        }

        Token token;
        token.offset = bufferPos;
        token.length = longestMatch;
        token.nextChar = ch;

        tokens.push_back(token);

        if (static_cast<int>(tokens.size()) > 4096) {
            tokens.erase(tokens.begin());
        }
    }

    for (const Token& token : tokens) {
        outputFile.write(reinterpret_cast<const char*>(&token.offset), sizeof(token.offset));
        outputFile.write(reinterpret_cast<const char*>(&token.length), sizeof(token.length));
        outputFile.write(reinterpret_cast<const char*>(&token.nextChar), sizeof(token.nextChar));
    }

    inputFile.close();
    outputFile.close();
}

void decompress(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cout << "Error opening files." << std::endl;
        return;
    }

    std::vector<Token> tokens;
    Token token;

    while (inputFile.read(reinterpret_cast<char*>(&token.offset), sizeof(token.offset))) {
        inputFile.read(reinterpret_cast<char*>(&token.length), sizeof(token.length));
        inputFile.read(reinterpret_cast<char*>(&token.nextChar), sizeof(token.nextChar));
        tokens.push_back(token);
    }

    std::string decodedString;

    for (const Token& token : tokens) {
        int startPos = static_cast<int>(decodedString.size()) - token.offset;
        int endPos = startPos + token.length;

        for (int i = startPos; i < endPos; ++i) {
            decodedString += decodedString[i];
        }

        decodedString += token.nextChar;
    }

    outputFile << decodedString;

    inputFile.close();
    outputFile.close();
}

int main() {
    std::string inputFilename;
    std::string compressedFilename = "compressed.bin";
    std::string decompressedFilename = "decompressed.txt";

    std::cout << "Enter the name of the file to compress: ";
    std::cin >> inputFilename;

    // Compression
    auto startCompress = std::chrono::high_resolution_clock::now();
    compress(inputFilename, compressedFilename);
    auto endCompress = std::chrono::high_resolution_clock::now();

    // Decompression
    auto startDecompress = std::chrono::high_resolution_clock::now();
    decompress(compressedFilename, decompressedFilename);
    auto endDecompress = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> compressTime = endCompress - startCompress;
    std::chrono::duration<double> decompressTime = endDecompress - startDecompress;

    std::cout << "Compression time: " << compressTime.count() << " seconds" << std::endl;
    std::cout << "Decompression time: " << decompressTime.count() << " seconds" << std::endl;

    return 0;
}
