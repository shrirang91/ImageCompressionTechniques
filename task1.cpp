// Phase 2
//
// Task 1: temporal predictive coding
//      [x] Predictor 1
//      [x] Predictor 2
//      [x] Predictor 3
//      [x] Predictor 4

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

typedef struct PredictiveCoderData {
    int ***errors;      // What actually gets output in the TPC file
    uchar ***actuals;   // Actual values, used in the predictor functions
} PredictiveCoderData;

typedef int (* PredictiveCoder)(PredictiveCoderData *encoding, uchar value, int row, int col, int frame);

// 1. No predictive coding
int predictorNone(PredictiveCoderData *encoding, uchar value, int row, int col, int frame) {
    encoding->errors[row][col][frame] = ERROR_USE_ACTUAL;
    encoding->actuals[row][col][frame] = value;
    
    // No error in the predictor-less algorithm.
    return 0;
}

// 2. Predictor s[t] = s[t − 1]
int predictor2(PredictiveCoderData *encoding, uchar value, int row, int col, int frame) {
    if (frame < 1) {
        // Not enough data to use predictor (need at least one previous)
        encoding->actuals[row][col][frame] = value;
        encoding->errors[row][col][frame] = ERROR_USE_ACTUAL;
        return 0;
    }
    
    int prediction = encoding->actuals[row][col][frame - 1];
    int error = value - prediction;
    encoding->actuals[row][col][frame] = value;
    encoding->errors[row][col][frame] = error;
    
    return error;
}

// 3. Predictor s[t] = (s[t−1] + s[t−2]) / 2
int predictor3(PredictiveCoderData *encoding, uchar value, int row, int col, int frame) {
    if (frame < 2) {
        // Not enough data to use predictor (need at least two previous)
        encoding->actuals[row][col][frame] = value;
        encoding->errors[row][col][frame] = ERROR_USE_ACTUAL;
        return 0;
    }
    
    int prediction = (encoding->actuals[row][col][frame - 1] + encoding->actuals[row][col][frame - 2]) / 2;
    int error = value - prediction;
    encoding->actuals[row][col][frame] = value;
    encoding->errors[row][col][frame] = error;
    
    return error;
}

// 4. Predictor s[t] = α1 × s[t−1] + α2 × s[t-2]
int predictor4(PredictiveCoderData *encoding, uchar value, int row, int col, int frame) {
    if (frame < 4) {
        // Not enough data to use predictor (need at least four previous to compute predictions)
        encoding->actuals[row][col][frame] = value;
        encoding->errors[row][col][frame] = ERROR_USE_ACTUAL;
        return 0;
    }
   
    int prev1 = encoding->actuals[row][col][frame - 1];
    int prev2 = encoding->actuals[row][col][frame - 2];
    int prev3 = encoding->actuals[row][col][frame - 3];
    int prev4 = encoding->actuals[row][col][frame - 4];
    
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
    int error = value - prediction;
    encoding->actuals[row][col][frame] = value;
    encoding->errors[row][col][frame] = error;

    return error;
}

