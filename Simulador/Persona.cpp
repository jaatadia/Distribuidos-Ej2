/* 
 * File:   persona.cpp
 * Author: knoppix
 *
 * Created on March 31, 2015, 4:50 PM
 */

#include <cstdlib>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sstream>
#include <unistd.h>
#include <signal.h>
#include "Logger.h"
#include "Simulador.h"


using namespace std;

/*
 * 
 */
#define PERSONA_ID "Persona"

void myHandler(int signal){
    
}

//argv[1] que puerta debe usar para entrar,argv[2] tiempo a dormir, argv[3] puerta para slir
int main(int argc, char** argv) {
    signal(SIGUSR1,myHandler);
    Logger::startLog(LOGGER_DEFAULT_PATH,PERSONA_ID);
    
    if (argc != 4 ){
        Logger::loggError("No se pasaron los parametros correctos 1: numero de puerta, 2: tiempo a dormri, 3: puerta para salir");
        exit(1);
    }
    
    
    int entrada = atoi(argv[1]);
    int dormir = atoi(argv[2]);
    int salida = atoi(argv[3]);
    
    stringstream ss;
    ss<<entrada;
    
    
    //busco las colas
    Logger::logg("Buscando la cola de entrada nro "+ss.str());
    int colaEntrada,colaEntradaRespuesta;
    if( (colaEntrada = msgget(ftok(DIRECTORIO_IPC,PUERTA_FILA + DESP * entrada),PERMISOS)) == -1){
        Logger::loggError("Error al encontrar la cola de entrada nro " +ss.str());
        exit(1);   
    }

    Logger::logg("Buscando la cola de respuesta"+ss.str());
    if( (colaEntradaRespuesta = msgget(ftok(DIRECTORIO_IPC,PUERTA_RESP + DESP * entrada),PERMISOS)) == -1){
        Logger::loggError("Error al encontrar la cola de respuesta nro "+ss.str());
        exit(1);   
    }
    
    
    //busco las colas de salida
    Logger::logg("Buscando la cola de entrada nro "+ss.str());
    int colaSalida,colaSalidaRespuesta;
    if( (colaSalida = msgget(ftok(DIRECTORIO_IPC,PUERTA_SALIDA_FILA + DESP * salida),PERMISOS)) == -1){
        Logger::loggError("Error al encontrar la cola de entrada nro " +ss.str());
        exit(1);   
    }

    Logger::logg("Buscando la cola de respuesta"+ss.str());
    if( (colaSalidaRespuesta = msgget(ftok(DIRECTORIO_IPC,PUERTA_SALIDA_RESP + DESP * salida),PERMISOS)) == -1){
        Logger::loggError("Error al encontrar la cola de respuesta nro "+ss.str());
        exit(1);   
    }
    
    int childpid;
    std::stringstream pid; pid<<":"<<getpid();
    if( ( childpid = fork() ) < 0 ){
        Logger::loggError("Error al atachearse a la memoria compartida");
        exit(1);   
    }else if(childpid==0){
        execlp(PATH_WAKER_EXEC,NAME_WAKER_EXEC,(PERSONA_ID+pid.str()).c_str(),COLA_MATAR_PERSONAS_STR,(char*)NULL);
    }
    
    
    Mensaje msg;
    msg.mensaje=getpid();
    Logger::logg("Enviando mensaje");
    if(msgsnd(colaEntrada,&msg,sizeof(Mensaje)-sizeof(long),0)==-1){
        Logger::loggError("Error al escribir el mensaje "+ss.str());
        exit(1);
    }
    
    Logger::logg("Esperando respuesta");
    if(msgrcv(colaEntradaRespuesta,&msg,sizeof(Mensaje)-sizeof(long),getpid(),0)==-1){
        Logger::loggError("Error al leer el mensaje ");
        exit(1);
    }
    
    if(msg.mensaje==MENSAJE_NO_PASAR){
        Logger::logg("El museo esta cerrado me voy");
        return 0;
    }
    
    
    Logger::logg("Entre al museo");
    int tiempo;
    if( (tiempo = sleep(dormir))!=0){
        Logger::logg("Me avisaron que salga");
    }
    
    msg.mensaje=getpid();
    Logger::logg("Enviando mensaje para salir");
    if(msgsnd(colaSalida,&msg,sizeof(Mensaje)-sizeof(long),0)==-1){
        Logger::loggError("Error al escribir el mensaje "+ss.str());
        exit(1);
    }
    
    Logger::logg("Esperando respuesta");
    if(msgrcv(colaSalidaRespuesta,&msg,sizeof(Mensaje)-sizeof(long),getpid(),0)==-1){
        Logger::loggError("Error al leer el mensaje ");
        exit(1);
    }
    
    if(tiempo != 0){kill(childpid,SIGUSR1);}
    
    Logger::logg("Sali del museo");
    
    return 0;
}
