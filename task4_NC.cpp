// Phase 2
//
// Task 4: Compressiong
//      [yes] No compression
//      [x] Shannon Fano
//      [x] LZW
//      [x] Arithemetic Coding

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace std;
using namespace cv;

#define SPACE_SYMBOL 256
#define NL_SYMBOL 257
#define ASTERISK_SYMBOL 258

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

void compress(string infilename, string outfilename) {
    string line;
    bool actual;
      int value;

    ifstream infile;
    infile.open(infilename);

    ofstream outfile;
    outfile.open(outfilename, ios::binary);

    if (infile.is_open()) {
      int c;
        // Write the values to the file

        while (!infile.eof() && infile.peek() != EOF) {
            // Skip each space
            if (infile.peek() == ' ') {
                infile.get();
                c=SPACE_SYMBOL;
                outfile.write((char*)&c, sizeof c);// << ' ';
                continue;
            }

            // If the next character is a newline, add it and continue
            if (infile.peek() == '\n') {
                infile.get();
                c=NL_SYMBOL;
                outfile.write((char*)&c, sizeof c);// << endl;
                continue;
            }
            // If the next character is an asterisk, mark it as an actual
            if(infile.peek() == '*'){
                infile.get();
                c=ASTERISK_SYMBOL;
                outfile.write((char*)&c, sizeof c);// << "*";
            }

            infile >> value;
            outfile.write((char*)&value, sizeof value);
              //outfile << value;
        }
    }

    cout << endl << "[*] Wrote compressed file to " << outfilename << endl;
}

void decompress(string infilename, string outfilename) {
    string line;
    bool actual;
      int value;

    ifstream infile;
    infile.open(infilename,ios::binary);

    ofstream outfile;
    outfile.open(outfilename);

    if (infile.is_open()) {

        while (!infile.eof() && infile.peek() != EOF) {
            infile.read((char*)&value, sizeof value);

            if (value == SPACE_SYMBOL) {
                outfile << ' ';
            } 
            else if (value == NL_SYMBOL) {
                outfile << '\n';
            }
            else if (value == ASTERISK_SYMBOL) {
                outfile << '*';
            }
            else {
                outfile << value;
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

        outfilename.append("_1"); // for No compression, option 1.
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
}
