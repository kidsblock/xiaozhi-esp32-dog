#include "esp_lcd_panel_sh1106.h"
#include "font_awesome_symbols.h"

#include <esp_log.h>
#include <esp_err.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <esp_lvgl_port.h>
#include <utility>
#include "sh1106_display.h"
#include "lazy_blink_gif.h"

#define TAG "Sh1106Display"

#define EYE_SIZE 40
#define EYE_BLINK_FREQ 80
#define EYE_GAP 25
#define EYE_RADIUS 10

LV_FONT_DECLARE(font_puhui_14_1);
LV_FONT_DECLARE(font_awesome_30_1);
LV_FONT_DECLARE(font_awesome_14_1);

Sh1106Display::Sh1106Display(void* i2c_master_handle, int width, int height, bool mirror_x, bool mirror_y)
    : mirror_x_(mirror_x), mirror_y_(mirror_y) {
    width_ = width;
    height_ = height;

    ESP_LOGI(TAG, "Initialize LVGL");
    lvgl_port_cfg_t port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&port_cfg);

    // SH1106 config
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = 0x3C,
        .on_color_trans_done = nullptr,
        .user_ctx = nullptr,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .flags = {
            .dc_low_on_data = 0,
            .disable_control_phase = 0,
        },
        .scl_speed_hz = 100 * 1000,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c_v2((i2c_master_bus_t*)i2c_master_handle, &io_config, &panel_io_));

    ESP_LOGI(TAG, "Install SH1106 driver");
    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.reset_gpio_num = -1;
    panel_config.bits_per_pixel = 1;

    esp_lcd_panel_sh1106_config_t sh1106_config = {
        // .height = EXAMPLE_LCD_V_RES,
    };
    panel_config.vendor_config = &sh1106_config;

    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1106(panel_io_, &panel_config, &panel_));
    ESP_LOGI(TAG, "SH1106 driver installed");

    // Reset the display
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
    if (esp_lcd_panel_init(panel_) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize display");
        flag_iic = 0;
        return;
    }

    // Set the display to on
    ESP_LOGI(TAG, "Turning display on");
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));

    ESP_LOGI(TAG, "Adding LCD screen");
    const lvgl_port_display_cfg_t display_cfg = {
        .io_handle = panel_io_,
        .panel_handle = panel_,
        .control_handle = nullptr,
        .buffer_size = static_cast<uint32_t>(width_ * height_),
        .double_buffer = false,
        .trans_size = 0,
        .hres = static_cast<uint32_t>(width_),
        .vres = static_cast<uint32_t>(height_),
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = mirror_x_,
            .mirror_y = mirror_y_,
        },
        .flags = {
            .buff_dma = 1,
            .buff_spiram = 0,
            .sw_rotate = 0,
            .full_refresh = 0,
            .direct_mode = 0,
        },
    };

    disp_ = lvgl_port_add_disp(&display_cfg);

    if (disp_ == nullptr) {
        ESP_LOGE(TAG, "Failed to add display");
        return;
    }

    if (height_ == 64) {
        SetupUI_128x64();
    } else {
        SetupUI_128x32();
    }
}

Sh1106Display::~Sh1106Display() {
    if (gif_img_ != nullptr) lv_obj_del(gif_img_);
    if (emotion_canvas_ != nullptr) lv_obj_del(emotion_canvas_);
    if (content_ != nullptr) {
        lv_obj_del(content_);
    }
    if (status_bar_ != nullptr) {
        lv_obj_del(status_bar_);
    }
    if (side_bar_ != nullptr) {
        lv_obj_del(side_bar_);
    }
    if (container_ != nullptr) {
        lv_obj_del(container_);
    }

    if (panel_ != nullptr) {
        esp_lcd_panel_del(panel_);
    }
    if (panel_io_ != nullptr) {
        esp_lcd_panel_io_del(panel_io_);
    }
    lvgl_port_deinit();
}

bool Sh1106Display::Lock(int timeout_ms) {
    return lvgl_port_lock(timeout_ms);
}

