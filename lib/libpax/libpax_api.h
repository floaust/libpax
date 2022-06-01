#ifndef _LIBPAX_API_H
#define _LIBPAX_API_H

#include <stdint.h>

// Build-time options
// #define LIBPAX_BLE  // enables BLE sniffing features in build

#define LIBPAX_ERROR_BLE_NOT_AVAILABLE 0b00000010

// configuration given to lib for sniffing parameters
struct libpax_config_t {
  int ble_rssi_threshold;       // Filter for how strong the bluetooth signal should be to be counted
  uint8_t blecounter;           // set to 0 if you do not want to install the BLE sniffer
  uint16_t blescanwindow;       // [milliseconds] scan window, see below, 3 ... 10240, default 80ms
  uint16_t blescaninterval;     // [milliseconds] scan interval, see below, 3 ... 10240, default 80ms = 100% duty cycle
  int ble_scan_count;           // number of scans to be done, 0 means infinite [default]
  void (*ble_callback)(void);   // callback function for executing after stopping BLE counter
};

// payload updated periodically or on demand
struct count_payload_t {
  uint32_t ble_count;  // detected ble_count in interval
};

/**
 *   Must be called before use of the lib.
 *   @param[out] current_count memory for pax count. Updated directly before
 * callback is called
 *   @param[in] pax_report_interval_sec defines interval in s between a pax
 * count callback.
 *   @param[in] countermode avalible modes TBD
 */
int libpax_counter_init(struct count_payload_t* current_count,
                        uint16_t pax_report_interval_sec, int countermode);

/**
 *   Starts hardware wifi layer and counting of pax
 */
int libpax_counter_start(libpax_config_t configuration);

/**
 *   Stops sniffing process after which no new macs will be received and the
 * wifi allocation is shutdown
 */
int libpax_counter_stop();

/**
 *  Optional external counter query outside of given time interval
 *  param[out] count
 */
int libpax_counter_count(struct count_payload_t* count);

/*
 * Size in bytes of a serialized config
 */
#define LIBPAX_CONFIG_SIZE 64

// disables wifi chip
void disable_wifi();
#endif
