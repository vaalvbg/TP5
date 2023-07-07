#include <soporte_placa/sp_tiempo.h>
#include <stdbool.h> // bool, true, false
#include <stdint.h>  // uint32_t
#include <stddef.h>  // size_t
#include <stm32f1xx.h> // __WFI

/* Temporización */

/**
 * @brief Variable actualizada una vez por milisegundo en el handler
 * de interrupción del timer del sistema (SysTick)
 * 
 */
static uint32_t volatile ticks;
//static uint32_t limiteRedondeo;

void SP_Tiempo_init(void){
    SystemCoreClockUpdate();
    uint32_t const cuentas_por_milisgundo = SystemCoreClock/1000;
    SysTick_Config(cuentas_por_milisgundo); // Configura SysTick y la interrupción
    
}

/*Delay*/
/**
 * @brief Crea un retraso en el programa durante un periodo de tiempo especificado
Verifica el tiempo transcurrido mediante la resta entre el valor actual de
"ticks" y "ticks_inicial"
 * 
 * @param tiempo 
 */
void SP_Tiempo_delay(uint32_t tiempo){
    uint32_t const ticks_inicial = ticks;
    uint32_t tiempo_transcurrido = ticks - ticks_inicial;
    while(tiempo_transcurrido < tiempo){
        __WFI();
        tiempo_transcurrido = ticks - ticks_inicial;
    }

}

#ifndef SP_MAX_TIMEOUTS
#define SP_MAX_TIMEOUTS 4
#endif

typedef struct SP_TimeoutDescriptor{
    uint32_t volatile tiempo;
    SP_TimeoutHandler volatile handler;
    void volatile *volatile  param;
} SP_TimeoutDescriptor;

SP_TimeoutDescriptor timeoutDescriptors[SP_MAX_TIMEOUTS];


bool SP_Tiempo_addTimeout(uint32_t const tiempo,SP_TimeoutHandler const handler,void volatile *const param){
    bool hecho = false;
    __disable_irq();
    for(size_t i=0;i<SP_MAX_TIMEOUTS;++i){
        SP_TimeoutDescriptor * const td = timeoutDescriptors + i;
        if (td->tiempo) continue;
        td->tiempo = tiempo;
        td->handler = handler;
        td->param = param;
        hecho = true;
        break;
    }
    __enable_irq();
    return hecho;
}

static void procesaTimeouts(void){
    for (size_t i=0;i<SP_MAX_TIMEOUTS;++i){
        SP_TimeoutDescriptor *const td = timeoutDescriptors + i;
        if (td->tiempo){
            const uint32_t tiempo_restante = --td->tiempo;
            if(!tiempo_restante && td->handler){
                td->handler(td->param);
            }
        } 
    }
}

void SysTick_Handler(void){
    ++ticks;
    procesaTimeouts();
}

uint32_t SP_Tiempo_getMilisegundos(void){
    return ticks;
}