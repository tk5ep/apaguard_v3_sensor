#ifndef BME_TEMPERATURE_H
#define BME_TEMPERATURE_H

#include "Sensor.h"
#include <Wire.h>

// Structure partagée pour éviter les lectures multiples
struct BME280_Shared_Data {
    double temp;
    double hum;
    double press;
    uint32_t last_update;
};

class BME_Temperature : public Sensor {
public:
    BME_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger);
    void update_data();
    double get_value();

    // Accès statique pour les autres classes (Hum/Press)
    static BME280_Shared_Data sharedData;

private:
    void read_calibration();
    int32_t t_fine;
    
    // Paramètres de calibration (obligatoires pour le BME280)
    uint16_t dig_T1; int16_t dig_T2, dig_T3;
    uint16_t dig_P1; int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
    uint8_t  dig_H1; int16_t dig_H2; uint8_t  dig_H3;
    int16_t  dig_H4, dig_H5; int8_t  dig_H6;
};

#endif