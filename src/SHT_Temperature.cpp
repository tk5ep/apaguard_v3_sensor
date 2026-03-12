/*
 * Copyright (C) 2018-2024 C. Pham, University of Pau, France
 * * Congduc.Pham@univ-pau.fr
 * 
 * modified by TK5EP 2026-02
 * */

#include "BoardSettings.h"
#include "SHT_Temperature.h"
#include <Wire.h>

SHT_Temperature::SHT_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger)
    : Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger) {
    
    if (get_is_connected()) {
        Wire.begin();
        if (get_pin_power() != -1) {  
            pinMode(get_pin_power(), OUTPUT);
            digitalWrite(get_pin_power(), PWR_LOW);

            if (get_pin_read() != -1)
                pinMode(get_pin_read(), INPUT);       
        
            if (get_is_low_power())
                digitalWrite(get_pin_power(), PWR_LOW);
            else
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
                power_soft_start(get_pin_power());
#else
                digitalWrite(get_pin_power(), PWR_HIGH);
#endif        
        }
        
        // Warmup time for the SHT31 to stabilize after power-on
        set_warmup_time(500);
    }
}

void SHT_Temperature::update_data() {
    if (get_is_connected()) {
        
        // Power management for low-power nodes
        if (get_is_low_power() && get_is_power_on_when_active()) {
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
            power_soft_start(get_pin_power());
#else
            digitalWrite(get_pin_power(), PWR_HIGH);   
#endif    
        }

        // Wait for sensor stabilization if power was just toggled
        delay(get_warmup_time());
            
        // --- I2C Communication ---
        Wire.beginTransmission(0x44);
        Wire.write(0x2C); // MSB: Single shot measurement
        Wire.write(0x06); // LSB: High repeatability
        
        if (Wire.endTransmission() != 0) {
            set_data((double)-99.0); // Bus error
            return;
        }

        // SHT31 takes ~15ms for a high-repeatability measurement
        delay(20); 

        // Request 6 bytes: [Temp MSB][Temp LSB][CRC][Hum MSB][Hum LSB][CRC]
        Wire.requestFrom(0x44, 6);
        
        if (Wire.available() == 6) {
            uint16_t t_raw = (Wire.read() << 8) | Wire.read();
            Wire.read(); // Skip Temp CRC
            
            uint16_t h_raw = (Wire.read() << 8) | Wire.read();
            Wire.read(); // Skip Hum CRC

            // Silence warning for h_raw since this is the Temperature class
            (void)h_raw;

            // Official SHT31 Temperature conversion formula:
            // T (°C) = -45 + 175 * (S_T / (2^16 - 1))
            double t = -45.0 + 175.0 * (double)t_raw / 65535.0;
            
            set_data(t);
        } 
        else {
            set_data((double)-99.0);
        }
          
        // Turn off power if configured for aggressive low power
        if (get_is_low_power() && get_is_power_off_when_inactive())   
            digitalWrite(get_pin_power(), PWR_LOW);   
    }
    else {
        // Provide fake data for testing without hardware
        if (has_fake_data())  
            set_data((double)random(-20, 40));
    }
}

double SHT_Temperature::get_value() {
    update_data();
    return get_data();
}