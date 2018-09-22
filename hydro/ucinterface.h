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
    bool getPH(float& pHValue);

    // EC Probe
    bool isECProbeCalibrated();
    void setECCalibrationPoint(CalibrationPoint cPoint);
    bool getEC(float& ecValue);

private:
    bool getResponseString(int fd, std::string& response);

    void writeI2CCommand(int fd, const std::string& command);
    unsigned int readI2CResponse(int fd, char* buffer, unsigned int maxLength);

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
