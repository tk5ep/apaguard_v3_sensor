#ifndef BME_HUMIDITY_H
#define BME_HUMIDITY_H
#include "BME_Temperature.h"

class BME_Humidity : public Sensor {
public:
    BME_Humidity(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger)
    : Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger) {}
    
    void update_data() { set_data(BME_Temperature::sharedData.hum); }
    double get_value() { return BME_Temperature::sharedData.hum; }
};
#endif