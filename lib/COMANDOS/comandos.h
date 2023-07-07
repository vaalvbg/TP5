#ifndef COMANDOS_H
#define COMANDOS_H

#include "stm32f1xx.h"
#include <stdbool.h>

typedef enum{
    RST,
    SET_ANGLE,
    GET_ANGLE,
    DESCONOCIDO
}Comandos;

typedef struct{
    Comandos comando;
    int angle;
    bool comandoValido;
}Comando_SCPI;

/**
 * @brief Verifica y procesa comandos contenidos en un bufer de caracteres
 * 
 * @param buffer //Puntero a una cadena de caracteres
 * @param cmd  //Puntero a una estructura 
 * @return true 
 * @return false 
 */
bool verificar_comandos(char * buffer, Comando_SCPI * cmd);

#endif