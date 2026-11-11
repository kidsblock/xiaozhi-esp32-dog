#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "pet_dog.h"
#include "application.h"

PetDog::PetDog()
{
    //配置定时器
    ledc_timer_ = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,     //输出频率
        .clk_cfg          = LEDC_AUTO_CLK       //时钟来源
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_));
}

PetDog::~PetDog()
{
    ledc_stop(LEDC_MODE, CHANNEL_0, 0);
    ledc_stop(LEDC_MODE, CHANNEL_1, 0);
    ledc_stop(LEDC_MODE, CHANNEL_2, 0);
    ledc_stop(LEDC_MODE, CHANNEL_3, 0);
}

void PetDog::InitializeDog(gpio_num_t LEDC_OUTPUT_IO_1, gpio_num_t LEDC_OUTPUT_IO_2, gpio_num_t LEDC_OUTPUT_IO_3, gpio_num_t LEDC_OUTPUT_IO_4)
{
    action_task_event_ = xEventGroupCreate();
    //配置通道0
    ledc_channel_config_t channel0 = create_ledc_channel(LEDC_OUTPUT_IO_1, CHANNEL_0);
    ledc_channel_config_t channel1 = create_ledc_channel(LEDC_OUTPUT_IO_2, CHANNEL_1);
    ledc_channel_config_t channel2 = create_ledc_channel(LEDC_OUTPUT_IO_3, CHANNEL_2);
    ledc_channel_config_t channel3 = create_ledc_channel(LEDC_OUTPUT_IO_4, CHANNEL_3);

    ESP_ERROR_CHECK(ledc_channel_config(&channel0));
    ESP_ERROR_CHECK(ledc_channel_config(&channel1));
    ESP_ERROR_CHECK(ledc_channel_config(&channel2));
    ESP_ERROR_CHECK(ledc_channel_config(&channel3));

    lf_.index = LEG1;
    rf_.index = LEG2;
    lb_.index = LEG3;
    rb_.index = LEG4;
    
    set_speed(10);

    xTaskCreate([](void* arg)
    {
        auto this_ = (PetDog*)arg;
        this_->ActionTask();
        vTaskDelete(NULL);
    },"actionTask",2560,this,1,nullptr);

    xTaskCreate([](void* arg)
    {
        auto this_ = (PetDog*)arg;
        this_->ActionIdleTask();
        vTaskDelete(NULL);
    },"action_idle_task",2560,this,1,NULL);
}


void PetDog::OnActionTask(std::function<void()> callback)
{
    action_task_ = callback;
}

