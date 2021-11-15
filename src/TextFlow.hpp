#ifndef __TEXTFLOW_HPP_
#define __TEXTFLOW_HPP_

#include "ofMain.h"
#include "LaserText.hpp"

/// A TextFlow is used to display text messages flying across the screen
class TextFlow {
    public:
        vector<LaserText> texts;
        LaserTextOptions options;
        float velocity = 10.0;


        TextFlow() {
            options = LaserTextOptions();
            options.size = 40.0;
            options.color = ofColor::red;
        }

        void add_text(string text, ofxLaser::Manager &laser, int width, int height) {
            transform(text.begin(), text.end(), text.begin(), ::toupper);
            float y = floor(ofRandom(0, (height / (options.size * 2)) -1)) * options.size * 2.0;
            LaserText lt = LaserText(text, options, 4, glm::vec2(0, y - (height/2)));
            float text_width = lt.get_width(laser);
            // Start to the left of the canvas
            lt.pos.x = width/-2 - text_width;
            texts.push_back(std::move(lt));
        }

        void update(int width) {
            for(auto& t : texts) {
                t.pos.x += velocity;
                t.update();
            }
            // Remove texts that are out of bounds
            for(size_t i = 0; i < texts.size(); i++) {
                if(texts[i].pos.x > width) {
                    texts.erase(texts.begin() + i);
                    i--;
                }
            }
        }
        void draw(ofxLaser::Manager &laser) {

            for(auto& t : texts) {
                t.draw(laser);
            }
        }

};

#endif
