#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdio>

using namespace std;

struct huffmanNode 
{
    char ch;
    int freq;
    huffmanNode* left;
    huffmanNode* right;

    huffmanNode(char character, int frequency)
        : ch(character), freq(frequency), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(huffmanNode* left, huffmanNode* right) {
        return left->freq > right->freq;
    }
};

class huffmanAlg 
{
private:
    string huffmanCodes[256];  
    string encodedText;       

public:
    huffmanAlg() {}

    void TextFileEncoder() 
    {
        string filename, content;
        cout << "Enter the name of the text file: ";
        cin >> filename;
        cin.ignore();
        cout << "Enter the content of the file: ";
        getline(cin, content);

        ofstream outFile(filename + ".txt");
        if (!outFile) {
            cerr << "Error creating file." << endl;
            return;
        }
        outFile << content;
        outFile.close();
        cout << "File saved to "<< filename <<"." << endl;
    }

    unordered_map<char, int> fileReader(const string& filename) 
    {
        ifstream myFile(filename);
        if (!myFile) {
            cerr << "File not found: " << filename << endl;
            return {};
        }

        unordered_map<char, int> frequencies;
        char ch;
        while (myFile.get(ch)) {
            frequencies[ch]++;
        }
        myFile.close();
        return frequencies;
    }

    huffmanNode* huffmanTree(const unordered_map<char, int>& frequencies) 
    {
        priority_queue<huffmanNode*, vector<huffmanNode*>, Compare> minHeap;

        for (const auto& [ch, freq] : frequencies) {
            minHeap.push(new huffmanNode(ch, freq));
        }

        while (minHeap.size() > 1) {
            huffmanNode* left = minHeap.top();
            minHeap.pop();
            huffmanNode* right = minHeap.top();
            minHeap.pop();

            huffmanNode* merged = new huffmanNode('\0', left->freq + right->freq);
            merged->left = left;
            merged->right = right;

            minHeap.push(merged);
        }

        return minHeap.top();
    }

    void generateHuffmanCodes(huffmanNode* root, const string& code) 
    {
        if (!root)
            return;

        if (!root->left && !root->right) {
            huffmanCodes[(unsigned char)root->ch] = code;
        }

        generateHuffmanCodes(root->left, code + "0");
        generateHuffmanCodes(root->right, code + "1");
    }

