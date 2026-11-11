#ifndef SSD1306_DISPLAY_H
#define SSD1306_DISPLAY_H

#include "display.h"
#include "emotion_bitmaps.h"
#include "emotion_manager.h"
#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>

class Sh1106Display : public Display {
private:
    int flag_iic = 1;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;

    TaskHandle_t blink_task_handle;
    TaskHandle_t idle_task_handle;

    bool mirror_x_ = false;
    bool mirror_y_ = false;

    lv_obj_t* status_bar_ = nullptr;
    lv_obj_t* content_ = nullptr;
    lv_obj_t* container_ = nullptr;
    lv_obj_t* side_bar_ = nullptr;

    lv_obj_t *square1 = NULL;   //眼睛
    lv_obj_t *square2 = NULL;
    lv_obj_t* emotion_canvas_ = nullptr;
    lv_obj_t* gif_img_ = nullptr;
    bool emotion_mode_active_ = false;
    bool gif_mode_active_ = false;

    int cur_square1_x = 0;
    int cur_square1_y = 0;
    int cur_square2_x = 0;
    int cur_square2_y = 0;

    virtual bool Lock(int timeout_ms = 0) override;
    virtual void Unlock() override;

    void SetupUI_128x64();
    void SetupUI_128x32();

    void to_any_position(int tar1x,int tar1y,int tarx2,int tary2);
    int bresenham_line(int x1, int y1, int x2, int y2,int ret[]);

    void close_eyes();
    void open_eyes();
    void blink_eyes();

    void blink_task();
    void eye_move_emtion_task();
    
public:
    Sh1106Display(void* i2c_master_handle, int width, int height, bool mirror_x = false, bool mirror_y = false);
    ~Sh1106Display();

    virtual void SetEmotion(const std::string& emotion) override;
    virtual void start_emtion() override;
    virtual void stop_emtion()  override;
    virtual void idle_emtion()  override;
    void SetEmotion(Emotion emotion);
    void SetEmotionForState(DeviceState state);
    void ClearEmotion();
    void ShowIdleGif();
    void HideIdleGif();
};

#endif // SSD1306_DISPLAY_H
