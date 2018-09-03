#include "config.h"
#include "log.h"
#include "ucinterface.h"
#include "util.h"

#if !_WINDOWS_BUILD
#include "wiringpi.h"
#endif

#include <stdint.h>

std::shared_ptr<UCInterface> UCInterface::Instance = nullptr;

void UCInterface::Init()
{
    if (Instance != nullptr) { return; }
    Instance = std::make_shared<UCInterface>();
}

UCInterface::UCInterface()
{
#if !_WINDOWS_BUILD
    wiringPiSetup();
    pinMode(PIN_LIGHT_RELAY, OUTPUT);
    pinMode(PIN_RESERVOIR_RELAY, OUTPUT);
    _nextDHT22Sample = GetCurrentTime();
#endif
}

UCInterface::~UCInterface()
{

}

bool UCInterface::sampleDHT22()
{
#if !_WINDOWS_BUILD
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0;
    uint8_t i;

    uint8_t data[5];
    for (i = 0; i < 5; ++i) { data[i] = 0; }

    // Wait until 2 seconds have elapsed since the last sample
    long long msToWait = (_nextDHT22Sample - GetCurrentTime()).count();
    delay(msToWait);

    // Notify DHT22 that we want to read data
    pinMode(PIN_DHT22, OUTPUT);
    digitalWrite(PIN_DHT22, LOW);
    delay(18);

    // Read data from DHT22
    pinMode(PIN_DHT22, INPUT);
    for (i = 0; i < DHT22_MAX_TIMINGS; i++)
    {
        counter = 0;
        while (digitalRead(PIN_DHT22) == laststate)
        {
            counter++;
            delayMicroseconds(1);
            if (counter == 255) { break; }
        }
        laststate = digitalRead(PIN_DHT22);

        if (counter == 255) { break; }

        // Ignore first 3 transitions
        if ((i >= 4) && (i % 2 == 0))
        {
            // Set appropriate bit
            data[j / 8] <<= 1;
            if (counter > 16)
                data[j / 8] |= 1;
            j++;
        }
    }

    // Verify number of bits read and checksum
    bool succeeded = false;
    if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)))
    {
        float h = (float)((data[0] << 8) + data[1]) / 10;
        float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
        if (data[2] & 0x80)
        {
            c = -c;
        }
        _lastTemperatureCelsius = c;
        _lastTemperature = c * 1.8f + 32;
        _lastHumidity = h;
        succeeded = true;
        LogInfo("Successfully sampled DHT22 (" << _lastTemperature << " degrees Fahrenheit, " << _lastHumidity << "% humidity");
    }
    else
    {
        LogInfo("Failed to sample DHT22, bad data");
    }

    _nextDHT22Sample = GetCurrentTime() + std::chrono::seconds(2);
    return succeeded;
#else
    return false;
#endif
}

float UCInterface::getLastTemperature()
{
    return _lastTemperature;
}

float UCInterface::getLastHumidity()
{
    return _lastHumidity;
}

void UCInterface::setReservoirState(bool flooded)
{
#if !_WINDOWS_BUILD
    digitalWrite(PIN_RESERVOIR_RELAY, flooded ? HIGH : LOW);
#endif
}

void UCInterface::setLightState(bool on)
{
#if !_WINDOWS_BUILD
    digitalWrite(PIN_LIGHT_RELAY, on ? HIGH : LOW);
#endif
}