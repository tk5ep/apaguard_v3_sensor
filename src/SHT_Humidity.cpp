/*
* Copyright (C) 2018-2024 C. Pham, University of Pau, France
*
* Congduc.Pham@univ-pau.fr
* modified by TK5EP 2026-02
*/

#include "BoardSettings.h"
#include "SHT_Humidity.h"
#include <Wire.h>

SHT_Humidity::SHT_Humidity(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger):Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger){
  if (get_is_connected()){
    Wire.begin();
  
    if (get_pin_power()!=-1) {	
    	pinMode(get_pin_power(),OUTPUT);
      digitalWrite(get_pin_power(), PWR_LOW);

      if (get_pin_read()!=-1)
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

    set_warmup_time(500);
  }
}


void SHT_Humidity::update_data() {
    if (!get_is_connected()) {
        if (has_fake_data()) set_data((double)random(15, 90));
        return;
    }

    Wire.beginTransmission(0x44);
    Wire.write(0x2C); // Command: High repeatability
    Wire.write(0x06); 
    
    if (Wire.endTransmission() != 0) {
        set_data(-99.0);
        return; 
    }

    delay(20); // Wait for measurement

    Wire.requestFrom(0x44, 6);
    if (Wire.available() == 6) {
        uint16_t t_raw = (Wire.read() << 8) | Wire.read();
        Wire.read(); // Skip Temp CRC
        uint16_t h_raw = (Wire.read() << 8) | Wire.read();
        Wire.read(); // Skip Hum CRC

        // We calculate both since the sensor sends both
        double temp_c = -45.0 + 175.0 * (double)t_raw / 65535.0;
        double hum_rel = 100.0 * (double)h_raw / 65535.0;

        set_data(hum_rel);
        
        // This line tells the compiler "I am aware of temp_c" 
        // without adding any code to the final binary.
        (void)temp_c; 

    } else {
        set_data(-99.0);
    }
}

double SHT_Humidity::get_value()
{
  update_data();
  return get_data();
}
