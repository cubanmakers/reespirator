/**
 * Sensors reading module
 */

#include "Sensors.h"

unsigned int Sensors::begin(void) {
    // Arrancar sensores de presion 1 y 2
    if(!_pres1Sensor.begin()) {
        return 1;
    }

    if(!_pres2Sensor.begin()) {
        return 2;
    }


    /* Default settings from datasheet. */  //TODO: valorar SAMPLING_NONE, lecturas mas rapidas?
    // Ver ejemplos: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/examples/advancedsettings/advancedsettings.ino
    _pres1Sensor.setSampling(Adafruit_BME280::MODE_NORMAL,     /* Operating Mode. */
                      Adafruit_BME280::SAMPLING_X1,     /* Temp. oversampling */
                      Adafruit_BME280::SAMPLING_X2,    /* Pressure oversampling */
                      Adafruit_BME280::SAMPLING_NONE,   /* humidity sampling */
                      Adafruit_BME280::FILTER_X16,      /* Filtering. */
                      Adafruit_BME280::STANDBY_MS_0_5); /* Standby time. */
    _pres2Sensor.setSampling(Adafruit_BME280::MODE_NORMAL,     /* Operating Mode. */
                      Adafruit_BME280::SAMPLING_X1,     /* Temp. oversampling */
                      Adafruit_BME280::SAMPLING_X2,    /* Pressure oversampling */
                      Adafruit_BME280::SAMPLING_NONE,   /* humidity sampling */
                      Adafruit_BME280::FILTER_X16,      /* Filtering. */
                      Adafruit_BME280::STANDBY_MS_0_5); /* Standby time. */

    return 0;
}

Sensors::Sensors(Adafruit_BME280 pres1, Adafruit_BME280 pres2) {
    _init(pres1, pres2);
}

Sensors::Sensors() {
    //TODO
}

void Sensors::_init (Adafruit_BME280 pres1, Adafruit_BME280 pres2) {

    _pres1Sensor = pres1;
    _pres2Sensor = pres2;
    _errorCounter = 0;
    _state = SensorStateFailed;
}

void Sensors::readPressure() {
    float pres1, pres2;
    // Acquire sensors data
    pres1 = _pres1Sensor.readPressure(); // Pa
    pres2 = _pres2Sensor.readPressure(); // Pa

    if (pres1 == 0.0 || pres2 == 0.0) {

        if (_errorCounter > SENSORS_MAX_ERRORS) {
            _state = SensorStateFailed;
            //TODO do something?
        } else {
            _errorCounter++;
        }

    } else {
        _state = SensorStateOK;
        _errorCounter = 0;
        _pressure1 = pres1;
        _pressure2 = pres2;
    }
}

/**
 * @brief Get absolute pressure in pascals.
 *
 * @return SensorValues_t - pressure values
 */
SensorValues_t Sensors::getAbsolutePressureInPascals() {
    SensorValues_t values;
    values.state = _state;
    values.pressure1 = _pressure1;
    values.pressure2 = _pressure2 + _sensorsOffset;
    return values;
}

/**
 * @brief Get relative pressure in pascals.
 *
 * @return SensorValues_t - pressure values
 */
SensorValues_t Sensors::getRelativePressureInPascals() {
    SensorValues_t values = getAbsolutePressureInPascals();
    values.pressure1 = values.pressure1 - DEFAULT_ABSOLUTE_PRESSURE;
    values.pressure2 = values.pressure2 - DEFAULT_ABSOLUTE_PRESSURE;
    return values;
}

/**
 * @brief Get absolute pressure in H20 cm.
 *
 * @return SensorValues_t - pressure values
 */
SensorValues_t Sensors::getAbsolutePressureInCmH20() {
    SensorValues_t values = getAbsolutePressureInPascals();
    values.pressure1 *= DEFAULT_PA_TO_CM_H20;
    values.pressure2 *= DEFAULT_PA_TO_CM_H20;
    return values;
}

/**
 * @brief Get relative pressure in H20 cm.
 *
 * @return SensorValues_t - pressure values
 */
SensorValues_t Sensors::getRelativePressureInCmH20() {
    SensorValues_t values = getRelativePressureInPascals();
    values.pressure1 *= DEFAULT_PA_TO_CM_H20;
    values.pressure2 *= DEFAULT_PA_TO_CM_H20;
    return values;
}

/**
 * @brief Get the Offset Between Pressure Sensors object
 *
 * This function must be called when flux is 0.
 *
 * @param sensors - pressure sensors that derive flux
 * @param samples - number of samples to compute offset
 * @return float - averaged offset bewteen pressure readings
 */
void Sensors::getOffsetBetweenPressureSensors(int samples)
{
    SensorValues_t values;
    float deltaPressure, deltaAvg;
    float cumDelta = 0.0;
    for (int i = 0; i < samples; i++)
    {
        readPressure();
        values = getAbsolutePressureInPascals();
        deltaPressure = values.pressure1 - values.pressure2;
        cumDelta += deltaPressure;
    }
    deltaAvg = cumDelta / samples;
    _sensorsOffset = deltaAvg;
}

/**
 * @brief Calculate estimated flow from delta pressure
 *
 * @return float - estimated flow, in liters per minute
 */
float Sensors::calculateFlow(void)
{
    SensorValues_t values;
    float flow;
    values = getAbsolutePressureInPascals();
    flow = DEFAULT_K_PA_TO_LPM * (values.pressure1 - values.pressure2);
    // flow *= 5.0/3.0;
    // flow -= 65.0;
    return flow;
}


float Sensors::calculateFilteredFlow(void)
{
    float flow, filteredFlow;
    flow = calculateFlow();
    filteredFlow = computeLPF(flow, _lpfFlowArray, 100);
    return filteredFlow;
}