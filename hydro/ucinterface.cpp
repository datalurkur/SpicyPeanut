#include "config.h"
#include "log.h"
#include "ucinterface.h"
#include "util.h"

#if !_WINDOWS_BUILD
#include "wiringPi.h"
#include "wiringPiI2C.h"
#endif

#include <algorithm>
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
    _nextDHT22Sample = SampleCurrentTime();

    _pHProbeFD = wiringPiI2CSetup(PH_PROBE_CHANNEL);
    _ecProbeFD = wiringPiI2CSetup(EC_PROBE_CHANNEL);
#endif

    _pHProbeReady = isPHProbeCalibrated();
    _ecProbeReady = isECProbeCalibrated();
}

UCInterface::~UCInterface()
{

}

bool UCInterface::sampleDHT22()
{
    bool succeeded = false;

    // Wait until 2 seconds have elapsed since the last sample
    std::chrono::system_clock::time_point now = SampleCurrentTime();
    long long msToWait = std::max(0LL, std::chrono::duration_cast<std::chrono::milliseconds>(_nextDHT22Sample - now).count());
    if (msToWait > 0)
    {
        LogDebug("Waiting " << msToWait << "ms to sample DHT22");
    }

#if !_WINDOWS_BUILD
    if (msToWait > 0)
    {
        delay(msToWait);
    }

    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0;
    uint8_t i;

    uint8_t data[5];
    for (i = 0; i < 5; ++i) { data[i] = 0; }

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
        LogInfo("Successfully sampled DHT22 (" << _lastTemperature << " degrees Fahrenheit, " << _lastHumidity << "% humidity)");
    }
    else
    {
        LogDebug("Failed to sample DHT22, bad data");
    }
#endif

    _nextDHT22Sample = SampleCurrentTime() + std::chrono::seconds(2);
    return succeeded;
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

bool UCInterface::getReservoirState()
{
    bool ret = false;
#if _WINDOWS_BUILD
    ret = true;
#else
    ret = (digitalRead(PIN_RESERVOIR_RELAY) == HIGH);
#endif
    LogInfo("Current reservoir state is " << (ret ? "flooded" : "drained"));
    return ret;
}

void UCInterface::setLightState(bool on)
{
#if !_WINDOWS_BUILD
    digitalWrite(PIN_LIGHT_RELAY, on ? HIGH : LOW);
#endif
}

bool UCInterface::getLightState()
{
    bool ret = false;
#if _WINDOWS_BUILD
    ret = true;
#else
    ret = (digitalRead(PIN_LIGHT_RELAY) == HIGH);
#endif
    LogInfo("Current light state is " << (ret ? "on" : "off"));
    return ret;
}

void UCInterface::resetProbes()
{
    _pHProbeReady = isPHProbeCalibrated();
    _ecProbeReady = isECProbeCalibrated();
}

bool UCInterface::isPHProbeCalibrated()
{
    writeI2CCommand(_pHProbeFD, "Cal,?");
#if !_WINDOWS_BUILD
    delay(300);
#endif
    std::string response;
    if (!getResponseString(_pHProbeFD, response))
    {
        LogError("Failed to get pH probe calibration state");
        return false;
    }
    LogInfo("pH probe calibration state: " << response);
    bool calibrated = (response.length() == 6 && response[5] != 0);
    LogInfo("pH probe is " << (calibrated ? "" : "NOT ") << "calibrated");
    return calibrated;
}

void UCInterface::setPHCalibrationPoint(CalibrationPoint cPoint)
{
    std::string command = "Cal,";
    switch (cPoint)
    {
    case CalibrationPoint::Mid:
        command.append("mid,7.00");
        break;
    case CalibrationPoint::Low:
        command.append("low,4.00");
        break;
    case CalibrationPoint::High:
        command.append("high,10.00");
        break;
    }
    writeI2CCommand(_pHProbeFD, command);
#if !_WINDOWS_BUILD
    delay(900);
#endif
    std::string response;
    if (!getResponseString(_pHProbeFD, response))
    {
        LogError("Failed to set calibration point");
    }
}

bool UCInterface::getPH(float& pHValue)
{
    if (_pHProbeReady)
    {
        LogWarn("pH probe is not calibrated");
        return false;
    }

    writeI2CCommand(_pHProbeFD, "R");
#if !_WINDOWS_BUILD
    delay(900);
#endif
    std::string response;
    if (!getResponseString(_pHProbeFD, response))
    {
        LogError("Failed to sample pH");
        return false;
    }

    pHValue = (float)::atof(response.c_str());
    LogInfo("Current pH value is " << pHValue);
    return true;
}

bool UCInterface::isECProbeCalibrated()
{
    writeI2CCommand(_ecProbeFD, "Cal,?");
#if !_WINDOWS_BUILD
    delay(300);
#endif
    std::string response;
    if (!getResponseString(_ecProbeFD, response))
    {
        LogError("Failed to get EC probe calibration state");
        return false;
    }
    LogInfo("EC probe calibration state: " << response);
    bool calibrated = (response.length() == 6 && response[5] != 0);
    LogInfo("EC probe is " << (calibrated ? "" : "NOT ") << "calibrated");
    return calibrated;
}

void UCInterface::setECCalibrationPoint(CalibrationPoint cPoint)
{
    std::string command = "Cal,";
    switch (cPoint)
    {
    case CalibrationPoint::Dry:
        command.append("dry");
        break;
    case CalibrationPoint::Low:
        command.append("low,12880");
        break;
    case CalibrationPoint::High:
        command.append("high,80000");
        break;
    }
    writeI2CCommand(_pHProbeFD, command);
#if !_WINDOWS_BUILD
    delay(600);
#endif
    std::string response;
    if (!getResponseString(_pHProbeFD, response))
    {
        LogError("Failed to set calibration point");
    }
}

bool UCInterface::getEC(float& ecValue)
{
    if (!_ecProbeReady)
    {
        LogWarn("EC probe is not calibrated");
        return false;
    }
    writeI2CCommand(_ecProbeFD, "R");
#if !_WINDOWS_BUILD
    delay(600);
#endif
    std::string response;
    if (!getResponseString(_ecProbeFD, response))
    {
        LogError("Failed to sample EC");
        return false;
    }

    ecValue = (float)::atof(response.c_str());
    LogInfo("Current EC value is " << ecValue);
    return true;
}

bool UCInterface::getResponseString(int fd, std::string& response)
{
    char buffer[255];
    int responseLength = readI2CResponse(fd, buffer, 255);
    if (responseLength == 0)
    {
        LogError("Got empty response string");
        return false;
    }
    if (responseLength == 255)
    {
        LogError("Response length exceeded buffer size");
        return false;
    }
    if (buffer[0] != 1)
    {
        LogError("Failed to get response string");
        return false;
    }
    response = std::string((char*)&buffer[1]);
    return true;
}

void UCInterface::writeI2CCommand(int fd, const std::string& command)
{
    for (unsigned int i = 0; i < command.length(); ++i)
    {
#if !_WINDOWS_BUILD
        wiringPiI2CWrite(fd, command[i]);
#endif
    }
}

unsigned int UCInterface::readI2CResponse(int fd, char* buffer, unsigned int maxLength)
{
    unsigned int i;
    for (i = 0; i < maxLength; ++i)
    {
#if _WINDOWS_BUILD
        buffer[i] = 0;
#else
        buffer[i] = wiringPiI2CRead(fd);
#endif
        if (buffer[i] == 0) { break; }
    }
    if (i == maxLength)
    {
        LogWarn("Max buffer length hit in i2c response");
    }
    return i - 1;
}