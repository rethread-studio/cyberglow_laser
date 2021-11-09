#ifndef _USER_GRID_HPP__
#define _USER_GRID_HPP__

#include "ofMain.h"

enum class UserEvent {
    ANSWER,
        USER_ANSWER,
        START,
        EMOTE,
        NEW,
        LEAVE,
        CLICK,
        MOVE,
        PLAY,
        EXIT_ANSWER,
        ENTER_ANSWER,
};

class UserData {
    public:
        float event_lifetime = 1.0; // in seconds
        vector<pair<UserEvent, float>> events;
        float time_since_last_event = 0.0;
        glm::vec2 position;

        UserData(glm::vec2 position): position(position) {}

        void register_event(string action, string argument) {
            UserEvent event;
            if(action == "answer") {
                event = UserEvent::ANSWER;
            }
            else if(action == "userAnswer") {
                event = UserEvent::USER_ANSWER;
            }
            else if(action == "start") {
                event = UserEvent::START;
            }
            else if(action == "emote") {
                event = UserEvent::EMOTE;
            }
            else if(action == "new") {
                event = UserEvent::NEW;
            }
            else if(action == "leave") {
                event = UserEvent::LEAVE;
            }
            else if(action == "click") {
                event = UserEvent::CLICK;
            }
            else if(action == "move") {
                event = UserEvent::MOVE;
            }
            else if(action == "play") {
                event = UserEvent::PLAY;
            }
            else if(action == "exitAnswer") {
                event = UserEvent::EXIT_ANSWER;
            }
            else if(action == "enterAnswer") {
                event = UserEvent::ENTER_ANSWER;
            }
        events.push_back(make_pair(event, event_lifetime));
            time_since_last_event = 0;
        }

        void update(float dt) {
            time_since_last_event += dt;
            for(size_t i = events.size()-1; i >= 0; i--) {
                auto& ep = events[i];
                ep.second -= dt;
                if(ep.second <= 0.0) {
                    // delete the event if it has expired
                    events.erase(events.begin() + i);
                }
            }
        }
};

class UserGrid {
    public:
        int cells_x = 5;
        int cells_y = 10;
        float cell_w;
        float cell_h;
        int cell_jump = 7;
        int current_cell = 0;
        unordered_map<string, UserData> user_datas;

        UserGrid() {}
        UserGrid(int width, int height) {
            cell_w = float(width)/float(cells_x);
            cell_h = float(height)/float(cells_y);
        }
        void update(float dt) {
            for(auto& ud: user_datas) {
                ud.second.update(dt);
            }
        }
        void register_event(string action, string arguments) {
            // first string of arguments, before ;, is the id
            auto delimiter = arguments.find(";");
            std::string id = arguments.substr(0, delimiter);
            arguments.erase(0, delimiter + 1);
            auto ud = user_datas.find(id);
            if(ud != user_datas.end()) {
                ud->second.register_event(action, arguments);
            } else {
                float x = (current_cell % cells_x) * cell_w;
                float y = floor(float(current_cell)/float(cells_x)) * cell_h;
                current_cell = (current_cell + cell_jump) % (cells_x * cells_y);
                auto ud = UserData(glm::vec2(x, y));
                ud.register_event(action, arguments);
                user_datas.insert({id, ud});
            }
        }
};
#endif
