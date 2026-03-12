#include "HX711_Reader.h"
#include <Arduino.h>

HX711_Reader::HX711_Reader(const char* nomenclature,
                           bool is_analog,
                           bool is_connected,
                           bool is_low_power,
                           int pin_dout_,
                           int pin_sck_,
                           int pin_pwr_,
                           double calib_offset_,
                           double calib_scale_,
                           uint8_t filt_window_)
    : Sensor(nomenclature, is_analog, is_connected, is_low_power, pin_dout_, pin_pwr_, pin_sck_),
      pin_dout(pin_dout_), pin_sck(pin_sck_),
      offset(calib_offset_), scale(calib_scale_)
{
    // filt_window_ ignoré désormais, gardé pour compatibilité d'appel
    pinMode(pin_sck, OUTPUT);
    pinMode(pin_dout, INPUT_PULLUP);
    digitalWrite(pin_sck, LOW);
}

// ---------------------------------------------------------------

double HX711_Reader::get_value() {
    update_data();
    return get_data();
}

// ---------------------------------------------------------------

//https://claude.ai/share/193e6e4d-5c5f-46c6-9e15-66de6c7a6fd3
long HX711_Reader::read_raw() {
    long count = 0;

    noInterrupts();

    for (uint8_t i = 0; i < 24; i++) {
        digitalWrite(pin_sck, HIGH);
        delayMicroseconds(1);
        count = count << 1;
        digitalWrite(pin_sck, LOW);
        if (digitalRead(pin_dout)) count++;
        delayMicroseconds(1);
    }

    // 25ème pulse → gain 128, channel A
    digitalWrite(pin_sck, HIGH);
    delayMicroseconds(1);
    digitalWrite(pin_sck, LOW);

    interrupts();

    // Extension signe 24-bit → 32-bit signé
    if (count & 0x800000) count |= 0xFF000000;

    return count;
}

// ---------------------------------------------------------------

void HX711_Reader::update_data() {

    if (!get_is_connected()) {
        set_data(-99.0);
        return;
    }

    // --- Power ON ---
    if (get_is_low_power() && get_pin_power() != -1) {
        digitalWrite(get_pin_power(), HIGH);
        delay(500);
    }

    // --- Attente DOUT prêt (1ère fois) ---
    unsigned long timeout = millis();
    while (digitalRead(pin_dout) == HIGH) {
        if (millis() - timeout > 300) {
            Serial.println("HX711: timeout DOUT");
            set_data(-99.0);
            if (get_is_low_power() && get_pin_power() != -1)
                digitalWrite(get_pin_power(), LOW);
            return;
        }
    }

    // --- 5 lectures ---
    const uint8_t NB = 5;
    long reads[NB];

    // attente HX711 prêt
    for (uint8_t i = 0; i < NB; i++) {
        if (i > 0) {
            unsigned long t = millis();
            while (digitalRead(pin_dout) == HIGH) {
                if (millis() - t > 300) {
                    Serial.println("HX711: timeout lecture");
                    set_data(-99.0);
                    if (get_is_low_power() && get_pin_power() != -1)
                        digitalWrite(get_pin_power(), LOW);
                    return;
                }
            }
        }
        reads[i] = read_raw();
    }

    // --- Détection saturation ---
    for (uint8_t i = 0; i < NB; i++) {
        if (reads[i] == 0x7FFFFF || reads[i] == (long)0xFF800000) {
            Serial.println("HX711: saturation");
            set_data(-99.0);
            if (get_is_low_power() && get_pin_power() != -1)
                digitalWrite(get_pin_power(), LOW);
            return;
        }
    }

    // --- Tri → médiane ---
    for (uint8_t i = 0; i < NB - 1; i++)
        for (uint8_t j = 0; j < NB - 1 - i; j++)
            if (reads[j] > reads[j + 1]) {
                long tmp  = reads[j];
                reads[j]  = reads[j + 1];
                reads[j + 1] = tmp;
            }

    long raw = reads[NB / 2]; // médiane = reads[2]

    /*
    // TEST
    Serial.print("HX711 raw (");
    Serial.print((uint32_t)this, HEX);
    Serial.print("): ");
    Serial.println(raw);
    */

    // --- Conversion poids ---
    double weight = (double)(raw - offset) * scale;
    set_data(weight);

    // --- Power OFF ---
    if (get_is_low_power() && get_pin_power() != -1)
        digitalWrite(get_pin_power(), LOW);
}

// ---------------------------------------------------------------
// fonctions pour couper le HX711 par soft
void HX711_Reader::power_down() {
    if (get_is_low_power()) {
        digitalWrite(pin_sck, HIGH);
        delayMicroseconds(100);
    }
}

void HX711_Reader::power_up() {
    if (get_is_low_power()) {
        digitalWrite(pin_sck, LOW);
        delay(500);
    }
}