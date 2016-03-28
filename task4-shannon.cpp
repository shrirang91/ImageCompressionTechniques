// Phase 2
//
// Task 4: Compressiong
//      [x] No compression
//      [x] Shannon Fano
//      [x] LZW
//      [x] Arithemetic Coding

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <vector>
#include <sstream>

#define NL_SYMBOL 767
#define EOF_SYMBOL 768

using namespace std;

typedef struct SymbolNode {
    int symbol;
    int frequency;
    string code;
    
    SymbolNode(int sym, int freq, string c) :
        symbol(sym),
        frequency(freq),
        code(c)
    {
    }
    
    static bool frequencyCompare(const SymbolNode &self, const SymbolNode &other) {
        return self.frequency > other.frequency;
    }
    
    static bool symbolCompare(const SymbolNode &self, const SymbolNode &other) {
        return self.symbol < other.symbol;
    }
    
} SymbolNode;

typedef vector<SymbolNode> SymbolVector;
typedef map<string, int> DecodeMap;

string removeExtension(string &name) {
    string::size_type index = name.rfind('.');
    
    if (index != string::npos) {
        string ext = name.substr(index);
        name.erase(index);
        return ext;
    }
    else {
        return string();
    }
}

string removeFilename(string &name) {
    string::size_type index = name.rfind('/');
    
    if (index != string::npos) {
        string ext = name.substr(index);
        name.erase(index);
        return ext;
    }
    else {
        return string();
    }
}

void generateCodes(SymbolVector &symbols, int start, int end) {
    // Don't need to encode the range if it is only one character wide
    if (end - start <= 1)
        return;
        
    // Compute the sum of frequencies and the split point
    int sum = 0, split = 0;
    int splitIndex = start;
    
    for (int i = start; i < end; i++) {
        sum += symbols[i].frequency;
    }
    
    split = sum / 2;
        
    // Search for the split index
    int splitSum = symbols[start].frequency;
    for (int i = start + 1; i < end; i++) {
        splitSum += symbols[i].frequency;
        
        if (splitSum > split) {
            splitIndex = i;
            break;
        }
    }
    
    // The "left" group is from start to splitIndex
    // The "right" group is from splitIndex to end
    //
    // Add 0 to the codes for left symbols
    // Add 1 to the codes for right symbols    
    for (int i = start; i < splitIndex; i++) 
        symbols[i].code += '0';
    
    for (int i = splitIndex; i < end; i++) 
        symbols[i].code += '1';
        
    // Generate codes for the left and right groups
    generateCodes(symbols, start, splitIndex);
    generateCodes(symbols, splitIndex, end);
}

void printSymbolVector(SymbolVector &symbols) {
    for (int i = 0; i < symbols.size(); i++) {
        SymbolNode node = symbols[i];
        int sym = node.symbol;
        int freq = node.frequency;
        string code = node.code;
        
        if (sym < 511) {
            cout << (sym - 255);
        }
        else if (sym < 767) {
            cout << "*" << (sym - 511);
        }
        else if (sym == NL_SYMBOL) {
            cout << "\\n";
        }
        else if (sym == EOF_SYMBOL) {
            cout << "EOF";
        }
        
        cout << "\t" << freq << "\t" << code << endl; 
    }
}

SymbolVector computeSymbolVector(string filename) {
    int freq[769];
    
    for (int i = 0; i < 769; i++) {
        freq[i] = 0;
    }
    
    fstream infile;
    infile.open(filename);
    
    if (infile.is_open()) { 
        // Skip first line
        string line;
        getline(infile, line);

        while (!infile.eof() && infile.peek() != EOF) {
            bool actual = false;
            int value;
            
            // Skip each space
            if (infile.peek() == ' ') {
                infile.get();
                continue;
            }
            
            // If the next character is a newline, add it and continue
            if (infile.peek() == '\n') {
                infile.get();
                freq[NL_SYMBOL] += 1;
                continue;
            }

            // If the next character is an asterisk, mark it as an actual
            actual = (infile.peek() == '*');
            
            // Skip the asterisk if it was present
            if (actual) {
                infile.get();
            }

            infile >> value;

            if (actual) {
                freq[511 + value] += 1;
            }
            else {
                freq[255 + value] += 1;
            }
        }
    }
    
    // Add one to the EOF frequency
    freq[EOF_SYMBOL] = 1;
    
    SymbolVector symbols;
    
    for (int i = 0; i < 769; i++) {
        // Add non-zero frequency symbols
        if (freq[i] > 0) {
            symbols.push_back(SymbolNode(i, freq[i], ""));
        }
    }
    
    // Sort the vector by frequency
    sort(symbols.begin(), symbols.end(), &SymbolNode::frequencyCompare);
    
    // Generate the codes for the symbols
    generateCodes(symbols, 0, symbols.size());
    
    return symbols;
}

void outputBitString(string code, ofstream &outfile) {
    if (code.size() < 8) {
        code.append(8 - code.size(), '1');
    }
    
    bitset<8> bits(code);
    unsigned char num = static_cast<unsigned char>(bits.to_ulong());
    
    outfile.write((char *)(void *)&num, sizeof(unsigned char));
}

void outputSymbolVector(SymbolVector &symbols, ofstream &outfile) {
    for (int i = 0; i < symbols.size(); i++) {
        SymbolNode node = symbols[i];
        
        outfile << node.code << "=";
        outfile << node.symbol << "|";
    }
    
    outfile << endl;
}

