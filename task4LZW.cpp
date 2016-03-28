// Phase 2
//
// Task 4: Compressiong
//  [x] No compression
//  [x] Shannon Fano
//  [x] LZW
//  [x] Arithemetic Coding
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <math.h>
#include <algorithm>
#include <unordered_map>
#include <bitset>
#include <sstream>

using namespace std;



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


string codeToBits(int bits, int code) {
    string bitstream;
    
    for (int x = bits - 1; x >= 0; x--){
        bitstream.append(1, (code % 2 == 0) ? '0' : '1');
        code = code / 2;
    }
    
    return bitstream;
}

int bitsToCode(string bits) {
    int code = 0;
    
    for (int i = 0; i < bits.size(); i++) {
        int bit = bits.at(i) - '0';
        
        code += bit * int(pow(2, i));
    }
    
    return code;
}

void outputBitString(string code, ofstream &outfile) {
    if (code.size() < 8) {
        code.append(8 - code.size(), '1');
    }
    
    bitset<8> bits(code);
    unsigned char num = static_cast<unsigned char>(bits.to_ulong());
    
    outfile.write((char *)(void *)&num, sizeof(unsigned char));
}

void compress(string infilename, int bits, string outfilename) {
    string line;
    char c = ' ';
    string s = "";
    int value,i;
    bool actual;
    int encoding = 0;
    unordered_map<string, int> dictionary;
    
    //open the input file and the output file
    fstream myfile (infilename);
    ofstream myofile;
    myofile.open (outfilename, ios::binary);
    myofile << bits << endl;
    
    //build the basic dictionary with base characters
    dictionary.emplace("*", encoding++);
    dictionary.emplace(" ", encoding++);
    dictionary.emplace("-", encoding++);
    dictionary.emplace("0", encoding++);
    dictionary.emplace("1", encoding++);
    dictionary.emplace("2", encoding++);
    dictionary.emplace("3", encoding++);
    dictionary.emplace("4", encoding++);
    dictionary.emplace("5", encoding++);
    dictionary.emplace("6", encoding++);
    dictionary.emplace("7", encoding++);
    dictionary.emplace("8", encoding++);
    dictionary.emplace("9", encoding++);
    dictionary.emplace("\n", encoding++);
    
    int maxentries = int(pow(2, bits));
    string codebuffer;
    
    if (myfile.is_open())
    {
        int chars = 0;
        while (!myfile.eof() && myfile.peek() != EOF)
        {
            c = myfile.get();
            chars++;
            
            if (dictionary.count(s + c) != 0) {
                // s + c exists in the dictionary
                s.append(1, c);
            }
            else {
                // s + c does not exist in the dictionary
                if (encoding < maxentries) {
                    int code = dictionary[s];
                    string binary = codeToBits(bits, code);

                    //print to EF s
                    codebuffer.append(binary);
                    
                    // There's still room in the dictionary to add the new code
                    //add to the dictionary with incremented encoding value
                    dictionary.emplace(s + c, encoding++); 
                    
                    //reset the string to the latest char read
                    s = c;
                }
                else {
                    // TODO: Flush the dictionary
                    
                    int code = dictionary[s];
                    string binary = codeToBits(bits, code);

                    //print to EF s
                    codebuffer.append(binary);
                    
                    //reset the string to the latest char read
                    s = c;
                }
            }
            
            if (codebuffer.size() >= 8) {
                // Extract the first 128 bits from the string
                string bits = codebuffer.substr(0, 8);
                codebuffer.erase(0, 8);
                
                // Output the bits
                outputBitString(bits, myofile);
            }
        }
        
        //done reading, print the last code to file
        if (s.size() != 0){
            int code = dictionary[s];
            string binary = codeToBits(bits, code);
            
            //print to EF s
            codebuffer.append(binary);
            
            //reset the string to the latest char read
            s = c;
        }
        
        while (codebuffer.size() >= 8) {
            // Extract the first 128 bits from the string
            string bits = codebuffer.substr(0, 8);
            codebuffer.erase(0, 8);
            
            // Output the bits
            outputBitString(bits, myofile);
        }
        
        if (!codebuffer.empty()) {
            outputBitString(codebuffer, myofile);
        }
        //while close
    }
    
    cout << "[*] Wrote compressed file to " << outfilename << endl;
}

void outputDictionary(string *dictionary, int maxentries) {
    for (int i = 0; i < maxentries; i++) {
        if (dictionary[i].size() > 0)
            cout << "d[" << i << "] = " << dictionary[i] << endl;
    }
    
    cout << endl; 
}

void decompress(string infilename, string outfilename) {
    ifstream myfile (infilename);
	ofstream myofile;
    
    string s;
    int codes;
    
	myofile.open (outfilename);	
    
    int bits;
    myfile >> bits;
    
    // Skip the new line
    myfile.ignore();
    
	codes = pow(2, bits); 
	string *dictionary = new string[codes];
	int encoding = 0, i;
	for(i =0; i < codes; i++){
		dictionary[i] = "";
	}

	dictionary[encoding++] = "*";
	dictionary[encoding++] = " ";
	dictionary[encoding++] = "-";
	dictionary[encoding++] = "0";
	dictionary[encoding++] = "1";
	dictionary[encoding++] = "2";
	dictionary[encoding++] = "3";
	dictionary[encoding++] = "4";
	dictionary[encoding++] = "5";
	dictionary[encoding++] = "6";
	dictionary[encoding++] = "7";
	dictionary[encoding++] = "8";
	dictionary[encoding++] = "9";
    dictionary[encoding++] = "\n";
    
    string codebuffer;
    int maxentries = int(pow(2, bits));
    
    cout << "Max entries: " << maxentries << endl;

    string decompressed;

	while(!myfile.eof() && myfile.peek() != EOF){ //loop through lines
        int c = myfile.get();
        bitset<8> nextbyte(c);
        codebuffer += nextbyte.to_string();
        
        if (codebuffer.size() < bits) {
            continue;
        }
        
        int encode = bitsToCode(codebuffer.substr(0, bits));
        codebuffer.erase(0, bits);
        
        string out;
        
		if (dictionary[encode].compare("") == 0) {
			cout << "error, unknown code " << encode << endl;
            out = s + s.front();
        }
        else {
            out = dictionary[encode];
        }
        
		myofile << out;
        
        if (encoding < maxentries) {
    		if(s.compare("") != 0) {
    			dictionary[encoding++] = s + out.front();
            }
        }
        
		s = out;	
        
	
	}	
    
    cout << "[*] Wrote decompressed file to " << outfilename << endl;
    
    string outnamepath = outfilename;
    removeFilename(outnamepath);
    outnamepath.append("/outname");
    system(("printf \"" + outfilename + "\" > \"" + outnamepath + "\"").c_str());
}

int main(int argc, char **argv)
{
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
        
        cout << endl << "Enter number of bits of dictionary: ";
        int bits;
        cin >> bits;
            
        outfilename = infilename;
        ext = removeExtension(outfilename);
        ext.back() = 'v'; // Compressed

        outfilename.append("_3"); // for LZW coding, option 2.
        outfilename.append(ext);
    
        compress(infilename, bits, outfilename);
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