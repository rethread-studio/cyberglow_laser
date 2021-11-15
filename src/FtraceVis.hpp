#ifndef _FTRACE_VIS_HPP
#define _FTRACE_VIS_HPP

#include "ofMain.h"
#include "ofxLaserManager.h"

class EventStats {
    public:
        int num_occurrences = 0;
        float time_since_trigger = 0.0;
        float time_since_last_event = 0.0;
        int occurrencies_to_trigger = 100;
        int triggers_without_release = 0;
        float dot_on_time = 0.05;
        int event_num = 0;

        EventStats(int event_num): event_num(event_num) {}

        void register_event() {
            num_occurrences += 1;
        }

        void update(float dt) {
            time_since_last_event += dt;
            time_since_trigger += dt;
            if(num_occurrences >= occurrencies_to_trigger) {
                num_occurrences = 0;
                if(time_since_trigger < dot_on_time) {
                    triggers_without_release += 1;
                    if(triggers_without_release > 20) {
                        occurrencies_to_trigger += 10;
                    }
                } else {
                    triggers_without_release = 0;
                }
                time_since_trigger = 0.0;
            }
        }

        void draw(ofxLaser::Manager &laser) {
            // if(time_since_last_event < 0.5) {
            if(time_since_trigger < dot_on_time) {
                float angle = -PI + event_num*0.45;
                float radius = 100.0 + (event_num % 16) * 23.5;
                float x = cos(angle) * radius;
                float y = sin(angle) * radius;
                const ofColor color = ofColor::white;
#ifdef DEBUG_MODE
                const float intensity = 1.;
#else
                const float intensity = 0.1;
#endif
                laser.drawDot(x, y, color, intensity, OFXLASER_PROFILE_DEFAULT);
            }
        }
};

class FtraceVis {
    public:

        ofColor color = ofColor::blue;
        map<string, EventStats> event_stats;

        void register_event(string event) {
            // process;timestamp;event;pid?;cpu?
            // event type until ' ' or '()'
            vector<string> tokens;
            string delimiter = ";";
            auto i = event.find(delimiter);
            while(i != string::npos) {
                tokens.push_back(event.substr(0, i));
                event.erase(0, i+delimiter.size());
                i = event.find(delimiter);
            }
            tokens.push_back(event); // add the last token

            string event_type;
            i = tokens[2].find("(");
            if(i != string::npos) {
                event_type = tokens[2].substr(0, i);
            } else {
                i = tokens[2].find(" ");
                event_type = tokens[2].substr(0, i);
            }

            auto es = event_stats.find(event_type);
            if(es != event_stats.end()) {
                es->second.register_event();
            } else {
                event_stats.insert({event_type, EventStats(event_stats.size())});
                cout << "event type: " << event_type << endl;
            }
        }

        void update(float dt) {
            for(auto& es: event_stats) {
                es.second.update(dt);
            }
        }

        void draw(ofxLaser::Manager &laser) {
                // laser.drawDot(0, 0 , color, 0.5, OFXLASER_PROFILE_FAST);
                for(auto& es: event_stats) {
                    es.second.draw(laser);
                }
        }
};

#endif