void compress(SymbolVector &symbols, string infilename, string outfilename) {
    string code, line;
    
    string encodemap[769];
    
    for (int i = 0; i < symbols.size(); i++) {
        SymbolNode node = symbols[i];
        encodemap[node.symbol] = node.code;
    }
    
    ifstream infile;
    infile.open(infilename);
    
    ofstream outfile;
    outfile.open(outfilename);
    
    if (infile.is_open()) { 
        // Write the first line out verbatim
        getline(infile, line);
        outfile << line << endl;
        
        // Write the symbol vector to the file
        outputSymbolVector(symbols, outfile);
        
        bool actual = false;
        int value;
        int sym;
        
        while (!infile.eof() && infile.peek() != EOF) {
            // Skip each space
            if (infile.peek() == ' ') {
                infile.get();
                continue;
            }
            
            // If the next character is a newline, add it and continue
            if (infile.peek() == '\n') {
                infile.get();
                sym = NL_SYMBOL;
            }
            else {
                // If the next character is an asterisk, mark it as an actual
                actual = (infile.peek() == '*');
                
                // Skip the asterisk if it was present
                if (actual) {
                    infile.get();
                }

                infile >> value;

                // Compute the symbol index from the value
                if (actual) {
                    sym = value + 511;
                }
                else {
                    sym = value + 255;
                }
            }
            
            code += encodemap[sym];
            
            // If the code string is long enough (1 byte = 8 bits), output
            if (code.size() >= 8) {
                // Extract the first 128 bits from the string
                string bits = code.substr(0, 8);
                code.erase(0, 8);
                
                // Output the bits
                outputBitString(bits, outfile);
            }
        }
    }
    
    code += encodemap[EOF_SYMBOL];
    
    if (!code.empty()) {
        outputBitString(code, outfile);
    }
    
    cout << endl << "[*] Wrote compressed file to " << outfilename << endl;
}

DecodeMap readDecodeMap(string infilename) {
    DecodeMap decoder;
    
    ifstream infile;
    infile.open(infilename);
    
    string line;
    
    // Skip the first line containing dimensions.
    getline(infile, line);
    
    // Read the first line.
    getline(infile, line);
    
    stringstream linestream(line);
    
    while (!linestream.eof() && linestream.peek() != EOF) {
        string code;
        int symbol;
        
        while (linestream.peek() != '=') {
            code.append(1, (char)linestream.get());
        }
            
        if (linestream.peek() == '=')
            linestream.ignore();
            
        linestream >> symbol;
        
        if (linestream.peek() == '|')
            linestream.ignore();
            
        decoder[code] = symbol;
    }
    
    return decoder;
}

void decompress(DecodeMap decoder, string infilename, string outfilename) {
    string bitstring, curcode, line;
    
    ifstream infile;
    infile.open(infilename);
    
    ofstream outfile;
    outfile.open(outfilename);
    
    // Write the first line (dimensions)
    getline(infile, line);
    outfile << line << endl;
    
    // Skip the second line (dictionary)
    getline(infile, line);
    
    bool cont = true;
    
    // Get the next character and parse the bytes
    while (!infile.eof() && cont) {
        int read = infile.get();
        bitset<8> bits((unsigned char)read);
        bitstring.append(bits.to_string());
                
        while (!bitstring.empty() && cont) {
            // Read the first bit of the string
            curcode.append(bitstring.substr(0, 1));
            bitstring.erase(0, 1);
            
            // Check if the symbol exists in the table
            if (decoder.count(curcode) == 1) {
                int symbol = decoder[curcode];
                
                if (symbol < 511) {
                    outfile << (symbol - 255) << " ";
                }
                else if (symbol < 767) {
                    outfile << "*" << (symbol - 511) << " ";
                }
                else if (symbol == NL_SYMBOL) {
                    outfile << endl;
                }
                else if (symbol == EOF_SYMBOL) {
                    cont = false;
                }
                
                curcode.clear();
            }
        } 
    }
    
    cout << endl << "[*] Wrote decompressed file to " << outfilename << endl;
    
    string outnamepath = outfilename;
    removeFilename(outnamepath);
    outnamepath.append("/outname");
    system(("printf \"" + outfilename + "\" > \"" + outnamepath + "\"").c_str());
}

int main(int argc, char **argv) {
    string infilename, outfilename, ext;
    int choice;
    
    if (argc == 3) {
        infilename = argv[1];
        choice = stoi(argv[2]);
    }
    else {
        cout << endl << "Enter the file name: ";
        cin >> infilename;
            
        cout << endl << "Choices: " << endl;
        cout << "\t1. Compress" << endl;
        cout << "\t2. Decompress" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
    }
    
    if (choice == 1) {        
        outfilename = infilename;
        ext = removeExtension(outfilename);
        ext.back() = 'v'; // Compressed

        outfilename.append("_2"); // for Shannon-Fano coding, option 2.
        outfilename.append(ext);
    
        SymbolVector symbols = computeSymbolVector(infilename);
        printSymbolVector(symbols);
        compress(symbols, infilename, outfilename);
    }
    else {        
        outfilename = infilename;
        ext = removeExtension(outfilename);
        ext.back() = 'c'; // Decompressed
        
        outfilename.erase(outfilename.length() - 2); // Remove the "_C" suffix
        outfilename.append(ext);
        
        DecodeMap decoder = readDecodeMap(infilename);
        decompress(decoder, infilename, outfilename);
    }

    return 0;
}

