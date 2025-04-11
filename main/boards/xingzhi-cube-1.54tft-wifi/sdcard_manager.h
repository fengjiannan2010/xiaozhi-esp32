#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include "esp_err.h"

class SdCardManager {
private:
    const char *mount_point_ = "/sdcard";
    sdmmc_card_t *card_ = nullptr;

    // 配置对象
    spi_bus_config_t spi_bus_config_;
    sdspi_device_config_t sdspi_device_config_;
    esp_vfs_fat_sdmmc_mount_config_t mount_config_;
    sdmmc_host_t host_ = SDSPI_HOST_DEFAULT();

    // GPIO 引脚
    int pin_mosi_;
    int pin_miso_;
    int pin_clk_;
    int pin_cs_;

    // 初始化配置
    void InitSpiBusConfig();
    void InitSlotConfig();
    void InitMountConfig();

public:
    // 构造函数，传入 GPIO 引脚
    SdCardManager(int pin_mosi, int pin_miso, int pin_clk, int pin_cs);
    ~SdCardManager();

    // 初始化 SD 卡
    esp_err_t Init();

    // 卸载 SD 卡
    void Unmount();

    // 写入文件
    esp_err_t WriteFile(const char *path, const char *data);

    // 读取文件
    esp_err_t ReadFile(const char *path, char *buffer, size_t buffer_size);
};

#endif // SDCARD_MANAGER_H