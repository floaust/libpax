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
#include "globals.h"

#include <Arduino.h>
#include <libpax_api.h>
#include <string.h>
#include "esp_log.h"
#include "libpax.h"

typedef uint32_t bitmap_t;
enum { BITS_PER_WORD = sizeof(bitmap_t) * CHAR_BIT };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b) ((b) % BITS_PER_WORD)
#define LIBPAX_MAX_SIZE 0xFFFF  // full enumeration of uint16_t
#define LIBPAX_MAP_SIZE (LIBPAX_MAX_SIZE / BITS_PER_WORD)

DRAM_ATTR bitmap_t seen_ids_map[LIBPAX_MAP_SIZE];
int seen_ids_count = 0;

uint16_t volatile macs_ble = 0;

uint8_t volatile channel = 0;  // channel rotation counter

IRAM_ATTR void set_id(bitmap_t *bitmap, uint16_t id) {
  bitmap[WORD_OFFSET(id)] |= ((bitmap_t)1 << BIT_OFFSET(id));
}

IRAM_ATTR int get_id(bitmap_t *bitmap, uint16_t id) {
  bitmap_t bit = bitmap[WORD_OFFSET(id)] & ((bitmap_t)1 << BIT_OFFSET(id));
  return bit != 0;
}

/** remember given id
 * returns 1 if id is new, 0 if already seen this is since last reset
 */
IRAM_ATTR int add_to_bucket(uint16_t id) {
  if (get_id(seen_ids_map, id)) {
    return 0;  // already seen
  } else {
    set_id(seen_ids_map, id);
    seen_ids_count++;
    return 1;  // new
  }
}

// resets the counter
void reset_bucket() {
  memset(seen_ids_map, 0, sizeof(seen_ids_map));
  seen_ids_count = 0;
}

// returns the number of unique mac addresses seen
int libpax_ble_counter_count() { return macs_ble; }

// adds the given mac address to the counter if it is new
IRAM_ATTR int mac_add(uint8_t *paddr) {
  uint16_t *id;
  // mac addresses are 6 bytes long, we only use the last two bytes
  id = (uint16_t *)(paddr + 4);

  int added = add_to_bucket(*id);

  // Count only if MAC was not yet seen
  if (added) {
    macs_ble++;
    ESP_LOGI(TAG, "MAC=%02x:%02x:%02x:%02x:%02x:%02x -> ID=%04x", paddr[0],
             paddr[1], paddr[2], paddr[3], paddr[4], paddr[5], *id);
    // added
  }

  return added;  // function returns bool if a new and unique BLE mac
                 // was counted (true) or not (false)
}
