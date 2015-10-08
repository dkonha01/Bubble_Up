#include "testApp.h"

using namespace ofxCv;
using namespace cv;



//--------------------------------------------------------------
void testApp::setup(){
	
    
        vidGrabber.setVerbose(true);
        cout<<"listing devices"<<endl;
        int numCam = vidGrabber.listDevices().size();
        cout<<"end listing devices, num devices: "<<numCam<<endl;
        vidGrabber.setDeviceID(numCam-1); //use the last cam in the chain, so if a firewire cam is attached, it will be used
        vidGrabber.setDesiredFrameRate(30);
        vidGrabber.initGrabber(camWidth,camHeight);
        ofSetVerticalSync(true);
    
    thresh.allocate(640, 480, OF_IMAGE_GRAYSCALE);
    
	
	// imitate() will set up previous and diff
	// so they have the same size and type as cam
//	imitate(previous, vidGrabber);
//	imitate(diff, vidGrabber);
    //imitate(thresh, cam);
    
    fbo.allocate(640, 480);
    // clear fbo
    fbo.begin();
    ofClear(255,255,255, 0);
    fbo.end();




    ofSetFrameRate(120);
    
    grayLastImage.allocate(camWidth, camHeight);
    grayDiffImage.allocate(camWidth, camHeight);
    grayTimeDepthImage.allocate(camWidth, camHeight);
    grayWarpImage.allocate(camWidth, camHeight);
    grayTimeDepthImage_x40y40.allocate(40, 40);
    grayTimeDepthImage_x8y8.allocate(8, 8);
    grayTimeDepthImage_x6y2.allocate(6, 2);
    grayTimeDepthImage_x4y1.allocate(4, 1);
    
 
    
    grayLastImage.set(0);
    grayDiffImage.set(0);
    grayTimeDepthImage.set(0);
    grayTimeDepthImage_x40y40.set(0);
    grayTimeDepthImage_x8y8.set(0);
    grayTimeDepthImage_x6y2.set(0);
    grayTimeDepthImage_x4y1.set(0);
    
    for(int i=0; i<bufferDepth; i++){
        ofxCvGrayscaleImage tempImage;
        tempImage.allocate(camWidth, camHeight);
        grayArray.push_back(tempImage);
    }
    

}


//--------------------------------------------------------------
void testApp::update(){
    bool bNewFrameReady = false;
    unsigned char * pixels;
    vidGrabber.update();
    
        if(vidGrabber.isFrameNew()){
            bNewFrameReady = true;
            pixels = vidGrabber.getPixels();
            
            
        }
 
    
    if(bNewFrameReady){
        colorImage.setFromPixels(pixels, camWidth, camHeight);
      
        grayImage=colorImage;
        
        if(bSourceInvert) grayImage.invert();
        
        grayImage.mirror(bFlipVertically, bFlipHorizontally);
        grayDiffImage.absDiff(grayImage, grayLastImage);
        grayDiffImage.brightnessContrast(0.2, 0.3);
       
        grayDiffImage.blurHeavily();
        grayDiffImage.blurHeavily();
        grayDiffImage.blurHeavily();
        
        grayDiffImage.threshold(13);
        
        
        
        grayTimeDepthImage-=4;
        // grayTimeDepthImage-=12;
       
        grayTimeDepthImage+=grayDiffImage;
        
        
       // grayTimeDepthImage.invert();
        
        grayTimeDepthImage.updateTexture();
        ofTexture& txt = grayTimeDepthImage.getTextureReference();
    
        
        grayTimeDepthImage_x40y40.scaleIntoMe(grayTimeDepthImage);
        grayTimeDepthImage_x8y8.scaleIntoMe(grayTimeDepthImage_x40y40);
        grayTimeDepthImage_x6y2.scaleIntoMe(grayTimeDepthImage);
        grayTimeDepthImage_x4y1.scaleIntoMe(grayTimeDepthImage_x8y8);
        
        setGrayArray();
        switch(mode){
            case 0:
                setGrayWarpImage();
                break;
            case 1:
                setGrayWarpImage_dxy(40, 40);
                break;
            case 2:
                setGrayWarpImage_dxy(8, 8);
                break;
            case 3:
                setGrayWarpImage_dxy(6, 2);
                break;
            case 4:
                setGrayWarpImage_dxy(4, 1);
                break;
            case 5:
                paintImage();
                break;
            case 6:
                drawDelayImage();
                break;
            default:
                cout<<"error in mode number\n";
                break;
        }
     
        grayLastImage = grayImage;
        numFrames++;
	}

    
}

//--------------------------------------------------------------

void testApp::draw(){
    ofBackground(0,0,0);
	ofSetHexColor(0xffffff);
       grayWarpImage.draw(0, 0, displayWidth, displayHeight);
 
	
    if(bShowVideo){
     
            vidGrabber.draw(20,20+displayHeight, camWidth/2, camHeight/2);
            ofDrawBitmapString("App FPS: "+ ofToString(ofGetFrameRate())  + " "+ ofToString(22%10), 20,15+displayHeight);
        }
        grayDiffImage.draw(40+ camWidth/2, 20+displayHeight, camWidth/2, camHeight/2);
        grayTimeDepthImage.draw(60+ camWidth, 20+displayHeight, camWidth/2, camHeight/2);
        grayTimeDepthImage_x40y40.draw(80+camWidth*3/2, 620);
        grayArray[nextFrame].draw(40+displayWidth, 200, camWidth/2, camHeight/2);
        
    }



