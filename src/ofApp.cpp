#include "ofApp.h"
#include "constants.h"

//--------------------------------------------------------------
void ofApp::setup() {

  // OSC setup
  receiver.setup(PORT);

  // Set up triangle positions
  triangle_positions[0] =
      glm::vec2(width * 0.2 - halfw, height * 0.65 - halfh); // visualisation
  triangle_positions[1] =
      glm::vec2(width * 0.5 - halfw, height * 0.15 - halfh); // server
  triangle_positions[2] =
      glm::vec2(width * 0.8 - halfw, height * 0.85 - halfh); // user

  for (int i = 0; i < 3; i++) {
    event_line_columns.push_back(EventLineColumn(
        glm::vec2(i * (width / 3) - halfw, -halfh), width / 3, height));
  }



  font.load("FT88-Expanded.otf", 20);
  large_font.load("FT88-Expanded.otf", 60);

  user_grid = UserGrid(width, height);
  ftrace_vis.init(width, height);
  overview.init(triangle_positions, width, height, font);
  text_flow = TextFlow(width, height);
  transition.type = TransitionType::NONE; // disable the transition at startup

  auto text_options = LaserTextOptions();
  text_options.size = 80.0;
  text_options.color = ofColor::red;

  laser_texts.push_back(
      LaserText("ABCDEFGHIJKLMNOPQ", text_options, 5,
                glm::vec2(width * 0.05 - halfw, height * 0.5 - halfh)));
  laser_texts.push_back(
      LaserText("RSTUVXY0123456789", text_options, 5,
                glm::vec2(width * 0.05 - halfw, height * 0.2 - halfh)));
  // laser_texts.push_back(LaserText("MOVE OPEN FILE", text_options, 4,
  // glm::vec2(width * 0.2 - halfw, height * 0.2 - halfh + (text_options.size *
  // 2))));
  //

  switch(vis_mode) {
    case VisMode::FTRACE:
      ftrace_vis.enable();
      break;
    case VisMode::ZOOMED_OUT:
      overview.enable();
      break;
  }
  ofBackground(0);
}

//--------------------------------------------------------------
void ofApp::update() {
  static float last_time = 0;
  if (last_time == 0) {
    last_time = ofGetElapsedTimef();
  }
  float now = ofGetElapsedTimef();
  dt = now - last_time;
  last_time = now;

  // Checking for OSC messages should be done even if paused to discard messages
  // and avoid a large peak
  checkOscMessages();

  if (!is_paused) {
    if (automatic_transitions &&
        !(transition_at_new_question || transition_at_answer) &&
        !idle_mode_on) {
      next_transition_countdown -= dt;
      if (next_transition_countdown <= 0.0) {
        cout << "transition timing met, going to the next vis mode" << endl;
        activateTransitionToNext();
        next_transition_countdown = time_per_vis;
      }
    }

    // For some visualisations, transition to self automatically
    // if (!transition.active() && transition_chain.size() == 0 &&
    //     (vis_mode == VisMode::FTRACE)) {
    //   transitionToFrom(vis_mode, vis_mode);
    // }

  switch (vis_mode) {
    case VisMode::FTRACE: {
      ftrace_vis.update(dt);
      break;
    }
    case VisMode::ZOOMED_OUT: {
      overview.update(dt);
      break;
    }
  }


    noise_counter += 0.001;
    rot_x = ofNoise(noise_counter, ofGetElapsedTimef() * 0.01) * 2.0 - 1.0;
    rot_y =
        ofNoise(noise_counter, ofGetElapsedTimef() * 0.01 + 2534.0) * 2.0 - 1.0;

  }
}

//--------------------------------------------------------------
void ofApp::draw() {

  ofBackground(0);

  ofPushMatrix();
  ofTranslate(halfw, halfh, 0.0);
  // ofRotateRad(rot_x * 0.5, 0.0, 1.0, 0.0);
  // ofRotateRad(rot_y * 0.5, 1.0, 0.0, 0.0);
  //
  // Translating the coordinate system also works

  if (transition.active()) {
    transition.update(dt);
    if(!transition.active()) {
      cout << "transition was just finished, activate" << endl;
      vis_mode = transition.to_vis;
      switch(vis_mode) {
        case VisMode::FTRACE:
          ftrace_vis.enable();
          break;
        case VisMode::ZOOMED_OUT:
          overview.enable();
          break;
      }
    }
  }
      drawVisualisation(vis_mode, 1.0);

  ofPopMatrix();
  // sends points to the DAC
}

