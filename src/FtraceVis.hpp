#ifndef _FTRACE_VIS_HPP
#define _FTRACE_VIS_HPP

#include "constants.h"
#include "ofMain.h"
#include "ofxFastParticleSystem.h"

#define POSITION_TEXTURE 0
#define COLOR_TEXTURE 1

class EventStats {
public:
  int num_occurrences = 0;
  float time_since_trigger = 0.0;
  float time_since_last_event = 0.0;
  int occurrencies_to_trigger = 100;
  int triggers_without_release = 0;
  int max_triggers_without_release = 20;
  float ratio_to_trigger_without_release = 1.0;
  float dot_on_time = 0.05;
  int event_num = 0;
  ofColor color = ofColor::white;

  EventStats(int event_num) : event_num(event_num) {}

  void register_event() { num_occurrences += 1; }

  void update(float dt) {
    time_since_last_event += dt;
    time_since_trigger += dt;
    if (num_occurrences >= occurrencies_to_trigger) {
      num_occurrences = 0;
      if (time_since_trigger <
          (dot_on_time * ratio_to_trigger_without_release)) {
        triggers_without_release += 1;
        if (triggers_without_release > max_triggers_without_release) {
          occurrencies_to_trigger += 10;
        }
      } else {
        triggers_without_release = 0;
      }
      time_since_trigger = 0.0;
    }
  }

  void draw_dot() {
    // if(time_since_last_event < 0.5) {
    if (time_since_trigger < dot_on_time) {
      float angle = -PI + event_num * 0.45;
      float radius = 100.0 + (event_num % 16) * 23.5;
      float x = cos(angle) * radius;
      float y = sin(angle) * radius;
#ifdef DEBUG_MODE
      const float intensity = 1.;
#else
      const float intensity = 0.1;
#endif
      ofFill();
      ofSetColor(color);
      ofDrawCircle(x, y, dotSize);
    }
  }
  void draw_rising(int width, int height) {
    // if(time_since_last_event < 0.5) {
    if (time_since_trigger < dot_on_time) {
      float x = (event_num * 173) % width;
      float y = height - (time_since_trigger / dot_on_time) * height;
#ifdef DEBUG_MODE
      const float intensity = 1.;
#else
      const float intensity = 0.01;
#endif
      ofFill();
      ofSetColor(color);
      ofDrawCircle(x - width / 2, y - height / 2, dotSize);
    }
  }
};

class FtraceVis {
  public:

    ofColor color = ofColor::blue;
    map<string, EventStats> event_stats;
    bool rising = false;
    bool category_colors = false; // TODO
    float dot_on_time = 0.05;

    ofFbo fboScreen;
    FastParticleSystem particles;
    ofEasyCam* cam;

    float timestep = 0.005;

    float cameraDist        = 20.0;
    float cameraRotation    = 0.0;
    float rotAmount         = 0.005;

    int num_triggered = 0;
    int trigger_start_id = 0;
    int total_num_particles = 0;

  FtraceVis(bool rising, int width, int height) : rising(rising) {
    if (rising) {
      dot_on_time = 1.0;
    } else {
      dot_on_time = 0.05;
    }
    // init fbo
    fboScreen.allocate(width, height, GL_RGB);
    fboScreen.begin();
    ofClear(0.0);
    fboScreen.end();

    // init camera for particle system

    cam = new ofEasyCam();
    cam->rotateDeg(-90, ofVec3f(1.0,0.0, 0.0));
    cam->setDistance(cameraDist);

    cam->setNearClip(0.1);
    cam->setFarClip(200000);

    // init particle system
    unsigned w = 1920;
    unsigned h = 1080;
    unsigned d = 5;

    total_num_particles = w*h;

    float* particlesPosns = new float [w * h  * 4];
    particles.init(w, h, ofPrimitiveMode::OF_PRIMITIVE_POINTS, 2);

    // random offset for particle's initial position
    // different attractors works better with different starting offset positions
    float startOffset = 10.0;//1.5;

    for (unsigned y = 0; y < h; y++){
        for(unsigned x = 0; x < w; x++){
            unsigned idx = y * w + x;

            particlesPosns[idx * 4] =    ofRandom(-startOffset, startOffset);
            particlesPosns[idx * 4 +1] = ofRandom(-startOffset, startOffset);
            particlesPosns[idx * 4 +2] = ofRandom(-startOffset, startOffset);
            particlesPosns[idx * 4 +3] = 0;
        }
    }

    particles.loadDataTexture(POSITION_TEXTURE, particlesPosns);
    delete[] particlesPosns;

    particles.zeroDataTexture(COLOR_TEXTURE);

    particles.addUpdateShader("shaders/updateParticle");
    particles.addDrawShader("shaders/drawParticle");
  }
  FtraceVis() : FtraceVis(false, 1920, 1080) {}

