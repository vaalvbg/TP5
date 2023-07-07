#include <soporte_placa.h>
#include "usart_config.h"

/*Habilitar el reloj para USART*/
static void habilitar_reloj_usart (void){
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
}

static void habilitar_reloj_GPIOA (void){
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
}

static void usart_reset(void){
    RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
    RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
}
static void configuracion_usart(uint32_t baudrate){
    usart_reset();
    USART1->BRR = SystemCoreClock/baudrate;
    USART1->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE) ;    
    USART1->CR1 &= ~USART_CR1_M;  
    USART1->CR2 &=~(USART_CR2_STOP); 
}

void USART1_Init(uint32_t baudrate) {
    // Habilitar el reloj para USART1 y el GPIOA
    SystemCoreClockUpdate();
    habilitar_reloj_usart();
    habilitar_reloj_GPIOA();

    // Configurar pines para USART1 (PA9 como TX y PA10 como RX)
    SP_Pin_set_Modo(SP_PA10, SP_PIN_ENTRADA);

    // Configurar PA9 como salida funcion alternativa PUSH/PULL de velocidad media
    GPIOA->CRH = (GPIOA->CRH & ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9)) | (GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1); 

    // Configurar los parámetros del USART1
    configuracion_usart(baudrate);
}

void USART1_write(uint8_t c) {
    while(!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}


bool USART1_read(char *caracter) {
    bool bandera=false;
    if(USART1->SR & USART_SR_RXNE){
        *caracter= (char) USART1->DR;
        bandera=true;
    }
    return bandera;
}

/**
 * @brief Esta función lee caracteres a través de USART1 y los almacena en el buffer
 * -Verifica si se ha recibido el carácter de retorno de carro ('\r') o salto de línea ('\n') 
 para determinar el final de la cadena.
 * 
 * @note la funcion devuelve valores booleanos
 * -True si se recibio la cadena 
 * -False si el tamaño del buffer se alcanzo antes de recibir el final de la cadena
 */


bool USART1_Receive_String(char* buffer, uint32_t bufferSize){
    uint32_t i=0; 
    while(i<bufferSize-1){
        if(USART1_read(&buffer[i])){
            if(buffer[i]=='\r'){
                buffer[i]='\0'; 
                return true;
            }
            i++;
        }
    }
    buffer[i]='\0';
    return false;
}


/**
 * @brief Recorre la cadena de caracteres y envia caracter por caracter
 * a traves del USART1 utilizando la funcion USART_write()
 * 
 * @param string 
 */
void USART1_Write_String(const char* string){
    while(*string != '\0'){
        USART1_write(*string);
        string++;
    }
}