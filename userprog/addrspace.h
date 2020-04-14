// addrspace.h
// Data structures to keep track of executing user programs (address spaces).
//
// For now, we don't keep any information about address spaces. The user level CPU state
// is saved and restored in the thread executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "noff.h"

#define UserStackSize		1024 	// Increase this as necessary!

// Enum para indicar el estatus de una pagina

#if defined(DEMAND_LOADING) || defined(PAGINATION)
enum PageStatus { 

  ON_MEMORY, ON_BINARY, ON_SWAP_FILE

};

#endif


class AddrSpace {
  public:

    AddrSpace(OpenFile *executable);    // Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code

    void SaveState();		        // Save/restore address space-specific
    void RestoreState();		// info on a context switch 
	

    void SetArguments(int argc, char **argv); // Seteamos argumentos a este espacion de dirección.

    char** GetArguments() { return argv_real; } // Getter

    TranslationEntry* GetPage(int numPage);     // Obtiene entrada de la page table
                                                // de este adress space.

  private:

    void TranslateMem(int virtualAddr, int* physicalAddr, int* offset);
    void CopySegment(Segment seg, OpenFile *executable);

    int PushArgsOnStack(int argc, char **argv); // Pusheamos argumentos al stack.

    // Common fields.

    TranslationEntry *pageTable;	// Assume linear page table translation 
                                        // for now!
    unsigned int numPages;		// Number of pages in the virtual 
                                        // address space.

    // Estos campos son usados para el Exec con argumentos de la Pr3.

    bool has_arguments;	  // Address space con argumentos.
    int argc_real;	  // Número de argumentos.
    char **argv_real;	  // Arreglo de argumentos.

    //-------------------
    // XXX Carga por demanda.
    //-------------------

    #if defined(DEMAND_LOADING) || defined(PAGINATION)

    void LoadPage(int pageToBeLoaded); // XXX Carga página.

    NoffHeader noffH;	           // Noff header ejecutable.
    PageStatus *pageStatus;	   // Tabla de status.
    OpenFile *execFile;	           // Ejecutable.

    #endif
 
    //-------------------
    // XXX Paginación.
    //-------------------

    #if defined(PAGINATION) 

    void CreateSwapFile();  // Crear archivo swap.
    OpenFile *swapFile;	// Archivo swap.
    char *swapName;		// Nombre de archivo swap

  public:
    
    void GetFromSwapFile(int numPage);  // Trae desde swap.
    void SaveOnSwapFile(int numPage);   // Graba a swap

#endif
};


#endif // ADDRSPACE_H

