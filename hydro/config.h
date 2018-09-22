#ifndef _CONFIG_H_
#define _CONFIG_H_

#define USE_WIRINGPI_PINS 1

#define DHT22_RETRIES 3
#define DHT22_MAX_TIMINGS 85

#if USE_WIRINGPI_PINS == 1
#define PIN_RESERVOIR_RELAY 0
#define PIN_LIGHT_RELAY 3
#define PIN_DHT22 2
#else
#define PIN_RESERVOIR_RELAY 17
#define PIN_LIGHT_RELAY 22
#define PIN_DHT22 27
#endif

#endif
