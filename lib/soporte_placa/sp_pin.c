#include <soporte_placa/sp_pin.h>
#include <stm32f1xx.h>

/* GPIO */

typedef struct Pin{
    GPIO_TypeDef * puerto;
    int nrPin;
    void (*init_especial)(void);
}Pin;

// Inicialización especial luego de reset para poder usar PA15 y PB3/7
static void desactivarJtag(void);

/**
 * @brief pines es un arreglo de 32 elementos, donde cada uno es tipo Pin (Con su puntero a puerto y numero)
 * 
 */

static Pin const pines[SP_NUM_PINES] = {
    [SP_PA0 ] = {.puerto = GPIOA, .nrPin = 0 },
    [SP_PA1 ] = {.puerto = GPIOA, .nrPin = 1 },
    [SP_PA2 ] = {.puerto = GPIOA, .nrPin = 2 },
    [SP_PA3 ] = {.puerto = GPIOA, .nrPin = 3 },
    [SP_PA4 ] = {.puerto = GPIOA, .nrPin = 4 },
    [SP_PA5 ] = {.puerto = GPIOA, .nrPin = 5 },
    [SP_PA6 ] = {.puerto = GPIOA, .nrPin = 6 },
    [SP_PA7 ] = {.puerto = GPIOA, .nrPin = 7 },
    [SP_PA8 ] = {.puerto = GPIOA, .nrPin = 8 },
    [SP_PA9 ] = {.puerto = GPIOA, .nrPin = 9 },
    [SP_PA10] = {.puerto = GPIOA, .nrPin = 10},
    [SP_PA11] = {.puerto = GPIOA, .nrPin = 11},
    [SP_PA12] = {.puerto = GPIOA, .nrPin = 12},
    [SP_PA15] = {.puerto = GPIOA, .nrPin = 15, .init_especial = desactivarJtag},
    [SP_PB0 ] = {.puerto = GPIOB, .nrPin = 0 },
    [SP_PB1 ] = {.puerto = GPIOB, .nrPin = 1 },
    [SP_PB3 ] = {.puerto = GPIOB, .nrPin = 3 , .init_especial = desactivarJtag},
    [SP_PB4 ] = {.puerto = GPIOB, .nrPin = 4 , .init_especial = desactivarJtag},
    [SP_PB5 ] = {.puerto = GPIOB, .nrPin = 5 },
    [SP_PB6 ] = {.puerto = GPIOB, .nrPin = 6 },
    [SP_PB7 ] = {.puerto = GPIOB, .nrPin = 7 },
    [SP_PB9 ] = {.puerto = GPIOB, .nrPin = 9 },
    [SP_PB8 ] = {.puerto = GPIOB, .nrPin = 8 },
    [SP_PB10] = {.puerto = GPIOB, .nrPin = 10},
    [SP_PB11] = {.puerto = GPIOB, .nrPin = 11},
    [SP_PB12] = {.puerto = GPIOB, .nrPin = 12},
    [SP_PB13] = {.puerto = GPIOB, .nrPin = 13},
    [SP_PB14] = {.puerto = GPIOB, .nrPin = 14},
    [SP_PB15] = {.puerto = GPIOB, .nrPin = 15},
    [SP_PC13] = {.puerto = GPIOC, .nrPin = 13},
    [SP_PC14] = {.puerto = GPIOC, .nrPin = 14},
    [SP_PC15] = {.puerto = GPIOC, .nrPin = 15},
};


static void desactivarJtag(void){
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    AFIO->MAPR = (AFIO->MAPR & ~(AFIO_MAPR_SWJ_CFG_Msk)) | AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
    RCC->APB2ENR &= ~RCC_APB2ENR_AFIOEN;
}

/**
 * @brief Obtiene un puntero a Pin a partir de su handle. 
 * A la dirección de memoria del elemento hPin-ésimo del arreglo pines, la almacena en un puntero a Pin
 * 
 * @param hPin Handle. Número que indica que pin es dentro de SP_Pines.
 * @return Pin const* Puntero al objeto Pin (solo lectura, contiene puerto y numero) 
 */
static Pin const * pinDeHandle(SP_HPin hPin){
    return &pines[hPin];
}


/**
 * @brief Habilita el reloj de un puerto GPIO
 * @note Debe ser llamada antes de intentar usar el puerto
 * por primera vez.
 * @param puerto Puntero al puerto GPIO 
 * @return uint32_t Máscara de habilitación de reloj
 */
