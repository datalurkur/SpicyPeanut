#ifndef _UCINTERFACE_H_
#define _UCINTERFACE_H_

#include <memory>

class UCInterface
{
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

    void setLightState(bool on);

private:
    std::chrono::system_clock::time_point _nextDHT22Sample;
    float _lastTemperature;
    float _lastTemperatureCelsius;
    float _lastHumidity;
};

#endif
