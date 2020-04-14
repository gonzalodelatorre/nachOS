// fdtable.cc
//
// Estructura de datos para la manipulación de archivos 
// (mapa clave-valor)
//
//


#include "fdtable.h"
#include "system.h"


//----------------------------------------------------------------------
// FDTable::FDTable() 
//
//                   Constructor
//----------------------------------------------------------------------
FDTable :: FDTable(){

   fdt = new OpenFile*[FDTSIZE];
   threadOwner = new Thread*[FDTSIZE];

  //Inicialización
  for(int i = 0; i < FDTSIZE; i++){
     fdt[i] = NULL;
     threadOwner[i] = NULL;


   }

}

//----------------------------------------------------------------------
// FDTable::~FDTable()
//
//                    Destructor
//----------------------------------------------------------------------
FDTable::~FDTable(){

   delete fdt;
   delete threadOwner;


}

//----------------------------------------------------------------------
// FDTable :: getFile(OpenFileId id)
//
//                         Getter.
//----------------------------------------------------------------------

OpenFile* FDTable :: getFile(OpenFileId id){

 //printf("FDTable :: getFile()\n");

 // Condiciones.
 if (id < 2 || id >= FDTSIZE) {
    printf("id fuera de rango o incorrecto\n");
    return NULL;
   }
 else if (currentThread != threadOwner[id]) {
    printf("Thread no puede acceder al no ser dueño.\n"); 
    return NULL;
  }
 else if (fdt[id] == NULL){
    printf("No existe archivo con id indicado.\n");
    return NULL;
  }

  //Retornamos el archivo si todo está bien.
  return fdt[id];

}

//----------------------------------------------------------------------
// FDTable:: addFile(OpenFile *openfile)
//         
//                     Agrega un archivo a la tabla con id único.
//----------------------------------------------------------------------
OpenFileId FDTable:: addFile(OpenFile *openfile){


 //Id disponible. 1, 0 y -1 reservados.

 int i = 2;
 while(fdt[i] != NULL && i < FDTSIZE)
  i++;


  if (i < FDTSIZE) {
    //printf("Agregado\n"); 
    fdt[i] = openfile;
    threadOwner[i] = currentThread;
    return i;
  } 
      printf("No se pudo agregar file a la tabla.\n"); 
      return -1;
}

//----------------------------------------------------------------------
// FDTable::removeFile(OpenFileId id)
//
//                 Remueve archivo de la tabla
//----------------------------------------------------------------------
bool FDTable::removeFile(OpenFileId id){
 
   

   //Condiciones
   if (id < 2 || id >= FDTSIZE){
          printf("Descriptor fuera de rango o incorrecto\n");
          return NULL;
 } 
   else if (currentThread != threadOwner[id]){
    printf("Thread no puede acceder al no ser dueño.\n"); 
          return NULL;
  }
  else if (fdt[id] == NULL){
         printf("No existe archivo con id indicado\n"); 
        return NULL;
  }
  
  //Pasaron los chequeos.
  fdt[id] = NULL;
  threadOwner[id] = NULL;
   return true;

}






































