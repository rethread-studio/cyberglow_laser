#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    // OSC setup
    receiver.setup(PORT);

    // No need to set anything up with the laser manager, it's all done automatically
    // Change canvas size
    laser.setCanvasSize(width, height);

    // Set up triangle positions
    triangle_positions[0] = glm::vec2(width * 0.2 - halfw, height * 0.85 - halfh); // visualisation
    triangle_positions[1] = glm::vec2(width * 0.5 - halfw, height * 0.15 - halfh); // server
    triangle_positions[2] = glm::vec2(width * 0.8 - halfw, height * 0.85 - halfh); // user

    for(int i = 0; i < 3; i++) {
        event_line_columns.push_back(EventLineColumn(glm::vec2(i*(width/3) - halfw, -halfh), width/3, height));
    }

    auto text_options = LaserTextOptions();
    text_options.size = 80.0;
    text_options.color = ofColor::red;

    laser_texts.push_back(LaserText("ABCDEFGHIJKLMNOPQ", text_options, 5, glm::vec2(width * 0.05 - halfw, height * 0.5 - halfh)));
    laser_texts.push_back(LaserText("RSTUVXY0123456789", text_options, 5, glm::vec2(width * 0.05 - halfw, height * 0.2 - halfh)));
    // laser_texts.push_back(LaserText("MOVE OPEN FILE", text_options, 4, glm::vec2(width * 0.2 - halfw, height * 0.2 - halfh + (text_options.size * 2))));
   // colors.push_back(ofColor::white);
   // colors.push_back(ofColor::red);
   colors.push_back(ofColor::white);
   // colors.push_back(ofColor::teal);
   // colors.push_back(ofColor::green);
}

//--------------------------------------------------------------
void ofApp::update(){

    static float last_time = 0;
    if(last_time == 0) {
        last_time = ofGetElapsedTimef();
    }
    float now = ofGetElapsedTimef();
    float dt = now - last_time;
    last_time = now;

    transition.update(dt);

   checkOscMessages();

   scan_x += 10;
   if(scan_x > halfw) {
       scan_x = -halfw;
   }

   noise_counter += 0.001;
   rot_x = ofNoise(noise_counter, ofGetElapsedTimef() * 0.01) * 2.0 - 1.0;
   rot_y = ofNoise(noise_counter, ofGetElapsedTimef() * 0.01 + 2534.0) * 2.0 - 1.0;

   for(int i = 0; i < ofRandom(0, 2); i++) {
       raindrops.push_back(RainDrop(width, height, colors[floor(ofRandom(0, colors.size()))]));
   }
   for(auto& d : raindrops) {
       d.update(dt);
   }
    for(int i = raindrops.size()-1; i >= 0; i--) {
        if(raindrops[i].position.y > halfh) {
            raindrops.erase(raindrops.begin() + i);
        }
    }

    for(auto& ap : activity_points) {
        ap.update();
    }
    // remove expired activity points
    for(int i = activity_points.size()-1; i >= 0; i--) {
        if(activity_points[i].frames_to_live <= 0) {
            activity_points.erase(activity_points.begin() + i);
        }
    }

    for(size_t i = 0; i < 3; i++) {
        triangle_activity[i] *= 0.95;
        if(triangle_activity[i] > 1.0) {
            triangle_activity[i] = 1.0;
        }
    }

    for(auto& lt : laser_texts) {
        lt.update();
    }
    text_flow.update(width);

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

    if(transition.active()) {
        ofPushMatrix();
        transition.applyTransitionFrom();
        drawVisualisation(transition.from_vis);
        ofPopMatrix();
        ofPushMatrix();
        transition.applyTransitionTo();
        drawVisualisation(transition.to_vis);
        ofPopMatrix();
    } else {
        drawVisualisation(vis_mode);
    }



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
    triangle_activity[source] += 0.1;
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
            addActivityPoint(TriangleSERVER);

        } else if(action == "async after FSREQCALLBACK") {
            addActivityPoint(TriangleSERVER);

        } else if(action == "async after TCPWRAP") {
            addActivityPoint(TriangleSERVER);

        }
    } else if(origin == "gameEngine") {
        if(action == "stateChanged") {
            addActivityPoint(TriangleSERVER);

        } else if(action == "newQuestion") {
            string question = arguments;
        }

    } else if(origin == "user") {

        string text = action; // + " " + arguments;
        text_flow.add_text(text, laser, width, height);
        addActivityPoint(TriangleUSER);
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
        web_server_vis.register_server(action);
        addActivityPoint(TriangleSERVER);
        if(action == "file") {

        }
    } else if(origin == "mongodb") {
        web_server_vis.register_mongodb(action);
    }
}