void Sh1106Display::Unlock() {
    lvgl_port_unlock();
}

/*
        //三角形
        square1 = lv_canvas_create(screen); // 创建第一个 canvas 对象，用于绘制第一个眼睛
        static lv_color_t buf1[LV_CANVAS_BUF_SIZE_TRUE_COLOR(EYE_SIZE, EYE_SIZE)]; // 定义第一个 canvas 的缓冲区
        lv_canvas_set_buffer(square1, buf1, EYE_SIZE, EYE_SIZE, LV_IMG_CF_TRUE_COLOR); // 设置第一个 canvas 的缓冲区和尺寸
        lv_canvas_fill_bg(square1, lv_color_hex(0x00), LV_OPA_COVER); // 填充第一个 canvas 的背景为黑色
        lv_point_t points1[] = {{EYE_SIZE / 2, 0}, {0, EYE_SIZE}, {EYE_SIZE, EYE_SIZE}}; // 定义第一个眼睛的三角形顶点
        lv_draw_rect_dsc_t rect_dsc; // 定义绘制矩形的描述符
        lv_draw_rect_dsc_init(&rect_dsc); // 初始化矩形描述符
        rect_dsc.bg_color = lv_color_hex(0xFF); // 设置矩形的背景颜色为白色
        lv_canvas_draw_polygon(square1, points1, 3, &rect_dsc); // 在第一个 canvas 上绘制三角形
        lv_obj_align(square1, LV_ALIGN_CENTER, EYE_GAP, 0); // 将第一个 canvas 对齐到屏幕中心并向右偏移 EYE_GAP

        square2 = lv_canvas_create(screen); // 创建第二个 canvas 对象，用于绘制第二个眼睛
        static lv_color_t buf2[LV_CANVAS_BUF_SIZE_TRUE_COLOR(EYE_SIZE, EYE_SIZE)]; // 定义第二个 canvas 的缓冲区
        lv_canvas_set_buffer(square2, buf2, EYE_SIZE, EYE_SIZE, LV_IMG_CF_TRUE_COLOR); // 设置第二个 canvas 的缓冲区和尺寸
        lv_canvas_fill_bg(square2, lv_color_hex(0x00), LV_OPA_COVER); // 填充第二个 canvas 的背景为黑色
        lv_point_t points2[] = {{EYE_SIZE / 2, 0}, {0, EYE_SIZE}, {EYE_SIZE, EYE_SIZE}}; // 定义第二个眼睛的三角形顶点
        lv_draw_rect_dsc_t rect_dsc2; // 定义绘制矩形的描述符
        lv_draw_rect_dsc_init(&rect_dsc2); // 初始化矩形描述符
        rect_dsc2.bg_color = lv_color_hex(0xFF); // 设置矩形的背景颜色为白色
        lv_canvas_draw_polygon(square2, points2, 3, &rect_dsc2); // 在第二个 canvas 上绘制三角形
        lv_obj_align(square2, LV_ALIGN_CENTER, -EYE_GAP, 0); // 将第二个 canvas 对齐到屏幕中心并向左偏移 EYE_GAP
*/
void Sh1106Display::SetupUI_128x64() {
    DisplayLockGuard lock(this);

    auto screen = lv_disp_get_scr_act(disp_);
    lv_obj_set_style_text_font(screen, &font_puhui_14_1, 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x00), 0);

