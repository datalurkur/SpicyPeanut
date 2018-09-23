#ifndef _UCINTERFACE_H_
#define _UCINTERFACE_H_

#include <chrono>
#include <memory>

class UCInterface
{
public:
    enum CalibrationType { pH, EC };
    enum CalibrationPoint { Dry, Low, Mid, High };

public:
    static std::shared_ptr<UCInterface> Instance;

    static void Init();

public:
    UCInterface();
    virtual ~UCInterface();

    bool sampleDHT22();
    float getLastTemperature();
    float getLastHumidity();

    void setReservoirState(bool flooded);
    bool getReservoirState();

    void setLightState(bool on);
    bool getLightState();

    void resetProbes();

    // pH Probe
    bool isPHProbeCalibrated();
    void setPHCalibrationPoint(CalibrationPoint cPoint);
    bool getPH(bool ignoreCalibration, float& pHValue);

    // EC Probe
    bool isECProbeCalibrated();
    void setECCalibrationPoint(CalibrationPoint cPoint);
    bool getEC(bool ignoreCalibration, float& ecValue, float& tdsValue);
    void setECParameter(const std::string& parameter, bool enabled);

private:
    void writeI2CCommand(int fd, const std::string& command);
    bool readI2CResponse(int fd, std::string& response, int expectedLength);

private:
    std::chrono::system_clock::time_point _nextDHT22Sample;
    float _lastTemperature;
    float _lastTemperatureCelsius;
    float _lastHumidity;

    int _pHProbeFD;
    bool _pHProbeReady;

    int _ecProbeFD;
    bool _ecProbeReady;
};

#endif
