// librairie pour le BME280
// les 3 capteurs sont lus dans cette bibliothèque en une seule fois
// les données humidité et pression sont récupérées ici

#include "BME_Temperature.h"

#define BME280_ADDR 0x76  // Adresse I2C par défaut

// Initialisation de la structure partagée
BME280_Shared_Data BME_Temperature::sharedData = {0.0, 0.0, 0.0, 0};

BME_Temperature::BME_Temperature(char* nomenclature, bool is_analog, bool is_connected, bool is_low_power, uint8_t pin_read, uint8_t pin_power, uint8_t pin_trigger)
: Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_read, pin_power, pin_trigger) {
    
    if (get_is_connected()) {
        Wire.begin();
        
        if (get_pin_power() != -1) {
            pinMode(get_pin_power(), OUTPUT);
#if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA
            power_soft_start(get_pin_power());
#else
            digitalWrite(get_pin_power(), HIGH);
#endif
            delay(10); 
        }

        // Configuration du BME280 (Mode Normal, Oversampling x1 partout)
        Wire.beginTransmission(BME280_ADDR);
        Wire.write(0xF2); Wire.write(0x01); // Humidité
        Wire.endTransmission();

        Wire.beginTransmission(BME280_ADDR);
        Wire.write(0xF4); Wire.write(0x27); // Temp & Press & Mode Normal
        Wire.endTransmission();

        read_calibration();
    }
}

void BME_Temperature::read_calibration() {
    // Lecture T1-T3, P1-P9 (24 octets à partir de 0x88)
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(0x88);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDR, 24);
    dig_T1 = Wire.read() | (Wire.read() << 8);
    dig_T2 = Wire.read() | (Wire.read() << 8);
    dig_T3 = Wire.read() | (Wire.read() << 8);
    dig_P1 = Wire.read() | (Wire.read() << 8);
    dig_P2 = Wire.read() | (Wire.read() << 8);
    dig_P3 = Wire.read() | (Wire.read() << 8);
    dig_P4 = Wire.read() | (Wire.read() << 8);
    dig_P5 = Wire.read() | (Wire.read() << 8);
    dig_P6 = Wire.read() | (Wire.read() << 8);
    dig_P7 = Wire.read() | (Wire.read() << 8);
    dig_P8 = Wire.read() | (Wire.read() << 8);
    dig_P9 = Wire.read() | (Wire.read() << 8);

    // Lecture H1 (1 octet à 0xA1)
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(0xA1);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDR, 1);
    dig_H1 = Wire.read();

    // Lecture H2-H6 (7 octets à 0xE1)
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(0xE1);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDR, 7);
    dig_H2 = Wire.read() | (Wire.read() << 8);
    dig_H3 = Wire.read();
    dig_H4 = (Wire.read() << 4) | (Wire.read() & 0x0F);
    dig_H5 = (Wire.read() >> 4) | (Wire.read() << 4);
    dig_H6 = (int8_t)Wire.read();
}

void BME_Temperature::update_data() {
    if (!get_is_connected()) return;

    // Lecture des 8 octets de données (Pres, Temp, Hum)
    Wire.beginTransmission(BME280_ADDR);
    Wire.write(0xF7);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDR, 8);
    
    uint32_t adc_P = ((uint32_t)Wire.read() << 12) | ((uint32_t)Wire.read() << 4) | ((uint32_t)Wire.read() >> 4);
    uint32_t adc_T = ((uint32_t)Wire.read() << 12) | ((uint32_t)Wire.read() << 4) | ((uint32_t)Wire.read() >> 4);
    uint32_t adc_H = ((uint32_t)Wire.read() << 8) | (uint32_t)Wire.read();

    // --- COMPENSATION TEMPÉRATURE ---
    int32_t v1T = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    int32_t v2T = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    t_fine = v1T + v2T;
    sharedData.temp = (double)((t_fine * 5 + 128) >> 8) / 100.0;

    // --- COMPENSATION PRESSION ---
    int64_t v1P = ((int64_t)t_fine) - 128000;
    int64_t v2P = v1P * v1P * (int64_t)dig_P6;
    v2P = v2P + ((v1P * (int64_t)dig_P5) << 17);
    v2P = v2P + (((int64_t)dig_P4) << 35);
    v1P = ((v1P * v1P * (int64_t)dig_P3) >> 8) + ((v1P * (int64_t)dig_P2) << 12);
    v1P = (((((int64_t)1) << 47) + v1P)) * ((int64_t)dig_P1) >> 33;
    if (v1P != 0) {
        int64_t p = 1048576 - adc_P;
        p = (((p << 31) - v2P) * 3125) / v1P;
        v1P = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
        v2P = (((int64_t)dig_P8) * p) >> 19;
        p = ((p + v1P + v2P) >> 8) + (((int64_t)dig_P7) << 4);
        sharedData.press = (double)p / 25600.0; // Résultat en hPa
    }

    // --- COMPENSATION HUMIDITÉ ---
    int32_t vH = (t_fine - ((int32_t)76800));
    vH = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * vH)) + ((int32_t)16384)) >> 15) * (((((((vH * ((int32_t)dig_H6)) >> 10) * (((vH * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    vH = (vH - (((((vH >> 15) * (vH >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    vH = (vH < 0 ? 0 : vH);
    vH = (vH > 419430400 ? 419430400 : vH);
    sharedData.hum = (double)(vH >> 12) / 1024.0;

    sharedData.last_update = millis();
    set_data(sharedData.temp);
}

double BME_Temperature::get_value() {
    update_data();
    return get_data();
}