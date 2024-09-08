#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
    
struct Node {
    char ch;
    int freq;
    Node *left, *right;
    
    Node(char c, int f, Node* l = nullptr, Node* r = nullptr)
        : ch(c), freq(f), left(l), right(r) {}
};

struct Compare {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq;
    }
};

class HuffmanCoding {
private:
    std::unordered_map<char, std::string> huffmanCodes;
    
    void generateCodes(Node* root, std::string str) {
        if (!root) return;
        if (root->ch != '\0')
            huffmanCodes[root->ch] = str;
        generateCodes(root->left, str + "0");
        generateCodes(root->right, str + "1");
    }

    Node* buildHuffmanTree(const std::string& text) {
        std::unordered_map<char, int> freq;
        for (char c : text) freq[c]++;

        std::priority_queue<Node*, std::vector<Node*>, Compare> minHeap;
        for (auto pair : freq)
            minHeap.push(new Node(pair.first, pair.second));

        while (minHeap.size() > 1) {
            Node *left = minHeap.top(); minHeap.pop();
            Node *right = minHeap.top(); minHeap.pop();
            minHeap.push(new Node('\0', left->freq + right->freq, left, right));
        }

        return minHeap.top();
    }

public:
    void compress(const std::string& inFile, const std::string& outFile) {
        std::ifstream input(inFile, std::ios::binary);
        if (!input) {
            std::cerr << "Error: Cannot open input file " << inFile << std::endl;
            return;
        }

        // This reads the entire content of the input file into a std::string named 
        // text by iterating over the file's characters using std::istreambuf_iterator.
        std::string text((std::istreambuf_iterator<char>(input)), {});
        input.close();

        std::cout << "Input file size: " << text.length() << " bytes" << std::endl;

        Node* root = buildHuffmanTree(text);
        generateCodes(root, "");

        std::ofstream output(outFile, std::ios::binary);
        if (!output) {
            std::cerr << "Error: Cannot open output file " << outFile << std::endl;
            return;
        }

        // Write the number of unique characters
        output.put(static_cast<char>(huffmanCodes.size()));

        // Write the Huffman table
        for (const auto& pair : huffmanCodes) {
            output.put(pair.first);
            output.put(static_cast<char>(pair.second.length()));
            output << pair.second;
        }

        // Encode the text
        std::string encodedText;
        for (char c : text)
            encodedText += huffmanCodes[c];

        // Write the encoded text length
        int encodedLength = encodedText.length();
        output.write(reinterpret_cast<const char*>(&encodedLength), sizeof(encodedLength));

        // Write the encoded text
        for (size_t i = 0; i < encodedText.length(); i += 8) {
            std::bitset<8> bits;
            for (int j = 0; j < 8 && i + j < encodedText.length(); ++j) {
                bits[7 - j] = (encodedText[i + j] == '1');
            }
            output.put(static_cast<char>(bits.to_ulong()));
        }

        output.close();
        std::cout << "Compression completed. Output file size: " << std::ifstream(outFile, std::ios::binary | std::ios::ate).tellg() << " bytes" << std::endl;
    }

    void decompress(const std::string& inFile, const std::string& outFile) {
        std::ifstream input(inFile, std::ios::binary);
        if (!input) {
            std::cerr << "Error: Cannot open input file " << inFile << std::endl;
            return;
        }

        std::ofstream output(outFile);
        if (!output) {
            std::cerr << "Error: Cannot open output file " << outFile << std::endl;
            return;
        }

        // Read the number of unique characters
        int tableSize = static_cast<unsigned char>(input.get());
        std::cout << "Huffman table size: " << tableSize << std::endl;

        // Read the Huffman table
        std::unordered_map<std::string, char> reverseHuffmanCodes;
        for (int i = 0; i < tableSize; ++i) {
            char ch = input.get();
            int codeLength = static_cast<unsigned char>(input.get());
            std::string code(codeLength, '0');
            input.read(&code[0], codeLength);
            reverseHuffmanCodes[code] = ch;
            std::cout << "Code: " << code << " -> Char: '" << ch << "'" << std::endl;
        }

        // Read the encoded text length
        int encodedLength;
        input.read(reinterpret_cast<char*>(&encodedLength), sizeof(encodedLength));
        std::cout << "Encoded text length: " << encodedLength << " bits" << std::endl;

        // Read and decode the text
        std::string currentCode;
        int decodedChars = 0;
        int bitsRead = 0;
        char byte;
        while (input.get(byte) && bitsRead < encodedLength) {
            std::bitset<8> bits(byte);
            for (int i = 7; i >= 0 && bitsRead < encodedLength; --i, ++bitsRead) {
                currentCode += (bits[i] ? '1' : '0');
                auto it = reverseHuffmanCodes.find(currentCode);
                if (it != reverseHuffmanCodes.end()) {
                    output << it->second;
                    currentCode.clear();
                    ++decodedChars;
                }
            }
        }

        std::cout << "Total decoded characters: " << decodedChars << std::endl;
        input.close();
        output.close();
        std::cout << "Decompression completed." << std::endl;
    }
};

int main() {
    HuffmanCoding huffman;
    
    std::cout << "Starting compression..." << std::endl;
    huffman.compress("input.txt", "compressed.bin");

    std::cout << "\nStarting decompression..." << std::endl;
    huffman.decompress("compressed.bin", "decompressed.txt");

    // Compare files
    std::ifstream input("input.txt", std::ios::binary);
    std::ifstream decompressed("decompressed.txt", std::ios::binary);
    std::string inputContent((std::istreambuf_iterator<char>(input)), {});
    std::string decompressedContent((std::istreambuf_iterator<char>(decompressed)), {});
    
    if (inputContent == decompressedContent) {
        std::cout << "Compression and decompression successful. Files match." << std::endl;
    } else {
        std::cout << "Error: Decompressed file does not match original file." << std::endl;
    }

    return 0;
}