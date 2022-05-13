#include <libpax_api.h>
#include <stdio.h>
#include <Arduino.h>

struct count_payload_t count_from_libpax;

void process_count(void) {
  printf("pax: %d;\n", count_from_libpax.ble_count);
}

void init() {
  struct libpax_config_t configuration; 
  //configuration.ble_rssi_threshold = 0;
  configuration.blescaninterval = 80;
  configuration.blescanwindow = 80;
  configuration.blecounter = 1;
  configuration.blescantime = 0; // infinite
  configuration.ble_rssi_threshold = -80;

  disable_wifi();

  //change clock speed
  setCpuFrequencyMhz(10);

  // internal processing initialization
  libpax_counter_init(process_count, &count_from_libpax, 10, 1); 
  libpax_counter_start(configuration);
}

#ifdef LIBPAX_ARDUINO
// arduino code
void setup() {
  init();
}

void loop() {}
#endif

#ifdef LIBPAX_ESPIDF
// espidf code
extern "C" void app_main();
void app_main() {
  init();
}
#endif
