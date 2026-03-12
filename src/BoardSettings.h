#ifndef BOARDSETTINGS_H
#define BOARDSETTINGS_H

  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif

  ////////////////////////////////////////////////////////////////////
  // uncomment for IRD PCB RAK version where the serial monitor is using software serial with pin 2 for TX
  // this is the case for the IRD PCBv5
  //#define SOFT_SERIAL_DEBUG
  
  ////////////////////////////////////////////////////////////////////
  // uncomment for WAZISENSE v2 board
  // #define WAZISENSE

  ////////////////////////////////////////////////////////////////////
  // uncomment for IRD PCB board (both v4.1 and v5)
  //#define IRD_PCB
  // also uncomment for IRD PCB board fully assembled by manufacturer
  // with all components, including solar circuit (both v4.1 and v5)
  //#define IRD_PCBA
  // pcb à la TK5EP
  #define PCB_UNIV_BOARD

  ////////////////////////////////////////////////////////////////////
  // uncomment only if the IRD PCB or PCBA is running on solar panel
  // MUST be commented if running on alkaline battery
  // code for SOLAR_BAT has been written by Jean-François Printanier from IRD
  #define SOLAR_BAT
  // do not change if you are not knowing what you are doing
  //#define NIMH

  // For powering humdity sensor and DS18B20
  #if (defined IRD_PCB && defined SOLAR_BAT) || defined IRD_PCBA

    #define PWR_SOFT_START_LOW   20
    #define PWR_SOFT_START_HIGH  20

    #define PWR_LOW     HIGH  // mosfet P
    #define PWR_HIGH    LOW   // mosfet P

void power_soft_start( uint8_t pin);

  #else

    #define PWR_LOW     LOW   // gpio
    #define PWR_HIGH    HIGH  // gpio

  #endif

  #ifdef IRD_PCBA
    #ifndef IRD_PCB
      #define IRD_PCB
    #endif
  #endif

  // PINS DEFINITION
// ADDED TK5EP
#ifdef PCB_UNIV_BOARD
    // DS18B20
    #define TEMP_DIGITAL_PIN 6
    #define TEMP_PWR_PIN 9
    // RTC
    #define RTC_PWR_PIN A0
    #define RTC_INT_PIN A2
    // INA219
    #define INA219_PWR_PIN A1
    // 3V3REG
    #define REG_PWR_PIN 8
    // MOSFET SW
    #define VCC_SW 5
    // SHT31
    #define SHT_SDA_PIN A4
    #define SHT_SCL_PIN A5
    #define SHT_PWR_PIN -1          // powered via Vcc

    #define BME_SDA_PIN A4
    #define BME_SCL_PIN A5
    #define BME_PWR_PIN -1          // powered via Vcc

    #define MCP3424_PWR_PIN 5

    #define HX711_PWR_PIN 5         // VCC switched
    #define HX711_DT_PIN 6          // DT
    #define HX711_SCK_PIN 9         // SCK
#endif

#endif // BOARDSETTINGS_H
