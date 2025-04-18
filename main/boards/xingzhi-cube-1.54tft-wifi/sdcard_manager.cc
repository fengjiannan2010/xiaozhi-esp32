#include "sdcard_manager.h"
#include "config.h"
#include <dirent.h>

const char *TAG = "SdCardManager";



SdCardManager::SdCardManager(int pin_mosi, int pin_miso, int pin_clk, int pin_cs)
    : pin_mosi_(pin_mosi), pin_miso_(pin_miso), pin_clk_(pin_clk), pin_cs_(pin_cs){
    ESP_LOGI(TAG, "SdCardManager 构造函数调用");
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
        .max_files = 100,
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

    RegisterLvglFilesystem();
    
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

void SdCardManager::RegisterLvglFilesystem() {
    static lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);

    drv.letter = 'S';  // 使用 "S:" 作为盘符
    drv.cache_size = 0; // 不使用内部缓存（你也可以设置缓存大小）
    drv.user_data = nullptr;

    // 文件操作函数
    drv.open_cb = [](lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode) -> void* {
        const char *flags = (mode == LV_FS_MODE_WR) ? "wb" :
                            (mode == LV_FS_MODE_RD) ? "rb" : "rb+";

        char full_path[256];
        snprintf(full_path, sizeof(full_path), "/sdcard/%s", path);
        FILE *f = fopen(full_path, flags);
        return f;
    };

    drv.close_cb = [](lv_fs_drv_t *drv, void *file_p) -> lv_fs_res_t {
        fclose((FILE *)file_p);
        return LV_FS_RES_OK;
    };

    drv.read_cb = [](lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br) -> lv_fs_res_t {
        *br = fread(buf, 1, btr, (FILE *)file_p);
        return LV_FS_RES_OK;
    };

    drv.write_cb = [](lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw) -> lv_fs_res_t {
        *bw = fwrite(buf, 1, btw, (FILE *)file_p);
        return LV_FS_RES_OK;
    };

    drv.seek_cb = [](lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence) -> lv_fs_res_t {
        int origin = (whence == LV_FS_SEEK_SET) ? SEEK_SET :
                     (whence == LV_FS_SEEK_CUR) ? SEEK_CUR : SEEK_END;
        fseek((FILE *)file_p, pos, origin);
        return LV_FS_RES_OK;
    };

    drv.tell_cb = [](lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p) -> lv_fs_res_t {
        *pos_p = ftell((FILE *)file_p);
        return LV_FS_RES_OK;
    };


    // 目录操作函数（可选）
    drv.dir_open_cb = [](lv_fs_drv_t *drv, const char *path) -> void* {
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "/sdcard/%s", path);
        return opendir(full_path);
    };

    drv.dir_read_cb = [](lv_fs_drv_t *drv, void *rddir_p, char *fn, uint32_t fn_len) -> lv_fs_res_t {
        struct dirent *entry = readdir((DIR *)rddir_p);
        if (entry == nullptr) {
            return LV_FS_RES_NOT_EX;
        }
        strncpy(fn, entry->d_name, fn_len);
        fn[fn_len - 1] = '\0';
        return LV_FS_RES_OK;
    };

    drv.dir_close_cb = [](lv_fs_drv_t *drv, void *rddir_p) -> lv_fs_res_t {
        closedir((DIR *)rddir_p);
        return LV_FS_RES_OK;
    };

    lv_fs_drv_register(&drv);
    ESP_LOGI(TAG, "LVGL v9 文件系统驱动注册成功，盘符: 'S:'");
}

void SdCardManager::ListDir(const char *path) {
    ESP_LOGI(TAG, "列出目录: %s", path);

    DIR *dir = opendir(path);
    if (!dir) {
        ESP_LOGE(TAG, "无法打开目录: %s (错误码: %d)", path, errno);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        const char *type = (entry->d_type == DT_DIR) ? "目录 " :
                           (entry->d_type == DT_REG) ? "文件 " : "其他 ";
        ESP_LOGI(TAG, "%s: %s", type, entry->d_name);
    }

    closedir(dir);
    ESP_LOGI(TAG, "目录读取完毕");
}
// 