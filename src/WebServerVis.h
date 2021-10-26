#ifndef _WEBSERVERVIS_H_
#define _WEBSERVERVIS_H_

#include "ofMain.h"
#include "ofxLaserManager.h"


class WebServerVis {
    public:
    static std::unordered_map<std::string, size_t> node_action_map;

        vector<float> node_activity;
        float radius = 400.0;
        ofColor color = ofColor::green;

        WebServerVis() {
            // init
            if(node_action_map.size() == 0) {
            }
            for(auto& el : node_action_map) {
                cout << "key: " << el.first << endl;
            }
            node_activity = vector<float>(13, 0.0);
        }

        void register_node(string action) {
            cout << "Register node action \"" << action << "\"" << endl;
            auto it = node_action_map.find(action);
            if(it != node_action_map.end()) {
                node_activity[it->second] += 1.0;
                cout << "Increasing node activity" << endl;
            }
        }

        void update() {
            for(auto& activity : node_activity) {
                activity *= 0.8;
            }
        }

        void draw(ofxLaser::Manager& laser) {
            ofColor col = color;
            ofPolyline poly;
            auto profile = OFXLASER_PROFILE_FAST;
            for(size_t i = 0; i <= node_activity.size(); i++) {
                float angle = (TWO_PI/float(node_activity.size())) * i;
                float r = radius - node_activity[i%node_activity.size()] * 40.0;
                poly.addVertex(cos(angle)*r, sin(angle)*r, 0);
            }
            laser.drawPoly(poly, col, profile);
        }
};
#endif
