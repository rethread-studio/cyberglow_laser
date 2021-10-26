#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    // OSC setup
    receiver.setup(PORT);

    // No need to set anything up with the laser manager, it's all done automatically
    // Change canvas size
    laser.setCanvasSize(width, height);

    // Set up triangle positions
    triangle_positions[0] = glm::vec2(width * 0.2 - halfw, height * 0.85 - halfh);
    triangle_positions[1] = glm::vec2(width * 0.5 - halfw, height * 0.15 - halfh);
    triangle_positions[2] = glm::vec2(width * 0.8 - halfw, height * 0.85 - halfh);

    for(int i = 0; i < 3; i++) {
        event_line_columns.push_back(EventLineColumn(glm::vec2(i*(width/3) - halfw, -halfh), width/3, height));
    }
}

//--------------------------------------------------------------
void ofApp::update(){

   checkOscMessages();

   scan_x += 10;
   if(scan_x > halfw) {
       scan_x = -halfw;
   }

   noise_counter += 0.001;
   rot_x = ofNoise(noise_counter, ofGetElapsedTimef() * 0.01) * 2.0 - 1.0;
   rot_y = ofNoise(noise_counter, ofGetElapsedTimef() * 0.01 + 2534.0) * 2.0 - 1.0;


    for(auto& ap : activity_points) {
        ap.update();
    }
    // remove expired activity points
    for(int i = activity_points.size()-1; i >= 0; i--) {
        if(activity_points[i].frames_to_live <= 0) {
            activity_points.erase(activity_points.begin() + i);
        }
    }

    auto pt = player_trails.find(current_player_trail_id);
    if(pt != player_trails.end()) {
        pt->second.draw(laser);
        if(pt->second.finished_cycle) {
            pickRandomPlayerTrail();
        }
    } else {
        pickRandomPlayerTrail();
    }
    //Update event line columns
    for(auto& elc : event_line_columns) {
        elc.update();
    }
    web_server_vis.update();
    // prepares laser manager to receive new graphics
    laser.update();
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofPushMatrix();
    ofTranslate(halfw, halfh, 0.0);
    // ofRotateRad(rot_x * 0.5, 0.0, 1.0, 0.0);
    // ofRotateRad(rot_y * 0.5, 1.0, 0.0, 0.0);
    // Draw using
    // laser.drawLine()
    // laser.drawDot()
    // laser.drawCircle()
    // laser.drawPoly()
    //
    // Translating the coordinate system also works


    switch(vis_mode) {
        case VisMode::WEBSERVER:
            {
                web_server_vis.draw(laser);
            }
            break;
        case VisMode::TEXT_DEMO:
        {
            auto text_options = LaserTextOptions();
            text_options.size = 80.0;
            text_options.color = ofColor::red;
            draw_laser_text(laser, "AXY0123456789", text_options, glm::vec2(width * 0.4 - halfw, height * 0.5 - halfh));
            draw_laser_text(laser, "57131", text_options, glm::vec2(width * 0.2 - halfw, height * 0.2 - halfh));
            draw_laser_text(laser, "MOVE OPEN FILE", text_options, glm::vec2(width * 0.2 - halfw, height * 0.2 - halfh + (text_options.size * 2)));


            text_options.size = 20.0;
            text_options.color = ofColor::red;
            // draw_laser_text(laser, to_string(scan_x), text_options, glm::vec2(scan_x, height * 0.5 - halfh));
        }
            break;
        case VisMode::USER:
        {
            auto pt = player_trails.find(current_player_trail_id);
            if(pt != player_trails.end()) {
                pt->second.draw(laser);
            }
        }
            break;
        case VisMode::ZOOMED_OUT:
        {
            // draw triangle positions
            // float intensity = ofNoise(ofGetElapsedTimef() * 0.05) * 0.7 + 0.3;
            // laser.drawDot(triangle_positions[0].x, triangle_positions[0].y, ofColor::green, intensity, OFXLASER_PROFILE_FAST);
            // laser.drawDot(triangle_positions[1].x, triangle_positions[1].y, ofColor::green, intensity, OFXLASER_PROFILE_FAST);
            // laser.drawDot(triangle_positions[2].x, triangle_positions[2].y, ofColor::green, intensity, OFXLASER_PROFILE_FAST);
        }
            break;
    }

    // draw point activity
    // for(auto& ap : activity_points) {
    //     ap.draw(laser);
    // }

    // for(auto& elc : event_line_columns) {
    //     elc.draw(laser, scan_x, scan_width);
    // }

    // Test drawing text





    ofPopMatrix();
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

void ofApp::addActivityPoint(int source) {
    if(source >= 3) {
        return;
    }
    float offset_angle = ofRandom(0, TWO_PI);
    float offset = ofRandom(0, width*0.07);
    glm::vec2 position = triangle_positions[source] - glm::vec2(cos(offset_angle) * offset, sin(offset_angle) * offset);
    auto ap = ActivityPoint(position, glm::vec2(0, 0), ofColor::green);
    glm::vec2 destination = glm::vec2(0, 0);
    switch(source) {
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
    float vel_amp = ofRandom(0.5, 4.0);
    ap.launch_towards(destination, vel_amp);
    ap.grow(ofRandom(0.0, 0.5));
    activity_points.push_back(ap);
}

void ofApp::pickRandomPlayerTrail() {
    vector<string> keys;
    for(auto it = player_trails.begin(); it != player_trails.end(); ++it) {
        keys.push_back(it->first);
    }
    if(keys.size() > 0) {
        current_player_trail_id = keys[rand() % keys.size()];
        auto pt = player_trails.find(current_player_trail_id);
        if(pt != player_trails.end()) {
            pt->second.reset_cycle();
        }

    }
}


void ofApp::checkOscMessages() {
  // check for waiting messages
	while( receiver.hasWaitingMessages() )
	{
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage( &m );

    //cout << "message: " << m << endl;

		// check for mouse moved message
		if (m.getAddress() == "/cyberglow"  )
		{
            string origin = m.getArgAsString(0);
            string action = m.getArgAsString(1);
            string arguments = m.getArgAsString(2);
            // cout << "OSC mess: " << origin << ", " << action << ", " << arguments << endl;
            parseOscMessage(origin, action, arguments);
		}
		else
		{
            // cout << "Received unknown message to " << m.getAddress() << endl;
			// unrecognized message

		}

  }
}

void ofApp::parseOscMessage(string origin, string action, string arguments) {
    std::string delimiter = ";";
    if(origin == "node") {
        web_server_vis.register_node(action);
        if(action == "async after Timeout") {
            addActivityPoint(2);

        } else if(action == "async after FSREQCALLBACK") {
            addActivityPoint(0);

        } else if(action == "async after TCPWRAP") {
            addActivityPoint(0);

        }
    } else if(origin == "gameEngine") {
        if(action == "stateChanged") {
            addActivityPoint(1);

        } else if(action == "newQuestion") {
            string question = arguments;
        }

    } else if(origin == "user") {

        if(action == "move") {
            arguments += ';';
            string user_id = "";
            int x = 0, y = 0;

            size_t pos = 0;
            std::string token;
            int token_num = 0;
            while ((pos = arguments.find(delimiter)) != std::string::npos) {
                token = arguments.substr(0, pos);
                switch(token_num) {
                    case 0:
                        user_id = token;
                        break;
                    case 1:
                        x = stoi(token);
                        break;
                    case 2:
                        y = stoi(token);
                        break;
                }
                token_num++;
                arguments.erase(0, pos + delimiter.length());
            }
            if(token_num == 3) { // correct number of arguments found
                auto it = player_trails.find(user_id);
                if(it == player_trails.end()) {
                    auto pt = PlayerTrail();
                    float grid_x = width/45;
                    float grid_y = height/25;
                    pt.move_to_point((x * grid_x) - halfw, (y * grid_y) - halfh);
                    player_trails.insert(make_pair<string, PlayerTrail>(move(user_id), move(pt)));
                } else {
                    it->second.move_to_point((x * 100) - halfw, (y * 100) - halfh);;
                }
            }

        } else if(action == "enterAnswer") {

        } else if(action == "userAnswer") {

        } else if(action == "new") {
            string user_id = arguments;
            player_trails.insert(make_pair<string, PlayerTrail>(move(user_id), PlayerTrail()));
        }
    } else if(origin == "server") {
        if(action == "file") {

        }
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case OF_KEY_RIGHT:
            vis_mode = static_cast<VisMode>((static_cast<int>(vis_mode)+1)%static_cast<int>(VisMode::LAST));
            break;
        case OF_KEY_LEFT:
            if(static_cast<int>(vis_mode) != 0) {
                vis_mode = static_cast<VisMode>(static_cast<int>(vis_mode)-1);
            }
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

    // scan_x = (float(x)/float(ofGetWidth())) * float(width) - halfw;
    mouse_rel_x = (float(x)/float(ofGetWidth())) * 2.0 - 1.0;
    mouse_rel_y = (float(y)/float(ofGetHeight())) * 2.0 - 1.0;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

    // addRandomActivityPoint();
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
