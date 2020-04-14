// processtable.cc
// 
// Estructura de datos para la manipulación de procesos (clave - valor).
//

#include "processtable.h"


//----------------------------------------------------------------------
//  ProcessTable::ProcessTable()
//   	
//         Constructor tabla de procesos.
//----------------------------------------------------------------------
ProcessTable::ProcessTable(){

 processList = new Process*[MAXPROCESS];

 //Inicialización.
 for(int i = 0; i < MAXPROCESS; i++){
  processList[i] = NULL;

 }

}

//----------------------------------------------------------------------
//  ProcessTable::~ProcessTable() 
//   	
//        Destructor
//----------------------------------------------------------------------
ProcessTable::~ProcessTable(){

   delete processList;

}

//----------------------------------------------------------------------
//   ProcessTable::addProcess(Thread *thread)
//   	
//         Agrega proceso a la tabla.
//----------------------------------------------------------------------
SpaceId ProcessTable::addProcess(Thread *thread){
  //printf("ProcessTable::addProcess()\n"); 

  int i = 0;

 //Busca el primero libre.
 while(processList[i] != NULL && i < MAXPROCESS)
 	i++;

 //Agregamos.
 if (i < MAXPROCESS) {
   Process* p = new Process;
   p-> thread = thread;
   p-> status = ALIVE;
   p-> exitValue = 0;
   processList[i]= p;
  
   return i;
 
}
  //Caso contrario false.
  return -1;

}

//----------------------------------------------------------------------
//   ProcessTable::removeProcess(SpaceId id, int exitValue)
//   	
//         Remueve proceso de la tabla.
//----------------------------------------------------------------------
bool ProcessTable::removeProcess(SpaceId id, int exitValue){
 //printf("ProcessTable::removeProcess()\n");

 //Controles.
 if(id>= MAXPROCESS || id<0 || processList[id] == NULL){
  return false; 
	}
  else if (processList[id]-> status == DEAD){
   return false;
}

//El proceso existe, marcamos como DEAD
 processList[id]-> status = DEAD;
 processList[id]-> exitValue = exitValue;

 return true;

}

//----------------------------------------------------------------------
// ProcessTable::getExitValue(SpaceId id, int& exitValue)
//   	
//          Devuelve valor de retorno.
//----------------------------------------------------------------------
bool ProcessTable::getExitValue(SpaceId id, int& exitValue){

 //Controles.
 if(id>= MAXPROCESS || id<0 || processList[id] == NULL){
   return false;
  }
 else if (processList[id]-> status == ALIVE){
   return false;
 }

  //Ok. 
  exitValue = processList[id] -> exitValue;
  delete processList[id];
  //Libero.
  processList[id] = NULL;


 return true;
}


//----------------------------------------------------------------------
//   ProcessTable::getThread(SpaceId id)
//   	
//                Getter proceso.
//----------------------------------------------------------------------
Thread* ProcessTable::getThread(SpaceId id){

if(id>= MAXPROCESS || id<0 || processList[id] == NULL){
  return NULL;

}

return processList[id] -> thread;
 
}

//----------------------------------------------------------------------
//   ProcessTable::getSpaceId(Thread *threadd)
//   	
//               Getter id
//----------------------------------------------------------------------
SpaceId ProcessTable::getSpaceId(Thread *threadd){
 

for(int i = 0; i < MAXPROCESS; i++){
 if (processList[i] != NULL && (processList[i]-> thread == threadd)){
       
       return i;
   }

}
return -1;
}


























