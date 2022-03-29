#ifndef _OVERVIEW_HPP_
#define _OVERVIEW_HPP_

#include "LaserText.hpp"
#include "constants.h"
#include "ofMain.h"
#include "ofxFastParticleSystem.h"

class OverviewParticleController {
  int num_triggered = 0;
  int trigger_start_id = 0;
  int total_num_particles = 0;
  FastParticleSystem particles;
  glm::vec2 origin_pos;
  glm::vec2 target_pos;

public:
  void init(string suffix, glm::vec2 origin_pos_, glm::vec2 target_pos_) {

    origin_pos = origin_pos_;
    target_pos = target_pos_;

    // init particle system
    unsigned w = 1920;
    unsigned h = 1080;
    unsigned d = 5;

    total_num_particles = w * h;

    float *particlesPosns = new float[w * h * 4];
    particles.init(w, h, ofPrimitiveMode::OF_PRIMITIVE_POINTS, 2);

    // random offset for particle's initial position
    float startOffset = 20.0; // 1.5;

    for (unsigned y = 0; y < h; y++) {
      for (unsigned x = 0; x < w; x++) {
        unsigned idx = y * w + x;

        particlesPosns[idx * 4] =
            origin_pos.x + ofRandom(-startOffset, startOffset);
        particlesPosns[idx * 4 + 1] =
            origin_pos.y + ofRandom(-startOffset, startOffset);
        particlesPosns[idx * 4 + 2] = 0;
        particlesPosns[idx * 4 + 3] = 0;
      }
    }

    particles.loadDataTexture(0, particlesPosns);
    delete[] particlesPosns;

    particles.zeroDataTexture(1);

    particles.addUpdateShader("shaders/" + suffix + "/updateParticle");
    particles.addDrawShader("shaders/" + suffix + "/drawParticle");
  }

  void reset() {
    num_triggered = 0;
    trigger_start_id = 0;
    particles.zeroDataTexture(0);
    particles.zeroDataTexture(1);
  }

  void trigger_particle() {
    num_triggered += 1;
    if (trigger_start_id + num_triggered > total_num_particles) {
      trigger_start_id = 0;
      cout << "Overshot total number of particles" << endl;
    }
  }

  void update(float dt) {

    ofShader &shader = particles.getUpdateShader();
    shader.begin();

    shader.setUniform1f("total_time", ofGetElapsedTimef());
    shader.setUniform1f("timestep", dt);
    shader.setUniform1i("num_triggered", num_triggered);
    shader.setUniform1i("trigger_start_id", trigger_start_id);
    shader.setUniform2f("origin_pos", origin_pos);
    shader.setUniform2f("target_pos", target_pos);

    trigger_start_id += num_triggered;
    num_triggered = 0;

    shader.end();

    particles.update();
  }
  void draw(glm::mat4 modelViewProjectionMatrix) {
    ofShader &shader = particles.getDrawShader();
    shader.begin();
    shader.setUniformMatrix4f("modelViewProjectionMatrix",
                              modelViewProjectionMatrix);
    shader.end();
    particles.draw();
  }
};

class FlickerText {
public:
  string text;
  glm::vec2 pos;
  bool on = false;
  float offMin = 0.1;
  float offMax = 2.0;
  float onMin = 0.05;
  float onMax = 0.5;
  float timeCounter = 0.0;

  FlickerText() {}

  FlickerText(string text_, glm::vec2 pos_) : text(text_), pos(pos_) {}

  void update(float dt) {
    timeCounter -= dt;
    if (timeCounter <= 0.0) {
      on = !on;
      if (on) {
        timeCounter = pow(ofRandom(0, 1.0), 5.0) * (onMax - onMin) + onMin;
      } else {
        timeCounter = ofRandom(offMin, offMax);
      }
    }
  }
  void draw(ofTrueTypeFont &font) {
    if (on) {
      ofSetColor(255);
      font.drawString(text, pos.x, pos.y);
    }
  }
};

class Overview {
public:
  glm::vec2 triangle_positions[3];
  vector<LaserText> location_texts;
  vector<FlickerText> flicker_texts;
  bool enabled = false;
  float time_since_enabled = 0.0;

  bool enable_flicker_labels = true;
  bool enable_labels_trigger = true;
  bool enable_geography_graphics = false;

  bool finished_init = false;
  int text_index = 0;

  OverviewParticleController opc_user;
  OverviewParticleController opc_server_to_user;
  OverviewParticleController opc_server_to_vis;
  OverviewParticleController opc_ftrace;

  ofFbo fboScreen;
  ofFbo fboFade;
  ofFbo fboText;
  ofFbo fboTextFade;
  ofShader trailShader;
  ofShader trailOverlayShader; // makes dark things transparent
  ofEasyCam *cam;
  float cameraDist = 1000.0;
  float cameraRotation = 0.0;
  float cameraRotationY = 0.0;
  float rotAmount = 0.005;

