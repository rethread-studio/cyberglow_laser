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

      total_num_particles = w*h;

      float* particlesPosns = new float [w * h  * 4];
      particles.init(w, h, ofPrimitiveMode::OF_PRIMITIVE_POINTS, 2);

      // random offset for particle's initial position
      float startOffset = 20.0;//1.5;

      for (unsigned y = 0; y < h; y++){
        for(unsigned x = 0; x < w; x++){
          unsigned idx = y * w + x;

          particlesPosns[idx * 4] =    origin_pos.x + ofRandom(-startOffset, startOffset);
          particlesPosns[idx * 4 +1] = origin_pos.y + ofRandom(-startOffset, startOffset);
          particlesPosns[idx * 4 +2] = 0;
          particlesPosns[idx * 4 +3] = 0;
        }
      }

      particles.loadDataTexture(0, particlesPosns);
      delete[] particlesPosns;

      particles.zeroDataTexture(1);

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
      shader.setUniformMatrix4f("modelViewProjectionMatrix", modelViewProjectionMatrix);
      shader.end();
      particles.draw();
    }
};

class Overview {
  public:
    glm::vec2 triangle_positions[3];
    vector<LaserText> location_texts;

    bool finished_init = false;
    int text_index = 0;

    OverviewParticleController opc_user;
    OverviewParticleController opc_server_to_user;
    OverviewParticleController opc_server_to_vis;
    OverviewParticleController opc_ftrace;


    ofFbo fboScreen;
    ofFbo fboFade;
    ofShader trailShader;
    ofEasyCam* cam;
    float cameraDist        = 1000.0;
    float cameraRotation    = 0.0;
    float cameraRotationY    = 0.0;
    float rotAmount         = 0.005;


  Overview() {

  }

    void init(glm::vec2 triangle_positions_[3], int width, int height) {


      for (int i = 0; i < 3; i++) {
        triangle_positions[i] = triangle_positions_[i];
        // flip the y axis because it needs to be flipped in the shader
        triangle_positions[i].y = -triangle_positions[i].y;
      }
      LaserTextOptions options;
      options.size = 40.0;
      options.color = ofColor(255, 0, 255);
      glm::vec2 o = glm::vec2(-10, 10);
      location_texts.push_back(LaserText("VISUALISATION", options, 4,
                                         triangle_positions[TriangleVIS] + o));
      location_texts.push_back(LaserText("SERVER", options, 3,
                                         triangle_positions[TriangleSERVER] + o));
      location_texts.push_back(
        LaserText("USER", options, 3, triangle_positions[TriangleUSER] + o));

      for (auto &text : location_texts) {
        text.offFrames = 12;
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

      trailShader.load("shaders/trail/shader");

      // init camera for particle system

      cam = new ofEasyCam();
      // cam->rotateDeg(-90, ofVec3f(0.0,0.0, 1.0));
      cam->setDistance(cameraDist);

      cam->setNearClip(0.1);
      cam->setFarClip(200000);

      // init OverviewParticleControllers
      opc_user.init("user", triangle_positions[TriangleUSER], triangle_positions[TriangleSERVER]);
      opc_server_to_user.init("server_to_user", triangle_positions[TriangleSERVER], triangle_positions[TriangleUSER]);
      opc_server_to_vis.init("server_to_vis", triangle_positions[TriangleSERVER], triangle_positions[TriangleVIS]);
      opc_ftrace.init("ftrace_overview", triangle_positions[TriangleVIS], triangle_positions[TriangleVIS]);
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
    if (finished_init) {
      // for(auto& text : location_texts) {
      //     text.update();
      // }
      location_texts[text_index].update();
      if (location_texts[text_index].resetFrame) {
        text_index = (text_index + 1) % location_texts.size();
      }
    }

          opc_user.trigger_particle();
          opc_server_to_user.trigger_particle();
          opc_server_to_vis.trigger_particle();
          opc_ftrace.trigger_particle();
      opc_user.update(dt);
      opc_ftrace.update(dt);
      opc_server_to_user.update(dt);
      opc_server_to_vis.update(dt);
  }

    void trigger_activity(int source) {
      switch(source) {
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

  void draw(int width, int height) {
    ofPushMatrix();
    fboScreen.begin();
    ofClear(0, 0, 0);
    // ofBackground(0, 0, 0);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);


    glEnable(GL_PROGRAM_POINT_SIZE);

    cam->begin();

    glm::mat4 modelViewProjectionMatrix = cam->getModelViewProjectionMatrix();
    opc_user.draw(modelViewProjectionMatrix);
    opc_server_to_user.draw(modelViewProjectionMatrix);
    opc_server_to_vis.draw(modelViewProjectionMatrix);
    opc_ftrace.draw(modelViewProjectionMatrix);

    //debug box drawing
    // ofSetColor(0, 255, 0);
    // ofFill();
    // ofDrawBox(ofPoint(0.0, 0, 0), 10);

    // ofDisableBlendMode();

    cam->end();
    fboScreen.end();
    ofPopMatrix();

    fboFade.begin();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    // ofSetColor(ofFloatColor(0.0, 0.0, 0.0, 1.1));
    // ofRect(0, 0, 1920, 1080);
      ofSetColor(255, 30);
      // draw the diff image to the mask with extreme contrast and in greyscale using the maskFilterShader
      trailShader.begin();
        trailShader.setUniform2f("resolution", glm::vec2(1920.0, 1080.0));
        trailShader.setUniformTexture("tex0", fboScreen.getTextureReference(), 1);
        trailShader.setUniformTexture("tex1", fboFade.getTextureReference(), 2);
        // DEBUG:
        //maskFilterShader.setUniform2f("mouse", float(ofGetMouseX())/float(ofGetWidth()), float(ofGetMouseY())/float(ofGetHeight()));
        ofSetColor(255, 255);
        fboScreen.draw(0, 0);
        // ofRect(0, 0, 1920, 1080);
      trailShader.end();
    fboFade.end();

    ofSetColor(255);
    // ofDisableBlendMode();
    fboFade.draw(width*-0.5, height*-0.5, width, height);
    // fboScreen.draw(width*-0.5, height*-0.5, width, height);

  }

  void draw_text() {
    if (!finished_init) {
      // location_texts[TriangleUSER].pos.x -=
      // location_texts[TriangleUSER].get_width(laser) + 20;
      location_texts[TriangleUSER].pos.x += 90;
      location_texts[TriangleVIS].pos.x -=
          location_texts[TriangleVIS].get_width() + 20;
      location_texts[TriangleSERVER].pos.x -=
          location_texts[TriangleSERVER].get_width() * 0.5;
      location_texts[TriangleSERVER].pos.y -= 120;
      finished_init = true;
    }

    // for(auto& text : location_texts) {
    //     text.draw(laser);
    // }
    location_texts[text_index].draw();
  }
};





#endif
