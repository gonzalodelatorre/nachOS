//----------------------------------------------------------------------------------------
// coremap.cc 
//
// Implementación del mapa de memoria para la práctica 4.
// El mapa indica a que página virtual de que proceso corresponde una página física.
//
//----------------------------------------------------------------------------------------



#if defined(PAGINATION) 
#include "coremap.h"
#include "thread.h" 
#include "system.h"


//----------------------------------------------------------------------------------------
// CoreMap::CoreMap
//  
//  Crea un CoreMap que va a administrar las páginas fisicas. El segundo 
//  argumento se usa para identificar la política de swap que se va a utilizar.
//----------------------------------------------------------------------------------------
CoreMap::CoreMap(int numPhysPages, Politic politic_){
  // BitMap asociado.
  bitmap = new BitMap(numPhysPages);

  // Arreglo que almacena informacion de los espacios de direcciones asociados a
  // las páginas físicas.
  spaceInfo = new AddrSpaceInfo*[numPhysPages];

  for (int i = 0; i < numPhysPages; i++){
	spaceInfo[i] = NULL;
           }

  // Número de paginas fisicas para este CoreMap 
  nPhysPages = numPhysPages;
  // Política de swap para este CoreMap.
  politic = politic_;

  // Lista para mantener el orden en que fueron asignadas las páginas fisicas.
  fifoList = new List<int>();
}

//----------------------------------------------------------------------------------------
// CoreMap::~CoreMap
//
//  Destructor. 
//  Libera la memoria que le fue asignada al CoreMap.
//----------------------------------------------------------------------------------------
CoreMap::~CoreMap()
{
  // Libera la memoria asignada al BitMap.
  delete bitmap;

  // Libera la memoria asociada a la informacion de los espacios de direcciones.
  for (int i = 0; i < nPhysPages; i++){
	delete spaceInfo[i];
	spaceInfo[i] = NULL;
	}

  delete spaceInfo;

  // Borramos la lista que mantiene el orden.
  delete fifoList;
}

//----------------------------------------------------------------------------------------
// CoreMap::Find(int virtualPage)
//
// Busca entrada libre y la asigna a la página virtual. (En caso de no encontrar
// una entrada disponible devuelve false).
//----------------------------------------------------------------------------------------
int 
CoreMap::Find(int virtualPage) {
  //printf("CoreMap::Find(...)\n");
  int i = bitmap->Find();

  // i es negativo cuando todas las entradas están ocupadas, dicho de otra manera,
  // no entra en memoria
  // Se aplica una política para desocupar una de las entradas.

  if (i < 0){
	
       if (politic == NO_ONE){
	     //printf("Ninguna...\n");	
             return i;
		}
		
       else if (politic == FIFO) {
              // printf("Fifo...\n");	
               i = useFifo();
		}
       else if (politic == SECOND_CHANCE){
 	      // printf("Second Chance..\n");
               i = useSecondChance();
		}
	}

  SpaceId id = processTable -> getSpaceId(currentThread);
  Mark(i, id, virtualPage);

  // Asignamos la pagina física asignada a la lista.
  fifoList-> Append(i);

  return i;
}

//----------------------------------------------------------------------------------------
// CoreMap::Clear(int which)
//
// Libera argumento del CoreMap.
//----------------------------------------------------------------------------------------
void
CoreMap::Clear(int which){

  //printf("CoreMap::Clear(...)\n");
  // Lo borramos del mapa
  bitmap->Clear(which);

  delete spaceInfo[which];
  spaceInfo[which] = NULL;

  // Tambíen lo eliminamos de la lista.
  RemoveFromList(which);
}

//----------------------------------------------------------------------------------------
// CoreMap::Test(int which)
//
// Comprueba si el argumento está libre u ocupado en el CoreMap.
//----------------------------------------------------------------------------------------
bool 
CoreMap::Test(int which){
  //printf("CoreMap::Test(int which)\n");
  return bitmap->Test(which);
}

//----------------------------------------------------------------------------------------
// CoreMap::Mark(int which, SpaceId spaceId, int virtualPage)
//
// Asigna asigna el argumento which del CoreMap a la página virtual (virtualAddr) del espacio
// de direcciones con id igual a spaceId.
//----------------------------------------------------------------------------------------
void 
CoreMap::Mark(int which, SpaceId spaceId, int virtualPage){

  //printf("CoreMap::Mark(..)\n");
  bitmap->Mark(which);

  AddrSpaceInfo *asInfo = new AddrSpaceInfo();
  asInfo->spaceId = spaceId;
  asInfo->virtualPage = virtualPage;

  spaceInfo[which] = asInfo;
}

