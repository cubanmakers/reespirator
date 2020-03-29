/** Mechanical ventilation.
 *
 * @file MechVentilation.cpp
 *
 * This is the mechanical ventilation software module.
 * It handles the mechanical ventilation control loop.
 */

#include "MechVentilation.h"

int currentWaitTriggerTime = 0;
int currentStopInsufflationTime = 0;
float currentFlow = 0;

MechVentilation::MechVentilation(
    FlexyStepper * stepper,
    Sensors * sensors,
    AutoPID * pid,
    VentilationOptions_t options) {

    _init(
        stepper,
        sensors,
        pid,
        options,
        LPM_FLOW_TRIGGER_VALUE_NONE
    );

}

MechVentilation::MechVentilation(
    FlexyStepper * stepper,
    Sensors * sensors,
    AutoPID * pid,
    VentilationOptions_t options,
    float lpmFlowTriggerValue
) {
    _init(
        stepper,
        sensors,
        pid,
        options,
        lpmFlowTriggerValue
    );
}

//TODO: use this method to play a beep in main loop, 1 second long for example.
boolean MechVentilation::getStartWasTriggeredByPatient() { //returns true if last respiration cycle was started by patient trigger. It is cleared when read.
    if (_startWasTriggeredByPatient) {
        return true;
    } else {
        return false;
    }
}

//TODO: use this method to play a beep in main loop, 2 second long for example.
boolean MechVentilation::getSensorErrorDetected() { //returns true if there was an sensor error detected. It is cleared when read.
    if (_sensor_error_detected) {
        return true;
    } else {
        return false;
    }
}

void MechVentilation::start(void) {
    _running = true;
}

void MechVentilation::stop(void) {
    _running = false;
}

uint8_t MechVentilation::getRPM(void) {
    return _rpm;
}
short MechVentilation::getExsuflationTime(void) {
    return _timeout_esp;
}
short MechVentilation::getInsuflationTime(void) {
    return _timeout_ins;
}
void MechVentilation::reconfigParameters (uint8_t newRpm) {
    _rpm = newRpm;
    _setInspiratoryCycle();
    // _positionInsufflated = _calculateInsuflationPosition();
}

void MechVentilation::_setInspiratoryCycle(void) {
  float tCiclo = ((float)60) * 1000/ ((float)_rpm); // Tiempo de ciclo en msegundos
  _timeout_ins = tCiclo * DEFAULT_POR_INSPIRATORIO/100;
  _timeout_esp = (tCiclo) - _timeout_ins;
}

/**
 * I's called from timer1Isr
 */