  float fade_alpha = 1.0;
  float fade_alpha_change_per_sec = 0.0;

  Overview() {}

  void init(glm::vec2 triangle_positions_[3], int width, int height,
            ofTrueTypeFont &font) {

    LaserTextOptions options;
    options.size = 40.0;
    options.color = ofColor(255, 0, 255);
    glm::vec2 o = glm::vec2(0, 0);
    location_texts.push_back(LaserText("VISUALISATION", options, 4,
                                       triangle_positions[TriangleVIS] + o));
    location_texts.push_back(LaserText("SERVER", options, 3,
                                       triangle_positions[TriangleSERVER] + o));
    location_texts.push_back(
        LaserText("USER", options, 3, triangle_positions[TriangleUSER] + o));

    flicker_texts.push_back(
        FlickerText("CORE", triangle_positions_[TriangleVIS] + o));
    flicker_texts.push_back(
        FlickerText("SERVER", triangle_positions_[TriangleSERVER] + o));
    flicker_texts.push_back(
        FlickerText("USER", triangle_positions_[TriangleUSER] + o));

    flicker_texts[TriangleVIS].pos.x -= font.stringWidth("CORE") * 0.5;
    flicker_texts[TriangleSERVER].pos.x -= font.stringWidth("CLOUD") * 0.5;
    flicker_texts[TriangleUSER].pos.x -= font.stringWidth("PLAYER") * 0.5;
    // location_texts[TriangleUSER].pos.x += 90;
    // location_texts[TriangleVIS].pos.x -=
    //     location_texts[TriangleVIS].get_width() + 20;
    // location_texts[TriangleSERVER].pos.x -=
    //     location_texts[TriangleSERVER].get_width() * 0.5;
    // location_texts[TriangleSERVER].pos.y -= 120;

    for (auto &text : location_texts) {
      text.offFrames = 12;
    }

    for (int i = 0; i < 3; i++) {
      triangle_positions[i] = triangle_positions_[i];
      // flip the y axis because it needs to be flipped in the shader
      triangle_positions[i].y = -triangle_positions[i].y;
    }
    // init fbo
    fboScreen.allocate(width, height, GL_RGBA);
    fboScreen.begin();
    ofClear(0.0);
    fboScreen.end();
    fboFade.allocate(width, height, GL_RGBA32F);
    fboFade.begin();
    ofClear(0.0);
    fboFade.end();
    fboText.allocate(width, height, GL_RGBA);
    fboText.begin();
    ofClear(0.0);
    fboText.end();
    fboTextFade.allocate(width, height, GL_RGBA32F);
    fboTextFade.begin();
    ofClear(0.0);
    fboTextFade.end();

    trailShader.load("shaders/trail/shader");
    trailOverlayShader.load("shaders/trail_overlay/shader");

    // init camera for particle system

    cam = new ofEasyCam();
    // cam->rotateDeg(-90, ofVec3f(0.0,0.0, 1.0));
    cam->setDistance(cameraDist);

    cam->setNearClip(0.1);
    cam->setFarClip(200000);

    // init OverviewParticleControllers
    opc_user.init("user", triangle_positions[TriangleUSER],
                  triangle_positions[TriangleSERVER]);
    opc_server_to_user.init("server_to_user",
                            triangle_positions[TriangleSERVER],
                            triangle_positions[TriangleUSER]);
    opc_server_to_vis.init("server_to_vis", triangle_positions[TriangleSERVER],
                           triangle_positions[TriangleVIS]);
    opc_ftrace.init("ftrace_overview", triangle_positions[TriangleVIS],
                    triangle_positions[TriangleVIS]);
  }

  void enable() {
    cout << "enable overview" << endl;

    fade_alpha = 1.0;
    fade_alpha_change_per_sec = 0.0;

    enable_flicker_labels = true;
    enabled = true;
    // remove trails
    fboScreen.begin();
    ofClear(0.0);
    fboScreen.end();
    fboFade.begin();
    ofClear(0.0);
    fboFade.end();
    fboText.begin();
    ofClear(0.0);
    fboText.end();
    opc_ftrace.reset();
    opc_server_to_user.reset();
    opc_server_to_vis.reset();
    opc_user.reset();
    time_since_enabled = 0.0;
  }
  void disable(float time_to_fade_out) {
    cout << "disable overview" << endl;
    fade_alpha = 1.0;
    fade_alpha_change_per_sec = 1.0 / time_to_fade_out;
    enabled = false;
  }
  void activate_between_transition() {
    enable_geography_graphics = !enable_geography_graphics;
  }

