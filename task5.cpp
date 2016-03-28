// Phase 2
//
// Task 5: Video Viewing

#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

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

string getOutname() {
    ifstream infile("/tmp/task5temp/outname");
    stringstream outname;
    
    outname << infile.rdbuf();
    
    return outname.str();
}

void displayVideo(string file) {
    VideoCapture cap(file);
    
    if (!cap.isOpened()) {
        cout << endl << "[*] ERROR: Couldn't open the video for display." << endl;
        system(("open \"" + file + "\"").c_str());
        return;
    }
    
    int fcount = cap.get(CV_CAP_PROP_FRAME_COUNT);
    Mat *frames = new Mat[fcount];
    Size size(200, 200);
    for (int i = 0; i < fcount; i++) {
        cap.read(frames[i]);
        resize(frames[i], frames[i], size, 0, 0, INTER_NEAREST);
    }
    
    // Show the frame
    double delay = 1000/24;
    int f = 0;
    while (true) {
        imshow("Video", frames[f]);
        char key = waitKey(delay);
        
        if (key == 27) // ESC
            return;
        
        f++;
        
        if (f >= fcount)
            f = 0;
    }
}

int main(int argc, char **argv) {
    string infilename, outfilename, ext;
    string outname;
    
    string decompressCommand;
    string decodeCommand;
    
    cout << endl << "Enter the file name: ";
    cin >> infilename;
    
    int compressionChoice;
    bool temporal;
    
    temporal = (infilename.at(infilename.size() - 3) == 't');
    compressionChoice = infilename.at(infilename.size() - 5) - '0';
    
    // Setup the temporary directories in /tmp
    system("mkdir /tmp/task5temp");
    system("rm /tmp/task5temp/*");
    system(("cp \"" + infilename + "\" /tmp/task5temp").c_str());
    
    string newfilename = "/tmp/task5temp/" + infilename;
    
    if (compressionChoice == 1) {
        decompressCommand = "task4_NC";
    }
    else if (compressionChoice == 2) {
        decompressCommand = "task4-shannon";
    }
    else if (compressionChoice == 3) {
        decompressCommand = "task4LZW";
    }
    else if (compressionChoice == 4) {
        decompressCommand = "task4_ACD";
    }
    
    if (temporal) {
        decodeCommand = "task5-1";
    }
    else {
        decodeCommand = "task5-2";
    }
    
    int status;
    
    // Fork to call the decompression child
    if (fork() == 0) {
        execl(decompressCommand.c_str(), decompressCommand.c_str(), newfilename.c_str(), "2", NULL);
    }
    else {
        // Wait for the child process
        wait(&status);
    }
        
    // Read the filename of the output of the decompressor
    outname = getOutname();
    
    // Fork to call the decoder child
    if (fork() == 0) {
        execl(decodeCommand.c_str(), decodeCommand.c_str(), outname.c_str(), NULL);
    }
    else {
        // Wait for the child process
        wait(&status);
    }
    
    // Read the filename of the output of the decoder
    outname = getOutname();
    
    // Display the video
    displayVideo(outname);
    
    return 0;
}

