#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    // No need to set anything up with the laser manager, it's all done automatically
    // Change canvas size
    laser.setCanvasSize(width, height);

    // Set up triangle positions
    triangle_positions[0] = glm::vec2(width * 0.2, height * 0.85);
    triangle_positions[1] = glm::vec2(width * 0.5, height * 0.15);
    triangle_positions[2] = glm::vec2(width * 0.8, height * 0.85);

    for(int i = 0; i < 3; i++) {
        event_line_columns.push_back(EventLineColumn(glm::vec2(i*(width/3), 0), width/3, height));
    }
}

//--------------------------------------------------------------
void ofApp::update(){

    for(auto& ap : activity_points) {
        ap.update();
    }
    // remove expired activity points
    for(int i = activity_points.size()-1; i >= 0; i--) {
        if(activity_points[i].frames_to_live <= 0) {
            activity_points.erase(activity_points.begin() + i);
        }
    }
    // Update event line columns
    for(auto& elc : event_line_columns) {
        elc.update();
    }
    // prepares laser manager to receive new graphics
    laser.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

    // Draw using
    // laser.drawLine()
    // laser.drawDot()
    // laser.drawCircle()
    // laser.drawPoly()
    //
    // Translating the coordinate system also works

    // int numDots = 6;
    // for(int x = 0; x<numDots; x++) {
    //     // intensity is a unit value (0 - 1) that determines how long the laser lingers
    //     // to make the dot. It defaults to 1.
    //     float intensity = (float)(x+1)/numDots;
    //     float xposition = ofMap(x, 0,numDots-1,200,600);
    //     laser.drawDot(xposition, 600, ofColor::red, intensity, OFXLASER_PROFILE_FAST);
    // }

    // draw triangle positions
    float intensity = 0.5;
    laser.drawDot(triangle_positions[0].x, triangle_positions[0].y, ofColor::green, intensity, OFXLASER_PROFILE_FAST);
    laser.drawDot(triangle_positions[1].x, triangle_positions[1].y, ofColor::green, intensity, OFXLASER_PROFILE_FAST);
    laser.drawDot(triangle_positions[2].x, triangle_positions[2].y, ofColor::green, intensity, OFXLASER_PROFILE_FAST);

    // draw point activity
    for(auto& ap : activity_points) {
        ap.draw(laser);
    }

    for(auto& elc : event_line_columns) {
        elc.draw(laser, scan_x, scan_width);
    }
    // sends points to the DAC
    laser.send();
    // draw the laser UI elements
    laser.drawUI();
}

void ofApp::addRandomActivityPoint() {

    size_t tri = ofRandom(0, 2.99);
    float offset_angle = ofRandom(0, TWO_PI);
    float offset = ofRandom(0, width*0.07);
    glm::vec2 position = triangle_positions[tri] - glm::vec2(cos(offset_angle) * offset, sin(offset_angle) * offset);
    // Velocity out from center point
    float vel_angle = offset_angle + PI;
    float vel_amp = ofRandom(0.5, 4.0);
    glm::vec2 vel = glm::vec2(cos(vel_angle) * vel_amp, sin(vel_angle) * vel_amp);
    auto ap = ActivityPoint(position, vel, ofColor::green);
    // Velocity towards neighbouring point
    glm::vec2 destination = glm::vec2(0, 0);
    switch(tri) {
        case 0:
            destination = triangle_positions[1];
            break;
        case 1:
            if(ofRandom(0, 1) > 0.5) {
                destination = triangle_positions[0];
            } else {
                destination = triangle_positions[2];
            }
            break;
        case 2:
            destination = triangle_positions[1];
            break;
    }
    ap.launch_towards(destination, vel_amp);
    ap.grow(ofRandom(0.0, 0.5));
    activity_points.push_back(ap);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

    scan_x = (float(x)/float(ofGetWidth())) * float(width);
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

    addRandomActivityPoint();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
