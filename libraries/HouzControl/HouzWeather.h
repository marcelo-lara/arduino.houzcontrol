#pragma once
#include <BlueDot_BME280.h>

typedef struct Weather{
public:
  bool online;
	float temp;
	float hum;
	float pressure;
	float alt;
};

class HouzWeather{
  public:
    HouzWeather(int sda, int scl);
    bool read();
    bool init();
    void dump();
    Weather getWeather();

  private:
    BlueDot_BME280 bme280;
    Weather _current;
};
