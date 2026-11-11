#include "iot/thing.h"
#include "board.h"
#include "audio_codec.h"

#include <driver/gpio.h>
#include <esp_log.h>
#include "driver/rmt_tx.h"
#include "led_strip.h"

#define TAG "Lamp"

#define LED_STRIP_GPIO_NUM1         GPIO_NUM_38
#define LED_STRIP_GPIO_NUM2         GPIO_NUM_8
#define LED_NUM                     8

namespace iot {

// 这里仅定义 Lamp 的属性和方法，不包含具体的实现
class Lamp : public Thing {
private:
    void InitializeGpio() {
        led_strip_config_t strip_config1 = {
            .strip_gpio_num = LED_STRIP_GPIO_NUM1,
            .max_leds = LED_NUM,
            .led_pixel_format = LED_PIXEL_FORMAT_GRB,
            .led_model = LED_MODEL_WS2812
        };

        led_strip_config_t strip_config2 = {
            .strip_gpio_num = LED_STRIP_GPIO_NUM2,
            .max_leds = LED_NUM,
            .led_pixel_format = LED_PIXEL_FORMAT_GRB,
            .led_model = LED_MODEL_WS2812
        };

        led_strip_rmt_config_t rmt_config = {
            .resolution_hz = 10 * 1000 * 1000,
        };

        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config1, &rmt_config, &strip_1));
        ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config2, &rmt_config, &strip_2));

        led_strip_clear(strip_1);
        led_strip_clear(strip_2);

        PowerOn();
    }

