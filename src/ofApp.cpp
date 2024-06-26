#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    // ofSetFrameRate(30);
    ofSetVerticalSync(true);

    ofPixels pixels;

    ofLoadImage(pixels, "background.png");
    background.allocate(pixels.getWidth(), pixels.getHeight());
    background.setFromPixels(pixels);

    IMG_WIDTH = pixels.getWidth();
    IMG_HEIGHT = pixels.getHeight();
    IMG_WIDTH_2 = IMG_WIDTH / 2;
    IMG_HEIGHT_2 = IMG_HEIGHT / 2;
    IMG_WIDTH_4 = IMG_WIDTH_2 / 2;
    IMG_HEIGHT_4 = IMG_HEIGHT_2 / 2;

    ofLoadImage(pixels, "frame.png");
    frame.allocate(pixels.getWidth(), pixels.getHeight());
    frame.setFromPixels(pixels);

    // (*zoomedImage).allocate(pixels.getWidth(), pixels.getHeight());
    // (*zoomedImage).clear();
    
    pixels.clear();
    pixels.allocate(IMG_WIDTH, IMG_HEIGHT, OF_IMAGE_GRAYSCALE);
    ofxCvGrayscaleImage _p;
    _p.allocate(IMG_WIDTH, IMG_HEIGHT);
    _p.setFromPixels(pixels);
    
    for (int i = 0; i < 10; i++) {
        steps.push_back(_p);
        stepn.push_back("");
    }

    gui.setup();
    gui.add(thresholdLow.set("far clipping", 150, 0, 255));
    gui.add(maskThreshold.set("mask threshold", 1, 0, 255));
    gui.add(maskBlur.set("mask blur", 11, 0, 255));
    gui.add(gaussianBlur.set("final gaussian blur", 11, 0, 255));
    gui.add(showNormal.set("show normalmap", false));
    gui.setPosition(IMG_WIDTH_2, 10);

    shader.load("normal");
    
}

