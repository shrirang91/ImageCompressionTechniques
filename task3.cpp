// Phase 2
//
// Task 3: error quantization
//      [ ] No quantization
//      [ ] Uniform quantization

#include <fstream>
#include <iostream>
#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

typedef struct QuantizerData {
    int bits;
    int bins;
} QuantizerData;

typedef int (* Quantizer)(QuantizerData *data, int value);

// 1. No quantizing
int quantizerNone(QuantizerData *data, int value) {
    return value;
}

// 2. Quantize uniformly in 2^m bins
int quantizerUniform(QuantizerData *data, int value) {
    // Error values will be between -255 and 255.
    int bin = floor(data->bins * (255 + value) / 512.0);
    float width = 512.0 / data->bins;
    int representative = -255 + floor(width*(bin + 0.5));
    
    return representative;
}

Quantizer getQuantizer(int algorithm) {
    switch (algorithm) {
        case 2: return &quantizerUniform;
        default: return &quantizerNone;
    }
}

bool hasSuffix(string const &full, string const &suffix) {
    if (full.length() >= suffix.length()) {
        return (0 == full.compare(full.length() - suffix.length(), suffix.length(), suffix));
    } 
    else {
        return false;
    }
}

int main(int argc, const char * argv[]) {
    bool running = true;
    
    while (running) {
        string filename;
        bool ok;        
        
        Quantizer quantizer; 
        int choice;
        
        // Obtain the desired predictive coding algorithm
        cout << endl << "Quantizing algorithms: " << endl;
        cout << "    1. No quantization" << endl;
        cout << "    2. Uniform quantization" << endl;
        cout << "Enter the number of the quantization algorithm to use: ";
        cin >> choice;
        quantizer = getQuantizer(choice);
                        
        // Obtain the filename the user wants to operate on in this run
        cout << endl << "Enter error file name: ";
        cin >> filename;
        
        
        // Allocate memory for the quantization process.
        QuantizerData *data = (QuantizerData *)malloc(sizeof(QuantizerData));
        data->bits = 0;
        data->bins = 1;
        
        // Obtain the number of bits to use, if the user chose quantization
        if (choice == 2) {
            cout << endl << "Enter number of bits to use for quantization: ";
            int bits;
            cin >> bits;
            data->bits = bits;
            
            for (int i = 0; i < bits; i++) 
                data->bins *= 2;
        }
        
        // Read the error file
        ifstream codedFile;
        codedFile.open(filename);        
        
        // Output the quantization values.
        ofstream quantFile;
        string outname = filename;
        outname[outname.length() - 1] = 'q';
        outname.insert(outname.length() - 4, "_" + to_string(data->bits));
        quantFile.open(outname);
        
        // Skip the first line.
        string line;
        getline(codedFile, line);
        quantFile << line << endl;
        
        while (getline(codedFile, line)) {
            stringstream linestream(line);
            int value;
            bool actual;
            
            while (!linestream.eof() && linestream.peek() != EOF) {
                // Skip each space.
                if (linestream.peek() == ' ') {
                    linestream.get();
                    continue;
                }
                
                // If the next character is an asterisk, mark it as an actual
                if (linestream.peek() == '*') {
                    actual = true;
                    linestream.get();
                }
                else {
                    actual = false;
                }
                
                linestream >> value;
                
                // Skip over the first values in each line that aren't errors
                if (actual) {
                    quantFile << "*" << value << " ";
                } 
                else {
                    quantFile << quantizer(data, value) << " ";
                }
            }
            
            quantFile << endl;
        }
        quantFile.close();
        
        cout << endl << "[*] Wrote output to " << outname << endl;
        
        ofstream tpcFile;
        
        
        while (true) {
            int opt;
            
            cout << endl << "Select which action to take: " << endl;
            cout << "    1. Quantize another error file" << endl;
            cout << "    2. Exit" << endl;
            cout << "Enter choice: ";
            cin >> opt;
            
            if (opt == 1) {
                break;
            }
            else if (opt == 2) {
                return 0;
            }
        }
    }
    
    return 0;
}
