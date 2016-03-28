// Phase 2
//
// Task 2: spacial predictive coding
//      [x] Predictor 1
//      [x] Predictor 2
//      [x] Predictor 3
//      [x] Predictor 4
//      [x] Predictor 5

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
    int ***errors;      // What actually gets output in the SPC file
    uchar ***actuals;   // Actual values, used in the predictor functions
} PredictiveCoderData;

typedef int (* PredictiveCoder)(PredictiveCoderData *encoding, uchar value, int row, int col, int frame);

// 1. Non-predictive coding
int predictorNone(PredictiveCoderData *encoding, uchar value, int y, int x, int frame) {
    encoding->errors[frame][y][x] = ERROR_USE_ACTUAL;
    encoding->actuals[frame][y][x] = value;
    
    // No error in the predictor-less algorithm.
    return 0;
}

//predictor A
int predictor2(PredictiveCoderData *encoding, uchar value, int y, int x, int frame) {
    if (x == 0) {
        // Not enough data to use predictor (need at least one previous)
        encoding->actuals[frame][y][x] = value;
        encoding->errors[frame][y][x] = ERROR_USE_ACTUAL;
        return 0;
    }
    
    int prediction = encoding->actuals[frame][y][x-1];
    int error = value - prediction;
         encoding->actuals[frame][y][x] = value;
         encoding->errors[frame][y][x] = error;
   

        return error;
}

//predictor B
int predictor3(PredictiveCoderData *encoding, uchar value, int y, int x, int frame) {
    if (y == 0) {
        // Not enough data to use predictor (need at least one previous)
        encoding->actuals[frame][y][x] = value;
        encoding->errors[frame][y][x] = ERROR_USE_ACTUAL;
        return 0;
    }
    
    int prediction = encoding->actuals[frame][y-1][x];
    int error = value - prediction;
         encoding->actuals[frame][y][x] = value;
         encoding->errors[frame][y][x] = error;
   

        return error;

}

//Predictor C
int predictor4(PredictiveCoderData *encoding, uchar value, int y, int x, int frame) {
    if (y == 0 || x == 0) {
        // Not enough data to use predictor (need at least one previous)
        encoding->actuals[frame][y][x] = value;
        encoding->errors[frame][y][x] = ERROR_USE_ACTUAL;
        return 0;
    }
    
    int prediction = encoding->actuals[frame][y-1][x-1];
    int error = value - prediction;
         encoding->actuals[frame][y][x] = value;
         encoding->errors[frame][y][x] = error;
 
         return error;
}

int predictor5(PredictiveCoderData *encoding, uchar value, int y, int x, int frame) {
    if (y == 0 || x < 4) {
        // Not enough data to use predictor (need at least one previous)
        encoding->actuals[frame][y][x] = value;
        encoding->errors[frame][y][x] = ERROR_USE_ACTUAL;
        return 0;
    }
 
    int p0 = encoding->actuals[frame][y - 1][x];
 
    int p1 = encoding->actuals[frame][y][x - 1];
    int p2 = encoding->actuals[frame][y][x - 2];
    int p3 = encoding->actuals[frame][y][x - 3];
    int p4 = encoding->actuals[frame][y][x - 4];
    
    int p5 = encoding->actuals[frame][y - 1][x - 1];
    int p6 = encoding->actuals[frame][y - 1][x - 2];
    int p7 = encoding->actuals[frame][y - 1][x - 3];
    int p8 = encoding->actuals[frame][y - 1][x - 4];    
    
    
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
    int error = value - prediction;
    encoding->actuals[frame][y][x] = value;
    encoding->errors[frame][y][x] = error;

    return error;
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

PredictiveCoder getPredictiveCoder(int algorithm) {
    switch (algorithm) {
        case 2: return &predictor2;
        case 3: return &predictor3;
        case 4: return &predictor4;
    	case 5: return &predictor5;
        default: return &predictorNone;
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
        int f, x, y, width, height;
        int yc, xc;
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
	
	//get the predictor to use from the user 
	cout << "Enter X coordinate of 10x10 region to process: ";
        cin >> xc;
        
        if (xc < 0 || xc > (width - 10)) {
            cout << endl << "[*] ERROR: Invalid X coordinate. Restarting." << endl;
            continue;
        }
        
        // Obtain the y coordinate of the 10x10 region
        cout << "Enter Y coordinate of 10x10 region to process: ";
        cin >> yc;
        
        if (yc < 0 || yc > (height - 10)) {
            cout << endl << "[*] ERROR: Invalid Y coordinate. Restarting." << endl;
            continue;
        }

	//get the predictor to use
        cout << endl << "Predictive coding algorithms: " << endl;
        cout << "    1. No PC (use original values)" << endl;
        cout << "    2. Predictor A" << endl;
        cout << "    3. Predictor B" << endl;
        cout << "    4. Predictor C" << endl;
    	cout << "    5. Predictor a X A + b X B + c X C" << endl;
        cout << "Enter the number of the predictive coding algorithm to use: ";
        cin >> choice;
        encoder = getPredictiveCoder(choice);
        
        // Allocate memory for the encoding process.
        PredictiveCoderData *encoding = (PredictiveCoderData *)malloc(sizeof(PredictiveCoderData));
        encoding->errors = (int ***)malloc(fcount * sizeof(int **));
        encoding->actuals = (uchar ***)malloc(fcount * sizeof(uchar **));
        for (f = 0; f < fcount; f++) {
            encoding->errors[f] = (int **)malloc(10 * sizeof(int *));
            encoding->actuals[f] = (uchar **)malloc(10 * sizeof(uchar *));
            
            for (y = 0; y < 10; y++) {
                encoding->errors[f][y] = (int *)malloc(10 * sizeof(int));
                encoding->actuals[f][y] = (uchar *)malloc( 10 * sizeof(uchar));
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
            
            for (y = yc; y < yc+10; y++) {
                for (x = xc; x < xc+10; x++) {
                    Vec3b pixel = frame.at<Vec3b>(y, x);
                    uchar yval = pixel.val[0];
                    error += abs(encoder(encoding, yval, y-yc, x-xc, findex));
                }
            }
            
            cout << "[*] Processed frame " << findex << endl;
        }
        
        // Output the SPC values.
        ofstream spcFile;
        string outname = removeExtension(filename) + "_" + to_string(choice) + ".spc";
        spcFile.open(outname);
        spcFile << fcount << " " << 100 << endl;

        for (findex = 0; findex < fcount; findex++) {
            for (y = 0; y < 10; y++) {
                for (x = 0; x < 10; x++) {
                    int error = encoding->errors[findex][y][x];
                    
                    if (error == ERROR_USE_ACTUAL) {
                        spcFile << "*" << int(encoding->actuals[findex][y][x]) << " ";
                    }
                    else {
                        spcFile << encoding->errors[findex][y][x] << " ";
                    }
                }
            }
            
            spcFile << endl;
        }
        spcFile.close();
        
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
