#include "comandos.h"
#include <soporte_placa.h>
#include <string.h>
#include <stdio.h>
#include "timer_config.h"
#include "usart_config.h"

/*Verificacion de Comandos*/
bool verificar_comandos(char * buffer, Comando_SCPI * cmd){
    if(strcmp(buffer,"reset")==0){ 
    
        cmd->comando=RST;
        cmd->angle=0;
        return true;

    }else 
        if(sscanf(buffer,"asignar angulo %d",&(cmd->angle))==1){   
        cmd->comando=SET_ANGLE;
         if(cmd->angle <-90 ||cmd->angle >90){
            cmd->comandoValido=false;
        }else{
            cmd->comandoValido=true;
        }
        return true;
    }else if(strcmp(buffer,"donde esta")==0){ 
        cmd->comando=GET_ANGLE;
        return true;
    }else{
        cmd->comando=DESCONOCIDO;
        return true;
    }
}