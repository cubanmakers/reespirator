
#ifndef _DEFAULTS_H
#define _DEFAULTS_H

#define DEBUG_UPDATE 1 //
//#define DEBUG_STATE_MACHINE 1
//#define PRUEBAS 1 // testing over arduino without sensors

// Base de tiempos. Periodo de llamada a mechVentilation.update
#define TIME_BASE 20                 // msec
#define TIME_SENSOR 20               // msec
#define TIME_SEND_CONFIGURATION 2000 // msec

// Sensores
#define ENABLED_SENSOR_VOLUME 1
#if ENABLED_SENSOR_VOLUME
#define ENABLED_SENSOR_VOLUME_SFM3300 1
#endif

// Valores motor
#define STEPPER_MICROSTEPS 4
#define STEPPER_STEPS_PER_REVOLUTION 200

#define STEPPER_MICROSTEPS_PER_REVOLUTION (STEPPER_STEPS_PER_REVOLUTION * STEPPER_MICROSTEPS)
#define STEPPER_DIR 1
#define STEPPER_HOMING_DIRECTION    (-1)
#define STEPPER_HOMING_SPEED        (STEPPER_MICROSTEPS * 1000)   // Steps/s
// #define STEPPER_LOWEST_POSITION     (STEPPER_MICROSTEPS * -100)   // Steps
// #define STEPPER_HIGHEST_POSITION    (STEPPER_MICROSTEPS *   50)   // Steps
#define STEPPER_LOWEST_POSITION     (STEPPER_MICROSTEPS *  85)   // Steps
#define STEPPER_HIGHEST_POSITION    (STEPPER_MICROSTEPS * -100)   // Steps
#define STEPPER_SPEED_DEFAULT       (STEPPER_MICROSTEPS *  800)   // Steps/s
#define STEPPER_ACC_EXSUFFLATION    (STEPPER_MICROSTEPS *  600)   // Steps/s2
#define STEPPER_ACC_INSUFFLATION    (STEPPER_MICROSTEPS *  450)   // Steps/s2

// Valores por defecto
#define DEFAULT_HEIGHT 170 // cm
#define DEFAULT_SEX 0 // 0: varón, 1: mujer
#define DEFAULT_ML_POR_KG_DE_PESO_IDEAL 7
#define DEFAULT_MAX_VOLUMEN_TIDAL 800
#define DEFAULT_MIN_VOLUMEN_TIDAL 240
#define DEFAULT_TRIGGER_THRESHOLD 3.0
#define DEFAULT_RPM 14
#define DEFAULT_MAX_RPM 24
#define DEFAULT_MIN_RPM 3
#define DEFAULT_POR_INSPIRATORIO 33.3333F // %
#define DEFAULT_PEAK_INSPIRATORY_PRESSURE 20
#define DEFAULT_PEAK_ESPIRATORY_PRESSURE 10
// #define DEFAULT_PEAK_ESPIRATORY_PRESSURE 6

// Presión
#define DEFAULT_PA_TO_CM_H20 0.0102F

// Recruitment
#define DEFAULT_RECRUITMENT_TIMEOUT 40000 // msec
#define DEFAULT_RECRUITMENT_PIP 40

// Alarmas
#define ALARM_MAX_PRESSURE 35 // cm H2O
#define ALARM_MIN_PRESSURE 1  // cm H2O

// PID constants
// PID settings and gains
#define PID_MIN -20000 // TODO: check direction implementation
#define PID_MAX 20000
#define PID_KP 1000
#define PID_KI 40
#define PID_KD 100
#define PID_TS TIME_BASE
#define PID_BANGBANG 8

// Solenoid
#define SOLENOID_CLOSED 0
#define SOLENOID_OPEN 1

typedef struct {
    short height;
    bool sex;
    short respiratoryRate;
    short peakInspiratoryPressure;
    short peakEspiratoryPressure;
    float triggerThreshold;
    bool hasTrigger;
} VentilationOptions_t;

#endif // DEFAULTS_H
