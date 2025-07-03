#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_app_desc.h"

void app_main(void) {
    const esp_app_desc_t* app_desc = esp_app_get_description();
    ESP_LOGI("HelloWorld", "Hello world!");
    printf("ELF file SHA256: %s\n", app_desc->app_elf_sha256);
    printf("Hello world!\n");
    
}
