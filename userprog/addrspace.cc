// addrspace.cc
// Routines to manage address spaces (executing user programs).
//
// In order to run a user program, you must:
//
// 1. link with the -N -T 0 option
// 2. run coff2noff to convert the object file to Nachos format (Nachos object code
//    format is essentially just a simpler version of the UNIX executable object
//    code format)
// 3. load the NOFF file into the Nachos file system (if you haven't implemented the
//    file system yet, you don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "copyright.h"
#include "system.h"
#include "addrspace.h"


//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void SwapHeader(NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------------------------
// AddrSpace::TranslateMem(int virtualAddr, int* physicalAddr, int* offset)
//
// Convierte dirección virtual a dirección física.
//----------------------------------------------------------------------------------------

void 
AddrSpace::TranslateMem(int virtualAddr, int* physicalAddr, int* offset){
    //printf("AddrSpace::TranslateMem(..)\n");
    //Existencia
    ASSERT(pageTable != NULL);

    int numOfPage = virtualAddr / PageSize;
    int physicalPageNum = pageTable[numOfPage].physicalPage;
    // Offset     
    *offset = virtualAddr % PageSize;
    // Dirección física
    *physicalAddr = (physicalPageNum * PageSize) + *offset;

}

//----------------------------------------------------------------------------------------
// AddrSpace::CopySegment(Segment seg, OpenFile *executable)
//
// Copia segmente de programa de usuario a dirección física.
//----------------------------------------------------------------------------------------

void 
AddrSpace::CopySegment(Segment seg, OpenFile *executable){
    //printf("AddrSpace::CopySegment(..)\n");
    int segSize;
    int offset; 
    int sizeToCopy;
    int spaceLeftOnPage;
    int physicalAddr;

    // Tamaño de segmento.
    if (seg.size <= 0){
	return;
	}

    // Copia segmento en dirección física.
    segSize = seg.size;

    while (segSize > 0){
	  sizeToCopy = segSize < PageSize ? segSize : PageSize;
          TranslateMem(seg.virtualAddr, &physicalAddr, &offset);

       	  if (offset != 0){
		spaceLeftOnPage = PageSize - offset;
		sizeToCopy = sizeToCopy < spaceLeftOnPage ? sizeToCopy : spaceLeftOnPage;
		}


	executable->ReadAt(&(machine->mainMemory[physicalAddr]), sizeToCopy, seg.inFileAddr);

	seg.virtualAddr += sizeToCopy;
	seg.inFileAddr += sizeToCopy;
	segSize -= sizeToCopy;
	}
}

//----------------------------------------------------------------------------------------
// AddrSpace::PushArgsOnStack(int argc, char **argv)
//
// Pushea argumentos y mueve stack pointer. Retorna el stack pointer.
//----------------------------------------------------------------------------------------

int 
AddrSpace::PushArgsOnStack(int argc, char **argv){
	
    int stack_ptr, arg_len;
    int argv_ptr[argc];
    char *arg;

    // Puntero al final del stack.
    stack_ptr = numPages * PageSize;

    // Agregamos los argumentos en el stack y recalculamos el S.P.
    // argv_ptr[i] almacena la dirección en el stack del argumento i.

    for (int i = 0; i < argc; i++){
	  arg = argv[i];
	  arg_len = strlen(arg) + 1;
	  stack_ptr -= arg_len;

    for (int k = 0; k < arg_len; k++)
	  if (!machine->WriteMem(stack_ptr + k, 1, arg[k]))
	 	ASSERT(machine->WriteMem(stack_ptr + k, 1, arg[k]));

		argv_ptr[i] = stack_ptr;
	}

    // Reservamos espacio.
    stack_ptr -= 4 * (argc + 1);
    stack_ptr -= (stack_ptr % 4);

    // Agregamos los punteros a los argumentos en el stack.
    for (int i = 0; i < argc; i++)
	if (!machine->WriteMem(stack_ptr + (4 * i), 4, argv_ptr[i]))
		ASSERT(machine->WriteMem(stack_ptr + (4 * i), 4, argv_ptr[i]));

    // Agregamos  NULL al stack.
        if (!machine->WriteMem(stack_ptr + (4 * argc), 4, 0)){
		ASSERT(machine->WriteMem(stack_ptr + (4 * argc), 4, 0));
         }

    // S.P.
    return stack_ptr;
}

