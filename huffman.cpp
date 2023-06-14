// Huffman Algorithm Implementation

#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <chrono>
#include <bitset>

using namespace std;
using namespace std::chrono;

struct HuffmanNode {
    char data;
    unsigned freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char data, unsigned freq) : data(data), freq(freq), left(nullptr), right(nullptr) {}
};

struct compare {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

unordered_map<char, string> huffmanCodes;

void encode(HuffmanNode* root, string str, unordered_map<char, string>& huffmanCodes) {
    if (root == nullptr)
        return;

    if (!root->left && !root->right) {
        huffmanCodes[root->data] = str;
    }

    encode(root->left, str + "0", huffmanCodes);
    encode(root->right, str + "1", huffmanCodes);
}

void decode(HuffmanNode* root, int& index, string str, ofstream& output) {
    if (root == nullptr)
        return;

    if (!root->left && !root->right) {
        output << root->data;
        return;
    }

    index++;

    if (str[index] == '0')
        decode(root->left, index, str, output);
    else
        decode(root->right, index, str, output);
}

void buildHuffmanTree(const unordered_map<char, unsigned>& freqTable, HuffmanNode*& root) {
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> minHeap;

    for (const auto& pair : freqTable) {
        minHeap.push(new HuffmanNode(pair.first, pair.second));
    }

    while (minHeap.size() != 1) {
        HuffmanNode* left = minHeap.top();
        minHeap.pop();
        HuffmanNode* right = minHeap.top();
        minHeap.pop();

        unsigned sumFreq = left->freq + right->freq;
        HuffmanNode* newNode = new HuffmanNode('\0', sumFreq);
        newNode->left = left;
        newNode->right = right;

        minHeap.push(newNode);
    }

    root = minHeap.top();
}

void compressFile(const string& inputFile, const string& outputFile) {
    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);

    unordered_map<char, unsigned> freqTable;
    char c;
    while (input.get(c)) {
        freqTable[c]++;
    }

    HuffmanNode* root = nullptr;
    buildHuffmanTree(freqTable, root);
    encode(root, "", huffmanCodes);

    input.clear();
    input.seekg(0, ios::beg);

    string encodedString;
    while (input.get(c)) {
        encodedString += huffmanCodes[c];
    }

    // Padding the encoded string with zeros to make it a multiple of 8
    unsigned paddingBits = 8 - encodedString.length() % 8;
    encodedString += string(paddingBits, '0');

    // Writing the padding information at the beginning of the output file
    output.write(reinterpret_cast<const char*>(&paddingBits), sizeof(paddingBits));

    // Writing the frequency table at the beginning of the output file
    size_t freqTableSize = freqTable.size();
    output.write(reinterpret_cast<const char*>(&freqTableSize), sizeof(freqTableSize));
    for (const auto& pair : freqTable) {
        output.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));
        output.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
    }

    // Writing the encoded string to the output file
    unsigned long long encodedStringLength = encodedString.length();
    for (size_t i = 0; i < encodedStringLength; i += 8) {
        string byte = encodedString.substr(i, 8);
        output.write(reinterpret_cast<const char*>(&byte), sizeof(char));
    }

    input.close();
    output.close();
}

void decompressFile(const string& inputFile, const string& outputFile) {
    ifstream input(inputFile, ios::binary);
    ofstream output(outputFile, ios::binary);

    unsigned paddingBits;
    input.read(reinterpret_cast<char*>(&paddingBits), sizeof(paddingBits));

    size_t freqTableSize;
    input.read(reinterpret_cast<char*>(&freqTableSize), sizeof(freqTableSize));

    unordered_map<char, unsigned> freqTable;
    for (size_t i = 0; i < freqTableSize; ++i) {
        char c;
        unsigned freq;
        input.read(reinterpret_cast<char*>(&c), sizeof(c));
        input.read(reinterpret_cast<char*>(&freq), sizeof(freq));
        freqTable[c] = freq;
    }

    HuffmanNode* root = nullptr;
    buildHuffmanTree(freqTable, root);

    string encodedString;
    char byte;
    while (input.get(byte)) {
        encodedString += byte;
    }

    string bitString;
    unsigned long long encodedStringLength = encodedString.length();
    for (size_t i = 0; i < encodedStringLength; ++i) {
        bitString += bitset<8>(encodedString[i]).to_string();
    }
    bitString = bitString.substr(0, bitString.length() - paddingBits);

    int index = -1;
    while (index < static_cast<int>(bitString.length()) - 2) {
        decode(root, index, bitString, output);
    }

    input.close();
    output.close();
}

int main() {
    string inputFile, compressedFile, decompressedFile;

    cout << "Enter the path of the input file: ";
    getline(cin, inputFile);

    cout << "Enter the path for the compressed file: ";
    getline(cin, compressedFile);

    cout << "Enter the path for the decompressed file: ";
    getline(cin, decompressedFile);

    auto startCompression = high_resolution_clock::now();
    compressFile(inputFile, compressedFile);
    auto stopCompression = high_resolution_clock::now();
    auto compressionDuration = duration_cast<milliseconds>(stopCompression - startCompression);
    cout << "Compression Time: " << compressionDuration.count() << " ms" << endl;

    auto startDecompression = high_resolution_clock::now();
    decompressFile(compressedFile, decompressedFile);
    auto stopDecompression = high_resolution_clock::now();
    auto decompressionDuration = duration_cast<milliseconds>(stopDecompression - startDecompression);
    cout << "Decompression Time: " << decompressionDuration.count() << " ms" << endl;

    return 0;
}