void testApp::setGrayArray(){
    //    copy current video image to buffer array
    unsigned char* ptrNewImage = grayImage.getPixels();
    //    cout<<currentFrame<<endl;
    grayArray[currentFrame].setFromPixels(ptrNewImage, camWidth, camHeight);
    lastFrame = currentFrame;
    currentFrame++;
    
    if(currentFrame>=bufferDepth) {currentFrame = 0;}
    nextFrame = currentFrame;
    
    //    cout<<"last: "<<lastFrame<<", cur: "<<currentFrame<<", next: "<<nextFrame<<endl;
}


void testApp::setGrayWarpImage(){
   grayWarpImage.invert();
    unsigned char* ptr = grayWarpImage.getPixels();
    unsigned char* ptrDepth = grayTimeDepthImage.getPixels();
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        for(int x=0; x<camWidth;x++){
            idx++;
            int depth=0;
            
            
            if(reverseTime){
                depth = (lastFrame - ptrDepth[idx]/cutRatio*bufferMulti);// % (256*bufferMulti);
            }else{
                depth = (lastFrame - (256 - ptrDepth[idx])/(cutRatio*bufferMulti));// % (256*bufferMulti);
            }
            
            
            
            if(depth<0){depth+= bufferDepth;}
            //            printf("%d\n", depth);
            unsigned char* tptr = grayArray[depth].getPixels();
            ptr[idx]=tptr[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}

void testApp::setGrayWarpImage_dxy(int x_size, int y_size){
    unsigned char* ptr = grayWarpImage.getPixels();
    unsigned char* ptrDepth;
    switch(mode){
        case 1:
            ptrDepth= grayTimeDepthImage_x40y40.getPixels();
            break;
        case 2:
            ptrDepth= grayTimeDepthImage_x8y8.getPixels();
            break;
        case 3:
            ptrDepth= grayTimeDepthImage_x6y2.getPixels();
            break;
        case 4:
            ptrDepth= grayTimeDepthImage_x4y1.getPixels();
            break;
        default:
            cout<< "error in mode\n";
            break;
    }
    
    
    int delta_x = camWidth/x_size;
    int delta_y = camHeight/y_size;
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        int idx_y = (y/delta_x)*x_size;
        for(int x=0; x<camWidth;x++){
            idx++;
            int idx_x = x / delta_y;
            int idx_depth = idx_x + idx_y;
            int depth = 0;
            /*
            if(reverseTime){
                depth = (currentFrame -1 - ptrDepth[idx_depth]/cutRatio);// % bufferDepth;
                
            }else{
                depth = (currentFrame -1 - (bufferDepth - ptrDepth[idx_depth])/cutRatio);// % bufferDepth;
            }
            
            */
            
            
            
            depth = (currentFrame -1 - (bufferDepth - ofNoise(ptrDepth[idx_depth]))/cutRatio);// % bufferDepth;

            
            
            
            if(depth<0){depth+= bufferDepth;}
            //            printf("%d\n", depth);
            unsigned char* tptr = grayArray[depth].getPixels();
            ptr[idx]=tptr[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}



void testApp::paintImage(){
    unsigned char* ptr = grayWarpImage.getPixels();
    unsigned char* ptrDepth = grayTimeDepthImage.getPixels();
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        for(int x=0; x<camWidth;x++){
            idx++;
            ptr[idx]=ptrDepth[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}


void testApp::drawDelayImage(){
    
    unsigned char* ptr = grayWarpImage.getPixels();
    int depth=(lastFrame - ofNoise(256/cutRatio*bufferMulti));// % (256*bufferMulti);
   // int depth=(lastFrame - 512/cutRatio*bufferMulti);// % (256*bufferMulti);
    if(depth<0){depth+= bufferDepth;}
    unsigned char* tptr = grayArray[depth].getPixels();
    
    for(int y=0; y<camHeight; y++){
        int idx = y*camWidth;
        for(int x=0; x<camWidth;x++){
            idx++;
            ptr[idx]=tptr[idx];
        }
    }
    grayWarpImage.setFromPixels(ptr, camWidth, camHeight);
}


void testApp::saveImage(){
    time_t now;
    char the_date[24];
    
    the_date[0] = '\0';
    
    now = time(NULL);
    
    if (now != -1)
    {
        strftime(the_date, 24, "%y_%m_%d_%H_%M_%S", gmtime(&now));
    }
    string fname;
    fname = "capture/"+string(the_date) + ".jpg";
    cout<<fname<<endl;
    
    ofImage tempImage;
    tempImage.setFromPixels(grayWarpImage.getPixels(), camWidth, camHeight, OF_IMAGE_GRAYSCALE);
    tempImage.saveImage(fname);
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
    
	switch(key){
        case 'v':
            bShowVideo = !bShowVideo;
            break;
        case 'h':
            bFlipHorizontally = !bFlipHorizontally;
            break;
        case 'f':
            ofToggleFullscreen();
            break;
        case ' ':
            saveImage();
            break;
        case '+':
            if(cutRatio>1) cutRatio/=2;
            cout<<"cutRaio is "<<cutRatio<<endl;
            break;
        case '-':
            if(cutRatio<128) cutRatio*=2;
            cout<<"cutRaio is "<<cutRatio<<endl;
            break;
        case 'm':
            mode++;
            if(mode>maxMode) mode=0;
            cout<<"mode is "<<mode<<endl;
            break;
        case 'n':
            if(mode!=6) {
                mode=6;
            }else{
                mode = 0;
            }
            cout<<"mode is "<<mode<<endl;
            break;
        case 'r':
            reverseTime = !reverseTime;
            break;
        case 'w':
            if(displayWidth==800){
                displayWidth = 1024;
                displayHeight = 768;
            }else{
                displayWidth=800;
                displayHeight=600;
            }
            break;
       
    }
    
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
    
}