void PetDog::ActionTask()
{
    set_right_front_angle(180);
    set_left_front_angle(180);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    set_right_back_angle(0);
    set_left_back_angle(0);
    while (1)
    {
        if(action_task_ != nullptr)
        {
            action_task_();
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }else
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
}

void PetDog::ActionIdleTask()
{
    auto& app = Application::GetInstance();
    int rand_time = 0;
    int rand_action = 0;
    
    while (1) {
        DeviceState dev_state = app.GetDeviceState();
        if (dev_state == kDeviceStateIdle) {
            rand_time = (rand() % 121) + 60;        //60-180
            // rand_time = (rand() % 31) + 2;        //test:2-32
            // printf("rand_time:%d\r\n",rand_time);
            rand_time *= 60; 
            rand_action = rand() % 3;
        }
        while(rand_time > 0)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            dev_state = app.GetDeviceState();
            if (dev_state != kDeviceStateIdle)
            {
                rand_time = 0;
                break;
            }else
            {
                rand_time --;
                if (rand_time == 0)
                {
                    idle_activate(rand_action);
                }
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void PetDog::idle_activate(int rand_action)
{
    auto display = Board::GetInstance().GetDisplay();
    display->start_emtion();
    if(rand_action == 0)
    {
        stretch();
    }else if (rand_action == 1)
    {
        stretch2();
    }else if (rand_action == 2)
    {
        scratching();
    }
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    petsleep();
    display->idle_emtion();
}

void PetDog::stand()
{
    to_any_angle_task(STAND_ANGLE_LF,STAND_ANGLE_RF,STAND_ANGLE_LB,STAND_ANGLE_RB);
    xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
}

void PetDog::sitdown()
{
    to_any_angle_task(SITDOWN_ANGLE_LF,SITDOWN_ANGLE_RF,SITDOWN_ANGLE_LB,SITDOWN_ANGLE_RB);
    xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
}

void PetDog::petsleep()
{
    to_any_angle_task(SLEEP_ANGLE_LF,SLEEP_ANGLE_RF,SLEEP_ANGLE_LB,SLEEP_ANGLE_RB);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);

    ledc_stop(LEDC_MODE, CHANNEL_0, 0);
    ledc_stop(LEDC_MODE, CHANNEL_1, 0);
    ledc_stop(LEDC_MODE, CHANNEL_2, 0);
    ledc_stop(LEDC_MODE, CHANNEL_3, 0);
}

void PetDog::stretch()
{
    set_speed(20);
    to_any_angle_task(STAND_ANGLE_LF,STAND_ANGLE_RF,STAND_ANGLE_LB,STAND_ANGLE_RB);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    to_any_angle_task(10,10,45,45);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    to_any_angle_task(135,135,170,170);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    to_any_angle_task(STAND_ANGLE_LF,STAND_ANGLE_RF,STAND_ANGLE_LB,STAND_ANGLE_RB);

    set_speed(10);

    xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
}

void PetDog::stretch2()
{
    stop();
    set_speed(20);
    to_any_angle_task(STAND_ANGLE_LF,STAND_ANGLE_RF,STAND_ANGLE_LB,STAND_ANGLE_RB);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    to_any_angle_task(0,0,180,180);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    to_any_angle_task(STAND_ANGLE_LF,STAND_ANGLE_RF,STAND_ANGLE_LB,STAND_ANGLE_RB);

    set_speed(10);

    xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
}

void PetDog::scratching()
{
    set_speed(20);

    to_any_angle_task(STAND_ANGLE_LF,STAND_ANGLE_RF,STAND_ANGLE_LB,STAND_ANGLE_RB);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    to_any_angle_task(90,180,0,0);
    vTaskDelay(1200 / portTICK_PERIOD_MS);
    int wave_num = 5;
    while(wave_num -- != 0)
    {
        if (xEventGroupGetBits(action_task_event_) & STOP_TASK_EVENT)
        {
            xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
            break;
        }
        for (size_t i = 0; i < 45; i+=4)
        {
            set_left_back_angle(i);
            vTaskDelay(15 / portTICK_PERIOD_MS);
        }
        for (size_t i = 44; i > 0; i-=4)
        {
            set_left_back_angle(i);
            vTaskDelay(25 / portTICK_PERIOD_MS);
        }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    stand();

    set_speed(10);
}

void PetDog::walkfront()
{
    auto& app = Application::GetInstance();
    int step = getFrontStep();
    while(step--)
    {
        if (xEventGroupGetBits(action_task_event_) & STOP_TASK_EVENT)
        {
            xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
            break;
        }
        set_angle(90,45,45,90);
        set_angle(135,45,45,135);
        set_angle(135,90,90,135);
        set_angle(90,90,90,90);
        set_angle(45,90,90,45);
        set_angle(45,135,135,45);
        set_angle(90,135,135,90);
        set_angle(90,90,90,90);
    }
    app.SetActionState(kActionStateStop);
    stand();
}

void PetDog::walkBack()
{
    auto& app = Application::GetInstance();
    int step = getBackStep();
    while(step--)
    {
        if (xEventGroupGetBits(action_task_event_) & STOP_TASK_EVENT)
        {
            xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
            break;
        }
        set_angle(90,90,90,90);
        set_angle(90,135,135,90);
        set_angle(45,135,135,45);
        set_angle(45,90,90,45);
        set_angle(90,90,90,90);
        set_angle(135,90,90,135);
        set_angle(135,45,45,135);
        set_angle(90,45,45,90);
    }
    app.SetActionState(kActionStateStop);
    stand();
}

void PetDog::turnLeft()
{
    auto& app = Application::GetInstance();
    int left_angle = getLeftAngle();
    int step = left_angle / 12;
    while(step--)
    {    
        if (xEventGroupGetBits(action_task_event_) & STOP_TASK_EVENT)
        {
            xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
            break;
        }
        set_right_front_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(90);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);

        set_right_front_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(130);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);

        set_right_front_angle(130);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(130);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);

        set_right_front_angle(130);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(90);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
    }
    app.SetActionState(kActionStateStop);
    stand();
}

void PetDog::turnRight()
{
    auto& app = Application::GetInstance();
    int right_angle = getRightAngle();
    int step = right_angle / 12;
    while(step--)
    {
        if (xEventGroupGetBits(action_task_event_) & STOP_TASK_EVENT)
        {
            xEventGroupClearBits(action_task_event_,STOP_TASK_EVENT);
            break;
        }
        set_right_front_angle(130);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(90);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);

        set_right_front_angle(130);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(130);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);

        set_right_front_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(50);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(130);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);

        set_right_front_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_right_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_front_angle(90);   //1
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
        set_left_back_angle(90);
        vTaskDelay(SPEED / portTICK_PERIOD_MS);
    }
    app.SetActionState(kActionStateStop);
    stand();
}