//----------------------------------------------------------------------------------------
// AddrSpace::AddrSpace
// Create an address space to run a user program. Load the program from a file
// "executable", and set everything up so that we can start executing user instructions.
//
// Assumes that the object code file is in NOFF format.
//
// First, set up the translation from program memory to physical memory. For now, this
// is really simple (1:1), since we are only uniprogramming, and we have a single
// unsegmented page table.
//
// "executable" is the file containing the object code to load into memory.
//----------------------------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{


//XXX Debug purposes.
    #if defined(DEMAND_LOADING)
    printf("DEMAND LOADING\n");
    #endif

    #if defined(PAGINATION)
    printf("PAGINATION\n");
    #endif

    #if !defined(DEMAND_LOADING) && !defined(PAGINATION)
    NoffHeader noffH;
    unsigned int pageOffset;
    int freeMemPageNum;
    #else

    execFile = executable;

    #endif


    unsigned int i, size;

    // Read the executable header.

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);

    if ((noffH.noffMagic != NOFFMAGIC) && (WordToHost(noffH.noffMagic) == NOFFMAGIC))
       	SwapHeader(&noffH);

    ASSERT(noffH.noffMagic == NOFFMAGIC);

    // How big is address space?
    // We need to increase the size to leave room for the stack.

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    // Check we're not trying to run anything too big -- at least until we have
    // virtual memory.

    #if !defined(PAGINATION)
    ASSERT(numPages <= (unsigned int)memoryBitMap->NumClear());
    #endif

    // Initialize the page status table.

    #if defined(DEMAND_LOADING) || defined(PAGINATION)
    pageStatus = new PageStatus[numPages]; 
    #endif

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
							numPages, size);

    // First, set up the translation.

    pageTable = new TranslationEntry[numPages];

    for (i = 0; i < numPages; i++) {

    #if !defined(DEMAND_LOADING) && !defined(PAGINATION)
    // Search for a free page in memory.

    freeMemPageNum = memoryBitMap->Find();
    if (freeMemPageNum < 0){
        DEBUG('a', "Error. No se pudo alocar pagina %d\n", i);
	break;
		}
    DEBUG('a', "Asignamos página física %d a página virtual %d\n", freeMemPageNum, i);
    // Setup the i-element of pageTable.

    pageTable[i].physicalPage = freeMemPageNum;
    pageTable[i].valid = true;
    #else

    pageTable[i].physicalPage = -1;
    pageTable[i].valid = false;
    pageStatus[i] = ON_BINARY;
    #endif

    pageTable[i].virtualPage = i;
    pageTable[i].use = false;
    pageTable[i].dirty = false;
    pageTable[i].readOnly = false; // If the code segment was entirely on a separate page, we could set its
    			           // pages to be read-only.

    #if !defined(DEMAND_LOADING) && !defined(PAGINATION)

    // Zero out 
    pageOffset = freeMemPageNum * PageSize;
    bzero(&machine->mainMemory[pageOffset], PageSize);
    #endif
	}

    #if !defined(DEMAND_LOADING) && !defined(PAGINATION)

    // Copio código en memoria.
    //printf("Copiando segmento CODE...\n");
    CopySegment(noffH.code, executable);
    //printf("Copiando segmento DATA...\n");
    CopySegment(noffH.initData, executable);

   #endif

    // Campos de Exec con argumentos.
    has_arguments = false;
    argc_real = 0;
    argv_real = NULL;

    // Campos asociados al archivo swap.
    #if defined(PAGINATION)
    swapName = NULL;
    swapFile = NULL;
    #endif
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------
AddrSpace::~AddrSpace(){

    for (int i = 0; i < argc_real; i++) {
	delete argv_real[i];
	}
    
    delete argv_real;

    // Eliminamos páginas en memoria.

    for (unsigned int i = 0; i < numPages; i++){
    #if defined(DEMAND_LOADING)
    if (pageStatus[i] == ON_MEMORY)
		memoryBitMap->Clear(pageTable[i].physicalPage);

    #elif defined(PAGINATION)
    if (pageStatus[i] == ON_MEMORY)
        memoryCoreMap->Clear(pageTable[i].physicalPage);

    #else
    memoryBitMap->Clear(pageTable[i].physicalPage);
    #endif
	}

    delete pageTable;

    // Cierro archivo ejecutable.
    #if defined(DEMAND_LOADING) || defined(PAGINATION)
    delete execFile;
    #endif

    // Elimono el archivo swap.
    #if defined(PAGINATION)
    delete swapName;
    delete swapFile;
    #endif
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------
void 
AddrSpace::InitRegisters(){
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start".

    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.

    machine->WriteRegister(NextPCReg, 4);

    // Pusheamos al stack si tiene argumentos.

    int sp = numPages * PageSize;

    if (has_arguments) {
	sp = PushArgsOnStack(argc_real, argv_real);
	machine->WriteRegister(4, argc_real);
	machine->WriteRegister(5, sp);
	}

    // Set the stack register to the end of the address space, where we allocated the
    // stack; but subtract off a bit, to make sure we don't accidentally reference off
    // the end!

    machine->WriteRegister(StackReg, sp - 16);
    DEBUG('a', "Initializing stack register to %d\n", sp - 16);
}


//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
    #ifdef USE_TLB
    for (int i = 0; i < TLBSize; i++)
	if (machine->tlb[i].valid && machine->tlb[i].dirty)
		pageTable[machine->tlb[i].virtualPage] = machine->tlb[i];
    #else
    
    pageTable = machine->pageTable;
    numPages = machine->pageTableSize;
    #endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
     #ifdef USE_TLB

    if (currentThread != tlbHandler->lastRelatedThread)
	for (int i = 0; i < TLBSize; i++)
		machine->tlb[i].valid = false;

    #else

    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;

    #endif
}

