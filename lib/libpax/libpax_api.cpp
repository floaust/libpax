/*
LICENSE

Copyright  2020      Deutsche Bahn Station&Service AG

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <Arduino.h>
#include <libpax_api.h>
#include "blescan.h"
#include "libpax.h"

#include <esp_coexist.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_spi_flash.h>  // needed for reading ESP32 chip attributes
#include <esp_wifi.h>
#include <string.h>
#include "freertos/task.h"    // needed for tasks
#include "freertos/timers.h"  // TimerHandle_t

int config_set = 0;

void (*report_callback)(void);
struct count_payload_t* pCurrent_count;
int counter_mode;

void fill_counter(struct count_payload_t* pCount) {
  pCount->ble_count = libpax_ble_counter_count();
}

void libpax_counter_reset() {
  macs_ble = 0;
  reset_bucket();
}

void report(TimerHandle_t xTimer) {
  fill_counter(pCurrent_count);
  report_callback();
  // clear counter if not in cumulative counter mode
  if (counter_mode != 1) {
    libpax_counter_reset();
  }
}

TimerHandle_t PaxReportTimer = NULL;
int libpax_counter_init(void (*init_callback)(void),
                        struct count_payload_t* init_current_count,
                        uint16_t init_pax_report_interval_sec,
                        int init_counter_mode) {
  if (PaxReportTimer != NULL && xTimerIsTimerActive(PaxReportTimer)) {
    ESP_LOGW("initialization", "lib already active. Ignoring new init.");
    return -1;
  }

  report_callback = init_callback;
  pCurrent_count = init_current_count;
  counter_mode = init_counter_mode;

  /* Initialize NVS — it is used to store PHY calibration data. */
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  libpax_counter_reset();

  PaxReportTimer = xTimerCreate(
      "PaxReportTimer", pdMS_TO_TICKS(init_pax_report_interval_sec * 1000),
      pdTRUE, (void*)0, report);
  xTimerStart(PaxReportTimer, 0);
  return 0;
}

int libpax_counter_start(libpax_config_t configuration) {
  if (config_set == 0) {
    ESP_LOGE("configuration", "Configuration was not yet set.");
    return -1;
  }
  if (configuration.blecounter) {
    set_BLE_rssi_filter(configuration.ble_rssi_threshold);
    start_BLE_scan(configuration.blescantime, configuration.blescanwindow,
                   configuration.blescaninterval);
  }
  return 0;
}

int libpax_counter_stop() {
  stop_BLE_scan();
  xTimerStop(PaxReportTimer, 0);
  PaxReportTimer = NULL;
  return 0;
}

int libpax_counter_count(struct count_payload_t* count) {
  fill_counter(count);
  return 0;
}

void disable_wifi() {
  ESP_ERROR_CHECK(
      esp_wifi_set_promiscuous(false));  // now switch off monitor mode
  ESP_ERROR_CHECK(esp_wifi_stop());
  ESP_ERROR_CHECK(esp_wifi_deinit());
  esp_wifi_set_mode(WIFI_MODE_NULL);
}
