#include "wifi_board.h"
#include "audio_codecs/no_audio_codec.h"
#include "display/lcd_display.h"
#include "system_reset.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "power_save_timer.h"
#include "iot/thing_manager.h"
#include "led/single_led.h"
#include "assets/lang_config.h"
#include "power_manager.h"
#include "settings.h"
#include <esp_log.h>
#include <esp_lcd_panel_vendor.h>
#include <wifi_station.h>
#include "dual_network_board.h"
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#include "sdcard_manager.h"
#include "led/circular_strip.h"
#include "led_strip_control.h"

#ifdef CONFIG_ENABLE_GLASS
#include "smart_glass.h"
#endif

#ifdef CONFIG_ENABLE_SERVO
#include "lightning_dog.h"
#endif

#define TAG "AiSpeakerDualBoard"

LV_FONT_DECLARE(font_puhui_20_4);
LV_FONT_DECLARE(font_awesome_20_4);


class AiSpeakerDualBoard : public DualNetworkBoard {
private:
    Button boot_button_;
    Button asr_button_;
    Button volume_up_button_;
    Button volume_down_button_;
    SpiLcdDisplay* display_;
    PowerSaveTimer* power_save_timer_;
    PowerManager* power_manager_;
    esp_lcd_panel_io_handle_t panel_io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    CircularStrip* led_strip_;

    void InitializePowerManager() {
        power_manager_ = new PowerManager(CHG_STA_PIN);
        power_manager_->OnChargingStatusChanged([this](bool is_charging) {
            if (is_charging) {
                power_save_timer_->SetEnabled(false);
            } else {
                power_save_timer_->SetEnabled(true);
            }
        });
    }

    void InitializePowerSaveTimer() {
        // rtc_gpio_init(CHG_CTRL_PIN);
        // rtc_gpio_set_direction(CHG_CTRL_PIN, RTC_GPIO_MODE_OUTPUT_ONLY);
        // rtc_gpio_set_level(CHG_CTRL_PIN, 1);

        power_save_timer_ = new PowerSaveTimer(-1, 60, 300);
        power_save_timer_->OnEnterSleepMode([this]() {
            ESP_LOGI(TAG, "Enabling sleep mode");
            display_->SetChatMessage("system", "");
            display_->SetEmotion("sleepy");
            GetBacklight()->SetBrightness(1);
            auto codec = GetAudioCodec();
            codec->EnableInput(false);
        });
        power_save_timer_->OnExitSleepMode([this]() {
            display_->SetChatMessage("system", "");
            display_->SetEmotion("neutral");
            GetBacklight()->RestoreBrightness();
            auto codec = GetAudioCodec();
            codec->EnableInput(true);
        });
        power_save_timer_->OnShutdownRequest([this]() {
            ESP_LOGI(TAG, "Shutting down");
            // rtc_gpio_set_level(CHG_CTRL_PIN, 0);
            // // 启用保持功能，确保睡眠期间电平不变
            // rtc_gpio_hold_en(CHG_CTRL_PIN);
            esp_lcd_panel_disp_on_off(panel_, false); //关闭显示
            esp_deep_sleep_start();
        });
        power_save_timer_->SetEnabled(true);
    }