//----------------------------------------------------------------------------------------
// AddrSpace::SetArguments
//
// Seteamos argc y argv para este espacio.
//----------------------------------------------------------------------------------------

void 
AddrSpace::SetArguments(int argc, char **argv){
	
    //printf("AddrSpace::SetArguments()\n");
    has_arguments = true;
    argc_real = argc;
    argv_real = argv;
}

//----------------------------------------------------------------------------------------
// AddrSpace::GetPage
//
// Getter de la tabla.
//----------------------------------------------------------------------------------------

TranslationEntry* AddrSpace::GetPage(int numPage){
    //printf("AddrSpace::GetPage(..)\n");
    #if defined(DEMAND_LOADING) || defined(PAGINATION)

    PageStatus status = pageStatus[numPage];

    if (status == ON_MEMORY) {
      // printf("En memoria\n");
     }
    else if (status == ON_BINARY){
       //printf("Cargar en memoria\n");
	LoadPage(numPage);
	}
    #if defined(PAGINATION)
    else if (status == ON_SWAP_FILE){
       //printf("En swap\n");
       GetFromSwapFile(numPage);
	}
    #endif
    else{
    //printf("Error status");
	}
    #endif

    return &pageTable[numPage];
}

//----------------------------------------------------------------------------------------
// AddrSpace::LoadPage
//
// Carga página desde ejecutable a memoria física.
//----------------------------------------------------------------------------------------

 #if defined(DEMAND_LOADING) || defined(PAGINATION)

void 
AddrSpace::LoadPage(int page){
    //printf("AddrSpace::LoadPage(..)\n");
    #if defined(PAGINATION) 

    // Creamos el archivo swap.
    if (swapFile == NULL){
	  CreateSwapFile();
      }

    #endif

    // Página libre en la memoria fisica.

    #if defined(PAGINATION)
    int physPage = memoryCoreMap->Find(page); // CoreMap. XXX
    #else
    int physPage = memoryBitMap->Find();
    #endif

    //Condición.
    ASSERT(physPage >= 0 && physPage < NumPhysPages);

    // Inicializamos la memoria de la pagina fisica.
    bzero(&machine->mainMemory[physPage * PageSize], PageSize);

    // Cargamos datos 
    //printf("Loading...\n"); //XXX

    int pMemPtr = physPage * PageSize;
    int vMemPtr = page * PageSize;
    int inFileAddr = 0;

    while (vMemPtr < (page + 1) * PageSize){

    if (vMemPtr >= noffH.code.virtualAddr && vMemPtr < noffH.code.virtualAddr + noffH.code.size){
	inFileAddr = noffH.code.inFileAddr + vMemPtr - noffH.code.virtualAddr;
	execFile->ReadAt(&(machine->mainMemory[pMemPtr]), 1, inFileAddr);
		}
    else if (vMemPtr >= noffH.initData.virtualAddr && vMemPtr < noffH.initData.virtualAddr + noffH.initData.size) {
          inFileAddr = noffH.initData.inFileAddr + vMemPtr - noffH.initData.virtualAddr;
	   execFile->ReadAt(&(machine->mainMemory[pMemPtr]), 1, inFileAddr);
		}
	else{
  		machine->mainMemory[pMemPtr] = 0;
		}

    pMemPtr++;
    vMemPtr++;
	}  

    //Configuracion página.
    pageTable[page].physicalPage = physPage;
    pageTable[page].valid = true;
    pageStatus[page] = ON_MEMORY; 

}