void MechVentilation::update(void) {

    static int totalCyclesInThisState = 0;
    static int currentTime = 0;
    static int flowSetpoint = 0;

    #if DEBUG_STATE_MACHINE
        extern volatile String debugMsg[];
        extern volatile byte debugMsgCounter;
    #endif


    
    SensorPressureValues_t pressures = _sensors -> getRelativePressureInCmH20();
    if (pressures.state != SensorStateOK) { // Sensor error detected: return to zero position and continue from there
        _sensor_error_detected = true; //An error was detected in sensors
        /* Status update, for this time */
        // TODO: SAVE PREVIOUS CYCLE IN MEMORY AND RERUN IT
        _setState(State_Exsufflation);
    } else {
        _sensor_error_detected = false; //clear flag
    }


    refreshWatchDogTimer();
    switch (_currentState) {
        case Init_Insufflation:
            {
                // Close Solenoid Valve
                digitalWrite(PIN_SOLENOID, SOLENOID_CLOSED);

                totalCyclesInThisState = (_timeout_ins) / TIME_BASE;

                /* Stepper control: set acceleration and end-position */
                _stepper->setSpeedInStepsPerSecond(_stepperSpeed);
                _stepper->setAccelerationInStepsPerSecondPerSecond(
                    STEPPER_ACC_INSUFFLATION
                );
                _stepper->setTargetPositionInSteps(STEPPER_HIGHEST_POSITION);

                #if DEBUG_STATE_MACHINE
                debugMsg[debugMsgCounter++] = "State: InitInsuflation at " + String(millis());
                debugMsg[debugMsgCounter++] = "Motor to " + String(_positionInsufflated);
                #endif

                /* Status update, reset timer, for next time, and reset PID integrator to zero */
                _setState(State_Insufflation);
                
                currentTime = 0;
            }
            //break;  MUST BE COMMENTED
        case State_Insufflation:
            {
                _currentPressure = pressures.pressure1;
                _pid->run(&_currentPressure, &_pip, &_stepperSpeed);

                /* Stepper control: set end position */

                #if DEBUG_UPDATE
                Serial.println("Motor:speed=" + String(_stepperSpeed));
                #endif

                // time expired
                if (currentTime > totalCyclesInThisState) {
                    if (!_stepper->motionComplete()) {
                        // motor not finished, force motor to stop in current position
                        _stepper->setTargetPositionInSteps(_stepper->getCurrentPositionInSteps());
                    }
                    _setState(Init_Exsufflation);
                    currentTime = 0;
                }  else {
                    // TODO: if _currentPressure > _pip + 5, trigger alarm
                    _stepper->setSpeedInStepsPerSecond(abs(_stepperSpeed));
                    if (_stepperSpeed >= 0) {
                        _stepper->setTargetPositionInSteps(STEPPER_HIGHEST_POSITION);
                    } else {
                        _stepper->setTargetPositionInSteps(STEPPER_LOWEST_POSITION);
                    }
                    currentTime++;
                }
            }
            break;
        case Init_Exsufflation:
            {
                // Open Solenoid Valve
                digitalWrite(PIN_SOLENOID, SOLENOID_OPEN);

                totalCyclesInThisState = _timeout_esp / TIME_BASE;

                #if DEBUG_STATE_MACHINE
                debugMsg[debugMsgCounter++] = "ExsuflationTime=" + String(totalCyclesInThisState);
                #endif
                /* Stepper control*/
                _stepper->setSpeedInStepsPerSecond(_stepperSpeed);
                _stepper->setAccelerationInStepsPerSecondPerSecond(
                    STEPPER_ACC_EXSUFFLATION
                );
                _stepper->setTargetPositionInSteps(
                    STEPPER_DIR * (STEPPER_LOWEST_POSITION)
                );
                #if DEBUG_STATE_MACHINE
                debugMsg[debugMsgCounter++] = "Motor: to exsuflation at " + String(millis());
                #endif

                /* Status update and reset timer, for next time */
                _setState(State_Exsufflation);
                currentTime = 0;
            }
            //break;  MUST BE COMMENTED
        case State_Exsufflation:
            {
                _currentPressure = pressures.pressure1;
                _pid->run(&_currentPressure, &_pep, &_stepperSpeed);

                if (_stepper->motionComplete()) {
                    if (currentFlow < FLOW__INSUFLATION_TRIGGER_LEVEL) { //The start was triggered by patient
                        _startWasTriggeredByPatient = true;

                        #if DEBUG_STATE_MACHINE
                            debugMsg[debugMsgCounter++] = "!!!! Trigered by patient";
                        #endif

                        /* Status update, for next time */
                        _setState(Init_Insufflation);

                    }
                }

                // Time has expired             
                if (currentTime > totalCyclesInThisState) {

                    /* Status update and reset timer, for next time */
                    _setState(Init_Insufflation);
                    _startWasTriggeredByPatient = false;

                    currentTime = 0;
                } else {
                    _stepper->setSpeedInStepsPerSecond(abs(_stepperSpeed));
                    if (_stepperSpeed >= 0) {
                        _stepper->setTargetPositionInSteps(STEPPER_HIGHEST_POSITION);
                    } else {
                        _stepper->setTargetPositionInSteps(STEPPER_LOWEST_POSITION);
                    }
                    currentTime++;
                }
            }
            break;

        case State_Homing:
            {
                // Open Solenoid Valve
                digitalWrite(PIN_SOLENOID, SOLENOID_OPEN);

                if (_sensor_error_detected) {
                    // error sensor reading
                    _running = false;
                    #if DEBUG_UPDATE
                    Serial.println("Sensor: FAILED");
                    #endif
                }

                if (!digitalRead(PIN_ENDSTOP)) { // If not in HOME, do Homing

                    /* Stepper control: homming */
                    // bool moveToHomeInMillimeters(long directionTowardHome, float
                    // speedInMillimetersPerSecond, long maxDistanceToMoveInMillimeters, int
                    // homeLimitSwitchPin)

                    while (
                        _stepper -> moveToHomeInSteps(
                            STEPPER_HOMING_DIRECTION,
                            STEPPER_HOMING_SPEED,
                            STEPPER_STEPS_PER_REVOLUTION * STEPPER_MICROSTEPS,
                            PIN_ENDSTOP
                        ));
                }

                /* Status update and reset timer, for next time */
                currentTime = 0;
                _setState(Init_Exsufflation);
            }
            break;

        case State_Error:
        default:
            //TODO
            break;
    }

}

void MechVentilation::_init(
    FlexyStepper * stepper,
    Sensors * sensors,
    AutoPID * pid,
    VentilationOptions_t options
) {
    /* Set configuration parameters */
    _stepper = stepper;
    _sensors = sensors;
    _pid = pid;
    _rpm = options.respiratoryRate;
    _pip = options.peakInspiratoryPressure;
    _pep = options.peakEspiratoryPressure;
    reconfigParameters(_rpm);
    if (options.hasTrigger) {
        _trigger_threshold = lpmFlowTriggerValue;
    } else {
        _trigger_threshold = FLT_MAX;
    }

    /* Initialize internal state */
    _currentState = State_Homing;
    _stepperSpeed = STEPPER_SPEED_DEFAULT;

    //
    // connect and configure the stepper motor to its IO pins
    //
    //;
    _stepper->connectToPins(PIN_STEPPER_STEP, PIN_STEPPER_DIRECTION);
    _stepper->setStepsPerRevolution(STEPPER_STEPS_PER_REVOLUTION * STEPPER_MICROSTEPS);

    _sensor_error_detected = false;
}

#if 0
int MechVentilation::_calculateInsuflationPosition (void) {
    short volumeTarget = (short) _cfgmlTidalVolume;
    short lastPosition = volumeCalibration[0].stepperPos;
    for (byte i = 1; i < sizeof(CalibrationVolume_t); i++) {
        if (volumeTarget > volumeCalibration[i].mlVolume) {
            lastPosition = volumeCalibration[i].stepperPos;
        } else {
            break;
        }
    }
    return lastPosition;
}

void MechVentilation::_increaseInsuflationSpeed (byte factor)
{
    _stepperSpeed += factor;
}
void MechVentilation::_decreaseInsuflationSpeed (byte factor) 
{
    _stepperSpeed -= factor;
}
void MechVentilation::_increaseInsuflation (byte factor) 
{
    _positionInsufflated -= factor;
}
void MechVentilation::_decreaseInsuflation (byte factor) 
{
    _positionInsufflated += factor;
}
#endif

void MechVentilation::_setState(State state) {
    //_previousState = _currentState;
    _currentState = state;
}
