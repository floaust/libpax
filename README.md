# LibPAX

A library to estimate passenger (PAX) count based on Bluetooth signals based on the ESP32 chip.
This is a library meant to be used for enabling pax-counting in other projects.
For an application facilitating pax-counting, see [ESP32-Paxcounter](https://github.com/cyberman54/ESP32-Paxcounter).
The library was extracted from the ESP32-Paxcounter project to enhance reusability.

## Usage

For using the library please refer to `libpax_api.h`.
All functions outside the `libpax_api.h` are considered internal and might change between releases.

### Compile time options

You must define one of the following compile time constants to select the framework you are working with:
```
LIBPAX_ARDUINO
LIBPAX_ESPIDF
```

The number of remembered devices is limited to a maximum of 65535, requiring 8 KBytes of RAM.
