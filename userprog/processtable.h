// processtable.h
// 
// Estructura de datos para la manipulación de procesos, básicamente es un tabla de procesos con un id
// asociado.
//

#ifndef PROCESSTABLE_H
#define PROCESSTABLE_H

#include "syscall.h"
#include "thread.h"

//Máxima cantidad de procesos simultáneos.
#define MAXPROCESS 128  
// pag #define MAXPROCESS 64 //TODO
//Enumerado que decribe el estado del proceso.
enum ProcessStatus {ALIVE,DEAD};

//Estructura que describe el proceso
struct Process {
 Thread * thread;
 ProcessStatus status;
 int exitValue;

}; 

// Estructura de datos.
// Tabla de procesos más sus operaciones.

class ProcessTable {

public:
 

 ProcessTable();
 ~ProcessTable();

  
  // Agrega proceso, generándole un id. Retorna el susodicho id, o -1 en caso de falla.
  SpaceId addProcess(Thread* thread);
  // Remuevo proceso a partir de id.
  bool removeProcess(SpaceId id, int exitValue);

  // Valor  de retorno.
  bool getExitValue(SpaceId id, int& exitValue);

  // Getter del thread a partir de su id.
  Thread* getThread(SpaceId id);

  // Getter de id a partir del proceso.
  SpaceId getSpaceId(Thread* thread);

 private:
  
    //Tabla de procesos.
    Process** processList;

};

#endif



