    void InitializeSpi() {
        spi_bus_config_t buscfg = {};
        buscfg.mosi_io_num = DISPLAY_SDA;
        buscfg.miso_io_num = GPIO_NUM_NC;
        buscfg.sclk_io_num = DISPLAY_SCL;
        buscfg.quadwp_io_num = GPIO_NUM_NC;
        buscfg.quadhd_io_num = GPIO_NUM_NC;
        buscfg.max_transfer_sz = DISPLAY_WIDTH * DISPLAY_HEIGHT * sizeof(uint16_t);
        ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, SPI_DMA_CH_AUTO));
    }

    void InitializeButtons() {
        
        boot_button_.OnClick([this]() {
            power_save_timer_->WakeUp();
            auto& app = Application::GetInstance();
            app.ToggleChatState();
        });

        asr_button_.OnClick([this]() {
            power_save_timer_->WakeUp();
            // auto& app = Application::GetInstance();
            // app.ToggleChatState();
            std::string wake_word="你好小智";
            Application::GetInstance().WakeWordInvoke(wake_word);
        });

        asr_button_.OnDoubleClick([this]() {
            power_save_timer_->WakeUp();
            auto& app = Application::GetInstance();
            if (GetNetworkType() == NetworkType::WIFI) {
                if (app.GetDeviceState() == kDeviceStateStarting && !WifiStation::GetInstance().IsConnected()) {
                    // cast to WifiBoard
                    auto& wifi_board = static_cast<WifiBoard&>(GetCurrentBoard());
                    wifi_board.ResetWifiConfiguration();
                }
            }
            app.ToggleChatState();
        });

        asr_button_.OnMultipleClick([this]() {
            auto& app = Application::GetInstance();
            app.Reboot();
        });
        
        asr_button_.OnLongPress([this]() {
            SwitchNetType();
            auto& app = Application::GetInstance();
            app.Reboot();
        });
        
        volume_up_button_.OnClick([this]() {
            power_save_timer_->WakeUp();
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() + 10;
            if (volume > 100) {
                volume = 100;
            }
            codec->SetOutputVolume(volume);
            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        volume_up_button_.OnLongPress([this]() {
            power_save_timer_->WakeUp();
            GetAudioCodec()->SetOutputVolume(100);
            GetDisplay()->ShowNotification(Lang::Strings::MAX_VOLUME);
        });

        volume_down_button_.OnClick([this]() {
            power_save_timer_->WakeUp();
            auto codec = GetAudioCodec();
            auto volume = codec->output_volume() - 10;
            if (volume < 0) {
                volume = 0;
            }
            codec->SetOutputVolume(volume);
            GetDisplay()->ShowNotification(Lang::Strings::VOLUME + std::to_string(volume));
        });

        volume_down_button_.OnLongPress([this]() {
            power_save_timer_->WakeUp();
            GetAudioCodec()->SetOutputVolume(0);
            GetDisplay()->ShowNotification(Lang::Strings::MUTED);
        });
    }

    void InitializeSt7789Display() {
        ESP_LOGD(TAG, "Install panel IO");
        esp_lcd_panel_io_spi_config_t io_config = {};
        io_config.cs_gpio_num = DISPLAY_CS;
        io_config.dc_gpio_num = DISPLAY_DC;
        io_config.spi_mode = 3;
        io_config.pclk_hz = 80 * 1000 * 1000;
        io_config.trans_queue_depth = 10;
        io_config.lcd_cmd_bits = 8;
        io_config.lcd_param_bits = 8;
        ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI3_HOST, &io_config, &panel_io_));

        ESP_LOGD(TAG, "Install LCD driver");
        esp_lcd_panel_dev_config_t panel_config = {};
        panel_config.reset_gpio_num = DISPLAY_RES;
        panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
        panel_config.bits_per_pixel = 16;
        ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io_, &panel_config, &panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_init(panel_));
        ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_, DISPLAY_SWAP_XY));
        ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_, true));

        display_ = new SpiLcdDisplay(panel_io_, panel_, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_OFFSET_X, DISPLAY_OFFSET_Y, 
            DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY, 
        {
            .text_font = &font_puhui_20_4,
            .icon_font = &font_awesome_20_4,
            .emoji_font = DISPLAY_HEIGHT >= 240 ? font_emoji_64_init() : font_emoji_32_init(),
        });
    }

    void InitializeIot() {
        auto& thing_manager = iot::ThingManager::GetInstance();
        thing_manager.AddThing(iot::CreateThing("Speaker"));
        thing_manager.AddThing(iot::CreateThing("Screen"));
        thing_manager.AddThing(iot::CreateThing("Battery"));
        led_strip_ = new CircularStrip(BUILTIN_LED_GPIO, 4);
        auto led_strip_control = new LedStripControl(led_strip_);
        thing_manager.AddThing(led_strip_control);
        
#ifdef CONFIG_ENABLE_SERVO
        auto servo_control = new ServoControl(LEDC_SPEED_MODE, LEDC_TIMER, LEDC_FREQUENCY, LEDC_RESOLUTION,
            LEDC_CHANNEL1, LEDC_CHANNEL2, LEDC_CHANNEL3, LEDC_CHANNEL4,
            SERVO1_PIN, SERVO2_PIN, SERVO3_PIN, SERVO4_PIN);
        auto lightningDog = new LightningDog(servo_control);
        thing_manager.AddThing(lightningDog);
#endif

#ifdef CONFIG_ENABLE_GLASS
        auto smart_glass_control = new SmartGlass(
        ECHO_UART_PORT_NUM,
        UART_ECHO_TXD,
        UART_ECHO_RXD,
        UART_ECHO_RTS,
        UART_ECHO_CTS,
        ECHO_UART_BAUD_RATE,
        BUF_SIZE);
        thing_manager.AddThing(smart_glass_control);
#endif
    }

