#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/spi_common.h"
#include "driver/sdspi_host.h"
#include <esp_log.h>
#include <esp_err.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "lvgl.h"

class SdCardManager {
private:
    int pin_mosi_;
    int pin_miso_;
    int pin_clk_;
    int pin_cs_;
    const char* mount_point_;  // 修改为 const char*

    spi_bus_config_t spi_bus_config_;
    sdspi_device_config_t sdspi_device_config_;
    esp_vfs_fat_sdmmc_mount_config_t mount_config_;
    sdmmc_host_t host_ = SDSPI_HOST_DEFAULT();
    sdmmc_card_t* card_ = nullptr;

    void InitSpiBusConfig();
    void InitSlotConfig();
    void InitMountConfig();

public:
    SdCardManager(int pin_mosi, int pin_miso, int pin_clk, int pin_cs, const char* mount_point);  // 修改参数类型
    esp_err_t Init();
    void Unmount();
    esp_err_t WriteFile(const char* path, const char* data);
    esp_err_t ReadFile(const char* path, char* buffer, size_t buffer_size);
    void RegisterLvglFilesystem();
    void ListDir(const char* path);
};

#endif  // SDCARD_MANAGER_H