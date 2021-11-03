#ifndef _WEBSERVERVIS_H_
#define _WEBSERVERVIS_H_

#include "ofMain.h"
#include "ofxLaserManager.h"


class WebServerVis {
    public:
    static std::unordered_map<std::string, size_t> node_action_map;

        vector<float> activity;
        vector<float> activity_smoothed;
        vector<int> activity_received; // tracks the number of frames since activity was received
        float radius = 400.0;
        ofColor color = ofColor::green;

        WebServerVis() {
            // init
            if(node_action_map.size() == 0) {
            }
            for(auto& el : node_action_map) {
                cout << "key: " << el.first << endl;
            }
            activity = vector<float>(17, 0.0);
            activity_smoothed = vector<float>(17, 0.0);
            activity_received = vector<int>(17, 0.0);
        }

        void register_node(string action) {
            auto it = node_action_map.find(action);
            if(it != node_action_map.end()) {
                register_activity(it->second);
            }
        }
        void register_server(string action) {
            size_t index = 0;
            if(action == "file") {
                index = 13;
            } else if(action == "api"){
                index = 14;
            }
            if(index != 0) {
                register_activity(index);
            }
        }
        void register_mongodb(string action) {
            size_t index = 0;
            if(action == "find") {
                index = 15;
            } else if(action == "findOne"){
                index = 16;
            }
            if(index != 0) {
                register_activity(index);
            }
        }
        void register_activity(size_t index) {
                activity[index]+= 1.0;
                activity_received[index] = 10;
        }

        void update() {
            for(auto& activity : activity) {
                activity *= 0.95;
            }
            float smooth_r = 0.1;
            for(int i = 0; i < activity.size(); i++) {
                activity_smoothed[i] = activity_smoothed[i] * (1. - smooth_r) + activity[i] * smooth_r;
            }
            for(auto& activity : activity_received) {
                activity -= 1;
            }
        }

        void draw(ofxLaser::Manager& laser) {
            ofColor col = color;
            ofPolyline poly;
            auto profile = OFXLASER_PROFILE_DEFAULT;
            float oversampling = 4;
            #ifdef DEBUG_MODE
            float dot_intensity = 1.0; // So we can see the dots on the screen
            #else
            float dot_intensity = 1.0 / float(activity.size());
            #endif
            float last_x = -1, last_y = -1;
            float last_value = 0.0;

            for(float i = 0; i <= activity.size(); i += 1.0/oversampling) {
                size_t down_i = size_t(floor(i)) % activity.size();
                size_t up_i = size_t(ceil(i)) % activity.size();
                float mix = i - floor(i);
                float v0 = activity_smoothed[down_i];
                float v1 = activity_smoothed[up_i];
                float value = v0 + mix * (v1-v0);
                float angle = (TWO_PI/float(activity.size())) * i;
                float r = radius * powf(1.0/(value + 1.0), 0.5);
                // float r = radius - value * 60.0;
                float x = cos(angle)*r;
                float y = sin(angle)*r;
                if(value > 0.1 || last_value > 0.1) {
                    if(last_x != -1 && last_y != -1) {
                        laser.drawLine(last_x, last_y, x, y, col, profile);
                    }
                } else if(abs(i-round(i)) < 0.01) {
                    // natural number dot
                    laser.drawDot(x, y, col, dot_intensity, profile);
                }
                last_x = x;
                last_y = y;
                last_value = value;
                // poly.addVertex(x, y, 0);
                if( mix < 1.0/oversampling ) {
                    if(activity_received[down_i] > 0) {
                        laser.drawLine(0, 0, x, y, col, profile);
                    }
                }
            }
            laser.drawPoly(poly, col, profile);
        }
};
#endif
