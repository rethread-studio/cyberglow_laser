#ifndef LASERTEXT_H_
#define LASERTEXT_H_

// Draw text as vector graphics

#include "ofMain.h"
#include "ofxLaserManager.h"


class LaserTextOptions {
    public:
        ofColor color = ofColor::white;
        float size = 40.0;
        float horizontal_character_margin = 0.6; // in fractions of the size

        LaserTextOptions() {}
        LaserTextOptions(float size): size(size) {}
        LaserTextOptions(float size, ofColor color): size(size), color(color) {}

        float get_horizontal_margin() {
            return horizontal_character_margin * size;
        }
};

// Returns the width of the character
//
// Draw using lines or from SVGs?
inline float draw_laser_character(ofxLaser::Manager &laser, char c, LaserTextOptions &options, glm::vec2 position) {
    ofPolyline line;
    float width = 0.0;
    float height = options.size;
    glm::vec2 p = position;
    ofColor col = options.color;
            ofPolyline poly;
    auto profile = OFXLASER_PROFILE_FAST;
    switch(c) {
        case 'A':
            width = options.size * 0.5;
            laser.drawLine(glm::vec2(p.x, p.y+height), glm::vec2(p.x+width*0.5, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y), glm::vec2(p.x+width, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.3, p.y+height*0.6), glm::vec2(p.x+width*0.7, p.y+height*0.6), col, profile);
            break;
        case 'E':
            width = options.size * 0.5;
            poly.addVertex(p.x+width, p.y+height, 0);
            poly.addVertex(p.x, p.y+height, 0);
            poly.addVertex(p.x, p.y+height*0.5, 0);
            poly.addVertex(p.x+width, p.y+height*0.5, 0);
            poly.addVertex(p.x, p.y+height*0.5, 0);
            poly.addVertex(p.x, p.y, 0);
            poly.addVertex(p.x+width, p.y, 0);
            laser.drawPoly(poly, col, profile);
            break;
        case 'F':
            width = options.size * 0.5;
            poly.addVertex(p.x, p.y+height, 0);
            poly.addVertex(p.x, p.y+height*0.5, 0);
            poly.addVertex(p.x+width, p.y+height*0.5, 0);
            poly.addVertex(p.x, p.y+height*0.5, 0);
            poly.addVertex(p.x, p.y, 0);
            poly.addVertex(p.x+width, p.y, 0);
            laser.drawPoly(poly, col, profile);
            break;
        case 'I':
            width = options.size * 0.1;
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y+height), glm::vec2(p.x+width*0.5, p.y), col, profile);
            break;
        case 'L':
            width = options.size * 0.6;
            poly.addVertex(p.x+width, p.y+height, 0);
            poly.addVertex(p.x, p.y+height, 0);
            poly.addVertex(p.x, p.y, 0);
            laser.drawPoly(poly, col, profile);
            break;
        case 'M':
            width = options.size * 1.0;
            poly.addVertex(p.x, p.y+height, 0);
            poly.addVertex(p.x, p.y, 0);
            poly.addVertex(p.x+width*0.5, p.y+height*0.5, 0);
            poly.addVertex(p.x+width, p.y, 0);
            poly.addVertex(p.x+width, p.y+height, 0);
            laser.drawPoly(poly, col, profile);
            break;

        case 'N':
            width = options.size * 0.6;
            poly.addVertex(p.x, p.y+height, 0);
            poly.addVertex(p.x, p.y, 0);
            poly.addVertex(p.x+width, p.y+height, 0);
            poly.addVertex(p.x+width, p.y, 0);
            laser.drawPoly(poly, col, profile);
            break;
        case 'O':
            width = options.size * 0.6;
            poly.addVertex(p.x, p.y+height, 0);
            poly.addVertex(p.x, p.y, 0);
            poly.addVertex(p.x+width, p.y, 0);
            poly.addVertex(p.x+width, p.y+height, 0);
            poly.addVertex(p.x, p.y+height, 0);
            laser.drawPoly(poly, col, profile);
            break;
        case 'P':
            width = options.size * 0.6;
            poly.addVertex(p.x, p.y+height, 0);
            poly.addVertex(p.x, p.y, 0);
            poly.addVertex(p.x+width, p.y, 0);
            poly.addVertex(p.x+width, p.y+height*0.4, 0);
            poly.addVertex(p.x, p.y+height*0.4, 0);
            laser.drawPoly(poly, col, profile);
            break;
        case 'V':
            width = options.size * 0.5;
            poly.addVertex(p.x, p.y, 0);
            poly.addVertex(p.x + width*0.5, p.y+height, 0);
            poly.addVertex(p.x+width, p.y, 0);
            laser.drawPoly(poly, col, profile);
            break;
        case 'X':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x+width, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height), glm::vec2(p.x+width, p.y), col, profile);
            break;
        case 'Y':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x+width*0.5, p.y+height*0.5), col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y+height*0.5), glm::vec2(p.x+width*0.5, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y+height*0.5), glm::vec2(p.x+width, p.y), col, profile);
            break;
        case '.':
        {
            width = options.size * 0.2;
            float radius = width * 0.5;
            laser.drawCircle(p+glm::vec2(width*0.5, height-radius), radius, col, profile);
            break;
        }
        case ':':
        {
            width = options.size * 0.2;
            float radius = width * 0.5;
            laser.drawCircle(p+glm::vec2(width*0.5, height-radius), radius, col, profile);
            laser.drawCircle(p+glm::vec2(width*0.5, radius), radius, col, profile);
            break;
        }
        case '0':
            width = options.size*0.6;
            // laser.drawCircle(p+glm::vec2(width*0.5, height*0.5), width*0.1, col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y), glm::vec2(p.x + width, p.y+height*0.5), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y+height*0.5), glm::vec2(p.x + width*0.5, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y+height), glm::vec2(p.x, p.y+height*0.5), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.5), glm::vec2(p.x + width*0.5, p.y), col, profile);
            break;
        case '1':
            width = options.size * 0.1;
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y+height), glm::vec2(p.x+width*0.5, p.y), col, profile);
            break;
        case '2':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x + width, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y), glm::vec2(p.x + width, p.y + height*0.2), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y+height*0.2), glm::vec2(p.x, p.y+height*0.8), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.8), glm::vec2(p.x, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height), glm::vec2(p.x + width, p.y+height), col, profile);
            break;
        case '3':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x + width, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y), glm::vec2(p.x+width, p.y+height*0.5), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.5), glm::vec2(p.x+width, p.y+height*0.5), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y+height*0.5), glm::vec2(p.x+width, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height), glm::vec2(p.x + width, p.y+height), col, profile);
            break;
        case '4':
            width = options.size * 0.7;
            laser.drawLine(glm::vec2(p.x+width*0.7, p.y+height), glm::vec2(p.x+width*0.7, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.7, p.y), glm::vec2(p.x, p.y+height*0.7), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.7), glm::vec2(p.x+width, p.y+height*0.7), col, profile);
            break;
        case '5':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x+width, p.y), glm::vec2(p.x, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x, p.y + height*0.4), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.4), glm::vec2(p.x+width, p.y+height*0.6), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y+height*0.6), glm::vec2(p.x+width, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y+height), glm::vec2(p.x, p.y+height), col, profile);
            break;
        case '6':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x+width, p.y), glm::vec2(p.x, p.y+height*0.6), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.6), glm::vec2(p.x+width*0.5, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x+width*0.5, p.y+height), glm::vec2(p.x+width, p.y+height*0.7), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y+height*0.7), glm::vec2(p.x+width*0.5, p.y+height*0.3), col, profile);
            break;
        case '7':
            width = options.size * 0.7;
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x + width, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y), glm::vec2(p.x+width*0.2, p.y+height), col, profile);
            break;
        case '8':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x + width, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y), glm::vec2(p.x+width, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y+height), glm::vec2(p.x, p.y+height), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height), glm::vec2(p.x, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.5), glm::vec2(p.x+width, p.y+height*0.5), col, profile);
            break;
        case '9':
            width = options.size * 0.6;
            laser.drawLine(glm::vec2(p.x+width, p.y+height*0.5), glm::vec2(p.x, p.y+height*0.5), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y+height*0.5), glm::vec2(p.x, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x, p.y), glm::vec2(p.x+width, p.y), col, profile);
            laser.drawLine(glm::vec2(p.x+width, p.y), glm::vec2(p.x+width, p.y+height), col, profile);
            break;
    }
    return width + options.get_horizontal_margin();
}

inline void draw_laser_text(ofxLaser::Manager &laser, string text, LaserTextOptions &options, glm::vec2 position) {

    for(char c : text) {
        float width = draw_laser_character(laser, c, options, position);
        position.x += width;
    }
}


#endif // LASERTEXT_H_
