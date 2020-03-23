
#ifndef _DEFAULTS_H
#define _DEFAULTS_H

#undef I2C // definido = Display i2c, sin definir Display parallel

// Base de tiempos. Periodo de llamada a mechVentilation.update
#define TIME_BASE 5 //msec

// Valores motor
#define STEPPER_MICROSTEPS 4
#define STEPPER_STEPS_PER_REVOLUTION 200
#define STEPPER_HOMING_DIRECTION -1
#define STEPPER_HOMING_SPEED 50        // Steps/S
#define STEPPER_LOWEST_POSITION 70      // Steps
#define STEPPER_HIGHEST_POSITION -120   // Steps
#define STEPPER_SPEED_DEFAULT 800       // Steps/s
#define STEPPER_SPEED_INSUFFLATION 800  // Steps/s
#define STEPPER_SPEED_EXSUFFLATION 800  // Steps/s
#define STEPPER_ACC_DEFAULT 600         // Steps/s2
#define STEPPER_ACC_EXSUFFLATION 600    // Steps/s2

// Valores por defecto
#define DEFAULT_ESTATURA 170 // cm
#define DEFAULT_SEXO 0 // 0: varón, 1: mujer
#define DEFAULT_ML_POR_KG_DE_PESO_IDEAL 7
#define DEFAULT_MAX_VOLUMEN_TIDAL 800
#define DEFAULT_MIN_VOLUMEN_TIDAL 240
#define DEFAULT_FLUJO_TRIGGER 3
#define DEFAULT_RPM 15
#define DEFAULT_MAX_RPM 24
#define DEFAULT_MIN_RPM 3
#define DEFAULT_POR_INSPIRATORIO 33.33  // %

#define DEFAULT_PRESSURE_V_FLOW_K1 0.1   //Constante proporcional que relaciona presión con caudal

#define FLOW__INSUFLATION_TRIGGER_LEVEL 3.0   //LPM

// Ventilation cycle timing
#define WAIT_BEFORE_EXSUFLATION_TIME 500    //msec

// PID constants
#define PID_MIN 0
#define PID_MAX 100

#define PID_P 1.0
#define PID_I 0.0 //TODO To be adjusted
#define PID_D 0.0 //TODO To be adjusted
#define PID_dt TIME_BASE //msec. I hast to match the MechVentilation.update() refresh period.

#define PID_KP PID_P
#define PID_KI (PID_I * PID_dt)
#define PID_KD (PID_D / PID_dt)

//Volume to Position and viceversa constants
#define STEPS_FOR_TOTALLY_PRESSED_AMBU (STEPPER_STEPS_PER_REVOLUTION / 2) //steps
#define VOLUME_FOR_TOTALLY_PRESSED_AMBU 800 //ml
#define K_VOL2POS (STEPS_FOR_TOTALLY_PRESSED_AMBU / VOLUME_FOR_TOTALLY_PRESSED_AMBU)
#define K_POS2VOL (VOLUME_FOR_TOTALLY_PRESSED_AMBU / STEPS_FOR_TOTALLY_PRESSED_AMBU)
#define K_FLOW2SPEED (25/12)

// @fm superñapa. parametrizar desde el inicio. se usa en mechVentilation
#define ventilationCycle_WaitBeforeInsuflationTime  800 //ms TODO parameter to mechVent
#define totalPatientVolume 0.8 // liters

#endif // DEFAULTS_H