void ofApp::drawVisualisation(VisMode vis) {

    switch(vis) {
        case VisMode::WEBSERVER:
            {
                web_server_vis.draw(laser);
            }
            break;
        case VisMode::TEXT_DEMO:
        {
            // auto text_options = LaserTextOptions();
            // text_options.size = 80.0;
            // text_options.color = ofColor::red;
            // draw_laser_text(laser, "AXY0123456789", text_options, glm::vec2(width * 0.4 - halfw, height * 0.5 - halfh));
            // draw_laser_text(laser, "57131", text_options, glm::vec2(width * 0.2 - halfw, height * 0.2 - halfh));
            // draw_laser_text(laser, "MOVE OPEN FILE", text_options, glm::vec2(width * 0.2 - halfw, height * 0.2 - halfh + (text_options.size * 2)));

            // for(auto& lt : laser_texts) {
            //     lt.draw(laser);
            // }
            text_flow.draw(laser);

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
            float intensity = 0.2;
            for(size_t i = 0; i < 3; i++) {
                laser.drawDot(triangle_positions[i].x, triangle_positions[i].y, ofColor::white, intensity, OFXLASER_PROFILE_FAST);
                float radius = powf(triangle_activity[i], 0.5) * height * 0.08 + 10;
                laser.drawCircle(triangle_positions[i].x, triangle_positions[i].y, radius, ofColor::white, OFXLASER_PROFILE_FAST);
            }
    // for(auto& elc : event_line_columns) {
    //     elc.draw(laser, scan_x, scan_width);
    // }
    // draw point activity
            for(auto& ap : activity_points) {
                ap.draw(laser);
            }
            break;
        }
        case VisMode::RAIN:
        {
            for(auto& d : raindrops) {
                d.draw(laser);
            }
            break;
        }
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case OF_KEY_RIGHT:
        {
            auto from = vis_mode;
            vis_mode = static_cast<VisMode>((static_cast<int>(vis_mode)+1)%static_cast<int>(VisMode::LAST));
            auto to = vis_mode;
            transitionToFrom(from, to);

            break;
        }
        case OF_KEY_LEFT:
            if(static_cast<int>(vis_mode) != 0) {
                auto from = vis_mode;
                vis_mode = static_cast<VisMode>(static_cast<int>(vis_mode)-1);
                auto to = vis_mode;
                transitionToFrom(from, to);
            }
            break;
        case '1':
        {
                auto from = vis_mode;
                vis_mode = static_cast<VisMode>(0);
                auto to = vis_mode;
                transitionToFrom(from, to);
                break;
        }
        case '2':
        {
                auto from = vis_mode;
                vis_mode = static_cast<VisMode>(1);
                auto to = vis_mode;
                transitionToFrom(from, to);
                break;
        }
        case '3':
        {
                auto from = vis_mode;
                vis_mode = static_cast<VisMode>(2);
                auto to = vis_mode;
                transitionToFrom(from, to);
                break;
        }
        case '4':
        {
                auto from = vis_mode;
                vis_mode = static_cast<VisMode>(3);
                auto to = vis_mode;
                transitionToFrom(from, to);
                break;
        }
        case '5':
        {
                auto from = vis_mode;
                vis_mode = static_cast<VisMode>(4);
                auto to = vis_mode;
                transitionToFrom(from, to);
                break;
        }
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

void ofApp::transitionToFrom(VisMode from, VisMode to) {
    Transition t = Transition();
    if (from == VisMode::ZOOMED_OUT) {
        t.type = TransitionType::ZOOM_IN;
        switch(to) {
            case VisMode::WEBSERVER:
            {
                t.zoom_target = triangle_positions[TriangleSERVER];
                t.duration = 4.0;
                break;
            }
            case VisMode::USER:
            {
                t.zoom_target = triangle_positions[TriangleUSER];
                break;
            }
        }
    }
    if (to == VisMode::ZOOMED_OUT) {
        t.type = TransitionType::ZOOM_OUT;
        switch(from) {
            case VisMode::WEBSERVER:
            {
                t.zoom_target = triangle_positions[TriangleSERVER];
                break;
            }
            case VisMode::USER:
            {
                t.zoom_target = triangle_positions[TriangleUSER];
                break;
            }
        }
    }
    t.from_vis = from;
    t.to_vis = to;
    transition = t;
}