#endif

//----------------------------------------------------------------------------------------
// AddrSpace::CreateSwapFile()
//
// Crea archivo swap para este adress space.
//----------------------------------------------------------------------------------------

#if defined(PAGINATION)

void 
AddrSpace::CreateSwapFile(){
    //printf("AddrSpace::CreateSwapFile()\n");
    char aux[16];
    swapName = new char[16];
    strcpy(swapName,"SWAP.");
    sprintf(aux,"%d", processTable -> getSpaceId(currentThread));
    strcat(swapName, aux);

    // Nuevo archivo swap
    int size = numPages * PageSize;
    fileSystem -> Create(swapName, size);
    swapFile = fileSystem->Open(swapName);

   // Initializes the swap file.
    for (int i = 0; i < size; i++){
		swapFile->WriteAt("0", 1, i);
      }
}

#endif

//----------------------------------------------------------------------------------------
// AddrSpace::GetFromSwapFile(int numPage)
//
// Carga página desde archivo swap a memoria.
//----------------------------------------------------------------------------------------

#if defined(PAGINATION)

void 
AddrSpace::GetFromSwapFile(int numPage){
    // printf("AddrSpace::GetFromSwapFile()\n");
    // Buscamos una página libre.
    int physPage = memoryCoreMap->Find(numPage);
    //Condición
    ASSERT(physPage >= 0 && physPage < NumPhysPages);

    // Valores para la memoria de la pagina física.
    int physAddr = physPage * PageSize;
    int swapAddr = numPage * PageSize;
    // Zero out.
    bzero(&machine->mainMemory[physAddr], PageSize);

    // Cargamos desde el archivo swap.
    swapFile->ReadAt(&(machine->mainMemory[physAddr]), PageSize, swapAddr);

    // Actualizamos estado 
    pageTable[numPage].physicalPage = physPage;
    pageTable[numPage].valid = true;
    //
    pageStatus[numPage] = ON_MEMORY;
}

#endif

//----------------------------------------------------------------------------------------
// AddrSpace::SaveOnSwapFile(int numPage)
//
// Grabamos página en archivo swap.
//----------------------------------------------------------------------------------------

#if defined(PAGINATION)

void 
AddrSpace::SaveOnSwapFile(int numPage){
    // printf("AddrSpace::SaveOnSwapFile()\n");       
    int physPage = pageTable[numPage].physicalPage;
    int physAddr = physPage * PageSize;
    int swapAddr = numPage * PageSize;
    int i = 0;

    swapFile->WriteAt(&(machine->mainMemory[physAddr]), PageSize, swapAddr);
    //On swap.
    pageStatus[numPage] = ON_SWAP_FILE;
    memoryCoreMap->Clear(physPage);
  
    // Se marca como inválida si la página virtual que estaba en swap estaba tambien en TLB
    // Si la entrada en la TLB estaba dirty, llevamos los cambios desde la TLB a la pageTable.

    while (i < TLBSize && machine->tlb[i].physicalPage != physPage)
 	i++;

    if (i < TLBSize){
	if (machine->tlb[i].valid && machine->tlb[i].dirty)
		pageTable[machine->tlb[i].virtualPage] = machine->tlb[i];

	machine->tlb[i].valid = false;
	}
}

#endif