void ofApp::addActivityPoint(int source) {
  if (source >= 3) {
    return;
  }
/*if (source != TriangleVIS) {
    float offset_angle = ofRandom(0, TWO_PI);
    float offset = ofRandom(0, width * 0.07);
    glm::vec2 position =
        triangle_positions[source] -
        glm::vec2(cos(offset_angle) * offset, sin(offset_angle) * offset);
    auto ap = ActivityPoint(position, glm::vec2(0, 0), ofColor::green);
    glm::vec2 destination = glm::vec2(0, 0);
    switch (source) {
    case 0:
      destination = triangle_positions[1]; bb
      break;
    case 1:
      if (ofRandom(0, 1) > 0.5) {
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
    while (activity_points.size() > max_num_activity_points) {
      activity_points.erase(activity_points.begin());
    }
  }
  float activity_level_increase = 0.1;
  if (source == TriangleVIS) {
    activity_level_increase = 0.00006;
  }
  triangle_activity[source] += activity_level_increase;

  }*/
  overview.trigger_activity(source);
}

void ofApp::activateTransitionToNext() {

  VisMode next_vis;
  if (use_fixed_order_transitions) {
    next_vis = vis_mode_order[vis_mode_order_index];
    vis_mode_order_index = (vis_mode_order_index + 1) % vis_mode_order.size();
  } else {
    do {
      int next_vis_num = int(ofRandom(0, static_cast<int>(VisMode::LAST)));
      next_vis = static_cast<VisMode>(next_vis_num);
    } while (next_vis == vis_mode);
  }
  transitionToFrom(vis_mode, next_vis);
}

void ofApp::checkOscMessages() {
  // check for waiting messages
  while (receiver.hasWaitingMessages()) {
    // get the next message
    ofxOscMessage m;
    receiver.getNextMessage(&m);

    // cout << "message: " << m << endl;

    // Only parse the message if we are not paused
    if (!is_paused) {
      // check for mouse moved message
      if (m.getAddress() == "/cyberglow") {
        string origin = m.getArgAsString(0);
        string action = m.getArgAsString(1);
        string arguments = m.getArgAsString(2);
        // cout << "OSC mess: " << origin << ", " << action << ", " << arguments
        // << endl;
        parseOscMessage(origin, action, arguments);
      } else if (m.getAddress() == "/ftrace") {
        ftrace_vis.register_event(m.getArgAsString(0));
        // cout << "ftrace: " << m.getArgAsString(0) << endl;
        addActivityPoint(TriangleVIS);
      }

      else if (m.getAddress() == "/idle") {
        // cout << "/idle: " << m.getArgAsString(0) << endl;
        auto arg = m.getArgAsString(0);
        if (arg == "on") {
          idle_mode_on = true;
          transition_chain.clear();
          transitionToFrom(vis_mode, idle_vis_mode);
        } else {
          // off
          idle_mode_on = false;
          transitionToFrom(idle_vis_mode, VisMode::ZOOMED_OUT);
        }
      } else {
        // cout << "Received unknown message to " << m.getAddress() << endl;
        // unrecognized message
      }
    }
  }
}

void ofApp::parseOscMessage(string origin, string action, string arguments) {
  std::string delimiter = ";";
  if (origin == "node") {
    web_server_vis.register_node(action);

    if (action == "async after Timeout") {
      addActivityPoint(TriangleSERVER);

    } else if (action == "async after FSREQCALLBACK") {
      addActivityPoint(TriangleSERVER);

    } else if (action == "async after TCPWRAP") {
      addActivityPoint(TriangleSERVER);
    }
  } else if (origin == "gameEngine") {
    if (action == "stateChanged") {
      addActivityPoint(TriangleSERVER);

    } else if (action == "newQuestion") {
      string question = arguments;
      if (automatic_transitions && transition_at_new_question &&
          !idle_mode_on) {
        activateTransitionToNext();
      }
      answer_for_current_question_received = false;
    }

  } else if (origin == "user") {

    string text = action; // + " " + arguments;
    if (action != "userAnswer") {
      text_flow.add_text(text, width, height);
    }
    if (action == "userAnswer") {
      if (automatic_transitions && transition_at_answer &&
          !answer_for_current_question_received) {
        activateTransitionToNext();
      }
      answer_for_current_question_received = true;
    }
    user_grid.register_event(action, arguments);
    addActivityPoint(TriangleUSER);
    if (action == "move") {
      // id, x, y, width_cells, height_cells
      arguments += ';';
      string user_id = "";
      int x = 0, y = 0;
      int w = 1, h = 1;

      size_t pos = 0;
      std::string token;
      int token_num = 0;
      while ((pos = arguments.find(delimiter)) != std::string::npos) {
        token = arguments.substr(0, pos);
        switch (token_num) {
        case 0:
          user_id = token;
          break;
        case 1:
          x = stoi(token);
          break;
        case 2:
          y = stoi(token);
          break;
        case 3:
          w = stoi(token);
          break;
        case 4:
          h = stoi(token);
          break;
        }
        token_num++;
        arguments.erase(0, pos + delimiter.length());
      }
      if (token_num >= 3) {
        float grid_x, grid_y;
        if (token_num == 3) { // message without w/h
          grid_x = width / 45;
          grid_y = height / 25;
        } else if (token_num == 5) { // message with w/h
          grid_x = float(width) / float(w + 2);
          grid_y = float(height) / float(h + 2);
        }
        auto it = player_trails.find(user_id);
        // The + grid_x at the end is for margins
        float calc_x = (x * grid_x) - halfw + grid_x;
        float calc_y = (y * grid_y) - halfh + grid_y;
        if (it == player_trails.end()) {
          auto pt = PlayerTrail();
          pt.move_to_point(calc_x, calc_y);
          player_trails.insert(
              make_pair<string, PlayerTrail>(move(user_id), move(pt)));
        } else {
          it->second.move_to_point(calc_x, calc_y);
        }
      }

    } else if (action == "enterAnswer") {
      // user moves inside the answer zone

    } else if (action == "userAnswer") {
      // the time has ended and the user has answered

    } else if (action == "new") {
      string user_id = arguments;
      player_trails.insert(
          make_pair<string, PlayerTrail>(move(user_id), PlayerTrail()));
    }
  } else if (origin == "server") {
    web_server_vis.register_server(action);
    addActivityPoint(TriangleSERVER);
    if (action == "file") {
    }
  } else if (origin == "mongodb") {
    web_server_vis.register_mongodb(action);
  }
}