public:
    bool power_ = false;
    bool bretheath_ = false;
    bool flash_ = false;
    bool rainbow_ = false;
    bool waterlight_ = false;
    int brightness_ = 50;
    std::string color_ = "白灯";

    int index_ = 0;

    led_strip_handle_t strip_1 = NULL;
    led_strip_handle_t strip_2 = NULL;

    void PowerOn() {
        xTaskCreate([](void* param) {
            int temp = 10;
            int delay_time = 1000;
            auto lamp = static_cast<Lamp*>(param);

            while (temp != 0)
            {
                temp--;
                for (int i = 0; i < LED_NUM; ++i) {
                    // 清除所有灯
                    led_strip_clear(lamp->strip_1);
                    led_strip_clear(lamp->strip_2);
    
                    led_strip_set_pixel(lamp->strip_1, i, 20 / 2, 0, 20);
                    led_strip_set_pixel(lamp->strip_2, i, 20 / 2, 0, 20);
                    // 刷新灯带
                    led_strip_refresh(lamp->strip_1);
                    led_strip_refresh(lamp->strip_2);
    
                    // 延迟
                    vTaskDelay(20 / portTICK_PERIOD_MS);
                }
            }

            

            for (size_t j = 0; j < LED_NUM; j++)
            {
                for (lamp->index_ = 0; lamp->index_ < LED_NUM - j; ++(lamp->index_)) {
                    for (int i = 0; i < LED_NUM - j; ++i) {
                        led_strip_set_pixel(lamp->strip_1, i, 0, 0, 0);
                        led_strip_set_pixel(lamp->strip_2, i, 0, 0, 0);
                    }
                    led_strip_refresh(lamp->strip_1);
                    led_strip_refresh(lamp->strip_2);
                    // 点亮当前灯

                    if (lamp->index_ == LED_NUM - j - 1) {
                        printf("index_1 = %d\n", lamp->index_);
                        led_strip_set_pixel(lamp->strip_1, lamp->index_, lamp->brightness_ / 2, 0, lamp->brightness_); // 25 0 50
                        led_strip_set_pixel(lamp->strip_2, lamp->index_, lamp->brightness_ / 2, 0, lamp->brightness_);
                        xTaskCreate([](void* param) {
                            auto lamp = static_cast<Lamp*>(param);
                            printf("index_2 = %d\n", lamp->index_);
                            for (size_t z = 0; z < lamp->brightness_; z += 5) {
                                led_strip_set_pixel(lamp->strip_1, lamp->index_, lamp->brightness_ + z / 2, z, lamp->brightness_);
                                led_strip_set_pixel(lamp->strip_2, lamp->index_, lamp->brightness_ + z / 2, z, lamp->brightness_);
                                led_strip_refresh(lamp->strip_1);
                                led_strip_refresh(lamp->strip_2);
                                vTaskDelay(40 / portTICK_PERIOD_MS);
                            }
                            vTaskDelete(nullptr); // 删除任务
                        }, "PurpleToWhiteTask", 4096, lamp, 5, nullptr);
                    } else
                    {
                        led_strip_set_pixel(lamp->strip_1, lamp->index_, lamp->brightness_ / 2, lamp->brightness_ / 2, lamp->brightness_ / 2);
                        led_strip_set_pixel(lamp->strip_2, lamp->index_, lamp->brightness_ / 2, lamp->brightness_ / 2, lamp->brightness_ / 2);
                    }
                    // 刷新灯带
                    led_strip_refresh(lamp->strip_1);
                    led_strip_refresh(lamp->strip_2);
    
                    // 延迟
                    vTaskDelay(20 / portTICK_PERIOD_MS);
                }
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }
            for (size_t z = 0; z < lamp->brightness_; z += 5) {
                led_strip_set_pixel(lamp->strip_1, 0, lamp->brightness_ + z / 2, z, lamp->brightness_);
                led_strip_set_pixel(lamp->strip_2, 0, lamp->brightness_ + z / 2, z, lamp->brightness_);
                led_strip_refresh(lamp->strip_1);
                led_strip_refresh(lamp->strip_2);
                vTaskDelay(40 / portTICK_PERIOD_MS);
            }
            vTaskDelay(1000 / portTICK_PERIOD_MS);

            temp = 3;
            while (temp != 0)
            {            
                temp--;
                for (size_t i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(lamp->strip_1, i, lamp->brightness_, lamp->brightness_, lamp->brightness_);
                    led_strip_set_pixel(lamp->strip_2, i, lamp->brightness_, lamp->brightness_, lamp->brightness_);
                }
                led_strip_refresh(lamp->strip_1);
                led_strip_refresh(lamp->strip_2);

                vTaskDelay(200 / portTICK_PERIOD_MS);
                for (size_t i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(lamp->strip_1, i, 0, 0, 0);
                    led_strip_set_pixel(lamp->strip_2, i, 0, 0, 0);
                }
                led_strip_refresh(lamp->strip_1);
                led_strip_refresh(lamp->strip_2);
                vTaskDelay(200 / portTICK_PERIOD_MS);
            }
            
            led_strip_clear(lamp->strip_1);
            led_strip_clear(lamp->strip_2);
            led_strip_refresh(lamp->strip_1);
            led_strip_refresh(lamp->strip_2);

            vTaskDelete(nullptr); // 删除任务
        }, "PowerOnEffectTask", 4096, this, 5, nullptr);
    }

    Lamp() : Thing("Lamp", "WS2812灯带"){
        InitializeGpio();
        // 定义设备可以被远程执行的指令
        methods_.AddMethod("SetBrightness", "设置灯的亮度", ParameterList({
            Parameter("brightness", "0到100之间的整数", kValueTypeNumber, true)
        }), [this](const ParameterList& parameters) {
            brightness_ = parameters["brightness"].number() * 2.55; // 将0-100的值转换为0-255
            brightness_ = std::clamp(brightness_, 0, 255); // 使用 std::clamp 简化边界检查
            if (color_ == "红灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, 0, 0);
                    led_strip_set_pixel(strip_2, i, brightness_, 0, 0);
                }
            } else if (color_ == "橙灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, brightness_ / 2, 0);
                    led_strip_set_pixel(strip_2, i, brightness_, brightness_ / 2, 0);
                }
            } else if (color_ == "黄灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, brightness_, 0);
                    led_strip_set_pixel(strip_2, i, brightness_, brightness_, 0);
                }
            } else if (color_ == "绿灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, 0, brightness_, 0);
                    led_strip_set_pixel(strip_2, i, 0, brightness_, 0);
                }
            } else if (color_ == "蓝灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, 0, 0, brightness_);
                    led_strip_set_pixel(strip_2, i, 0, 0, brightness_);
                }
            } else if (color_ == "靛灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, 0, brightness_ / 2, brightness_);
                    led_strip_set_pixel(strip_2, i, 0, brightness_ / 2, brightness_);
                }
            } else if (color_ == "紫灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_ / 2, 0, brightness_);
                    led_strip_set_pixel(strip_2, i, brightness_ / 2, 0, brightness_);
                }
            } else if (color_ == "关灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    power_ = false;
                    printf("power: %d, brightness: %d\n", !power_, brightness_);
                    led_strip_set_pixel(strip_1, i, 0, 0, 0);
                    led_strip_set_pixel(strip_2, i, 0, 0, 0);
                }
            } else if (color_ == "白灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, brightness_, brightness_);
                    led_strip_set_pixel(strip_2, i, brightness_, brightness_, brightness_);
                }
            } else {
                ESP_LOGW(TAG, "未知的颜色: %s", color_.c_str());
                return;
            }
            led_strip_refresh(strip_1);
            led_strip_refresh(strip_2);
        });

        methods_.AddMethod("SetLightColor", "设置灯的颜色", ParameterList({
            Parameter("lightcolor", "红灯，橙灯，黄灯，绿灯，蓝灯，紫灯，白灯，关灯", kValueTypeString, true)
        }), [this](const ParameterList& parameters) {
            color_ = parameters["lightcolor"].string();
            printf("设置灯的颜色: %s, 亮度: %d\n", color_.c_str(), brightness_);
            if (color_ == "红灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, 0, 0);
                    led_strip_set_pixel(strip_2, i, brightness_, 0, 0);
                }
            } else if (color_ == "橙灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, brightness_ / 2, 0);
                    led_strip_set_pixel(strip_2, i, brightness_, brightness_ / 2, 0);
                }
            } else if (color_ == "黄灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, brightness_, 0);
                    led_strip_set_pixel(strip_2, i, brightness_, brightness_, 0);
                }
            } else if (color_ == "绿灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, 0, brightness_, 0);
                    led_strip_set_pixel(strip_2, i, 0, brightness_, 0);
                }
            } else if (color_ == "蓝灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, 0, 0, brightness_);
                    led_strip_set_pixel(strip_2, i, 0, 0, brightness_);
                }
            } else if (color_ == "靛灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, 0, brightness_ / 2, brightness_);
                    led_strip_set_pixel(strip_2, i, 0, brightness_ / 2, brightness_);
                }
            } else if (color_ == "紫灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_ / 2, 0, brightness_);
                    led_strip_set_pixel(strip_2, i, brightness_ / 2, 0, brightness_);
                }
            } else if (color_ == "白灯") {
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, brightness_, brightness_, brightness_);
                    led_strip_set_pixel(strip_2, i, brightness_, brightness_, brightness_);
                }
            } else if (color_ == "关灯") {
                power_ = false;
                for (int i = 0; i < LED_NUM; ++i) {
                    led_strip_set_pixel(strip_1, i, 0, 0, 0);
                    led_strip_set_pixel(strip_2, i, 0, 0, 0);
                }
            }else {
                ESP_LOGW(TAG, "未知的颜色: %s", color_.c_str());
                return;
            }
            led_strip_refresh(strip_1);
            led_strip_refresh(strip_2);
        });

        methods_.AddMethod("flashlight", "闪光灯", ParameterList(), [this](const ParameterList& parameters) 
        {
            power_ = true;
            if (bretheath_ == true || rainbow_ == true || waterlight_ == true) {
                ESP_LOGW(TAG, "请先关闭其他灯光效果");
                return;
            }
            if (flash_ == true) {
                ESP_LOGW(TAG, "闪光灯已开启");
                return;
            }
            flash_ = true;
    
            xTaskCreate([](void* param) {
                auto lamp = static_cast<Lamp*>(param);
                while (lamp->power_) {
                    for (int state = 0; state < 2; ++state) {
                        int brightness = state ? 255 : 0;
                        for (int j = 0; j < LED_NUM; ++j) {
                            if (lamp->color_ == "红灯") {
                                led_strip_set_pixel(lamp->strip_1, j, brightness, 0, 0);
                                led_strip_set_pixel(lamp->strip_2, j, brightness, 0, 0);
                            } else if (lamp->color_ == "橙灯") {
                                led_strip_set_pixel(lamp->strip_1, j, brightness, brightness / 2, 0);
                                led_strip_set_pixel(lamp->strip_2, j, brightness, brightness / 2, 0);
                            } else if (lamp->color_ == "黄灯") {
                                led_strip_set_pixel(lamp->strip_1, j, brightness, brightness, 0);
                                led_strip_set_pixel(lamp->strip_2, j, brightness, brightness, 0);
                            } else if (lamp->color_ == "绿灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, brightness, 0);
                                led_strip_set_pixel(lamp->strip_2, j, 0, brightness, 0);
                            } else if (lamp->color_ == "蓝灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, 0, brightness);
                                led_strip_set_pixel(lamp->strip_2, j, 0, 0, brightness);
                            } else if (lamp->color_ == "紫灯") {
                                led_strip_set_pixel(lamp->strip_1, j, brightness / 2, 0, brightness);
                                led_strip_set_pixel(lamp->strip_2, j, brightness / 2, 0, brightness);
                            } else{
                                led_strip_set_pixel(lamp->strip_1, j, brightness, brightness, brightness);
                                led_strip_set_pixel(lamp->strip_2, j, brightness, brightness, brightness);
                            } 
                        }
                        led_strip_refresh(lamp->strip_1);
                        led_strip_refresh(lamp->strip_2);
                        vTaskDelay(100 / portTICK_PERIOD_MS);
                    }
                }
                // 最后清除所有灯
                led_strip_clear(lamp->strip_1);
                led_strip_clear(lamp->strip_2);
                led_strip_refresh(lamp->strip_1);
                led_strip_refresh(lamp->strip_2);
                lamp->flash_ = false; // 任务创建后立即设置为false
                vTaskDelete(nullptr); // 删除任务
            }, "FlashlightTask", 4096, this, 5, nullptr);
        });

        methods_.AddMethod("waterlamps", "流水灯", ParameterList(), [this](const ParameterList& parameters) 
        {
            if (bretheath_ == true || rainbow_ == true || flash_ == true) {
                ESP_LOGW(TAG, "请先关闭其他灯光效果");
                return;
            }
            if(waterlight_ == true) {
                ESP_LOGW(TAG, "流水灯已开启");
                return;
            }
            waterlight_ = true;
            power_ = true;
            xTaskCreate([](void* param) {
                auto lamp = static_cast<Lamp*>(param);
                int delay = 200; // 初始延迟时间
                while(lamp->power_) { 
                    for (int i = 0; i < LED_NUM; ++i) {
                        // 清除所有灯
                        led_strip_clear(lamp->strip_1);
                        led_strip_clear(lamp->strip_2);

                        // 点亮当前灯及其后两个灯
                        for (int offset = 0; offset < 2; ++offset) {
                            int index = (i + offset) % LED_NUM;
                            if (lamp->color_ == "红灯") {
                                led_strip_set_pixel(lamp->strip_1, index, lamp->brightness_, 0, 0);
                                led_strip_set_pixel(lamp->strip_2, index, lamp->brightness_, 0, 0);
                            } else if (lamp->color_ == "橙灯") {
                                led_strip_set_pixel(lamp->strip_1, index, lamp->brightness_, lamp->brightness_ / 2, 0);
                                led_strip_set_pixel(lamp->strip_2, index, lamp->brightness_, lamp->brightness_ / 2, 0);
                            } else if (lamp->color_ == "黄灯") {
                                led_strip_set_pixel(lamp->strip_1, index, lamp->brightness_, lamp->brightness_, 0);
                                led_strip_set_pixel(lamp->strip_2, index, lamp->brightness_, lamp->brightness_, 0);
                            } else if (lamp->color_ == "绿灯") {
                                led_strip_set_pixel(lamp->strip_1, index, 0, lamp->brightness_, 0);
                                led_strip_set_pixel(lamp->strip_2, index, 0, lamp->brightness_, 0);
                            } else if (lamp->color_ == "蓝灯") {
                                led_strip_set_pixel(lamp->strip_1, index, 0, 0, lamp->brightness_);
                                led_strip_set_pixel(lamp->strip_2, index, 0, 0, lamp->brightness_);
                            } else if (lamp->color_ == "靛灯") {
                                led_strip_set_pixel(lamp->strip_1, index, 0, lamp->brightness_ / 2, lamp->brightness_);
                                led_strip_set_pixel(lamp->strip_2, index, 0, lamp->brightness_ / 2, lamp->brightness_);
                            } else if (lamp->color_ == "紫灯") {
                                led_strip_set_pixel(lamp->strip_1, index, lamp->brightness_ / 2, 0, lamp->brightness_);
                                led_strip_set_pixel(lamp->strip_2, index, lamp->brightness_ / 2, 0, lamp->brightness_);
                            } else {
                                led_strip_set_pixel(lamp->strip_1, index, lamp->brightness_, lamp->brightness_, lamp->brightness_);
                                led_strip_set_pixel(lamp->strip_2, index, lamp->brightness_, lamp->brightness_, lamp->brightness_);
                            }
                        }

                        // 刷新灯带
                        led_strip_refresh(lamp->strip_1);
                        led_strip_refresh(lamp->strip_2);

                        // 延迟
                        vTaskDelay(delay / portTICK_PERIOD_MS);
                    }
                }
                // 最后清除所有灯
                led_strip_clear(lamp->strip_1);
                led_strip_clear(lamp->strip_2);
                led_strip_refresh(lamp->strip_1);
                led_strip_refresh(lamp->strip_2);
                lamp->waterlight_ = false; // 任务完成后设置为false
                vTaskDelete(nullptr); // 删除任务
            }, "WaterLightTask", 4096, this, 5, nullptr);
        });

        methods_.AddMethod("breathe", "呼吸灯", ParameterList(), [this](const ParameterList& parameters) 
        {
            if (flash_ == true || rainbow_ == true || waterlight_ == true) {
                ESP_LOGW(TAG, "请先关闭其他灯光效果");
                return;
            }
            if (bretheath_ == true) {
                ESP_LOGW(TAG, "呼吸灯已开启");
                return;
            }
            bretheath_ = true;
            power_ = true;
            xTaskCreate([](void* param) {
                auto lamp = static_cast<Lamp*>(param);
                while (lamp->power_) {
                    for (int i = 0; i <= 255; ++i) {
                        for (int j = 0; j < LED_NUM; ++j) {
                            if (lamp->color_ == "红灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i, 0, 0);
                                led_strip_set_pixel(lamp->strip_2, j, i, 0, 0);
                            } else if (lamp->color_ == "橙灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i, i / 2, 0);
                                led_strip_set_pixel(lamp->strip_2, j, i, i / 2, 0);
                            } else if (lamp->color_ == "黄灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i, i, 0);
                                led_strip_set_pixel(lamp->strip_2, j, i, i, 0);
                            } else if (lamp->color_ == "绿灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, i, 0);
                                led_strip_set_pixel(lamp->strip_2, j, 0, i, 0);
                            } else if (lamp->color_ == "蓝灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, 0, i);
                                led_strip_set_pixel(lamp->strip_2, j, 0, 0, i);
                            } else if (lamp->color_ == "靛灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, i / 2, i);
                                led_strip_set_pixel(lamp->strip_2, j, 0, i / 2, i);
                            } else if (lamp->color_ == "紫灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i / 2, 0, i);
                                led_strip_set_pixel(lamp->strip_2, j, i / 2, 0, i);
                            } else if (lamp->color_ == "白灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i, i, i);
                                led_strip_set_pixel(lamp->strip_2, j, i, i, i);
                            } else {
                                led_strip_set_pixel(lamp->strip_1, j, 0, 0, 0);
                                led_strip_set_pixel(lamp->strip_2, j, 0, 0, 0);
                            }
                        }
                        led_strip_refresh(lamp->strip_1);
                        led_strip_refresh(lamp->strip_2);
                        vTaskDelay(5 / portTICK_PERIOD_MS);
                    }

                    for (int i = 255; i >= 0; --i) {
                        for (int j = 0; j < LED_NUM; ++j) {
                            if (lamp->color_ == "红灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i, 0, 0);
                                led_strip_set_pixel(lamp->strip_2, j, i, 0, 0);
                            } else if (lamp->color_ == "橙灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i, i / 2, 0);
                                led_strip_set_pixel(lamp->strip_2, j, i, i / 2, 0);
                            } else if (lamp->color_ == "黄灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i, i, 0);
                                led_strip_set_pixel(lamp->strip_2, j, i, i, 0);
                            } else if (lamp->color_ == "绿灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, i, 0);
                                led_strip_set_pixel(lamp->strip_2, j, 0, i, 0);
                            } else if (lamp->color_ == "蓝灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, 0, i);
                                led_strip_set_pixel(lamp->strip_2, j, 0, 0, i);
                            } else if (lamp->color_ == "靛灯") {
                                led_strip_set_pixel(lamp->strip_1, j, 0, i / 2, i);
                                led_strip_set_pixel(lamp->strip_2, j, 0, i / 2, i);
                            } else if (lamp->color_ == "紫灯") {
                                led_strip_set_pixel(lamp->strip_1, j, i / 2, 0, i);
                                led_strip_set_pixel(lamp->strip_2, j, i / 2, 0, i);
                            } else {
                                led_strip_set_pixel(lamp->strip_1, j, i, i, i);
                                led_strip_set_pixel(lamp->strip_2, j, i, i, i);
                            } 
                        }
                        led_strip_refresh(lamp->strip_1);
                        led_strip_refresh(lamp->strip_2);
                        vTaskDelay(10 / portTICK_PERIOD_MS);
                    }
                }
                // 最后清除所有灯
                led_strip_clear(lamp->strip_1);
                led_strip_clear(lamp->strip_2);
                led_strip_refresh(lamp->strip_1);
                led_strip_refresh(lamp->strip_2);
                lamp->bretheath_ = false; // 任务完成后设置为false
                vTaskDelete(nullptr); // 删除任务
            }, "BreatheLightTask", 4096, this, 5, nullptr);
        });

        methods_.AddMethod("rainbowlight", "彩虹灯", ParameterList(), [this](const ParameterList& parameters) 
        {
            if (flash_ == true || rainbow_ == true || waterlight_ == true || bretheath_ == true) {
                ESP_LOGW(TAG, "请先关闭其他灯光效果");
                return;
            }
            rainbow_ = true;
            power_ = true;
            xTaskCreate([](void* param) {
                auto lamp = static_cast<Lamp*>(param);
                int delay = 20; // 每次更新的延迟时间，20ms
                int hue = 0; // 初始色调
                while (lamp->power_) {
                    for (int i = 0; i < LED_NUM; ++i) {
                        // 计算每个LED的色调
                        int led_hue = (hue + i * (360 / LED_NUM)) % 360;

                        // 将色调转换为RGB
                        int r, g, b;
                        if (led_hue < 120) {
                            r = (255 * (120 - led_hue)) / 120;
                            g = (255 * led_hue) / 120;
                            b = 0;
                        } else if (led_hue < 240) {
                            led_hue -= 120;
                            r = 0;
                            g = (255 * (120 - led_hue)) / 120;
                            b = (255 * led_hue) / 120;
                        } else {
                            led_hue -= 240;
                            r = (255 * led_hue) / 120;
                            g = 0;
                            b = (255 * (120 - led_hue)) / 120;
                        }

                        // 设置LED颜色
                        led_strip_set_pixel(lamp->strip_1, i, r * lamp->brightness_ / 255, g * lamp->brightness_ / 255, b * lamp->brightness_ / 255);
                        led_strip_set_pixel(lamp->strip_2, i, r * lamp->brightness_ / 255, g * lamp->brightness_ / 255, b * lamp->brightness_ / 255);
                    }

                    // 刷新灯带
                    led_strip_refresh(lamp->strip_1);
                    led_strip_refresh(lamp->strip_2);

                    // 延迟
                    vTaskDelay(delay / portTICK_PERIOD_MS);

                    // 增加色调
                    hue = (hue + 1) % 360;
                }
                // 最后清除所有灯
                led_strip_clear(lamp->strip_1);
                led_strip_clear(lamp->strip_2);
                led_strip_refresh(lamp->strip_1);
                led_strip_refresh(lamp->strip_2);
                lamp->rainbow_ = false; // 任务创建后立即设置为false

                vTaskDelete(nullptr); // 删除任务
            }, "RainbowLightTask", 4096, this, 5, nullptr);
        });
    }
};

} // namespace iot


DECLARE_THING(Lamp);