    void FileCompressor() 
    {
        string filename;
        cout << "Enter the name of the text file to compress: ";
        cin >> filename;

        auto frequencies = fileReader(filename + ".txt");
        if (frequencies.empty()) {
            cerr << "File is empty or not found." << endl;
            return;
        }

        huffmanNode* root = huffmanTree(frequencies);
        generateHuffmanCodes(root, "");

        ifstream inFile(filename + ".txt");
        string content((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
        inFile.close();

        encodedText = "";
        for (char ch : content) {
            encodedText += huffmanCodes[(unsigned char)ch];
        }

        saveCompressedFile(filename + "-CompressedFile.bin");
        saveCompressionDetails(filename + "-CompressionDecoderInfo.txt");

        remove((filename + ".txt").c_str());
        cout << "File compressed successfully." << endl;
    }

    void FileDecompressor() 
    {
        string filename;
        cout << "Enter the name of the compressed file (without -Compressed.bin): ";
        cin >> filename;

        string compressedFilename = filename + "-CompressedFile.bin";
        string detailsFilename = filename + "-CompressionDecoderInfo.txt";

        if (!loadCompressionDetails(detailsFilename)) 
        {
            cerr << "Compression details file not found." << endl;
            return;
        }

        huffmanNode* root = buildTreeFromCodes();

        string binaryData = readBinaryFile(compressedFilename);
        string decompressedText = decodeText(root, binaryData);

        saveDecompressedFile(filename + ".txt", decompressedText);

        remove(compressedFilename.c_str());
        remove(detailsFilename.c_str());
        cout << "File decompressed successfully." << endl;
    }
 
private:
    void saveCompressedFile(const string& fileName) 
    {
        ofstream outFile(fileName, ios::binary);
        unsigned char byte = 0;
        int bitCount = 0;

        for (char bit : encodedText) 
        {
            byte = (byte << 1) | (bit - '0');
            bitCount++;
            if (bitCount == 8) {
                outFile.put(byte);
                byte = 0;
                bitCount = 0;
            }
        }
        if (bitCount > 0) 
        {
            byte <<= (8 - bitCount);
            outFile.put(byte);
        }
        outFile.close();
    }

    void saveCompressionDetails(const string& fileName) 
    {
        ofstream outFile(fileName);
        for (int i = 0; i < 256; i++) 
        {
            if (!huffmanCodes[i].empty()) 
            {
                outFile << (char)i << ": " << huffmanCodes[i] << "\n";
            }
        }
        outFile.close();
    }

    bool loadCompressionDetails(const string& fileName) 
    {
        ifstream inFile(fileName);
        if (!inFile) {
            return false;
        }

        for (int i = 0; i < 256; i++) {
            huffmanCodes[i].clear();
        }

        string line;
        while (getline(inFile, line)) {
            if (line.empty())
                continue;
            char ch = line[0];
            string code = line.substr(3);
            huffmanCodes[(unsigned char)ch] = code;
        }
        return true;
    }

    huffmanNode* buildTreeFromCodes() 
    {
        huffmanNode* root = new huffmanNode('\0', 0);

        for (int i = 0; i < 256; i++) {
            if (!huffmanCodes[i].empty()) {
                huffmanNode* current = root;
                for (char bit : huffmanCodes[i]) {
                    if (bit == '0') {
                        if (!current->left) {
                            current->left = new huffmanNode('\0', 0);
                        }
                        current = current->left;
                    } else {
                        if (!current->right) {
                            current->right = new huffmanNode('\0', 0);
                        }
                        current = current->right;
                    }
                }
                current->ch = i;
            }
        }
        return root;
    }

    string readBinaryFile(const string& fileName) 
    {
        ifstream inFile(fileName, ios::binary);
        string binaryData = "";

        unsigned char byte;
        while (inFile.read(reinterpret_cast<char*>(&byte), 1)) {
            for (int i = 7; i >= 0; --i) {
                binaryData += ((byte >> i) & 1) ? '1' : '0';
            }
        }
        return binaryData;
    }

    string decodeText(huffmanNode* root, const string& binaryData) 
    {
        string decodedText = "";
        huffmanNode* currentNode = root;

        for (char bit : binaryData) {
            currentNode = (bit == '0') ? currentNode->left : currentNode->right;
            if (!currentNode->left && !currentNode->right) {
                decodedText += currentNode->ch;
                currentNode = root;
            }
        }

        return decodedText;
    }


    void saveDecompressedFile(const string& fileName, const string& decompressedText) 
    {
        ofstream outFile(fileName);
        outFile << decompressedText;
        outFile.close();
    }
};

int main() {
    huffmanAlg huffAlg;
    int choice;
    while (true) 
    {
        system("cls");
        cout << "+---------------------------------------------------------------+" << endl;
        cout << "|          THIS IS FILE COMPRESSION DECOMPRESSION PROGRAM       |" << endl;
        cout << "+---------------------------------------------------------------+" << endl;
        cout << "|                                                               |" << endl;
        cout << "|                   (1) CREATE A TEXT FILE                      |" << endl;
        cout << "|                   (2) COMPRESS A TEXT FILE                    |" << endl;
        cout << "|                   (3) DECOMPRESS A FILE                       |" << endl;
        cout << "|                   (4) EXIT                                    |" << endl;
        cout << "|                                                               |" << endl;
        cout << "+---------------------------------------------------------------+" << endl;
        cout << "Select (1 - 4): ";
        cin >> choice;

        switch (choice) 
        {
            case 1:
                system("cls");
                cout << "+--------------------------------------+" << endl;
                cout << "|           ENTER FILE CONTENT         |" << endl;
                cout << "+--------------------------------------+" << endl;
                huffAlg.TextFileEncoder();
                break;
            case 2:
                system("cls");
                cout << "+------------------------------------+" << endl;
                cout << "|          FILE COMPRESSION          |" << endl;
                cout << "+------------------------------------+" << endl;
                huffAlg.FileCompressor();
                break;
            case 3:
                cout << "+------------------------------------+" << endl;
                cout << "|         FILE DECOMPRESSION         |" << endl;
                cout << "+------------------------------------+" << endl;
                huffAlg.FileDecompressor();
                break;
            case 4:
                system("cls");
                cout << "+-----------------------------------------------------------------------------+" << endl; 
                cout << "|                      THANK YOU FOR USING THIS PROGRAM!                      |" << endl;
                cout << "+-----------------------------------------------------------------------------+" << endl;  
                cout << "|                      GROUP LEADER: CLIFFORD ROY TORION                      |" << endl;
                cout << "+-----------------------------------------------------------------------------+" << endl; 
                cout << "|                                                                             |" << endl; 
                cout << "|                                  MEMBERS                                    |" << endl;
                cout << "|                                IRWEN FRONDA                                 |" << endl;
                cout << "|                             MARCJUSTIN JADAONE                              |" << endl;
                cout << "|                            JULIUS CAESAR INCIONG                            |" << endl;
                cout << "|                                                                             |" << endl; 
                cout << "+-----------------------------------------------------------------------------+" << endl; 
                exit(0);
            default:
                system("cls");
                cout << "Invalid Input. Please try again.\n";
                system("pause");
                break;
        }
    }
}
