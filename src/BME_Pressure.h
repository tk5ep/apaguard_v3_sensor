#ifndef BME_PRESSURE_H
#define BME_PRESSURE_H
#include "BME_Temperature.h"

class BME_Pressure : public Sensor {
public:
    BME_Pressure(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger)
    : Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger) {}
    
    void update_data() { set_data(BME_Temperature::sharedData.press); }
    double get_value() { return BME_Temperature::sharedData.press; }
};
#endif