//--------------------------------------------------------------
void ofApp::update(){

    if (gaussianBlur%2 == 0) gaussianBlur++;

    // frame far clip
    stepn[0] = "frame far clip";
    cvThreshold(frame.getCvImage(), steps[0].getCvImage(), thresholdLow, 0, CV_THRESH_TOZERO_INV);

    // background far clip
    stepn[1] = "bg far clip";
    cvThreshold(background.getCvImage(), steps[1].getCvImage(), thresholdLow, 0, CV_THRESH_TOZERO_INV);

    // absdiff frame - background
    stepn[2] = "absdiff bg - frame";
    steps[2] = steps[0];
    cvAbsDiff(steps[1].getCvImage(), steps[0].getCvImage(), steps[2].getCvImage());
    //steps[2] -= steps[1];

    // create mask
    stepn[3] = "b/w (threshold)";
    steps[3] = steps[2];
    steps[3].threshold(maskThreshold);


    // remove holes (floorfill)
    // preparation
    stepn[4] = "floodfill holes";
    steps[4] = steps[3];    
    mask.allocate(steps[4].getWidth()+2, steps[4].getHeight()+2);
    mask.clear();

    // extract holes
    stepn[7] = "*holes";
    cvFloodFill(steps[4].getCvImage(), cvPoint(0,0), cvScalar(255), cvScalar(0), cvScalarAll(0), comp CV_DEFAULT(NULL), CV_FLOODFILL_FIXED_RANGE, mask.getCvImage());
    steps[4].invert();
    steps[7] = steps[4]; // for debugging only

    // merge inversed holes with original
    cvOr(steps[3].getCvImage(), steps[4].getCvImage(), steps[4].getCvImage());

    // erode mask
    stepn[5] = "erode\nremove noise";
    steps[5] = steps[4];
    steps[5].erode();

    // gaussianblur
    stepn[6] = "blur";
    steps[6] = steps[5];
    steps[6].blurGaussian(gaussianBlur);

    // try 
    // normalmap with sobel
    // https://gamedev.stackexchange.com/questions/165575/calculating-normal-map-from-height-map-using-sobel-operator
    // https://docs.opencv.org/4.x/d5/d0f/tutorial_py_gradients.html

    // // mask the frame
    // stepn[7] = "mask\nODR1 * frame";
    // steps[7] = steps[0];
    // steps[7] *= steps[4];
    
    // stepn[8] = "contours";
    // getPolyContour(5, 8);

    for (int i = 0; i < 10; i++) {
        steps[i].flagImageChanged();
    }

    glReadPixels(mouseX, ofGetHeight() - mouseY, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, &colorPicker);
    //ofLog() << ofToString(colorPicker);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);

    background.draw(0, 0, IMG_WIDTH_4, IMG_HEIGHT_4);
    frame.draw(IMG_WIDTH_4, 0, IMG_WIDTH_4, IMG_HEIGHT_4);
    ofDrawBitmapString("background", 0, IMG_HEIGHT_4 + 10);
    ofDrawBitmapString("frame", IMG_WIDTH_4, IMG_HEIGHT_4 + 10);

    for (int i = 0; i < 10; i++) {
        steps[i].draw(IMG_WIDTH_4 * i, IMG_HEIGHT_4 + 40, IMG_WIDTH_4, IMG_HEIGHT_4);
        ofDrawBitmapString(ofToString(i) + "\n" + stepn[i], IMG_WIDTH_4 * i, (IMG_HEIGHT_2 + 40) + 10);
    }
    if (zoomedImage != nullptr) {
        (*zoomedImage).draw(0, IMG_HEIGHT_2 + 80, IMG_WIDTH, IMG_HEIGHT);
    }
    ofDrawBitmapString("zooming " + ofToString(zoomed) + "\n" + stepn[zoomed], 0, IMG_HEIGHT + IMG_HEIGHT_2 + 10);

    stringstream colorString;
    colorString << "Color under mouse: \n" << ofToString(colorPicker);
    ofDrawBitmapString(colorString.str(), IMG_WIDTH, 20);

    if (showNormal) {
        steps[6].getTextureReference().bind();
        shader.begin();
        steps[6].draw(IMG_WIDTH, IMG_HEIGHT_2+80, IMG_WIDTH, IMG_HEIGHT);
        shader.end();
        steps[6].getTextureReference().unbind();
    }

    //thresholdLow = (int) ofRandom(0, 100);

    gui.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

void ofApp::mousePressed(int x, int y, int button) {
    int i = ceil (x / IMG_WIDTH_4);
    if ((y > IMG_HEIGHT_4 + 40) && (y < IMG_HEIGHT_2 + 40)) {
        i = clamp(i, 0, 9);
        zoomed = i;
        zoomedImage = & (steps[i]);
        ofLog() << "zooming step " + ofToString(zoomed);
    }
    else if ((y < IMG_HEIGHT_4)) {
        if (x < IMG_WIDTH_4) zoomedImage = & background;
        else if (x < IMG_WIDTH_4*2) zoomedImage = & frame;
    }
}


void ofApp::getPolyContour(int from, int to) {
    contourFinder.findContours(steps[from], minArea, maxArea, 10, false, true);

    polygons.clear();
    _polyLineCount = 0;
    for (const auto& blob : contourFinder.blobs) {
        ofPolyline polyline;
        for (const auto& point : blob.pts) {
            polyline.addVertex(point.x, point.y);
        }
        polyline.setClosed(true);
        polyline.simplify(2.0f);
        polygons.push_back(polyline);
        _polyLineCount += polyline.size();
    }

    cv::Mat mat = ofxCvGrayscaleImage().getCvMat();
    for (int i = 0; i < polygons.size(); i++) {
        vector<cv::Point> contour;
        for (int j = 0; j < polygons[i].size(); j++) {
            cv::Point pt(polygons[i][j].x, polygons[i][j].y);
            contour.push_back(pt);
        }

        cv::fillConvexPoly(mat, &contour[1], contour.size(), cv::Scalar(250));
    }
    steps[to].set(0);
    steps[to].setFromPixels(mat.data, mat.rows, mat.cols);
}

void ofApp::saveImage(ofxCvGrayscaleImage img, string name, string step) {
    const string& filename = ofGetTimestampString() + "_" + name + "_" + step + ".png";
    ofSaveImage(img.getPixels(), filename);
}

