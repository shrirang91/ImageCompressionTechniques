// Phase 2
//
// Task 4: Compressiong
//      [x] No compression
//      [x] Shannon Fano
//      [x] LZW
//      [yes] Arithemetic Coding

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <gmpxx.h>

using namespace std;

#define NL_SYMBOL 767
#define EOF_SYMBOL 768

typedef struct Symbol {
    int value;
    mpf_class probability;
    mpf_class low;
    mpf_class high;
    
    Symbol(int val, mpf_class prob) :
        value(val),
        probability(prob) 
    {
    
    }
    
    Symbol(int val, mpf_class prob, mpf_class l, mpf_class h) :
        value(val),
        probability(prob),
        low(l),
        high(h)
    {
    
    }
} Symbol;

typedef vector<Symbol> SymbolVector;

int mpf_out_raw(FILE *f, mpf_t x) {
    int expt; mpz_t Z; size_t nz;
    expt = x->_mp_exp;
    fwrite(&expt, sizeof(int), 1, f);
    nz = x->_mp_size;
    Z->_mp_alloc = nz; 
    Z->_mp_size  = nz; 
    Z->_mp_d     = x->_mp_d;
    return (mpz_out_raw(f, Z) + sizeof(int));
}

void mpf_inp_raw(FILE *f, mpf_t x) { 
    int expt; mpz_t Z; size_t nz;
    mpz_init (Z);
    fread(&expt, sizeof(int), 1, f);
    mpz_inp_raw  (Z, f);
    mpf_set_z    (x, Z); 
    x->_mp_exp   = expt;
    mpz_clear (Z);
}

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

void outputSymbolVector(SymbolVector &symbols) {
    for(int i = 0; i < symbols.size(); i++) {
        Symbol &symbol = symbols[i];
        int value = symbol.value;
        
        if (value < 511) {
            cout << " " << (value - 255);
        }
        else if (value < 767) {
            cout << "*" << (value - 511);
        }
        else if (value == NL_SYMBOL) {
            cout << "\\n"; 
        }
        else if (value == EOF_SYMBOL) {
            cout << "EOF";
        }
        
        cout << "\t" << symbol.probability;
        cout << "\t" << symbol.low;
        cout << "\t" << symbol.high;
        cout << endl;
     }
}

void writeSymbolVector(FILE *f, SymbolVector &symbols) {
    int size = symbols.size();
    
    fwrite(&size, sizeof(int), 1, f);
    
    for (int i = 0; i < size; i++) {
        Symbol &symbol = symbols[i];
        
        fwrite(&(symbol.value), sizeof(int), 1, f);
        mpf_out_raw(f, symbol.probability.get_mpf_t());
    }
}

SymbolVector readSymbolVector(FILE *f) {
    SymbolVector symbols;
    
    int size = 0;
    fread(&size, sizeof(int), 1, f);
    
    mpf_class previous(0);
    
    for (int i = 0; i < size; i++) {
        int value;
        mpf_class probability, low, high;
        
        fread(&value, sizeof(int), 1, f);
        mpf_inp_raw(f, probability.get_mpf_t());
        low = previous;
        high = previous + probability;
        previous = high;
        
        symbols.push_back(Symbol(value, probability, low, high));
    }
    
    return symbols;
}