//----------------------------------------------------------------------------------------
// CoreMap::NumClear()
//
// Retorna el númerode entradas libres del CoreMap.
//----------------------------------------------------------------------------------------
int 
CoreMap::NumClear(){
  //printf("CoreMap::NumClear()\n");
  return bitmap->NumClear();
}

//----------------------------------------------------------------------------------------
// CoreMap::GetSpaceId(int arg)
//
// Devuelve el id asociado al argumento. False si está libre.
//----------------------------------------------------------------------------------------
SpaceId CoreMap::GetSpaceId(int arg){
     
  //printf("CoreMap::GetSpaceId(..)\n");
  ASSERT(arg >= 0 && arg < nPhysPages);

  if (Test(arg)){
	ASSERT(spaceInfo[arg] != NULL);
	return spaceInfo[arg]->spaceId;
  }
  else	{
	return -1;
	}
}

//----------------------------------------------------------------------------------------
// CoreMap::RemoveFromList(int arg)
//
// Elimina el elemento argumento de la lista.
//----------------------------------------------------------------------------------------
void 
CoreMap::RemoveFromList(int arg){
  //printf("CoreMap::RemoveFromList(..)\n");
  List<int> *auxList = new List<int>;

  int item = 0;
  
  // Vamos sacando hasta encotrar el que buscamos, lo agregamos en una lista aparte
  while ((!fifoList->IsEmpty()) && ((item = fifoList->Remove()) != arg)){
	auxList->Prepend(item);
	}
  // Los metemos de nuevo, conservando el orden 

  while (!auxList->IsEmpty()){
	item = auxList->Remove();
	fifoList->Prepend(item);
	}

   // Eliminamos lista auxiliar.
   delete auxList;
}


//----------------------------------------------------------------------------------------
// CoreMap::GetVirtualPage(int arg)
//
// Retorna la página virtual asociada al argumento which. False si está libre.
//----------------------------------------------------------------------------------------

int 
CoreMap::GetVirtualPage(int arg){
  //printf("CoreMap::GetVirtualPage(...)\n");
  ASSERT(arg >= 0 && arg < nPhysPages);

  if (Test(arg)){
     ASSERT(spaceInfo[arg] != NULL);
     return spaceInfo[arg]->virtualPage;
	}

      else{
	return -1;
	}
}

//----------------------------------------------------------------------------------------
// CoreMap::useFifo()
//
// Aplica FIFO para swaping. Devuelve número de página física liberada.
//----------------------------------------------------------------------------------------

int 
CoreMap::useFifo(){
  //printf("CoreMap::useFifo()\n");
  int out = fifoList->Remove();
  int ownerSpaceId = GetSpaceId(out);
  int ownerVirtualPage = GetVirtualPage(out);

  Thread *ownerThread = processTable -> getThread(ownerSpaceId);
  ASSERT(ownerThread);
  ownerThread -> space -> SaveOnSwapFile(ownerVirtualPage);

  return out;
}

//----------------------------------------------------------------------------------------
// CoreMap::useSecondChance()
//
// Aplica Second-Chance para swaping. Devuelve número de página física liberada.
//----------------------------------------------------------------------------------------
int 
CoreMap::useSecondChance(){
  // printf("CoreMap::useSecondChance()\n");
  int out;
  int ownerSpaceId;
  int  ownerVirtualPage;
  bool found = false; 
  Thread *ownerThread;
  TranslationEntry* entry;

  while (!found){
	out = fifoList->Remove();
	ownerSpaceId = GetSpaceId(out);
	ownerVirtualPage = GetVirtualPage(out); 
	ownerThread = processTable -> getThread(ownerSpaceId);

	ASSERT(ownerThread);
	entry = ownerThread->space->GetPage(ownerVirtualPage);

	if (entry->use){ 
		entry -> use = false; 
		fifoList->Append(out);
	}
	else  {
		found = true;
	}
  } 
  //On swap.
  ownerThread -> space -> SaveOnSwapFile(ownerVirtualPage);
  return out;
}



#endif //PAGINATION
