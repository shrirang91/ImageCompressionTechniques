// Phase 2
//
// Task 4: Compression

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

int main(int argc, char **argv) {
    string infilename;
    
    cout << endl << "Enter the file name: ";
    cin >> infilename;
    
    int compressionChoice;
    string compressionCommand;
    
    // Obtain the desired predictive coding algorithm
    cout << endl << "Compression algorithms: " << endl;
    cout << "    1. No compression" << endl;
    cout << "    2. Shannon-Fano encoding" << endl;
    cout << "    3. LZW encoding" << endl;
    cout << "    4. Arithmetic encoding" << endl;
    cout << "Enter the number of the compression algorithm to use: ";
    cin >> compressionChoice;
    
    switch (compressionChoice) {
        case 1:
            compressionCommand = "task4_NC";
            break;
            
        case 2:
            compressionCommand = "task4-shannon";
            break;
            
        case 3:
            compressionCommand = "task4LZW";
            break;
    
        case 4:
            compressionCommand = "task4_ACD";
            break;
    }
    
    execl(compressionCommand.c_str(), compressionCommand.c_str(), infilename.c_str(), "1", NULL);
    
    return 0;
}