void compress(string infilename, string outfilename) {
    int freq[769];
    bool actual, cont;
    int value;
    
    //initialize the probability array
    for(int i=0; i < 769; i++) {
        freq[i] = 0;
    }
    
    
    ifstream myfile;
    myfile.open(infilename);
    
    while (!myfile.eof() && myfile.peek() != EOF)
    {
        // Skip each space.
        if (myfile.peek() == ' ') {
            myfile.ignore();
            continue;
        }
        
        // If the next character is a newline, add it and continue
        if (myfile.peek() == '\n') {
            myfile.ignore();
            freq[NL_SYMBOL] += 1;
            continue;
        }

        // If the next character is an asterisk, mark it as an actual
        if (myfile.peek() == '*') {
            actual = true;
            myfile.ignore();
        }
        else {
            actual = false;
        }

        myfile >> value;

        if (actual) {
            freq[value + 511] += 1;
        }
        else {
            freq[value + 255] += 1;
        }
    }
    freq[EOF_SYMBOL] = 1;
    myfile.close();

    int count = 0, numsymbols = 0;
    SymbolVector symbols;

    for(int i = 0; i < 769; i++) {
        if (freq[i] != 0) {
            count++;
            numsymbols += freq[i];
        }
    }
    
    cout << "count is " << count << "\n";
    cout << "numsymbols is " << numsymbols << "\n";

    for (int i = 0; i < 769; i++) {
        if (freq[i] != 0) {
            symbols.push_back(Symbol(i, mpf_class(freq[i], 64) / numsymbols));
        }
    }
    
    mpf_class previous(0, 64);
    for (int i = 0; i < symbols.size(); i++) {
        Symbol &symbol = symbols[i];
        symbol.low = previous;
        symbol.high = previous + symbol.probability;
        previous = symbol.high;
    }
    
    outputSymbolVector(symbols);

    
    mpf_class l(0), h(1), r(h - l), code(0);

    int sym, numread = 0;

    ifstream read(infilename);
    cont = true;
    
    FILE *outfile = fopen(outfilename.c_str(), "w");
    
    writeSymbolVector(outfile, symbols);
    
    while (cont) {
        // Skip each space.
        if (read.peek() == ' ')
        {
            read.ignore();
            continue;
        }
        
        // If the next character is a newline, add it and continue
        if (read.peek() == '\n') {
            value = read.get();
            sym = NL_SYMBOL;
            cout<< "\n ------\\n \n";
        }
        else if (read.peek() == EOF) {
            value = read.get();
            sym = EOF_SYMBOL;
            cont = false;
            cout<< "\n ------EOF \n";
        }
        else {
            // If the next character is an asterisk, mark it as an actual
            if (read.peek() == '*') {
                actual = true;
                read.ignore();
            }
            else {
                actual = false;
            }

            read >> value;
            
            // Compute the symbol from the value
            if (actual) {
                sym = value + 511;
            }
            else {
                sym = value + 255;
            }
            
            cout<< "\n ------";
            if (actual)
                cout << "*";
            cout << value << "\n";
        }

        for (int i = 0; i < symbols.size(); i++) {
            Symbol &symbol = symbols[i];
            
            if(symbol.value == sym) {
                h = l + (r * symbol.high);
                l = l + (r * symbol.low);
                r = h - l;
                
                cout<<"\n low = " << l;
                cout<<"\n high = " << h;
                cout<<"\n range = " << r;
                cout<<"\n";
                break;
            }
        }
        
        numread++;
        
        // Write the current code if we reach 100 numbers or the end
        if (numread == 100 || !cont) {
            
            // Output an EOF code
            for (int i = 0; i < symbols.size(); i++) {
                Symbol &symbol = symbols[i];
                
                if(symbol.value == EOF_SYMBOL) {
                    h = l + (r * symbol.high);
                    l = l + (r * symbol.low);
                    r = h - l;
                    
                    cout<< "\n ------EOF \n";
                    cout<<"\n low = " << l;
                    cout<<"\n high = " << h;
                    cout<<"\n range = " << r;
                    cout<<"\n";
                    break;
                }
            }
            
            code = (l + h) / 2;
            cout<<"\n next arithmetic code for the file is : " << code << endl;

            mpf_out_raw(outfile, code.get_mpf_t());
            l = 0;
            h = 1;
            r = 1;
            code = 0;
            
            numread = 0;
        }
    }
    fclose(outfile);
    
    cout << endl << "[*] Wrote compressed file to " << outfilename << endl;
}

void decompress(string infilename, string outfilename) {
    SymbolVector symbols; 
    bool cont;
    
    mpf_class decode(0), l(0), h(1), r(1);
    ofstream decodefile(outfilename);
    cont = true;
    
    FILE *infile = fopen(infilename.c_str(), "rb");
    symbols = readSymbolVector(infile);

    while (true) {
        int c = fgetc(infile);
        
        if (c == EOF) {
            break;
        }
        else {
            ungetc(c, infile);
        }
        mpf_inp_raw(infile, decode.get_mpf_t());
        
        cout << "code: " << decode << endl;
        
        while (cont) {
            for(int i = 0; i < symbols.size(); i++) {
                Symbol &symbol = symbols[i];
        
                if (decode >= symbol.low && decode < symbol.high)
                {
                    int value = symbol.value;
                    
                    if (value < 511) {
                        decodefile << (value - 255) << " ";
                    }
                    else if (value < 767) {
                        decodefile << "*" << (value - 511) << " ";
                    }
                    else if (value == NL_SYMBOL) {
                        decodefile << endl;
                    }
                    else if (value == EOF_SYMBOL) {
                        cont = false;
                    }
                    
                    l = symbol.low;
                    h = symbol.high;
                    r = h - l;
                    decode = (decode - l) / r;
                    
                    // Stop looking for the right symbol
                    break;
                }
            }
        }
        
        cont = true;
    }
    
    cout << endl << "[*] Wrote decompressed file to " << outfilename << endl;
    
    string outnamepath = outfilename;
    removeFilename(outnamepath);
    outnamepath.append("/outname");
    system(("printf \"" + outfilename + "\" > \"" + outnamepath + "\"").c_str());
}

int main(int argc, char **argv)
{
    mpf_set_default_prec(700);
    
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

        outfilename.append("_4"); // for Arithmetic coding, option 4
        outfilename.append(ext);
    
        compress(infilename, outfilename);
    }
    else {        
        outfilename = infilename;
        ext = removeExtension(outfilename);
        ext.back() = 'c'; // Decompressed
        
        outfilename.erase(outfilename.length() - 2); // Remove the "_C" suffix
        outfilename.append(ext);
        
        decompress(infilename, outfilename);
    }

    return 0;
    
    // string filename,line;
    // bool actual, cont;
    // int value;
    // 
    // cout.precision(100);
    // mpf_set_default_prec(700);
    // 
    // 
    // //file open to calculate the frequency and probability
    // cout << "Enter the file name: ";
    // cin >> filename;
    // 
    // compress(filename);
    // 
    // decompress("value");
    // 
    // 
    // cout<<"\n\n";
    // return 0;
}//close main