  void draw_symbols() {
    ofColor color = ofColor(128, 0, 255);
    // Draw cloud at server position
    auto p = triangle_positions[TriangleSERVER];
    float w = 300;
    float h = 150;
    ofSetColor(color);
    ofPolyline poly;
    poly.addVertex(p.x - w * 0.5, p.y, 0);
    poly.addVertex(p.x, p.y - h * 0.5, 0);
    poly.addVertex(p.x + w * 0.5, p.y, 0);
    poly.addVertex(p.x, p.y + h * 0.5, 0);
    poly.addVertex(p.x - w * 0.5, p.y, 0);
    poly.draw();

    // Draw a smartphone
    p = triangle_positions[TriangleUSER];
    w = 300;
    h = 150;
    poly.addVertex(p.x - w * 0.5, p.y, 0);
    poly.addVertex(p.x, p.y - h * 0.5, 0);
    poly.addVertex(p.x + w * 0.5, p.y, 0);
    poly.addVertex(p.x, p.y + h * 0.5, 0);
    poly.addVertex(p.x - w * 0.5, p.y, 0);
    poly.draw();
  }

  void update(float dt) {
    time_since_enabled += dt;
    fade_alpha -= fade_alpha_change_per_sec;
    // for(auto& text : location_texts) {
    //     text.update();
    // }
    location_texts[text_index].update();
    if (location_texts[text_index].resetFrame) {
      text_index = (text_index + 1) % location_texts.size();
    }

    for (auto &ft : flicker_texts) {
      ft.update(dt);
    }

    // opc_user.trigger_particle();
    // opc_server_to_user.trigger_particle();
    // opc_server_to_vis.trigger_particle();
    // opc_ftrace.trigger_particle();
    opc_user.update(dt);
    opc_ftrace.update(dt);
    opc_server_to_user.update(dt);
    opc_server_to_vis.update(dt);
  }

  void register_ftrace_trigger(string type) { enable_labels_trigger = true; }

  void trigger_activity(int source) {
    if (enabled) {
      switch (source) {
      case TriangleUSER:
        opc_user.trigger_particle();
        break;
      case TriangleSERVER:
        opc_server_to_user.trigger_particle();
        opc_server_to_vis.trigger_particle();
        // opc_user.trigger_particle();
        break;
      case TriangleVIS:
        opc_ftrace.trigger_particle();
        break;
      }
    }
  }

