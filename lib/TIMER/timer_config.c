#include "soporte_placa.h"
#include "timer_config.h"

#define ANCHO_PULSO_MIN 180 //Ancho de pulso minimo en microsegundos
#define ANCHO_PULSO_MAX 900 //Ancho de pulso maximo en microsegundos

/*Habilita el reloj para el TIM2*/

static void habilitar_reloj_TIM2(void){
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
}

/*Habilita el reloj */
static void habilitar_reloj_GPIOA(void){
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
}

#if 0
static void tim2_reset(void){
    RCC->APB1RSTR = RCC_APB1RSTR_TIM2RST;
    RCC->APB1RSTR =0;
}
#endif


static void pin_PA0_reset (void){
    GPIOA->BRR = GPIO_BRR_BR0;
}

/**
 * @brief Calcula y devuelve el valor del ciclo de trabajo en funcion de un angulo dado
 * @note retorna un entero sin signo de 32 bits que representa un angulo en grados
 * 
 * @param angulo 
 * @return uint32_t 
 */
static uint32_t determinar_ciclo_trabajo(uint32_t angulo){
    uint32_t valor= ANCHO_PULSO_MIN + ((ANCHO_PULSO_MAX-ANCHO_PULSO_MIN)*angulo)/180;
    return valor;
}

/**
 * @brief Calcula el ancho del pulso correspondiente 
 * @note asigna el ancho del pulso al registro CCR1 del temp. TIM2
 * 
 * @param angulo 
 */
void configurar_TIM2(int angulo){
    uint32_t ancho_pulso;
    ancho_pulso=determinar_ciclo_trabajo(angulo+90);
    TIM2->CCR1=ancho_pulso;
}


/*TIMER 2*/

void TIM2_init(void) {

    SystemCoreClockUpdate();
    habilitar_reloj_TIM2();
    habilitar_reloj_GPIOA();

    //Resetea el pin PA0
    pin_PA0_reset();

    //Habilitar pin PA0 como salida alternativa push/pull de velocidad media (2MHz)
    GPIOA->CRL = (GPIOA->CRH & ~(GPIO_CRL_MODE0 | GPIO_CRL_CNF0)) | (GPIO_CRL_MODE0_1 | GPIO_CRL_CNF0_1);
    
    //configuro ARR register (frecuencia) y preescaler
    TIM2->ARR = 7200; 
    TIM2->PSC = SystemCoreClock/360000; //para que la frecuencia del reloj sea 360KHz

    
    TIM2->CR1 = (TIM2->CR1 & ~TIM_CR1_DIR) | TIM_CR1_ARPE;
    
    //Habilita el contador del timer
    TIM2->CR1 |= TIM_CR1_CEN;

    //Configura modo PWM en Modo 1 
    TIM2->CCMR1 = (TIM2->CCMR1 & ~TIM_CCMR1_OC1M) | (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2) | TIM_CCMR1_OC1PE ;
  
    //configuro canal 1 como salida
    TIM2->CCER |= TIM_CCER_CC1E;

    configurar_TIM2(0);
}


/*Obtencion del Angulo*/

int32_t obtener_angulo(void){
    uint32_t ciclo_trabajo=TIM2->CCR1;
    int32_t angulo= ((ciclo_trabajo-ANCHO_PULSO_MIN)*180)/(ANCHO_PULSO_MAX-ANCHO_PULSO_MIN);
    return angulo-90;    
}