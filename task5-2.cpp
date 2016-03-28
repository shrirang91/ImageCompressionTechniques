// Phase 2
//
// Task 5-2: decoding spacial predictive coding

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

// 1. Non-predictive coding
int decodePredictorNone(Mat *decoding, int error, int y, int x, int frame) {
    return error;
}

//predictor A
int decodePredictor2(Mat *decoding, int error, int y, int x, int frame) {

    //DECODING
    int prediction = decoding[frame].at<uchar>(y, x-1);
    int actual = prediction + error;
    
    return actual;
}

//predictor B
int decodePredictor3(Mat *decoding, int error, int y, int x, int frame) {

    int prediction = decoding[frame].at<uchar>(y-1, x);
    int actual = prediction + error;
    
    return actual;
}

//Predictor C
int decodePredictor4(Mat *decoding, int error, int y, int x, int frame) {
    
    int prediction = decoding[frame].at<uchar>(y-1, x-1);
    int actual = prediction + error;
    
    return actual;
}


//Predictor a1*A+a2*B+a3*C
int decodePredictor5(Mat *decoding, int error, int y, int x, int frame) {

    int p0 = decoding[frame].at<uchar>(y - 1, x);
        
    int p1 = decoding[frame].at<uchar>(y, x - 1);
    int p2 = decoding[frame].at<uchar>(y, x - 2);
    int p3 = decoding[frame].at<uchar>(y, x - 3);
    int p4 = decoding[frame].at<uchar>(y, x - 4);
    
    int p5 = decoding[frame].at<uchar>(y - 1, x - 1);
    int p6 = decoding[frame].at<uchar>(y - 1, x - 2);
    int p7 = decoding[frame].at<uchar>(y - 1, x - 3);
    int p8 = decoding[frame].at<uchar>(y - 1, x - 4); 
    
    
    /*
     x8  x7  x6  x5  x0
     x4  x3  x2  x1  x
    */
     
    Mat A = (Mat_<double>(4, 3) << 
        1,  1,  1, 
        p2, p5, p6,
        p3, p6, p7,
        p4, p7, p8
    );
    
    Mat B = (Mat_<double>(4, 1) <<
        1,
        p1,
        p2,
        p3
    );
    
    Mat X(Mat_<double>(3, 1));
    solve(A, B, X, DECOMP_QR);  // Solve Ax = B for x
     
    double aa = X.at<double>(0, 0);
    double ab = X.at<double>(1, 0);
    double ac = X.at<double>(2, 0);
    
    // Assignment says "αa and αb and αc are three non-negative values", so if either 
    // coefficient is negative, use 0.3 instead.
    if (aa < 0 || ab < 0 || ac < 0)
        aa = ab = ac = 1.0/3;

    int prediction = int(aa*p1 + ab*p0 + ac*p5);
    int actual = prediction + error;

    return actual;
}

Decoder getDecoder(int algorithm) {
    switch (algorithm) {
        case 2: return &decodePredictor2;   //predictorA
        case 3: return &decodePredictor3;   //predictorB
        case 4: return &decodePredictor4;   //predictorC
        case 5: return &decodePredictor5;   //predictorA+B+C
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
    dimstream >> fcount;
    dimstream >> dummy;
    
    // Allocate memory for the decoding process.
    Mat *decoded = new Mat[fcount];
    for (f = 0; f < fcount; f++) {
        decoded[f] = Mat(10, 10, CV_8UC1);
    }
    
    cout << endl << "[*] Allocated memory for " << fcount << " frames" << endl;;
    
    int frameIndex = 0;
    while (getline(codedFile, line)) {
        
        stringstream linestream(line);
        int value;
        bool actual;
        
        int pixelIndex = 0;
        
        if (frameIndex >= fcount)
            break;
        
        while (!linestream.eof() && linestream.peek() != EOF) {
            
            int r = pixelIndex / 10;
            int c = pixelIndex % 10;
            
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
            //cout << "Row: " << r << endl;
            //cout << "Column: " << c << endl;
            //cout << "Frame: " <<frameIndex << endl;
            //cout << "Read value: " << value << endl;
            
            // Decode the values that aren't correct
            if (!actual) {
                value = decoder(decoded, value, r, c, frameIndex);
                //cout << "Decoded value: " << value << endl;
            }
            
            // Write the value to the frame
            Mat frame = decoded[frameIndex];
            frame.at<uchar>(r, c) = value;
            
            pixelIndex++;
        }
        
        frameIndex++;
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