void ofApp::drawVisualisation(VisMode vis, float scale) {

  switch (vis) {
  case VisMode::FTRACE: {
    ftrace_vis.draw(width, height, large_font);
    break;
  }
  case VisMode::ZOOMED_OUT: {
    // draw triangle positions
    // float intensity = 0.2;
    // for (size_t i = 0; i < 3; i++) {
    //   ofFill();
    //   ofSetColor(ofColor::blue);
    //   ofDrawCircle(triangle_positions[i].x * scale,
    //                triangle_positions[i].y * scale, dotSize);
    //   float radius = powf(triangle_activity[i], 0.5) * height * 0.08 + 10;
    //   if (transition.active()) {
    //     radius = 15.0;
    //   }
    //   ofNoFill();
    //   ofSetColor(ofColor::blue);
    //   ofDrawCircle(triangle_positions[i].x, triangle_positions[i].y, radius);
    // }
    // for(auto& elc : event_line_columns) {
    //     elc.draw(laser, scan_x, scan_width);
    // }
    // overview.draw_symbols(laser);
    // if (!transition.active()) {
    //   overview.draw_text();
    //   // draw point activity
    //   for (auto &ap : activity_points) {
    //     ap.draw(scale);
    //   }
    // }
    overview.draw(width, height, font, large_font);
    break;
  }
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  switch (key) {
  case OF_KEY_RIGHT: {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>((static_cast<int>(vis_mode) + 1) %
                                    static_cast<int>(VisMode::LAST));
    auto to = vis_mode;
    transitionToFrom(from, to);

    break;
  }
  case OF_KEY_LEFT:
    if (static_cast<int>(vis_mode) != 0) {
      auto from = vis_mode;
      vis_mode = static_cast<VisMode>(static_cast<int>(vis_mode) - 1);
      auto to = vis_mode;
      transitionToFrom(from, to);
    }
    break;
  case ' ': {
    is_paused = !is_paused;
    break;
  }
  case '1': {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>(0);
    auto to = vis_mode;
    transitionToFrom(from, to);
    break;
  }
  case '2': {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>(1);
    auto to = vis_mode;
    transitionToFrom(from, to);
    break;
  }
  case '3': {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>(2);
    auto to = vis_mode;
    transitionToFrom(from, to);
    break;
  }
  case '4': {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>(3);
    auto to = vis_mode;
    transitionToFrom(from, to);
    break;
  }
  case '5': {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>(4);
    auto to = vis_mode;
    transitionToFrom(from, to);
    break;
  }
  case '6': {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>(5);
    auto to = vis_mode;
    transitionToFrom(from, to);
    break;
  }
  case '7': {
    auto from = vis_mode;
    vis_mode = static_cast<VisMode>(6);
    auto to = vis_mode;
    transitionToFrom(from, to);
    break;
  }
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

  // scan_x = (float(x)/float(ofGetWidth())) * float(width) - halfw;
  mouse_rel_x = (float(x) / float(ofGetWidth())) * 2.0 - 1.0;
  mouse_rel_y = (float(y) / float(ofGetHeight())) * 2.0 - 1.0;
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

  // addRandomActivityPoint();
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {}

int visModeCategory(VisMode vis) {
  if (vis == VisMode::FTRACE ) {
    return TriangleVIS;
  } else if (vis == VisMode::ZOOMED_OUT) {
    return 3;
  }
  return -1;
}

bool vismodesAreInTheSamePlace(VisMode vis1, VisMode vis2) {
  return visModeCategory(vis1) == visModeCategory(vis2);
}

void ofApp::transitionToFrom(VisMode from, VisMode to) {
  Transition t = Transition();
  t.type = TransitionType::DISABLE_AND_FADE;
  t.from_vis = from;
  t.to_vis = to;
  transition = t;
  if (from == VisMode::ZOOMED_OUT) {
    overview.disable();
  } else {
    ftrace_vis.disable();
  }
}
