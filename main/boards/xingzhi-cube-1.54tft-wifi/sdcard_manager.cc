#include "sdcard_manager.h"
#include "config.h"

const char *TAG = "SdCardManager";

SdCardManager::SdCardManager(int pin_mosi, int pin_miso, int pin_clk, int pin_cs)
    : pin_mosi_(pin_mosi), pin_miso_(pin_miso), pin_clk_(pin_clk), pin_cs_(pin_cs) {
    ESP_LOGI(TAG, "SdCardManager 构造函数调用");
}

SdCardManager::~SdCardManager() {
    Unmount();
    ESP_LOGI(TAG, "SdCardManager 析构函数调用");
}

void SdCardManager::InitSpiBusConfig() {
    spi_bus_config_ = {
        .mosi_io_num = pin_mosi_,
        .miso_io_num = pin_miso_,
        .sclk_io_num = pin_clk_,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
}

void SdCardManager::InitSlotConfig() {
    sdspi_device_config_ = SDSPI_DEVICE_CONFIG_DEFAULT();
    sdspi_device_config_.gpio_cs = (gpio_num_t)pin_cs_;
    sdspi_device_config_.host_id = (spi_host_device_t)host_.slot;
}

void SdCardManager::InitMountConfig() {
    mount_config_ = {
        .format_if_mount_failed = false,
        .max_files = 10,
        .allocation_unit_size = 16 * 1024,
    };
}

esp_err_t SdCardManager::Init() {
    ESP_LOGI(TAG, "初始化 SD 卡");

    // 初始化配置
    InitSpiBusConfig();
    InitSlotConfig();
    InitMountConfig();

    // 初始化 SPI 总线
    esp_err_t ret = spi_bus_initialize((spi_host_device_t)host_.slot, &spi_bus_config_, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "初始化 SPI 总线失败: %s", esp_err_to_name(ret));
        return ret;
    }

    // 挂载文件系统
    ret = esp_vfs_fat_sdspi_mount(mount_point_, &host_, &sdspi_device_config_, &mount_config_, &card_);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "挂载 SD 卡失败: %s", esp_err_to_name(ret));
        spi_bus_free((spi_host_device_t)host_.slot); // 释放 SPI 总线
        return ret;
    }

    ESP_LOGI(TAG, "文件系统挂载成功");
    sdmmc_card_print_info(stdout, card_);
    return ESP_OK;
}

void SdCardManager::Unmount() {
    if (card_) {
        esp_vfs_fat_sdcard_unmount(mount_point_, card_);
        spi_bus_free((spi_host_device_t)host_.slot);
        card_ = nullptr;
        ESP_LOGI(TAG, "SD 卡已卸载");
    }
}

esp_err_t SdCardManager::WriteFile(const char *path, const char *data) {
    ESP_LOGI(TAG, "写入文件: %s", path);
    FILE *f = fopen(path, "w");
    if (!f) {
        ESP_LOGE(TAG, "无法打开文件: %s", path);
        return ESP_FAIL;
    }
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(TAG, "文件写入成功");
    return ESP_OK;
}

esp_err_t SdCardManager::ReadFile(const char *path, char *buffer, size_t buffer_size) {
    ESP_LOGI(TAG, "读取文件: %s", path);
    FILE *f = fopen(path, "r");
    if (!f) {
        ESP_LOGE(TAG, "无法打开文件: %s", path);
        return ESP_FAIL;
    }
    fgets(buffer, buffer_size, f);
    fclose(f);

    char *pos = strchr(buffer, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "读取内容: '%s'", buffer);
    return ESP_OK;
}