/*************************************************************************************/
    if(flag_iic) {
        
        square1 = lv_obj_create(screen);
        lv_obj_set_size(square1, EYE_SIZE, EYE_SIZE);  // 容器大小设置为屏幕的宽高
        lv_obj_set_flex_flow(square1, LV_FLEX_FLOW_COLUMN);  // 容器设置为按行布局
        lv_obj_set_style_radius(square1, EYE_RADIUS, 0);
        lv_obj_set_scrollbar_mode(square1, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_bg_color(square1, lv_color_hex(0xFF), 0);
    
        square2 = lv_obj_create(screen);
        lv_obj_set_size(square2, EYE_SIZE, EYE_SIZE);  // 容器大小设置为屏幕的宽高
        lv_obj_set_flex_flow(square2, LV_FLEX_FLOW_COLUMN);  // 容器设置为按行布局
        lv_obj_set_style_radius(square2, EYE_RADIUS, 0);
        lv_obj_set_scrollbar_mode(square2, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_bg_color(square2, lv_color_hex(0xFF), 0);
    
        lv_obj_align(square1, LV_ALIGN_CENTER, EYE_GAP, 0);
        lv_obj_align(square2, LV_ALIGN_CENTER, -EYE_GAP, 0);
        
        cur_square1_x = lv_obj_get_x_aligned(square1);
        cur_square1_y = lv_obj_get_y_aligned(square1);
        cur_square2_x = lv_obj_get_x_aligned(square2);
        cur_square2_y = lv_obj_get_y_aligned(square2);
    
        lv_obj_add_flag(square1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(square2, LV_OBJ_FLAG_HIDDEN);
    }
/*************************************************************************************/


    /* Container */
    container_ = lv_obj_create(screen);
    lv_obj_set_size(container_, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);  //LV_FLEX_FLOW_COLUMN 表示容器的元素应该按列的方式排列
    lv_obj_set_style_pad_all(container_, 0, 0);             //外边距
    lv_obj_set_style_border_width(container_, 0, 0);        //边框宽度
    lv_obj_set_style_pad_row(container_, 0, 0);             //行间距
    lv_obj_set_style_text_color(container_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(container_, lv_color_hex(0x00), 0);

    /* Status bar */
    status_bar_ = lv_obj_create(container_);
    lv_obj_set_size(status_bar_, LV_HOR_RES, 18);           //宽度设置为屏幕宽度 (LV_HOR_RES)，高度设置为 18 像素
    lv_obj_set_style_radius(status_bar_, 0, 0);             //确保状态栏没有圆角
    lv_obj_set_style_text_color(status_bar_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(status_bar_, lv_color_hex(0x00), 0);

    /* Content */
    content_ = lv_obj_create(container_);                               
    lv_obj_set_scrollbar_mode(content_, LV_SCROLLBAR_MODE_OFF);         //确保 content_ 区域的内容不会显示滚动条
    lv_obj_set_style_radius(status_bar_, 0, 0);                         //确保状态栏没有圆角
    lv_obj_set_width(content_, LV_HOR_RES);                             //宽度为屏幕宽度128
    lv_obj_set_flex_grow(content_, 1);                                  //根据容器的剩余空间自动扩展并填充。
    lv_obj_set_style_text_color(content_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(content_, lv_color_hex(0x00), 0);


    emotion_label_ = lv_label_create(content_);                         
    lv_obj_set_style_text_font(emotion_label_, &font_awesome_30_1, 0);  
    lv_label_set_text(emotion_label_, FONT_AWESOME_AI_CHIP);            //设置机器人图标
    lv_obj_center(emotion_label_);                                      //居中
    lv_obj_set_style_text_color(emotion_label_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(emotion_label_, lv_color_hex(0x00), 0);

    /* Status bar */
    lv_obj_set_flex_flow(status_bar_, LV_FLEX_FLOW_ROW);                // 布局流动方向为水平方向
    lv_obj_set_style_pad_all(status_bar_, 0, 0);                        //所有边距（内边距）设置为 0
    lv_obj_set_style_border_width(status_bar_, 0, 0);                   //移除 status_bar_ 的边框
    lv_obj_set_style_pad_column(status_bar_, 0, 0);                     //垂直方向上的内边距设置为 0
    lv_obj_set_style_text_color(status_bar_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(status_bar_, lv_color_hex(0x00), 0);
    
    network_label_ = lv_label_create(status_bar_);                      //网络标签
    lv_label_set_text(network_label_, "");
    lv_obj_set_style_text_font(network_label_, &font_awesome_14_1, 0);
    lv_obj_set_style_text_color(network_label_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(network_label_, lv_color_hex(0x00), 0);
    
    notification_label_ = lv_label_create(status_bar_);                 //通知标签
    lv_obj_set_flex_grow(notification_label_, 1);
    lv_obj_set_style_text_align(notification_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(notification_label_, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
    lv_label_set_text(notification_label_, "通知");
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_color(notification_label_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(notification_label_, lv_color_hex(0x00), 0);
    
    status_label_ = lv_label_create(status_bar_);
    lv_obj_set_flex_grow(status_label_, 1);
    lv_label_set_text(status_label_, "正在初始化");
    lv_obj_set_style_text_align(status_label_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(status_label_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(status_label_, lv_color_hex(0x00), 0);
    
    mute_label_ = lv_label_create(status_bar_);
    lv_label_set_text(mute_label_, "");
    lv_obj_set_style_text_font(mute_label_, &font_awesome_14_1, 0);
    lv_obj_set_style_text_color(mute_label_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(mute_label_, lv_color_hex(0x00), 0);
    
    battery_label_ = lv_label_create(status_bar_);
    lv_label_set_text(battery_label_, "");
    lv_obj_set_style_text_font(battery_label_, &font_awesome_14_1, 0);
    lv_obj_set_style_text_color(battery_label_, lv_color_hex(0xff), 0);
    lv_obj_set_style_bg_color(battery_label_, lv_color_hex(0x00), 0);
    
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void Sh1106Display::SetupUI_128x32() {
}

//Bresenham 直线算法
int Sh1106Display::bresenham_line(int x1, int y1, int x2, int y2,int ret[]) {
    // 计算直线的增量
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1; // 方向向量
    int sy = (y1 < y2) ? 1 : -1; // 方向向量
    int err = dx - dy; // 误差项
    int count = 0;
    while (1) {
        // 输出当前点
        ret[count] = x1;
        ret[count+1] = y1;
        count+=2;
        // 判断是否到达终点
        if (x1 == x2 && y1 == y2) {
            break;
        }
        // 计算误差并更新坐标
        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
    return count;
}


void Sh1106Display::to_any_position(int tar1x,int tar1y,int tarx2,int tary2)
{
    if (flag_iic == 0)
    {
        return;
    }
    int ret1[100];
    int ret2[100];
    int num = bresenham_line(cur_square1_x, cur_square1_y, tar1x, tar1y,ret1);
    bresenham_line(cur_square2_x, cur_square2_y, tarx2, tary2,ret2);

    for (size_t i = 0; i < num; i+=2)
    {
        for (size_t j = 0; j < 5; j++)
        {
            lv_obj_align(square1, LV_ALIGN_CENTER, ret1[i], ret1[i+1]);
            lv_obj_align(square2, LV_ALIGN_CENTER, ret2[i], ret2[i+1]);
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }
    //更新位置
    cur_square1_x = lv_obj_get_x_aligned(square1);
    cur_square1_y = lv_obj_get_y_aligned(square1);
    cur_square2_x = lv_obj_get_x_aligned(square2);
    cur_square2_y = lv_obj_get_y_aligned(square2);
}

void Sh1106Display::close_eyes()
{
    if (flag_iic == 0)
    {
        return;
    }
    for (size_t i = EYE_SIZE; i > 10; i--)
    {
        lv_obj_set_size(square1,EYE_SIZE+i*0.1,i);
        lv_obj_set_size(square2,EYE_SIZE+i*0.1,i);
        vTaskDelay(15 / portTICK_PERIOD_MS);
    }
}


void Sh1106Display::open_eyes()
{
    if (flag_iic == 0)
    {
        return;
    }
    for (size_t i = 10; i < EYE_SIZE; i++)
    {
        lv_obj_set_size(square1,EYE_SIZE,i);
        lv_obj_set_size(square2,EYE_SIZE,i);
        vTaskDelay(15 / portTICK_PERIOD_MS);
    }
}

void Sh1106Display::blink_eyes()
{
    close_eyes();
    open_eyes();
}


void Sh1106Display::blink_task()
{
    if (flag_iic == 0)
    {
        return;
    }
    int rand_ = 0;
    
    int flag = 0;
    while(1)
    {
        srand(time(NULL));
        rand_ = rand() % 51;
        flag = rand() % 101;
        if(rand_ < 10)
            rand_ = 1;
        else if(rand_ > 10 && rand_ < 20)
            rand_ = 2;
        else if(rand_ > 20 && rand_ < 30)
            rand_ = 3;
        else if(rand_ > 30 && rand_ < 40)
            rand_ = 4;
        else if(rand_ > 40 && rand_ < 50)
            rand_ = 5;
        else
            rand_ = 6;
        vTaskDelay(rand_ * 1000 / portTICK_PERIOD_MS);
        if(flag < EYE_BLINK_FREQ)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            blink_eyes();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void Sh1106Display::eye_move_emtion_task()
{
    if (flag_iic == 0)
    {
        return;
    }
    int x = 0;
    int y = 0;
    while (1)
    {
        // 使用当前时间作为随机数种子
        srand(time(NULL));
        // 随机生成 x 和 y 的偏移量，范围为 -15 到 15
        x = rand() % 31 - 15;
        y = rand() % 31 - 18;
        // 延迟 3000 毫秒（3 秒）
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        // 将眼睛移动到新的目标位置
        to_any_position(EYE_GAP + x, 0 + y, -EYE_GAP + x, 0 + y);
    }
}


void Sh1106Display::SetEmotion(const std::string& emotion) {
    static const std::pair<const char*, Emotion> map[] = {
        {"neutral", EMOJI_HAPPY}, {"happy", EMOJI_HAPPY}, {"laughing", EMOJI_EXCITED},
        {"funny", EMOJI_EXCITED}, {"sad", EMOJI_SAD}, {"angry", EMOJI_EXCITED},
        {"crying", EMOJI_SAD}, {"loving", EMOJI_LOVE}, {"thinking", EMOJI_THINKING},
        {"sleepy", EMOJI_SQUINT}, {"confused", EMOJI_SONFUSED}, {"surprised", EMOJI_EXCITED},
        {"speaking", EMOJI_SPEAKING}
    };
    Emotion selected = EMOJI_HAPPY;
    for (const auto& item : map) {
        if (emotion == item.first) { selected = item.second; break; }
    }
    SetEmotion(selected);
}

void Sh1106Display::SetEmotion(Emotion emotion) {
    if (!flag_iic) return;
    DisplayLockGuard lock(this);
    const EmotionInfo* info = get_emotion_info(emotion);
    if (!info || !content_) return;
    if (gif_img_) lv_obj_add_flag(gif_img_, LV_OBJ_FLAG_HIDDEN);
    if (square1) lv_obj_add_flag(square1, LV_OBJ_FLAG_HIDDEN);
    if (square2) lv_obj_add_flag(square2, LV_OBJ_FLAG_HIDDEN);
    if (container_) lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    if (!emotion_canvas_) {
        emotion_canvas_ = lv_canvas_create(content_);
        static uint8_t canvas_buf[LV_CANVAS_BUF_SIZE_INDEXED_1BIT(128, 48)];
        lv_canvas_set_buffer(emotion_canvas_, canvas_buf, 128, 48, LV_IMG_CF_INDEXED_1BIT);
        lv_canvas_set_palette(emotion_canvas_, 0, lv_color_hex(0x000000));
        lv_canvas_set_palette(emotion_canvas_, 1, lv_color_hex(0xFFFFFF));
        lv_obj_set_size(emotion_canvas_, 128, 48);
        lv_obj_center(emotion_canvas_);
    } else {
        lv_obj_clear_flag(emotion_canvas_, LV_OBJ_FLAG_HIDDEN);
    }
    lv_canvas_fill_bg(emotion_canvas_, lv_color_hex(0), LV_OPA_COVER);
    const uint16_t stride = (info->width + 7) / 8;
    for (uint16_t y = 0; y < 48; ++y) {
        for (uint16_t x = 0; x < 128; ++x) {
            const uint16_t sx = (x * info->width) / 128;
            const uint16_t sy = (y * info->height) / 48;
            const uint32_t idx = sy * stride + sx / 8;
            if (idx < info->size && ((info->data[idx] >> (7 - (sx % 8))) & 1)) {
                lv_canvas_set_px_color(emotion_canvas_, x, y, lv_color_hex(0xFF));
            }
        }
    }
    emotion_mode_active_ = true;
    gif_mode_active_ = false;
}

void Sh1106Display::SetEmotionForState(DeviceState state) {
    SetEmotion(get_emotion_for_state(state));
}

void Sh1106Display::ClearEmotion() {
    DisplayLockGuard lock(this);
    if (emotion_canvas_) lv_obj_add_flag(emotion_canvas_, LV_OBJ_FLAG_HIDDEN);
    if (emotion_label_) lv_obj_clear_flag(emotion_label_, LV_OBJ_FLAG_HIDDEN);
    emotion_mode_active_ = false;
}

void Sh1106Display::ShowIdleGif() {
    if (!flag_iic) return;
    DisplayLockGuard lock(this);
    if (emotion_canvas_) lv_obj_add_flag(emotion_canvas_, LV_OBJ_FLAG_HIDDEN);
    if (square1) lv_obj_add_flag(square1, LV_OBJ_FLAG_HIDDEN);
    if (square2) lv_obj_add_flag(square2, LV_OBJ_FLAG_HIDDEN);
    if (container_) lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    auto screen = lv_disp_get_scr_act(disp_);
    if (!gif_img_) {
        gif_img_ = lv_gif_create(screen);
        lv_obj_set_size(gif_img_, 128, 64);
        lv_obj_center(gif_img_);
        lv_gif_set_src(gif_img_, &lazy_blink_gif);
    } else {
        lv_obj_clear_flag(gif_img_, LV_OBJ_FLAG_HIDDEN);
        lv_gif_restart(gif_img_);
    }
    gif_mode_active_ = true;
    emotion_mode_active_ = false;
}

void Sh1106Display::HideIdleGif() {
    if (!flag_iic) return;
    DisplayLockGuard lock(this);
    if (gif_img_) lv_obj_add_flag(gif_img_, LV_OBJ_FLAG_HIDDEN);
    if (container_) lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
    gif_mode_active_ = false;
}

void Sh1106Display::start_emtion()
{
    HideIdleGif();
    if (flag_iic == 0)
    {
        return;
    }
    open_eyes();
    xTaskCreate([](void* arg)
    {
        auto this_ = (Sh1106Display*)arg;
        this_->blink_task();
        vTaskDelete(NULL);
    }, "blink_task", 2048, this, 5, &blink_task_handle);

    xTaskCreate([](void* arg)
    {
        auto this_ = (Sh1106Display*)arg;
        this_->eye_move_emtion_task();
        vTaskDelete(NULL);
    }, "eye_move_emtion_task", 3096, this, 5, &idle_task_handle);

    lv_obj_clear_flag(square1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(square2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void Sh1106Display::stop_emtion()
{
    HideIdleGif();
    if (flag_iic == 0)
    {
        return;
    }
    if (blink_task_handle != NULL)
    {
        vTaskDelete(blink_task_handle);
    }
    if(idle_task_handle != NULL)
    {
        vTaskDelete(idle_task_handle);
    }
    
    lv_obj_add_flag(square1, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(square2, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(container_, LV_OBJ_FLAG_HIDDEN);
}

void Sh1106Display::idle_emtion()
{
    ShowIdleGif();
    return;
    if (flag_iic == 0)
    {
        return;
    }
    if (blink_task_handle != NULL)
    {
        vTaskDelete(blink_task_handle);
    }
    if(idle_task_handle != NULL)
    {
        vTaskDelete(idle_task_handle);
    }
    if (square1 != NULL)
    {
        lv_obj_clear_flag(square1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(square2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(container_, LV_OBJ_FLAG_HIDDEN);
    }

    to_any_position(EYE_GAP,0,-EYE_GAP,0);
    close_eyes();
}
