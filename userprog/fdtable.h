// fdtable.h
//
// Estructura de datos para la manipulación de archivos 
// Simplemente es un mapa clave -  valor entre un id y un
// archivo.
//
//
 
#ifndef FDTABLE_H
#define FDTABLE_H

#include "syscall.h"
#include "filesys.h"
#include "thread.h"
 
// Tamaño de la tabla.
#define FDTSIZE 128  
//PAG #define FDTSIZE 64 TODO 

class FDTable {

public: 
 
   FDTable();
   ~FDTable();

   // Getter del archivo a partir del id.
   OpenFile* getFile(OpenFileId id);
   
   // Agrega un archivo a la tabla asignándole un id único.
   // Retorna id, ó -1 en caso de no poder agregarlo.
   OpenFileId addFile(OpenFile *openfile);

   //Remueve el archivo de la estructura a partir de su id.
   bool removeFile(OpenFileId id);

private:
  
  OpenFile **fdt;
  Thread **threadOwner;


};

#endif
