//----------------------------------------------------------------------------------------
// coremap.h
//
// Mapa de memoria para la práctica 4.
// El mapa indica a que página virtual de que proceso corresponde una página física.
//----------------------------------------------------------------------------------------


#if defined(PAGINATION)     

#ifndef COREMAP_H
#define COREMAP_H

#include "bitmap.h"                                                              
#include "syscall.h"
#include "list.h"


// Información relatica al un espacio de dirección..
class AddrSpaceInfo {

public:
  int virtualPage;
  SpaceId spaceId;
};

// Enumerado que define la política del SWAP.
enum Politic {
	
   NO_ONE, SECOND_CHANCE, FIFO

};


// CoreMap.
class CoreMap {

public:

  // Constructor 
  CoreMap(int numPhysPages, Politic politic);
  //Destructor.
  ~CoreMap();

  // Funciones de BitMap aggiornadas para nuestro CoreMap.
  void Mark(int which, SpaceId spaceId, int virtualPage);
  int Find(int virtualPage);
  int NumClear();
  void Clear(int which);
  bool Test(int which);

private:
   
  //XXX Uso interno.
  // Funciones nuevas relativas a CoreMap.
  SpaceId GetSpaceId(int arg);
  int GetVirtualPage(int arg);

  //Remove
  void RemoveFromList(int arg);
    
  // Politicas de swap.

  int useSecondChance();
  int useFifo();
        
private: 

  // Info relativa los espacios de direcciones.
  AddrSpaceInfo **spaceInfo;
  //Lista que mantiene el orden en el que fueron asignadas las páginas físicas.
  List<int> *fifoList;
  //N° de páginas
  int nPhysPages;
  //Bitmap asociado.
  BitMap *bitmap;
  //Política de Swap.
  Politic politic;
        
};


#endif // COREMAP_H
#endif // PAGINATION

