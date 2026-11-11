#include "driver/gpio.h"
#include <esp_log.h>

#include "pet_dog.h"
#include "application.h"
#include "board.h"
#include "iot/thing.h"

#define TAG "Action"


namespace iot {

// 这里仅定义 Speaker 的属性和方法，不包含具体的实现
class Action : public Thing {
private:
    bool CanChangeState(ActionState state) {
        return state != kActionStateWalk && state != kActionStateWalkBack &&
               state != kActionStateTurnLeft && state != kActionStateTurnRight;
    }

public:
    Action() : Thing("Action", "当前 AI 机器人的行为（站立，坐下，睡觉,左转，右转，前进，后退)") {
        properties_.AddNumberProperty("front_step", "前进步数", [this]() -> int {
            auto& app = Application::GetInstance();
            return app.dog.getFrontStep();
        });
        properties_.AddNumberProperty("back_step", "后退步数", [this]() -> int {
            auto& app = Application::GetInstance();
            return app.dog.getBackStep();
        });
        properties_.AddNumberProperty("left_angle", "左转角度", [this]() -> int {
            auto& app = Application::GetInstance();
            return app.dog.getLeftAngle();
        });
        properties_.AddNumberProperty("right_angle", "右转角度", [this]() -> int {
            auto& app = Application::GetInstance();
            return app.dog.getRightAngle();
        });
        methods_.AddMethod("SetFrontStep", "前进", ParameterList({
            Parameter("front_step", "前进的步数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            app.dog.setFrontStep(static_cast<int>(parameters["front_step"].number()));
            ActionState state = app.GetActionState();
            if (CanChangeState(state)) {
                app.SetActionState(kActionStateWalk);
            } else {
                printf("no exe kActionStateWalk\r\n");
            }
        });

        methods_.AddMethod("SetBackStep", "后退", ParameterList({
            Parameter("back_step", "后退的步数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            app.dog.setBackStep(static_cast<int>(parameters["back_step"].number()));
            ActionState state = app.GetActionState();
            if (CanChangeState(state)) {
                app.SetActionState(kActionStateWalkBack);
            } else {
                printf("no exe kActionStateWalkBack\r\n");
            }
        });

        methods_.AddMethod("Swing", "摇摆/跳舞", ParameterList({
            Parameter("swing_num", "左转步数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            app.dog.setFrontStep(static_cast<int>(parameters["swing_num"].number()));
            ActionState state = app.GetActionState();
            if (CanChangeState(state)) {
                app.SetActionState(kActionStateSwing);
            } else {
                printf("no exe kActionStateTurnLeft\r\n");
            }
        });

        methods_.AddMethod("SetLeftAngle", "左转", ParameterList({
            Parameter("left_angle", "左转角度", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            app.dog.setLeftAngle(static_cast<int>(parameters["left_angle"].number()));
            ActionState state = app.GetActionState();
            if (CanChangeState(state)) {
                app.SetActionState(kActionStateTurnLeft);
            } else {
                printf("no exe kActionStateTurnLeft\r\n");
            }
        });

        methods_.AddMethod("SetRightAngle", "右转", ParameterList({
            Parameter("right_angle", "右转角度", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            app.dog.setRightAngle(static_cast<int>(parameters["right_angle"].number()));
            ActionState state = app.GetActionState();
            if (CanChangeState(state)) {
                app.SetActionState(kActionStateTurnRight);
            } else {
                printf("no exe kActionStateTurnLeft\r\n");
            }
        });

        methods_.AddMethod("stand", "站立", ParameterList(), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            ActionState state = app.GetActionState();
            printf("state=%d\r\n",state);
            if( state != kActionStateTurnRight &&  state != kActionStateTurnLeft &&  state != kActionStateWalk &&  state != kActionStateWalkBack)
            {
                app.SetActionState(kActionStateStand);
            }else{
                printf("no exe kActionStateStand\r\n");
            }
        });
        
        methods_.AddMethod("sitdown", "坐下", ParameterList(), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            ActionState state = app.GetActionState();
            printf("state=%d\r\n",state);
            if( state != kActionStateTurnRight &&  state != kActionStateTurnLeft &&  state != kActionStateWalk &&  state != kActionStateWalkBack)
            {
                app.SetActionState(kActionStateSitdown);
            }else{
                printf("no exe kActionStateSitdown\r\n");
            }
        });
        methods_.AddMethod("sleep", "睡觉", ParameterList(), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            ActionState state = app.GetActionState();
            printf("state=%d\r\n",state);
            if( state != kActionStateTurnRight &&  state != kActionStateTurnLeft &&  state != kActionStateWalk &&  state != kActionStateWalkBack)
            {
                app.SetActionState(kActionStateSleep);
            }else{
                printf("no exe kActionStateSleep\r\n");
            }
        });

        methods_.AddMethod("wave", "挥挥手", ParameterList(), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            ActionState state = app.GetActionState();
            printf("state=%d\r\n",state);
            if( state != kActionStateTurnRight &&  state != kActionStateTurnLeft &&  state != kActionStateWalk &&  state != kActionStateWalkBack)
            {
                app.SetActionState(kActionStateWave);
            }else{
                printf("no exe kActionStateWave\r\n");
            }
        });

        methods_.AddMethod("stretch", "伸懒腰", ParameterList(), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            ActionState state = app.GetActionState();
            printf("state=%d\r\n",state);
            if( state != kActionStateTurnRight &&  state != kActionStateTurnLeft &&  state != kActionStateWalk &&  state != kActionStateWalkBack)
            {
                app.SetActionState(kActionStateStretch);
            }else{
                printf("no exe kActionStateStretch\r\n");
            }
        });

        methods_.AddMethod("scratching", "挠痒痒", ParameterList(), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            ActionState state = app.GetActionState();
            printf("state=%d\r\n",state);
            if( state != kActionStateTurnRight &&  state != kActionStateTurnLeft &&  state != kActionStateWalk &&  state != kActionStateWalkBack)
            {
                app.SetActionState(kActionStateScratching);
            }else{
                printf("no exe kActionStateScratching\r\n");
            }
        });

        methods_.AddMethod("stop", "停下来", ParameterList(), [this](const ParameterList& parameters) {
            auto& app = Application::GetInstance();
            app.SetActionState(kActionStateStop);
        });
    }
};

} // namespace iot

DECLARE_THING(Action);