#ifdef CONFIG_ENABLE_SD_CARD
    void InitializeSdCard() {
        auto sd_card_manager = new SdCardManager(PIN_NUM_MOSI, PIN_NUM_MISO, PIN_NUM_CLK, PIN_NUM_CS , MOUNT_POINT);
        esp_err_t ret = sd_card_manager->Init();
        int sd_card_status = 0;
        if (ret == ESP_OK) {
            sd_card_status = 1;
            ESP_LOGI(TAG, "SDCard 初始化成功");
        } else {
            sd_card_status = 0;
            ESP_LOGE(TAG, "SDCard 初始化失败: %s", esp_err_to_name(ret));
        }
        // 保存设置
        Settings settings("sc_card", true);
        settings.SetInt("enable", sd_card_status);
    }
#endif
//DISPLAY_HEIGHT >= 240 ? font_emoji_64_init() : font_emoji_32_init()
public:
    AiSpeakerDualBoard() :
        DualNetworkBoard(ML307_TX_PIN, ML307_RX_PIN, ENABLE_4G_BUF_SIZE, (ENABLE_4G_MODULE ? 1 : 0)),
        boot_button_(BOOT_BUTTON_GPIO),
        asr_button_(ASR_BUTTON_GPIO),    
        volume_up_button_(VOLUME_UP_BUTTON_GPIO),
        volume_down_button_(VOLUME_DOWN_BUTTON_GPIO) {
#if CONFIG_ENABLE_SD_CARD
        InitializeSdCard();
#endif
        InitializePowerManager();
        InitializePowerSaveTimer();
        InitializeSpi();
        InitializeButtons();
        InitializeSt7789Display();  
        InitializeIot();
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            GetBacklight()->RestoreBrightness();
        }
    }
    
    virtual Led* GetLed() override {
        return led_strip_;
    }

    virtual AudioCodec* GetAudioCodec() override {
        static NoAudioCodecSimplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_SPK_GPIO_BCLK, AUDIO_I2S_SPK_GPIO_LRCK, AUDIO_I2S_SPK_GPIO_DOUT, AUDIO_I2S_MIC_GPIO_SCK, AUDIO_I2S_MIC_GPIO_WS, AUDIO_I2S_MIC_GPIO_DIN);
        return &audio_codec;
    }

    virtual Display* GetDisplay() override {
        return display_;
    }
    
    virtual Backlight* GetBacklight() override {
        if (DISPLAY_BACKLIGHT_PIN != GPIO_NUM_NC) {
            static PwmBacklight backlight(DISPLAY_BACKLIGHT_PIN, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
            return &backlight;
        }
        return nullptr;
    }

    virtual bool GetBatteryLevel(int& level, bool& charging, bool& discharging) override {
        static bool last_discharging = false;
        charging = power_manager_->IsCharging();
        discharging = power_manager_->IsDischarging();
        if (discharging != last_discharging) {
            power_save_timer_->SetEnabled(discharging);
            last_discharging = discharging;
        }
        level = power_manager_->GetBatteryLevel();
        return true;
    }

    virtual void SetPowerSaveMode(bool enabled) override {
        if (!enabled) {
            power_save_timer_->WakeUp();
        }
        DualNetworkBoard::SetPowerSaveMode(enabled);
    }
};

DECLARE_BOARD(AiSpeakerDualBoard);
