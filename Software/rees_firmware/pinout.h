#ifndef PINOUT_H
#define PINOUT_H

#include "Arduino.h"

#ifdef I2C  //i2c

//Dirección de LCD I2C
#define I2C_DIR 0x3F

#else

//pines pantalla parallel  //parallel
#define LCD_RS A0
#define LCD_E  A1
#define LCD_D4 A3
#define LCD_D5 A3
#define LCD_D6 A4
#define LCD_D7 A5

#endif


//Rotary encoder
#define CLKpin 2
#define DTpin 3
#define SWpin  9

//Stepper driver
#define PULpin 6
#define DIRpin 7
#define ENpin 8

//Buzzer
#define BUZZpin 11

//Sensor hall
#define ENDSTOPpin 5


#endif // ENCODER_H
