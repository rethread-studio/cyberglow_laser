#pragma once

#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ActivityPoint.hpp"
#include "EventLine.hpp"
#include "LaserText.hpp"
#include "PlayerTrail.hpp"

class ofApp : public ofBaseApp{

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
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void addRandomActivityPoint();

	ofxLaser::Manager laser;
		glm::vec2 triangle_positions[3];
		vector<ActivityPoint> activity_points;
		vector<EventLineColumn> event_line_columns;
		PlayerTrail player_trail;
		// Dimensions of the laser canvas
		int width = 2000;
		int height = 1000;

		float scan_x = 0.0;
		float scan_width = 40.0;

};
