#include "config.h"
#include "device.h"
#include "log.h"
#include "ucinterface.h"
#include "util.h"

#if !_WINDOWS_BUILD
#include "wiringPi.h"
#include "wiringPiI2C.h"
#include "mcp23017.h"
#include <unistd.h>
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
    mcp23017Setup(RELAY_BANK_1_PINBASE, RELAY_BANK_1_CHANNEL);
    mcp23017Setup(RELAY_BANK_2_PINBASE, RELAY_BANK_2_CHANNEL);

    // Output Devices
    _binaryDevices[State::Property::LightOn] = std::make_shared<BinaryDevice>(
        "Lights",
        LIGHTING_PIN,
        false,
        false
    );
    _binaryDevices[State::Property::ReservoirFlooded] = std::make_shared<BinaryDevice>(
        "Flood Pump",
        FLOOD_PUMP_PIN,
        FLOOD_PUMP_INVERT,
        false
    );
    _binaryDevices[State::Property::OxygenatorOn] = std::make_shared<BinaryDevice>(
        "Oxygenator",
        OXYGENATOR_PIN,
        OXYGENATOR_INVERT,
        false
    );
    _binaryDevices[State::Property::DrainCycleActive] = std::make_shared<BinaryDevice>(
        "Drain Pump",
        DRAIN_PUMP_PIN,
        DRAIN_PUMP_INVERT,
        false
    );
    _binaryDevices[State::Property::FillCycleActive] = std::make_shared<BinaryDevice>(
        "Fill Pump",
        FILL_PUMP_PIN,
        FILL_PUMP_INVERT,
        false
    );

    // Sensors
    _binaryDevices[State::Property::ReservoirEmpty] = std::make_shared<BinaryDevice>(
        "Reservoir Empty Sensor",
        RESERVOIR_EMPTY_SENSOR_PIN,
        false,
        true
    );
    _binaryDevices[State::Property::ReservoirFull] = std::make_shared<BinaryDevice>(
        "Reservoir Full Sensor",
        RESERVOIR_FULL_SENSOR_PIN,
        false,
        true
    );
    _binaryDevices[State::Property::FillReservoirEmpty] = std::make_shared<BinaryDevice>(
        "Fill Reservoir Empty Sensor",
        FILL_RESERVOIR_EMPTY_SENSOR_PIN,
        false,
        true
    );

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

std::shared_ptr<BinaryDevice> UCInterface::getBinaryDeviceForProperty(State::Property property)
{
    std::map<State::Property, std::shared_ptr<BinaryDevice>>::iterator itr = _binaryDevices.find(property);
    if (itr == _binaryDevices.end()) { return nullptr; }
    return itr->second;
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
    if (!readI2CResponse(_pHProbeFD, response, 6))
    {
        LogError("Failed to get pH probe calibration state");
        return false;
    }
    LogInfo("pH probe calibration state: " << response);
    bool calibrated = (response[5] != '0');
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
    if (readI2CResponse(_pHProbeFD, response, 0))
    {
        LogInfo("Set pH " << cPoint << " calibration point");
    }
    else
    {
        LogError("Failed to set calibration point");
    }
}

bool UCInterface::getPH(bool ignoreCalibration, float& pHValue)
{
    if (!_pHProbeReady && !ignoreCalibration)
    {
        LogWarn("pH probe is not calibrated");
        return false;
    }

    writeI2CCommand(_pHProbeFD, "R");
#if !_WINDOWS_BUILD
    delay(900);
#endif
    std::string response;
    if (!readI2CResponse(_pHProbeFD, response, 16))
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
    if (!readI2CResponse(_ecProbeFD, response, 6))
    {
        LogError("Failed to get EC probe calibration state");
        return false;
    }
    LogInfo("EC probe calibration state: " << response);
    bool calibrated = (response[5] != '0');
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
    writeI2CCommand(_ecProbeFD, command);
#if !_WINDOWS_BUILD
    delay(600);
#endif
    std::string response;
    if (readI2CResponse(_ecProbeFD, response, 0))
    {
        LogInfo("Set EC " << cPoint << " calibration point");
    }
    else
    {
        LogError("Failed to set calibration point");
    }
}

bool UCInterface::getEC(bool ignoreCalibration, float& ecValue, float& tdsValue)
{
    if (!_ecProbeReady && !ignoreCalibration)
    {
        LogWarn("EC probe is not calibrated");
        return false;
    }
    writeI2CCommand(_ecProbeFD, "R");
#if !_WINDOWS_BUILD
    delay(600);
#endif
    std::string response;
    if (!readI2CResponse(_ecProbeFD, response, 24))
    {
        LogError("Failed to sample EC");
        return false;
    }

    int commaIndex = response.find(',', 0);
    if (commaIndex < 0)
    {
        LogError("Failed to parse EC response");
        return false;
    }

    ecValue = (float)::atof(response.substr(0, commaIndex).c_str());
    tdsValue = (float)::atof(response.substr(commaIndex + 1).c_str());

    LogInfo("Current EC value is " << ecValue << ", TDS value is " << tdsValue);

    return true;
}

void UCInterface::setECParameter(const std::string& parameter, bool enabled)
{
    std::string command = "O,";
    command.append(parameter);
    command.append(",");
    command.append(enabled ? "1" : "0");

    writeI2CCommand(_ecProbeFD, command);
    delay(300);
    std::string response;
    if (!readI2CResponse(_ecProbeFD, response, 0))
    {
        LogWarn("Failed to set EC parameter " << parameter);
    }
}

bool UCInterface::readI2CResponse(int fd, std::string& response, int expectedLength)
{
    int actualExpected = expectedLength + 2; // 1-byte response code + 1-byte null terminator
    char buffer[512];
    int bytesRead = read(fd, &buffer, actualExpected);
    if (bytesRead != actualExpected)
    {
        LogError("Expected " << actualExpected << " bytes, but read " << bytesRead << " instead.");
        return false;
    }

    int responseCode = buffer[0];
    switch (responseCode)
    {
    case 2:
        LogWarn("Syntax error in previous command");
        return false;
    case 254:
        LogInfo("Data not ready, waiting");
        return false;
    case 255:
        LogWarn("No data to send");
        return false;
    }

    response = std::string((char*)&buffer[1]);
    LogInfo("Got response: '" << response << "'");
    return true;
}

void UCInterface::writeI2CCommand(int fd, const std::string& command)
{
    if (fd < 0)
    {
        LogWarn("Invalid i2c handle");
        return;
    }
    LogInfo("Writing command: '" << command << "'");
    write(fd, command.c_str(), command.length());
}
