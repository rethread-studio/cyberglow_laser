#pragma once

// Flag for seeing everything on the computer screen
#define DEBUG_MODE true



#include "ofMain.h"
#include <unordered_map>
#include "ofxOsc.h"
#include "ofxLaserManager.h"
#include "ActivityPoint.hpp"
#include "EventLine.hpp"
#include "LaserText.hpp"
#include "TextFlow.hpp"
#include "PlayerTrail.hpp"
#include "WebServerVis.h"
#include "RainDrop.hpp"


enum class VisMode {
WEBSERVER = 0,
USER,
ZOOMED_OUT,
TEXT_DEMO,
RAIN,
LAST,
};

enum class TransitionType {
NONE,
SPIN,
ZOOM_IN,
ZOOM_OUT,
};

class Transition {
	public:
		TransitionType type;
		VisMode from_vis;
		VisMode to_vis;
		float phase = 0.0;
		float duration = 2.0;
		glm::vec3 spin_axis = glm::vec3(0, 0, 1.0);
		float spin_radians = 0.0;
		glm::vec2 zoom_target = glm::vec2(0, 0);
		float zoom_distance = 4000;

		Transition() {
			type = TransitionType::SPIN;
			from_vis = VisMode::ZOOMED_OUT;
			to_vis = VisMode::WEBSERVER;
			spin_axis= glm::vec3(0, 1.0, 0);
			phase = 0.0;
			duration = 2.0;
		}


		void update(float dt) {
			phase += dt/duration;
			if(phase >= 1.0) {
				type = TransitionType::NONE;
			}

			switch(type) {
				case TransitionType::SPIN:
				{
					spin_radians = phase * PI * 0.5;
					break;
				}
				case TransitionType::ZOOM_IN:
				{
					break;
				}
				case TransitionType::ZOOM_OUT:
				{
					break;
				}
				case TransitionType::NONE:
					break;
			}

		}

		void applyTransitionFrom() {
			switch(type) {
				case TransitionType::SPIN:
				{
					ofRotateRad(spin_radians, spin_axis.x, spin_axis.y, spin_axis.z);
					break;
				}
				case TransitionType::ZOOM_IN:
				{
					float target_phase = min(phase * 2.0, 1.0) * -1;
					ofTranslate(zoom_target.x*target_phase, zoom_target.y*target_phase, zoom_distance * phase);
					break;
				}
				case TransitionType::ZOOM_OUT:
				{
					float target_phase = min(phase * 2.0, 1.0) * -1;
					ofTranslate(zoom_target.x*target_phase, zoom_target.y*target_phase, zoom_distance * phase * -1);
					break;
				}
			}
		}

		void applyTransitionTo() {
			switch(type) {
				case TransitionType::SPIN:
				{
					ofRotateRad(PI * -0.5 + spin_radians, spin_axis.x, spin_axis.y, spin_axis.z);
					break;
				}
				case TransitionType::ZOOM_IN:
				{
					float target_phase = (1.0 - min(phase * 3.0, 1.0)) * -1;
					ofTranslate(zoom_target.x*target_phase, zoom_target.y*target_phase, zoom_distance * -4 + (zoom_distance * phase * 4));

					break;
				}
				case TransitionType::ZOOM_OUT:
				{
					float target_phase = (1.0 - min(phase * 3.0, 1.0)) * -1;
					ofTranslate(zoom_target.x*target_phase, zoom_target.y*target_phase, zoom_distance  - (zoom_distance * phase ));
					break;
				}
			}
		}

		bool active() {
			return type != TransitionType::NONE;
		}
};

const size_t TriangleVIS = 0;
const size_t TriangleSERVER = 1;
const size_t TriangleUSER = 2;

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

		void drawTransition(Transition transition_);
		void drawVisualisation(VisMode vis);

                void transitionToFrom(VisMode from, VisMode to);

                VisMode vis_mode = VisMode::WEBSERVER;
		Transition transition;

		ofxLaser::Manager laser;
		glm::vec2 triangle_positions[3];
		float triangle_activity[3];
		vector<ActivityPoint> activity_points;
		vector<EventLineColumn> event_line_columns;
		unordered_map<string, PlayerTrail> player_trails;

		vector<RainDrop> raindrops;
		vector<ofColor> colors;

		WebServerVis web_server_vis;
		vector<LaserText> laser_texts;
		TextFlow text_flow;
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
