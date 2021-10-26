#ifndef _WEBSERVERVIS_H_
#define _WEBSERVERVIS_H_

#include "ofMain.h"
#include "ofxLaserManager.h"


class WebServerVis {
    public:
    static std::unordered_map<std::string, size_t> node_action_map;

        vector<float> node_activity;
        vector<int> node_activity_received; // tracks the number of frames since activity was received
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
            node_activity_received = vector<int>(13, 0.0);
        }

        void register_node(string action) {
            auto it = node_action_map.find(action);
            if(it != node_action_map.end()) {
                node_activity[it->second] += 1.0;
                node_activity_received[it->second] = 5;
            }
        }

        void update() {
            for(auto& activity : node_activity) {
                activity *= 0.90;
            }
            for(auto& activity : node_activity_received) {
                activity -= 1;
            }
        }

        void draw(ofxLaser::Manager& laser) {
            ofColor col = color;
            ofPolyline poly;
            auto profile = OFXLASER_PROFILE_DEFAULT;
            float oversampling = 4;
            for(float i = 0; i <= node_activity.size(); i += 1.0/oversampling) {

                size_t down_i = size_t(floor(i)) % node_activity.size();
                size_t up_i = size_t(ceil(i)) % node_activity.size();
                float mix = i - floor(i);
                float v0 = node_activity[down_i];
                float v1 = node_activity[up_i];
                float value = v0 + mix * (v1-v0);
                float angle = (TWO_PI/float(node_activity.size())) * i;
                float r = radius - value * 60.0;
                float x = cos(angle)*r;
                float y = sin(angle)*r;
                poly.addVertex(x, y, 0);
                if( mix < 1.0/oversampling ) {
                    if(node_activity_received[down_i] > 0) {
                        laser.drawLine(0, 0, x, y, col, profile);
                    }
                }
            }
            laser.drawPoly(poly, col, profile);
        }
};
#endif