PredictiveCoder getPredictiveCoder(int algorithm) {
    switch (algorithm) {
        case 2: return &predictor2;
        case 3: return &predictor3;
        case 4: return &predictor4;
        default: return &predictorNone;
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

int main(int argc, const char * argv[]) {
    string path;
    bool running = true;
    
    cout << "Enter the path of the folder containing the videos: ";
    cin >> path;
    
    while (running) {
        string filename;
        string mapname;
        bool ok;        
        
        Mat frame, ychan;
        int x, y, width, height;
        int r, c;
        int findex, fcount;
        
        PredictiveCoder encoder; 
        int choice;
                        
        // Obtain the filename the user wants to operate on in this run
        cout << endl << "Enter video file name: ";
        cin >> filename;
        
        // Open a capture object to the video
        VideoCapture cap(path + "/" + filename);
        if (!cap.isOpened()) {
            cout << endl << "[*] ERROR: Couldn't open the video for processing. Restarting." << endl;
            continue; // Restart processing
        }
        
        fcount = cap.get(CV_CAP_PROP_FRAME_COUNT);
        cout << "[*] Frame count for video is: " << fcount << endl;
        
        width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
        height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
        
        cout << "[*] Frame size for video is: " << width << " x " << height << endl;
                
        // Obtain the x coordinate of the 10x10 region
        cout << endl << "Enter X coordinate of 10x10 region to process: ";
        cin >> x;
        
        if (x < 0 || x > (width - 10)) {
            cout << endl << "[*] ERROR: Invalid X coordinate. Restarting." << endl;
            continue;
        }
        
        // Obtain the y coordinate of the 10x10 region
        cout << "Enter Y coordinate of 10x10 region to process: ";
        cin >> y;
        
        if (y < 0 || y > (height - 10)) {
            cout << endl << "[*] ERROR: Invalid Y coordinate. Restarting." << endl;
            continue;
        }
        
        // Obtain the desired predictive coding algorithm
        cout << endl << "Predictive coding algorithms: " << endl;
        cout << "    1. No PC (use original values)" << endl;
        cout << "    2. Predictor s[t] = s[t − 1]" << endl;
        cout << "    3. Predictor s[t] = (s[t−1] + s[t−2]) / 2" << endl;
        cout << "    4. Predictor s[t] = α1 × s[t−1] + α2 × s[t-2]" << endl;
        cout << "Enter the number of the predictive coding algorithm to use: ";
        cin >> choice;
        encoder = getPredictiveCoder(choice);
        
        // Allocate memory for the encoding process.
        // Create a multidimensional array with size
        // 10x10x{fcount}, which is row by column by frame.
        PredictiveCoderData *encoding = (PredictiveCoderData *)malloc(sizeof(PredictiveCoderData));
        encoding->errors = (int ***)malloc(10 * sizeof(int **));
        encoding->actuals = (uchar ***)malloc(10 * sizeof(uchar **));
        for (r = 0; r < 10; r++) {
            encoding->errors[r] = (int **)malloc(10 * sizeof(int *));
            encoding->actuals[r] = (uchar **)malloc(10 * sizeof(uchar *));
            
            for (c = 0; c < 10; c++) {
                encoding->errors[r][c] = (int *)malloc(fcount * sizeof(int));
                encoding->actuals[r][c] = (uchar *)malloc(fcount * sizeof(uchar));
            }
        }
        
        cout << endl << "[*] Allocated encoder memory" << endl;
        
        // Extract and process each frame
        int error = 0;
        for (findex = 0; findex < fcount; findex++) {
            cap.set(CV_CAP_PROP_POS_FRAMES, findex);
            ok = cap.read(frame);
            if(!ok){
                cout << endl << "[*] ERROR: Couldn't extract frame " << findex << ". Restarting." << endl;
                continue;
            }
            
            // Convert the frame into YUV from BGR.
            cvtColor(frame, frame, CV_BGR2YUV);
            
            for (r = y; r < y + 10; r++) {
                for (c = x; c < x + 10; c++) {
                    Vec3b pixel = frame.at<Vec3b>(r, c);
                    uchar yval = pixel.val[0];
                    error += abs(encoder(encoding, yval, r - y, c - x, findex));
                }
            }
            
            cout << "[*] Processed frame " << findex << endl;
        }
        
        // Output the TPC values.
        ofstream tpcFile;
        string outname = removeExtension(filename) + "_" + to_string(choice) + ".tpc";
        tpcFile.open(outname);
        // Number of lines (100 signals), followed by number of columns of values (fcount values)
        tpcFile << 100 << " " << fcount << endl;
        for (r = 0; r < 10; r++) {
            for (c = 0; c < 10; c++) {
                for (findex = 0; findex < fcount; findex++) {
                    int error = encoding->errors[r][c][findex];
                    
                    if (error == ERROR_USE_ACTUAL) {
                        tpcFile << "*" << int(encoding->actuals[r][c][findex]) << " ";
                    }
                    else {
                        tpcFile << encoding->errors[r][c][findex] << " ";
                    }
                }
                
                tpcFile << endl;
            }
        }
        tpcFile.close();
        
        cout << endl << "[*] Wrote output to " << outname << endl;
        cout << endl << "[*] Total amount of error: " << error << endl;
        
        while (true) {
            int opt;
            
            cout << endl << "Select which action to take: " << endl;
            cout << "    1. Process another video" << endl;
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