static void habilitaRelojPuerto(GPIO_TypeDef const *puerto){
    int const offset_habilitacion = (((uint32_t)(puerto) >> 10) & 0xF);
    RCC->APB2ENR |= 1 << offset_habilitacion;
}



/**
 * @brief Escribe los bits de modo en la posición adecuada
 * de CRL o CRH según el pin
 * 
 * @param pin Puntero a una variable tipo Pin (Contiene puntero a puerto y numero)
 * @param bits_modo Contiene los bits que deseo esribir en los registros
 */
static void config_modo(Pin const *pin, int bits_modo){
    
    int const offset = (pin->nrPin % 8)*4; 
    uint32_t const mascara = 0xF;  
    if (pin->nrPin < 8){          
        pin->puerto->CRL =  (pin->puerto->CRL & ~(mascara << offset)) //Pongo los bits en 0
                          | ((bits_modo & mascara)<<offset); //Pongo el valor de bits modo
    }else{ 
        pin->puerto->CRH =  (pin->puerto->CRH & ~(mascara << offset)) //Pongo los bits en 0
                          | ((bits_modo & mascara)<<offset); //Pongo el valor de bits modo
    }
}





/*Modos*/

void SP_Pin_set_Modo(SP_HPin hPin,SP_Pin_Modo modo){
    
    enum ConfigsPin{
        /** 
         * Bits[1:0]: Modo E/S, 00 es modo entrada
         * Bits[3:2]: Configuración de entrada, 01 es entrada flotante
         */
        ENTRADA_FLOTANTE = 0b0100,
        /** 
         * Bits[1:0]: Modo E/S, 00 es modo entrada
         * Bits[3:2]: Configuración de entrada, 10 es entrada con pull-up/pull-dn
         */
        ENTRADA_PULLUP_PULLDN = 0b1000,
        /** 
         * Bits[1:0]: Modo E/S, 10 es modo salida con frec. máxima de 2MHz
         * Bits[3:2]: Configuración de salida, 00 es salida de propósito general normal (push/pull)
         */
        SALIDA_2MHz = 0b0010,
        /** 
         * Bits[1:0]: Modo E/S, 10 es modo salida con frec. máxima de 2MHz
         * Bits[3:2]: Configuración de salida, 01 es salida de propósito general open drain
         */
        SALIDA_2MHz_OPEN_DRAIN = 0b0110

        
    };
    
    Pin const *self = pinDeHandle(hPin); 
    __disable_irq(); // deshabilita las interrupciones durante la ejecucion de la funcion
                    // esto evita que las interrupciones afecten la config. del pin.
    
    habilitaRelojPuerto(self->puerto); // la funcion habilitaRelojPuerto habilita el reloj del puerto al que esta
                                       // conectado el pin.

    if (self->init_especial) self->init_especial(); //Desactivo Jtag si es necesario
    
    switch (modo)
    {
    case SP_PIN_ENTRADA:
        config_modo(self,ENTRADA_FLOTANTE);
    break;
    
    case SP_PIN_ENTRADA_PULLUP:
        config_modo(self,ENTRADA_PULLUP_PULLDN);
        SP_Pin_write(hPin,1);
    break;
    
    case SP_PIN_ENTRADA_PULLDN:
        config_modo(self,ENTRADA_PULLUP_PULLDN);
        SP_Pin_write(hPin,0);
    break;
    
    case SP_PIN_SALIDA:
        config_modo(self,SALIDA_2MHz);
    break;
    
    case SP_PIN_SALIDA_OPEN_DRAIN:
        config_modo(self,SALIDA_2MHz_OPEN_DRAIN);
    break;
    default:
    // Debiera generar un error
    break;
    }
    __enable_irq();
}



/*Lectura*/
bool SP_Pin_read(SP_HPin hPin){
    return pines[hPin].puerto->IDR & (1 << pines[hPin].nrPin); //Accedo al registro IDR  del pin, y leo el nrPin-ésimo bit
}


/*Escritura*/
void SP_Pin_write(SP_HPin hPin, bool valor){
    enum {PIN_SET = 0,PIN_RESET = 16}; 
    pines[hPin].puerto->BSRR = 1 << (pines[hPin].nrPin + ((valor)? PIN_SET:PIN_RESET)); 
}
