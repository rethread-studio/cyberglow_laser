#ifndef _FTRACE_VIS_HPP
#define _FTRACE_VIS_HPP

#include "constants.h"
#include "ofMain.h"
#include "ofxFastParticleSystem.h"

#define POSITION_TEXTURE 0
#define COLOR_TEXTURE 1

static const float TIMESTEP = 0.3;

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

class FtraceParticleController {
    int num_triggered = 0;
    int trigger_start_id = 0;
    int total_num_particles = 0;
    FastParticleSystem particles;
  public:
    void init(string suffix) {

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

          // particlesPosns[idx * 4] =    ofRandom(-startOffset, startOffset);
          // particlesPosns[idx * 4 +1] = ofRandom(-startOffset, startOffset);
          // particlesPosns[idx * 4 +2] = ofRandom(-startOffset, startOffset);
          // particlesPosns[idx * 4 +3] = 0;
          particlesPosns[idx * 4] =   float(x*20)/float(w/2);
          particlesPosns[idx * 4 +1] = float(y*20)/float(h/2);
          particlesPosns[idx * 4 +2] = 0;
          particlesPosns[idx * 4 +3] = 0;
        }
      }

      particles.loadDataTexture(POSITION_TEXTURE, particlesPosns);
      delete[] particlesPosns;

      particles.zeroDataTexture(COLOR_TEXTURE);

      particles.addUpdateShader("shaders/"+suffix+"/updateParticle");
      particles.addDrawShader("shaders/"+suffix+"/drawParticle");
    }

    void trigger_particle() {
        num_triggered += 1;
        if(trigger_start_id + num_triggered > total_num_particles) {
          trigger_start_id = 0;
          cout << "Overshot total number of particles" << endl;
        }
    }

    void update(float dt) {

      ofShader &shader = particles.getUpdateShader();
      shader.begin();

      shader.setUniform1f("timestep", TIMESTEP * dt);
      shader.setUniform1i("num_triggered", num_triggered);
      shader.setUniform1i("trigger_start_id", trigger_start_id);

      trigger_start_id += num_triggered;
      num_triggered = 0;

      shader.end();

      particles.update();
    }
    void draw(glm::mat4 modelViewProjectionMatrix) {
      ofShader &shader = particles.getDrawShader();
      shader.begin();
      shader.setUniformMatrix4f("modelViewProjectionMatrix", modelViewProjectionMatrix);
      shader.end();
      particles.draw();
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
    ofEasyCam* cam;

    FtraceParticleController fpc_random;
    FtraceParticleController fpc_syscall;
    FtraceParticleController fpc_tcp;
    FtraceParticleController fpc_irq;


    float cameraDist        = 30.0;
    float cameraRotation    = 0.0;
    float rotAmount         = 0.005;


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

    // init FtraceParticleControlllers
    fpc_syscall.init("syscall");
    fpc_random.init("random");

  }
  FtraceVis() : FtraceVis(false, 1920, 1080) {}

  void register_event(string ftrace_kind) {
    trigger_particle(ftrace_kind);
    // process;timestamp;event;pid?;cpu?
    // event type until ' ' or '()'

    auto es = event_stats.find(ftrace_kind);
    if (es != event_stats.end()) {
      es->second.register_event();
    } else {
      // cout << "ftrace type: \"" << event_type << "\" event: " << event_copy
      // << endl;
      EventStats e = EventStats(event_stats.size());
      string event_prefix;
      // Set color of event based on its type
      if (ftrace_kind == "Random") {
        // random type
        e.color = ofColor::red;
      } else if (ftrace_kind == "Syscall") {
        // syscall type
        e.color = ofColor::white;
      } else if (ftrace_kind == "Tcp") {
        // tcp type
        e.color = ofColor::green;
      } else if (ftrace_kind == "IrqMatrix") {
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
      event_stats.insert({ftrace_kind, e});
    }
  }

    void trigger_particle(string ftrace_kind) {
      if (ftrace_kind == "Random") {
        fpc_random.trigger_particle();
      } else if (ftrace_kind == "Syscall") {
        fpc_syscall.trigger_particle();
      } else if (ftrace_kind == "Tcp") {
      } else if (ftrace_kind == "IrqMatrix") {
      }
    }

    void update(float dt) {
      for (auto &es : event_stats) {
        es.second.update(dt);
      }

      cam->lookAt(ofVec3f(0.0, 0.0, 0.0));
      cam->setPosition(cameraDist*sin(cameraRotation),
                       0.0,
                       cameraDist*cos(cameraRotation));

      cameraRotation += rotAmount;

      fpc_random.update(dt);
      fpc_syscall.update(dt);

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

    glm::mat4 modelViewProjectionMatrix = cam->getModelViewProjectionMatrix();
    fpc_random.draw(modelViewProjectionMatrix);
    fpc_syscall.draw(modelViewProjectionMatrix);

    //debug box drawing
    // ofSetColor(0, 255, 0);
    // ofFill();
    // ofDrawBox(ofPoint(0.0, 0, 0), 10);

    ofDisableBlendMode();

    cam->end();
    fboScreen.end();
    ofPopMatrix();

    ofSetColor(255);
    fboScreen.draw(width*-0.5, height*-0.5, width, height);
  }
};

#endif
