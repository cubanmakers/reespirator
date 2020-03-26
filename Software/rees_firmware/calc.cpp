#include "calc.h"

/**
 * @brief estima el volumen tidal en función de estatura y sexo, en ml.
 *
 * @param estatura en cm, del paciente
 * @param sexo 0: varón, 1: mujer, sexo del paciente
 * @return *volumenTidal volumen tidal estimado, en mililitros
 */
int calcularVolumenTidal(int estatura, int sexo) {
  float peso0, pesoIdeal, volumenEstimado;
  if (sexo == 0) { // Varón
    peso0 = 50.0;
  } else if (sexo == 1) { // Mujer
    peso0 = 45.5;
  }
  pesoIdeal = peso0 + 0.91 * (estatura - 152.4); // en kg

  return ((int)(round(pesoIdeal * DEFAULT_ML_POR_KG_DE_PESO_IDEAL)));
}



/**
 * @brief Constrains the value within the limits
 */
float constrainFloat(float value, float lowLimit, float highLimit) {
  if (value < lowLimit) {
    value = lowLimit;
  } else if (value > highLimit) {
    value = highLimit;
  }
  return value;
}

/**
 * PID step calculation
 */
float proportional = 0;
static float integral = 0;
float derivative = 0;
float previous_feedbackInput = 0;
float computePID(float setpoint, float feedbackInput) {
  //dt is set to TIME_BASE msec by timer interrupt

  float error = setpoint - feedbackInput;

  proportional = PID_KP * error;
  integral += PID_KI * error;
  integral = constrainFloat(integral, PID_MIN, PID_MAX);
  derivative = -PID_KD * (feedbackInput - previous_feedbackInput);

  float output = constrainFloat((proportional + integral + derivative), PID_MIN, PID_MAX);

  // Guardar ultimo tiempo y error
  previous_feedbackInput = feedbackInput;
  return output;
}

/**
 * @brief Refresca el WDT (Watch Dog Timer)
 */
void refreshWatchDogTimer() {
  //TODO implementar
}

// float flow2Position(float flow) { //returns position
//   //TODO implementar
//   return flow;
// }


float pos2vol(float position) { //converts position [steps] to volume [ml]
  float volume = position * K_POS2VOL;

  return volume;
}


void integratorFlowToVolume(float* currentVolume, float currentFlow) {
  //We add to currentVolume the ml that flowed in TIME_BASE msec;
  *currentVolume += currentFlow * 60 * TIME_BASE;
  //currentVolume += currentFlow * (1000/1) * (60/1) * (1/1000)   * TIME_BASE;
}                 //  [l/m]       [l]/[ml]  [min]/[s]  [s]/[msec]   [msec]


#if 0
/**
 * @brief Filtro de media móvil, de low-pass filter, para la diferencia de presiones
 *
 * @param parameter variable a filtrar
 * @param lpfArray array con muestras anteriores
 * @return float filtered value
 */
float computeLPF(int parameter, int lpfArray[])
{
  int samples = sizeof(lpfArray);
  int k = samples - 1;           // tamano de la matriz
  int cumParameter = parameter; // el acumulador suma ya el param
  lpfArray[0] = parameter;
  while (k > 0)
  {
    lpfArray[k] = lpfArray[k - 1];   // desplaza el valor una posicion
    cumParameter += lpfArray[k];     // acumula valor para calcular la media
    k--;
  }
  float result = cumParameter / samples;
  return result;
}
#endif

//float curveInterpolator(int[] curve, float min, float max, float currentProgressFactor);
//float curveInterpolator(float maxValue, float currentProgressFactor);
float curveInterpolator(float inValue, float currentProgressFactor) {
	float outValue = 0;

	if (currentProgressFactor < 0.03) {
		outValue = 0.7 * inValue;
	} else if ((currentProgressFactor >= 0.03) && (currentProgressFactor < 0.97))  {
		outValue = 1.2 * inValue;
	} else if (currentProgressFactor >= 0.97)  {
		outValue = 0.7 * inValue;
	}
		
	return outValue;
}

float flow2speed(float flow) { //converts flow [LPM] to speed [steps/sec]
  //float speed  = flow *  (1/60) * (1000/1) * (1/800)                 * (100/1);
  //     [steps/s]  [l/m]  [min/sec] [ml/l]    [fully_pressed_ambu/ml]   [steps/fully_pressed_ambu]
  //float position  = volume * (25/12);
  //  #define K_VOL2POS (STEPS_FOR_TOTALLY_PRESSED_AMBU / VOLUME_FOR_TOTALLY_PRESSED_AMBU)
  //  K_FLOW2SPEED = (25/12);
  
  float speed = flow * K_FLOW2SPEED;

  return speed;
}
