#ifndef HX711_READER_H
#define HX711_READER_H

#include "Sensor.h"
#include <Arduino.h>

class HX711_Reader : public Sensor {
public:
    HX711_Reader(const char* nomenclature,
                 bool is_analog,
                 bool is_connected,
                 bool is_low_power,
                 int pin_dout,
                 int pin_sck,
                 int pin_pwr,
                 double calib_offset,
                 double calib_scale,
                 uint8_t filt_window);

    void update_data() override;
    double get_value();
    void power_down();
    void power_up();

private:
    int pin_dout;
    int pin_sck;
    double offset;
    double scale;
    //int8_t filter_window;
    //double* filter_buf;
    //uint8_t filter_index;
    //uint8_t filter_count;

    long read_raw(); // lecture 24-bit du HX711
};

#endif