// puerto.cc 
//	
//      Ejercicio 2 de práctica 2.
//



#include "puerto.h"
#include "system.h"


//----------------------------------------------------------------------------------------
// Puerto::Puerto
//
// Constructor
//----------------------------------------------------------------------------------------
Puerto::Puerto(const char* debugName) {
  //printf("Puerto::Puerto(...)\n");
  name = debugName;
  // Generamos el lock asociado al puerto. 
  int nameSize = strlen(name);
  lockName = new char[nameSize + 10];
  strcpy(lockName, name);
  strcat(lockName, ".lock");
  portLock = new Lock(lockName);

  // Generamos la variable de condicion para senders.
  senderConditionName = new char[nameSize + 10];
  strcpy(senderConditionName, name);
  strcat(senderConditionName, ".sndCond");
  senderCondition = new Condition(senderConditionName, portLock);

  // Generamos la variable de condicion para receivers.
  receiverConditionName = new char[nameSize + 10];
  strcpy(receiverConditionName, name);
  strcat(receiverConditionName, ".rcvCond");
  receiverCondition = new Condition(receiverConditionName, portLock);

  // 
  buffer = new List<int>;
  senderNumber = 0;
  receiverNumber = 0;
}


//----------------------------------------------------------------------------------------
// Puerto::~Puerto
//
// Destructor
//----------------------------------------------------------------------------------------
Puerto::~Puerto() {
  //printf("Puerto::~Puerto()\n");
  delete receiverCondition; 
  delete receiverConditionName;
  delete portLock; 
  delete lockName;
  delete senderCondition; 
  delete senderConditionName;
  delete buffer;
}


//----------------------------------------------------------------------------------------
// Port::Send(int msg)
//
// Envía el mensaje por el puerto.
//----------------------------------------------------------------------------------------
void Puerto::Send(int msg){
  //printf("Puerto::Send()\n");
  // Intentamos adquirir el lock del puerto.
  portLock->Acquire();
  // El mensaje se agrega al buffer una vez que fue adquirido
  senderNumber++;
  buffer->Append(msg);
  // Bloqueamos al thread llamante (no hay receptores).
  while (receiverNumber <= 0){
   senderCondition->Wait();
   }
  // Si hay receptor en espera lo despierto
  receiverNumber--;
  receiverCondition -> Signal();
  // Liberamos lock
  portLock -> Release();
}


//----------------------------------------------------------------------------------------
// Port::Receive(int *msg)
//
// Recibe el mensaje por el puerto..
//----------------------------------------------------------------------------------------
void Puerto::Receive(int *msg){
  //printf("Puerto::Receive()\n");
  // Intentamos adquirir el lock del puerto.
  portLock->Acquire();
  // Incrementamos el nro. de receptores, una vez adquirido
  receiverNumber++;
  // Bloqueamos thread (no hay emisores).
  while (senderNumber <= 0){
    receiverCondition->Wait();
   }
  // Recibimos el mensaje.
  *msg = buffer->Remove();
  
  // Si hay emisor en espera lo despierto
  senderNumber--;
  senderCondition->Signal();

  // Liberamos lock
  portLock->Release();
}
