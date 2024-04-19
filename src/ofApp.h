#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mousePressed(int x, int y, int button);

		ofxCvGrayscaleImage background;
		ofxCvGrayscaleImage frame;		
		int zoomed;
		vector<ofxCvGrayscaleImage> steps;
		ofxCvGrayscaleImage* zoomedImage;
		vector<string> stepn;

		ofxCvContourFinder contourFinder;
		vector<ofPolyline> polygons;
		int _polyLineCount;
		vector<cv::Point> contours;

		void saveImage(ofxCvGrayscaleImage img, string name, string step);
		void getPolyContour(int from, int to);

		int IMG_WIDTH;
		int IMG_HEIGHT;
		int IMG_HEIGHT_2;
		int IMG_WIDTH_2;
		int IMG_HEIGHT_4;
		int IMG_WIDTH_4;

		ofxPanel gui;
		ofParameter<int> thresholdLow;
		ofParameter<int> thresholdHigh;
		ofParameter<int> maskThreshold;
		ofParameter<int> minArea;
		ofParameter<int> maxArea;
		ofParameter<int> gaussianBlur;
		ofParameter<int> maskBlur;

		ofColor colorPicker;
		//stringstream colorString;
		//unsigned char[3] pixel;

};