  void register_event(string event) {
    trigger_particle();
    // process;timestamp;event;pid?;cpu?
    // event type until ' ' or '()'
    string event_copy = event;
    vector<string> tokens;
    string delimiter = ";";
    auto i = event.find(delimiter);
    while (i != string::npos) {
      tokens.push_back(event.substr(0, i));
      event.erase(0, i + delimiter.size());
      i = event.find(delimiter);
    }
    tokens.push_back(event); // add the last token

    string event_type;
    i = tokens[2].find("(");
    if (i != string::npos) {
      event_type = tokens[2].substr(0, i);
    } else {
      i = tokens[2].find(" ");
      event_type = tokens[2].substr(0, i);
    }

    auto es = event_stats.find(event_type);
    if (es != event_stats.end()) {
      es->second.register_event();
    } else {
      // cout << "ftrace type: \"" << event_type << "\" event: " << event_copy
      // << endl;
      EventStats e = EventStats(event_stats.size());
      string event_prefix;
      i = event_type.find("_");
      if (i != string::npos) {
        event_prefix = event_type.substr(0, i);
      }
      // Set color of event based on its type
      if (event_prefix == "random" || event_prefix == "dd" ||
          event_prefix == "redit") {
        // random type
        e.color = ofColor::red;
      } else if (event_prefix == "ys") {
        // syscall type
        e.color = ofColor::white;
      } else if (event_prefix == "cp") {
        // tcp type
        e.color = ofColor::green;
      } else if (event_prefix == "ix") {
        // irq_matrix type
        e.color = ofColor::blue;
      } else {
        e.color = ofColor(255, 0, 255);
      }
      if (rising) {
        e.occurrencies_to_trigger = 20;
        e.max_triggers_without_release = 2;
        e.ratio_to_trigger_without_release = 3.0;
      }
      e.dot_on_time = dot_on_time;
      event_stats.insert({event_type, e});
    }
  }

    void trigger_particle() {
        num_triggered += 1;
        if(trigger_start_id + num_triggered > total_num_particles) {
          trigger_start_id = 0;
          cout << "Overshot total number of particles" << endl;
        }
    }

    void update(float dt) {
      for (auto &es : event_stats) {
        es.second.update(dt);
      }

    cout << "num triggered: " << num_triggered << endl;
    cam->lookAt(ofVec3f(0.0, 0.0, 0.0));
    cam->setPosition(cameraDist*sin(cameraRotation),
                    0.0,
                    cameraDist*cos(cameraRotation));

    cameraRotation += rotAmount;

    ofShader &shader = particles.getUpdateShader();
    shader.begin();


    shader.setUniform1f("timestep", timestep);
    shader.setUniform1i("num_triggered", num_triggered);
    shader.setUniform1i("trigger_start_id", trigger_start_id);

    trigger_start_id += num_triggered;
    num_triggered = 0;

    shader.end();

    particles.update();
  }

  void draw(int width, int height) {
    if (rising) {

      for (auto &es : event_stats) {
        es.second.draw_rising(width, height);
      }
    } else {

      for (auto &es : event_stats) {
        es.second.draw_dot();
      }
    }


    ofPushMatrix();
    fboScreen.begin();
    ofClear(0, 0, 0);
    ofBackground(0, 0, 0);

    ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    glEnable(GL_PROGRAM_POINT_SIZE);

    cam->begin();


    //debug box drawing
    // ofSetColor(0, 255, 0);
    // ofFill();
    // ofDrawBox(ofPoint(0.0, 0, 0), 10);

    ofShader &shader = particles.getDrawShader();
    shader.begin();
    shader.setUniformMatrix4f("modelViewProjectionMatrix", cam->getModelViewProjectionMatrix());
    shader.end();
    particles.draw();
    ofDisableBlendMode();

    cam->end();
    fboScreen.end();
    ofPopMatrix();

    ofSetColor(255);
    fboScreen.draw(width*-0.5, height*-0.5, width, height);
  }
};

#endif