  void draw(int width, int height, ofTrueTypeFont &font,
            ofTrueTypeFont &large_font) {
    ofPushMatrix();
    fboScreen.begin();
    ofClear(0, 0, 0);
    // ofBackground(0, 0, 0);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    glEnable(GL_PROGRAM_POINT_SIZE);

    // if (enabled) {
    cam->begin();

    glm::mat4 modelViewProjectionMatrix = cam->getModelViewProjectionMatrix();
    opc_user.draw(modelViewProjectionMatrix);
    opc_server_to_user.draw(modelViewProjectionMatrix);
    opc_server_to_vis.draw(modelViewProjectionMatrix);
    opc_ftrace.draw(modelViewProjectionMatrix);

    // debug box drawing
    //  ofSetColor(0, 255, 0);
    //  ofFill();
    //  ofDrawBox(ofPoint(0.0, 0, 0), 10);

    // ofDisableBlendMode();

    cam->end();
    // }

    // font.drawString("TEST test Server", 0, 0);
    fboScreen.end();
    ofPopMatrix();

    fboFade.begin();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    // ofSetColor(ofFloatColor(0.0, 0.0, 0.0, 1.1));
    // ofRect(0, 0, 1920, 1080);
    ofSetColor(255, 30);
    // draw the diff image to the mask with extreme contrast and in greyscale
    // using the maskFilterShader
    trailShader.begin();

    trailShader.setUniform1f("fadeCoeff", 1.0);
    if (enabled) {
      // trailShader.setUniform1f("brightnessFade", 0.9995);
      trailShader.setUniform1f("brightnessFade",
                               0.9995 + sin(ofGetElapsedTimef() * 1.2) * 0.002 +
                                   0.002);
      trailShader.setUniform1f("brightnessFadeLow", 2.4);
    } else {
      trailShader.setUniform1f("brightnessFade", 0.998);
      trailShader.setUniform1f("brightnessFadeLow", 0);
    }
    trailShader.setUniform2f("resolution", glm::vec2(width, height));
    trailShader.setUniformTexture("tex0", fboScreen.getTextureReference(), 1);
    trailShader.setUniformTexture("tex1", fboFade.getTextureReference(), 2);
    // DEBUG:
    // maskFilterShader.setUniform2f("mouse",
    // float(ofGetMouseX())/float(ofGetWidth()),
    // float(ofGetMouseY())/float(ofGetHeight()));
    ofSetColor(255, 255);
    fboScreen.draw(0, 0);
    // ofRect(0, 0, 1920, 1080);
    trailShader.end();
    fboFade.end();

    ofSetColor(255, fade_alpha * 255);
    // ofDisableBlendMode();
    fboFade.draw(width * -0.5, height * -0.5, width, height);
    // fboScreen.draw(width*-0.5, height*-0.5, width, height);
    // for(auto& ft : flicker_texts) {
    //   ft.draw(font);
    // }

    // if (enable_flicker_labels && ofRandom(0.0, 1.0) > 0.97) {
    //   cout << "disable flicker labels" << endl;
    //   enable_flicker_labels = false;
    // } else if (!enable_flicker_labels && ofRandom(0.0, 1.0) > 0.995) {
    //   cout << "enable flicker labels" << endl;
    //   enable_flicker_labels = true;
    // }

    fboText.begin();
    ofClear(0.0);
    ofTranslate(width * 0.5, height * 0.5, 0);
    // if (enable_flicker_labels && ofRandom(0.0, 1.0) > 0.8) {
    //   draw_text(font);
    // }
    if (enable_labels_trigger && time_since_enabled > 3.0 && enabled) {
      draw_text(font);
      enable_labels_trigger = false;
    }
    fboText.end();
    fboTextFade.begin();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    // ofSetColor(ofFloatColor(0.0, 0.0, 0.0, 1.1));
    // ofRect(0, 0, 1920, 1080);
    ofSetColor(255, 30);
    // draw the diff image to the mask with extreme contrast and in greyscale
    // using the maskFilterShader
    trailOverlayShader.begin();

    trailOverlayShader.setUniform1f("fadeCoeff", 1.04);
    if (enabled) {
      trailOverlayShader.setUniform1f("brightnessFade", 0.83);
      trailOverlayShader.setUniform1f("brightnessFadeLow", 2.6);
    } else {
      trailOverlayShader.setUniform1f("brightnessFade", 0.8);
      trailOverlayShader.setUniform1f("brightnessFadeLow", 0);
    }
    trailOverlayShader.setUniform2f("resolution", glm::vec2(width, height));
    trailOverlayShader.setUniformTexture("tex0", fboText.getTextureReference(),
                                         1);
    trailOverlayShader.setUniformTexture("tex1",
                                         fboTextFade.getTextureReference(), 2);
    // DEBUG:
    // maskFilterShader.setUniform2f("mouse",
    // float(ofGetMouseX())/float(ofGetWidth()),
    // float(ofGetMouseY())/float(ofGetHeight()));
    ofSetColor(255, 255);
    fboText.draw(0, 0);
    // ofRect(0, 0, 1920, 1080);
    trailOverlayShader.end();
    fboTextFade.end();

    ofSetColor(255, 150);
    fboTextFade.draw(width * -0.5, height * -0.5, width, height);

    if (time_since_enabled < 15.0) {
      draw_title(width, height, large_font);
    } else if (time_since_enabled > 20.0) {
      if (enable_geography_graphics) {
        draw_geography(large_font);
      }
    }
  }

  void draw_text(ofTrueTypeFont &font) {
    ofSetColor(200);
    for (auto &lt : flicker_texts) {
      font.drawString(lt.text, lt.pos.x, lt.pos.y);
    }
  }

  void draw_geography(ofTrueTypeFont &font) {
    static float line_rotation = 0.0;
    static float line_y = 0.0;
    static float line_y_vel = ofRandom(-1.0, 1.0);
    static float line_rotation_vel = ofRandom(-0.001, 0.001);
    if (ofRandom(0.0, 1) > 0.9) {
      line_rotation_vel = ofRandom(-0.001, 0.001);
    }
    if (ofRandom(0.0, 1) > 0.9) {
      line_y_vel = ofRandom(-1.0, 1.0);
    }
    line_rotation += line_rotation_vel;
    line_y += line_y_vel;
    ofSetColor(255);
    ofPushMatrix();
    ofRotateRad(0.15);
    ofTranslate(0, -50, 0);
    font.drawString("Amsterdam", -700, -150);
    font.drawString("Stockholm", -370, 250);
    // ofRotateRad(line_rotation);
    // ofDrawRectangle(glm::vec2(-1200, line_y), 2400, 4);

    ofPopMatrix();
  }
  void draw_title(int width, int height, ofTrueTypeFont &font) {
    int margin = width * 0.05;
    ofSetColor(255);
    ofNoFill();
    ofSetLineWidth(5);
    ofDrawRectangle(glm::vec2(width * -0.5 + margin, height * -0.5 + margin),
                    width - (margin * 2), height - (margin * 2));
    font.drawString("NETWORK", -300, height * 0.5 - margin - 30);
    ofPushMatrix();
    ofRotateRad(PI);
    font.drawString("NETWORK", -300, height * 0.5 - margin - 30);

    ofPopMatrix();
  }
};

#endif