void PetDog::stop()
{
    xEventGroupSetBits(action_task_event_,STOP_TASK_EVENT);
}

void PetDog::petwave()
{
    to_any_angle_task(90,90,50,0);
    vTaskDelay(600 / portTICK_PERIOD_MS);
    int wave_num = 5;
    while(wave_num -- != 0)
    {
        for (size_t i = 0; i < 65; i+=4)
        {
            set_left_front_angle(i);
            vTaskDelay(25 / portTICK_PERIOD_MS);
        }
        for (size_t i = 64; i > 0; i-=4)
        {
            set_left_front_angle(i);
            vTaskDelay(25 / portTICK_PERIOD_MS);
        }
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    stand();
}

//跳舞指令
void PetDog::swing()
{
    to_any_angle_task(90,90,90,90);
    vTaskDelay(600 / portTICK_PERIOD_MS);
    int wave_num = 5;

    srand((unsigned)time(NULL));
    int random_number = rand() % 100; // Generates a random number between 0 and 99
    if(random_number < 50)
    {
        while(wave_num -- != 0)
        {
            for (size_t i = 0; i < 45; i+=5)
            {
                set_left_front_angle(90+i);
                set_right_front_angle(90+i);
                set_left_back_angle(90+i);
                set_right_back_angle(90+i);
                vTaskDelay(25 / portTICK_PERIOD_MS);
            }
            for (size_t i = 0; i < 90; i+=5)
            {
                set_left_front_angle(135-i);
                set_right_front_angle(135-i);
                set_left_back_angle(135-i);
                set_right_back_angle(135-i);
                vTaskDelay(25 / portTICK_PERIOD_MS);
            }
    
            for (size_t i = 0; i < 45; i+=5)
            {
                set_left_front_angle(45+i);
                set_right_front_angle(45+i);
                set_left_back_angle(45+i);
                set_right_back_angle(45+i);
                vTaskDelay(25 / portTICK_PERIOD_MS);
            }
        }
    }else
    {
        while(wave_num -- != 0)
        {
            for (size_t i = 0; i < 45; i+=5)
            {
                set_left_front_angle(90+i);
                set_right_front_angle(90+i);
                set_left_back_angle(90-i);
                set_right_back_angle(90-i);
                vTaskDelay(25 / portTICK_PERIOD_MS);
            }
            for (size_t i = 0; i < 90; i+=5)
            {
                set_left_front_angle(135-i);
                set_right_front_angle(135-i);
                set_left_back_angle(45+i);
                set_right_back_angle(45+i);
                vTaskDelay(25 / portTICK_PERIOD_MS);
            }
    
            for (size_t i = 0; i < 45; i+=5)
            {
                set_left_front_angle(45+i);
                set_right_front_angle(45+i);
                set_left_back_angle(135-i);
                set_right_back_angle(135-i);
                vTaskDelay(25 / portTICK_PERIOD_MS);
            }
        }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
    stand();
}

//状态机
void PetDog::Action(int  action)
{
    int rand_ = 0;
    switch(action)
    {
        case kActionStateTurnLeft:
            xTaskCreate([](void* arg)
            {
                auto this_ = (PetDog*)arg;
                this_->turnLeft();
                vTaskDelete(NULL);
            },"TurnLeft",2560,this,1,NULL);
            break;
        case kActionStateTurnRight:
            xTaskCreate([](void* arg)
            {
                auto this_ = (PetDog*)arg;
                this_->turnRight();
                vTaskDelete(NULL);
            },"TurnRight",2560,this,1,NULL);
            break;
        case kActionStateWalk:
            xTaskCreate([](void* arg)
            {
                auto this_ = (PetDog*)arg;
                this_->walkfront();
                vTaskDelete(NULL);
            },"walkfront",2560,this,1,NULL);
            break;
        case kActionStateWalkBack:
            xTaskCreate([](void* arg)
            {
                auto this_ = (PetDog*)arg;
                this_->walkBack();
                vTaskDelete(NULL);
            },"walkBack",2560,this,1,NULL);
            break;
        case kActionStateSleep:
            stop();
            petsleep();
            break;
        case kActionStateStand:
            stop();
            stand();
            break;
        case kActionStateSitdown:
            stop();
            sitdown();
            break;
        case kActionStateStop:
            stop();
            break;
        case kActionStateWave:
            stop();
            petwave();
            break;
        case kActionStateStretch:
            srand(time(NULL));
            rand_ = rand() % 51;
            if(rand_ > 25) stretch2();
            else stretch();
            break;
        case kActionStateScratching:
            stop();
            scratching();
            break;
        case kActionStateSwing:
            stop();
            swing();
            break;
    }
}

void PetDog::to_any_angle_task(uint8_t lf_angle,uint8_t rf_angle,uint8_t lb_angle,uint8_t rb_angle)
{
    lf_.angle = lf_angle;
    rf_.angle = rf_angle;
    lb_.angle = lb_angle;
    rb_.angle = rb_angle;

    xTaskCreate([](void* arg)
    {
        auto this_ = (PetDog*)arg;
        this_->to_tar_angle(&this_->lf_);
        vTaskDelete(NULL);
    }, "to_tar_angle", 2560, this, 1, NULL);

    xTaskCreate([](void* arg)
    {
        auto this_ = (PetDog*)arg;
        this_->to_tar_angle(&this_->rf_);
        vTaskDelete(NULL);
    }, "to_tar_angle", 2560, this, 1, NULL);

    xTaskCreate([](void* arg)
    {
        auto this_ = (PetDog*)arg;
        this_->to_tar_angle(&this_->lb_);
        vTaskDelete(NULL);
    }, "to_tar_angle", 2560, this, 1, NULL);

    xTaskCreate([](void* arg)
    {
        auto this_ = (PetDog*)arg;
        this_->to_tar_angle(&this_->rb_);
        vTaskDelete(NULL);
    }, "to_tar_angle", 2560, this, 1, NULL);

    vTaskDelay(500 / portTICK_PERIOD_MS);       //很重要
    xEventGroupSetBits(action_task_event_,START_TASK_EVENT);
}

void PetDog::to_tar_angle(target_angle_config_t *tar)
{
    if (tar == NULL)    
    {
        return;
    }
    int speed = tar->speed;
    
    if(xEventGroupWaitBits(action_task_event_,START_TASK_EVENT,pdTRUE,pdTRUE,portMAX_DELAY))
    {
        if(tar->index == LEG1)
        {
            if(left_front_angle > tar->angle)
            {
                for (size_t i = left_front_angle; i > tar->angle; i--)
                {   
                    set_left_front_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }else if (left_front_angle < tar->angle)
            {   
                for (size_t i = left_front_angle; i < tar->angle; i++)
                {
                    set_left_front_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }
        }else if(tar->index == LEG2)
        {
            if(right_front_angle > tar->angle)
            {
                for (size_t i = right_front_angle; i > tar->angle; i--)
                {   
                    set_right_front_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }else if (right_front_angle < tar->angle)
            {   
                for (size_t i = right_front_angle; i < tar->angle; i++)
                {
                    set_right_front_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }
        }else if(tar->index == LEG3)
        {
            if(left_back_angle > tar->angle)
            {
                for (size_t i = left_back_angle; i > tar->angle; i--)
                {   
                    set_left_back_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }else if (left_back_angle < tar->angle)
            {
                for (size_t i = left_back_angle; i < tar->angle; i++)
                {
                    set_left_back_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }
        }else if(tar->index == 3)
        {
            if(right_back_angle > tar->angle)
            {
                for (size_t i = right_back_angle; i > tar->angle; i--)
                {   
                    set_right_back_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }else if (right_back_angle < tar->angle)
            {
                for (size_t i = right_back_angle; i < tar->angle; i++)
                {
                    set_right_back_angle(i);
                    vTaskDelay(speed / portTICK_PERIOD_MS); 
                }
            }
        }
    }
}
void PetDog::set_right_back_angle(int angle)
{
    int anglex = 180 - angle;
    right_back_angle = angle;
    ledc_set_duty(LEDC_MODE, CHANNEL_0, anglex * per_angle + LEDC_MIN_DUTY);//加上偏移量
    ledc_update_duty(LEDC_MODE, CHANNEL_0);
}
void PetDog::set_left_front_angle(int angle)
{
    left_front_angle = angle;
    ledc_set_duty(LEDC_MODE, CHANNEL_1, angle * per_angle + LEDC_MIN_DUTY);//加上偏移量
    ledc_update_duty(LEDC_MODE, CHANNEL_1);
}
void PetDog::set_right_front_angle(int angle)
{
    int anglex = 180 - angle;
    right_front_angle = angle;
    ledc_set_duty(LEDC_MODE, CHANNEL_2, anglex * per_angle + LEDC_MIN_DUTY);//加上偏移量
    ledc_update_duty(LEDC_MODE, CHANNEL_2);
}
void PetDog::set_left_back_angle(int angle)
{
    left_back_angle = angle;
    ledc_set_duty(LEDC_MODE, CHANNEL_3, angle * per_angle + LEDC_MIN_DUTY);//加上偏移量
    ledc_update_duty(LEDC_MODE, CHANNEL_3);
}
void PetDog::set_angle(uint8_t lf_angle,uint8_t rf_angle,uint8_t lb_angle,uint8_t rb_angle)
{
    set_left_front_angle(lf_angle);
    vTaskDelay(40 / portTICK_PERIOD_MS);
    set_right_front_angle(rf_angle);
    vTaskDelay(40 / portTICK_PERIOD_MS);
    set_left_back_angle(lb_angle);
    vTaskDelay(40 / portTICK_PERIOD_MS);
    set_right_back_angle(rb_angle);
    vTaskDelay(40 / portTICK_PERIOD_MS);
}

ledc_channel_config_t PetDog::create_ledc_channel(gpio_num_t gpio, int channel)
{
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = gpio,
        .speed_mode     = LEDC_MODE,
        .channel        = (ledc_channel_t)channel,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0
    };
    return ledc_channel;
} 

void PetDog::set_speed(int speed)
{
    lf_.speed = speed;
    rf_.speed = speed;
    lb_.speed = speed;
    rb_.speed = speed;
}