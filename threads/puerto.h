// puerto.h 
//	
//      Ejercicio 2 de práctica 2.
//


#include "synch.h"


class Puerto {

 public:
    //Constructor.
    Puerto(const char* debugName);
    // Destructor
    ~Puerto();
    //
    void Receive(int* msg);
    //
    void Send(int msg);
    //
    const char* getName() { return name; }

 private:
   // Nombre del puerto.
   const char* name; 
   // Lock del puerto.
   Lock* portLock; 
   // Variable de condición (emisor).
   Condition* senderCondition; 
   // Variable de condición (receptor).
   Condition* receiverCondition; 
   // Nombre del lock.
   char* lockName; 
   // Nombre de la variable de condicion para emisores.
   char* senderConditionName; 
   // Nombre de la variable de condicion para receptores.
   char* receiverConditionName;
   // N° emisores.
   int senderNumber;  
   // N° receptores.
   int receiverNumber;
   // Buffer de mensajes.
   List<int>* buffer; 
};









