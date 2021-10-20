#pragma once

#include "ofMain.h"
#include <unordered_map>
#include "ofxOsc.h"
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
		void pickRandomPlayerTrail();
		void addActivityPoint(int source);

	ofxLaser::Manager laser;
		glm::vec2 triangle_positions[3];
		vector<ActivityPoint> activity_points;
		vector<EventLineColumn> event_line_columns;
		unordered_map<string, PlayerTrail> player_trails;
		string current_player_trail_id = "";
		// Dimensions of the laser canvas
		int width = 2000;
		int halfw = width/2;
		int height = 1000;
		int halfh = height/2;

		float scan_x = 0.0;
		float scan_width = 100.0;
		float mouse_rel_x = 0.0;
		float mouse_rel_y = 0.0;
		float noise_counter = 0.0;
		float rot_y = 0.0;
		float rot_x = 0.0;


		// **************** OSC ****************
		ofxOscReceiver receiver;
		int PORT = 57131;
		void checkOscMessages();
		void parseOscMessage(string origin, string action, string arguments);
};
