#pragma once
#include <Wire.h>
#include <BlueDot_BME280.h>
#include <HouzWeather.h>

Weather _current;

HouzWeather::HouzWeather(int _sda, int _scl){
};

bool HouzWeather::init(){
		bme280.parameter.communication =		0;		//Choose communication protocol
		bme280.parameter.I2CAddress =			0x76;	//Choose I2C Address
		bme280.parameter.sensorMode =			0b11;	//Choose sensor mode
		bme280.parameter.IIRfilter =			0b100;	//Setup for IIR Filter
		bme280.parameter.humidOversampling =	0b101;	//Setup Humidity Oversampling
		bme280.parameter.tempOversampling =		0b101;	//Setup Temperature Ovesampling
		bme280.parameter.pressOversampling =	0b101;	//Setup Pressure Oversampling 
		bme280.parameter.pressureSeaLevel =		1013.25;//default value of 1013.25 hPa
		bme280.parameter.tempOutsideCelsius =	15;		//default value of 15�C
		_current.online = (bme280.init() ==		0x60);
		return _current.online;
}
bool HouzWeather::read(){
	if(!_current.online) init();
  _current.temp=bme280.readTempC();
  _current.hum=bme280.readHumidity();
  _current.pressure=bme280.readPressure();
  _current.alt=bme280.readAltitudeMeter();
  return true;
};

void HouzWeather::dump(){
};

Weather HouzWeather::getWeather(){
	if(!_current.online) read();

	return _current;
};