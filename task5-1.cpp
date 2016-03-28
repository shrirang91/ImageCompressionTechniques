// Phase 2
//
// Task 5-1: decoding temporal predictive coding

#include <fstream>
#include <iostream>
#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <climits>
#define ERROR_USE_ACTUAL INT_MIN

using namespace std;
using namespace cv;

typedef int (* Decoder)(Mat *decoding, int error, int row, int col, int frame);

// 1. No predictive coding
int decodePredictorNone(Mat *decoding, int error, int row, int col, int frame) {
    return error;
}

// 2. Predictor s[t] = s[t − 1]
int decodePredictor2(Mat *decoding, int error, int row, int col, int frame) {
    int prediction = decoding[frame - 1].at<uchar>(row, col);
    int actual = prediction + error;
    
    return actual;
}

// 3. Predictor s[t] = (s[t−1] + s[t−2]) / 2
int decodePredictor3(Mat *decoding, int error, int row, int col, int frame) {
    int prev1 = decoding[frame - 1].at<uchar>(row, col);
    int prev2 = decoding[frame - 2].at<uchar>(row, col);
    int prediction = (prev1 + prev2) / 2;
    int actual = prediction + error;
    
    return actual;
}

// 4. Predictor s[t] = α1 × s[t−1] + α2 × s[t-2]
int decodePredictor4(Mat *decoding, int error, int row, int col, int frame) {
    int prev1 = decoding[frame - 1].at<uchar>(row, col);
    int prev2 = decoding[frame - 2].at<uchar>(row, col);
    int prev3 = decoding[frame - 3].at<uchar>(row, col);
    int prev4 = decoding[frame - 4].at<uchar>(row, col);

    
    /* 
     * Solve the linear system of equations:
     *      a1*1  + a2*1  = 1
     *      a1*s2 + a2*s3 = s1
     *      a1*s3 + a2*s4 = s2
     */
     
    Mat A = (Mat_<double>(3, 2) << 
        1,     1,
        prev2, prev3, 
        prev3, prev4
    );
    
    Mat B = (Mat_<double>(3, 1) <<
        1,
        prev1,
        prev2
    );
    
    Mat x(Mat_<double>(2, 1));
    solve(A, B, x, DECOMP_QR);  // Solve Ax = B for x
     
    double a1 = x.at<double>(0, 0);
    double a2 = x.at<double>(1, 0);
    
    // Assignment says "α1 and α2 are two non-negative values", so if either 
    // coefficient is negative, use 0.5 instead.
    if (a1 < 0 || a2 < 0)
        a1 = a2 = 0.5;

    int prediction = int(a1*prev1 + a2*prev2);
    int actual = prediction + error;

    return actual;
}

Decoder getDecoder(int algorithm) {
    switch (algorithm) {
        case 2: return &decodePredictor2;
        case 3: return &decodePredictor3;
        case 4: return &decodePredictor4;
        default: return &decodePredictorNone;
    }
}

string removeExtension(string name) {
    string::size_type index = name.rfind('.');
    
    if (index != string::npos) {
        return name.substr(0, index);
    }
    else {
        return name;
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

int main(int argc, const char * argv[]) {
    string filename;
    bool ok;        
    
    int f, r, c;
    
    if (argc == 2) {
        filename = argv[1];
    }
    else {
        // Obtain the filename the user wants to operate on in this run
        cout << endl << "Enter error file name: ";
        cin >> filename;
    }
        
    // Obtain the predictor choice from the file name
    string withoutext = removeExtension(filename);
    int predictor = withoutext.at(withoutext.size() - 3) - '0';
    Decoder decoder = getDecoder(predictor);
    
    // Read the error file
    ifstream codedFile;
    codedFile.open(filename);        
    
    // Skip the first line.
    string line;
    getline(codedFile, line);
    
    // Get the number of frames to decode into
    int dummy, fcount;
    stringstream dimstream(line);
    dimstream >> dummy;
    dimstream >> fcount;
    
    // Allocate memory for the decoding process.
    Mat *decoded = new Mat[fcount];
    for (f = 0; f < fcount; f++) {
        decoded[f] = Mat(10, 10, CV_8UC1);
    }
    
    cout << endl << "[*] Allocated memory for " << fcount << " frames" << endl;;
    
    int pixelIndex = 0;
    while (getline(codedFile, line)) {
        int r = pixelIndex / 10;
        int c = pixelIndex % 10;
        
        stringstream linestream(line);
        int value;
        bool actual;
        
        int frameIndex = 0;
        
        while (!linestream.eof() && linestream.peek() != EOF) {
            if (frameIndex >= fcount)
                break;
                
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
            
            // Decode the values that aren't correct
            if (!actual) {
                value = decoder(decoded, value, r, c, frameIndex);
            }
                            
            // Write the value to the frame
            Mat frame = decoded[frameIndex];
            frame.at<uchar>(r, c) = value;
            
            frameIndex++;
        }
                    
        pixelIndex++;
    }
    
    string outname = removeExtension(filename) + "_decoded.mov";
    int codec = CV_FOURCC('m', 'p', '4', 'v');
    VideoWriter writer;
    writer.open(outname, codec, 24.0, Size(10,10), true);
    if (!writer.isOpened()) {
        cout << endl << "[*] FATAL: Couldn't open file to write video" << endl;
        return -1;
    }
    
    Mat cframe;
    
    for (f = 0; f < fcount; f++) {
        cvtColor(decoded[f], cframe, CV_GRAY2BGR);
        writer.write(cframe);
    }
    
    cout << endl << "[*] Wrote output to " << outname << endl;
    
    string outnamepath = outname;
    removeFilename(outnamepath);
    outnamepath.append("/outname");
    system(("printf \"" + outname + "\" > \"" + outnamepath + "\"").c_str());
    
    return 0;
}

