#pragma once
#include <BlueDot_BME280.h>
#include <HouzDevicesModel.h>

class HouzWeather{
  public:
    HouzWeather(int sda, int scl);
    bool init();
    Weather getWeather();

  private:
    BlueDot_BME280 bme280;
    Weather _current;
};
