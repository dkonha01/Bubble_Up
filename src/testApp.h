#pragma once

#include "ofMain.h"


#include "ofxOpenCv.h"
#include "ofxCv.h"



enum AppMode {Scan, Streak, Magic}; //{like a scanner, like a streak camera, magic}
class testApp : public ofBaseApp{
	
public:
    
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void saveImage();
    

    ofVideoGrabber 		vidGrabber;
    
    ofPixels previous;
	ofImage diff;
    ofFbo fbo;
    ofPixels fboPixels;
    ofImage image;
    ofImage thresh;
    
    
    
    unsigned char * pixels;
    unsigned char * videoMirror;
    
    unsigned char * diffMirror;
    
    
    int 				camWidth = 640;
    int 				camHeight = 480;
    int     displayWidth=800;
    int     displayHeight=600;
    
    bool         reverseTime = true;
    int         mode = 0;
    int         maxMode = 6; //0:raw 1: 40x40 2: 8x8; 3: 6x2; 4: 4x1; 5 paint; 6: no Bubble
    
    bool        bShowVideo=false;
    bool bSourceInvert = false;
    bool bFlipVertically=false, bFlipHorizontally=true;
    
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayLastImage;
    //    ofxCvGrayscaleImage grayPaintImage;
    
    ofxCvGrayscaleImage grayDiffImage;
    ofxCvGrayscaleImage grayTimeDepthImage;
    ofxCvGrayscaleImage grayTimeDepthImage_x40y40;
    ofxCvGrayscaleImage grayTimeDepthImage_x8y8;
    ofxCvGrayscaleImage grayTimeDepthImage_x6y2;
    ofxCvGrayscaleImage grayTimeDepthImage_x4y1;
    
    
    ofxCvColorImage colorImage;
    ofxCvGrayscaleImage grayWarpImage;
    
    int bufferMulti =1;
    int bufferDepth=256*bufferMulti+2;
    int cutRatio=4;
    std::vector<ofxCvGrayscaleImage> grayArray;
    
    int numFrames = 0;
    int currentFrame = 0;
    int lastFrame = 0;
    int nextFrame = 0;
    
    
    void setGrayArray(); //for StreakMagic
    void setGrayWarpImage();
    void setGrayWarpImage_dxy(int x_size, int y_size);
    void paintImage();
    void drawDelayImage();
    

};