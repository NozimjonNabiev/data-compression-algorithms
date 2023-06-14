// LZW Algorithm Implementation

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <chrono>

void compress(const std::string& inputFilename, const std::string& outputFilename) {
    std::ifstream inputFile(inputFilename, std::ios::binary);
    std::ofstream outputFile(outputFilename, std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cout << "Error opening files." << std::endl;
        return;
    }

    std::unordered_map<std::string, int> dictionary;
    for (int i = 0; i <= 255; ++i) {
        std::string ch(1, static_cast<char>(i));
        dictionary[ch] = i;
    }

    int nextCode = 256;
    std::string current;
    char ch;
    while (inputFile.get(ch)) {
        std::string currentPlusCh = current + ch;
        if (dictionary.find(currentPlusCh) != dictionary.end()) {
            current = currentPlusCh;
        } else {
            outputFile.write(reinterpret_cast<const char*>(&dictionary[current]), sizeof(dictionary[current]));
            dictionary[currentPlusCh] = nextCode++;
            current = std::string(1, ch);
        }
    }

    if (!current.empty()) {
        outputFile.write(reinterpret_cast<const char*>(&dictionary[current]), sizeof(dictionary[current]));
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

    std::unordered_map<int, std::string> dictionary;
    for (int i = 0; i <= 255; ++i) {
        std::string ch(1, static_cast<char>(i));
        dictionary[i] = ch;
    }

    int nextCode = 256;
    int previousCode, currentCode;
    inputFile.read(reinterpret_cast<char*>(&previousCode), sizeof(previousCode));
    std::string current = dictionary[previousCode];
    outputFile << current;

    while (inputFile.read(reinterpret_cast<char*>(&currentCode), sizeof(currentCode))) {
        std::string entry;
        if (dictionary.find(currentCode) != dictionary.end()) {
            entry = dictionary[currentCode];
        } else if (currentCode == nextCode) {
            entry = current + current[0];
        }

        outputFile << entry;
        dictionary[nextCode++] = current + entry[0];
        current = entry;
    }

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
