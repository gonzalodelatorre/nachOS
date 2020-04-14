// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "syscall_helper.h"
#include "filesys.h"
#include "addrspace.h"
#include <map> 


#define NEWFILESIZE 1024

//Exec con args
#define MAXARGS 10


//----------------------------------------------------------------------
// IncreaseProgramCounter()
//   	
//         Incrementa el program counter.
//----------------------------------------------------------------------
void IncreaseProgramCounter(){
  
  int regVal;
  regVal = machine -> ReadRegister(PCReg);    //Current program counter
  machine -> WriteRegister(PrevPCReg,regVal); // Previous program counter (for debugging)
  regVal = machine -> ReadRegister(NextPCReg);// Next program counter (for branch delay) 
  machine -> WriteRegister(PCReg, regVal);
  regVal += 4;
  machine -> WriteRegister(NextPCReg, regVal);

}


//----------------------------------------------------------------------
// CommandParser(char *inputCmd, char **filePath, char **argv)
//   	      
// Parser de comandos utilizado por exec con argumentos. Parsea strings de
// la forma "path arg1 arg2 ... argn"
//----------------------------------------------------------------------


int CommandParser(char *inputCmd, char **filePath, char **argv)
{

 int argc = 0;
 char *token = NULL;
 // filePath
 token = strtok(inputCmd, " \n");

 //printf("TOKEN_1: %s\n", token);XXX
 if (token == NULL){
     return -1;
   }
 *filePath = new char[strlen(token) + 1];
 strncpy(*filePath, token, strlen(token) + 1);
 // argumentos
 while ((token = strtok(NULL, " \n")) != NULL) {
  //printf("TOKEN: %s\n", token); XXX
  argv[argc] = new char[strlen(token) + 1];
  strncpy(argv[argc], token, strlen(token) + 1);
  argc++;
}
 
   return argc;
}








//----------------------------------------------------------------------
// processCreator(void *arg)
//   	
//         
//----------------------------------------------------------------------
void processCreator(void *arg){
 //printf("ProcessCreator()\n");

 currentThread -> space -> RestoreState();
 currentThread -> space -> InitRegisters(); 

 machine -> Run();

 ASSERT(false);
}


//----------------------------------------------------------------------
// SysCall_Halt()
//
//          Halt system call.
//----------------------------------------------------------------------
void SysCall_Halt() {
  
  //printf("SysCall_Halt()\n");  
  interrupt->Halt();
}

//----------------------------------------------------------------------
// SysCall_Create()
//
//----------------------------------------------------------------------
void SysCall_Create(){
  
  //printf("SysCall_Create()\n");  
  int arg1 = machine -> ReadRegister(4); 
  
  int stringLength = getStrLengthFromUsr(arg1);
  char *fileName = new char[stringLength +1];
  readStrFromUsr(arg1, fileName);

  //filesys.h
  bool opRes = fileSystem -> Create(fileName, NEWFILESIZE);
  if (opRes){
     // printf("Archivo creado...\n");
     machine -> WriteRegister(2,0);
     
     }
  else {
     printf("Falla al intentar crear archivo\n");
     machine -> WriteRegister(2,-1);
     }

  delete fileName;
}



//----------------------------------------------------------------------
// SysCall_Open()
//
//             Open system call.
//----------------------------------------------------------------------
void SysCall_Open(){

 // printf("SysCall_Open()...\n "); 
 int  arg1 = machine -> ReadRegister(4); 
 int stringLength = getStrLengthFromUsr(arg1);

 char *fileName = new char[stringLength +1];
 readStrFromUsr(arg1, fileName);
   
  
  //filesys.h 
  OpenFile *file = fileSystem -> Open(fileName);


  if (file == NULL){
     printf("No existe el archivo\n");
     machine -> WriteRegister(2,-1);
     }
  else {
     printf("El Archivo se abrió exitosamente\n");
     OpenFileId id = fileDescriptorTable -> addFile(file);
     machine -> WriteRegister(2,id); 
     }

  delete fileName;


}

//----------------------------------------------------------------------
// SysCall_Read()
//
//            Read system call.
//----------------------------------------------------------------------
void SysCall_Read(){ 
  // printf("SysCall_Read()\n");
   
   int usrBuffAddr = machine -> ReadRegister(4);
   int sizeToRead = machine -> ReadRegister(5);
   OpenFileId fileDescriptor = machine -> ReadRegister(6);
 
   char *readBuff = new char[sizeToRead + 1];

   if(fileDescriptor == ConsoleInput){

    //  printf("Reading...\n");
      synchConsole -> GetBuffer(readBuff,sizeToRead);
      writeBuffToUsr(readBuff, usrBuffAddr, sizeToRead);
      machine -> WriteRegister(2, sizeToRead);
    }

   else {
     OpenFile * file = fileDescriptorTable -> getFile (fileDescriptor);

       if(file == NULL) {
          printf("Error. No existe archivo para este id\n"); 
          machine -> WriteRegister(2, -1);
		}      
       else {
          int charsReaded = file -> Read(readBuff, sizeToRead);
          readBuff[charsReaded] = '\0';
          writeBuffToUsr(readBuff, usrBuffAddr, charsReaded);
          machine -> WriteRegister(2,charsReaded);

   } 
 }
  delete readBuff;

}


//----------------------------------------------------------------------
// Syscall_Write()
//
//                Write system call.
//----------------------------------------------------------------------
void SysCall_Write(){ 
   //printf("SysCall_Write()\n");

   int usrBuffAddr = machine -> ReadRegister(4);
   int sizeToWrite = machine -> ReadRegister(5);

   OpenFileId fileDescriptor = machine -> ReadRegister(6);

   //Buffer para caracteres.
   char *writeBuff = new char[sizeToWrite + 1];
   

  if(fileDescriptor == ConsoleInput) {
    //printf("Warning\n");
    machine -> WriteRegister(2, -1);

   }


  else if(fileDescriptor == ConsoleOutput){

     //printf("Writing..\n");
     readBuffFromUsr(usrBuffAddr, writeBuff, sizeToWrite);
     synchConsole -> PutBuffer(writeBuff,sizeToWrite);
     machine -> WriteRegister(2, 0);
    }
 
    else {
   
        OpenFile *file = fileDescriptorTable -> getFile(fileDescriptor);
      
     if (file == NULL) {
          printf("Error. No existe archivo para este id\n"); 
          machine -> WriteRegister(2, -1);
         }
     else {
     
     readBuffFromUsr(usrBuffAddr, writeBuff, sizeToWrite);
     writeBuff[sizeToWrite] = '\0';
     int charsWritten = file -> Write(writeBuff, sizeToWrite);
     if (charsWritten < sizeToWrite){
       // printf("Warning\n"); 
       }
      machine -> WriteRegister(2, 0);
         }
   }
  delete writeBuff;

}


//----------------------------------------------------------------------
//   SysCall_Close()
//   	
//                Close system call.
//----------------------------------------------------------------------
void SysCall_Close(){

 // printf("SysCall_Close()\n");  


  OpenFileId id = (OpenFileId) machine -> ReadRegister(4); 

  OpenFile *file = fileDescriptorTable -> getFile(id);

  if(file ==  NULL){
   printf("No existe file para ese id\n");
   machine -> WriteRegister(2,-1);
   } 
  else{
   fileDescriptorTable -> removeFile(id);
   delete file;
   printf("Archivo removido exitosamente\n");
   machine -> WriteRegister(2,0);
  }
 
}


//----------------------------------------------------------------------
// SysCall_ExecNew()
//   	
//         Exec system call con argumentos.
//----------------------------------------------------------------------
void SysCall_ExecNew(){
   // printf("SysCall_ExecNew()\n");

    int cmdStrAddr = machine -> ReadRegister(4);

    // Comando a ejecutar desde espacio de usuario.
    int strLen = getStrLengthFromUsr(cmdStrAddr);
    char *cmdStr = new char[strLen + 1];
    readStrFromUsr(cmdStrAddr, cmdStr);

    //Obtenemos el path del file.
    char *filePath = NULL;
    //Obtenemos los argumentos.
    char **argv = new char *[MAXARGS];  

    int argc = CommandParser(cmdStr, &filePath, argv);



    if(argc < 0){
      printf("No se pudo parsear el comando\n"); 
      delete cmdStr;
      delete argv;
      machine -> WriteRegister(2, -1);
      return;
     }
    else{
      delete cmdStr;

    }
    
    //Abre ejecutable
    OpenFile *execFile = fileSystem -> Open(filePath);
 
    if(execFile == NULL){
     printf("No se pudo abrir el archivo\n"); 
     delete filePath;
     for(int i = 0; i < argc; i++){
       delete argv[i];
     }
     delete argv;
     machine -> WriteRegister(2, -1);
     return;
    }

    printf("El archivo se abrió satisfactoriamente\n"); 
    //Creamos espacio de memoria.
    AddrSpace *addrSpace = new AddrSpace(execFile);
    

#if !defined(DEMAND_LOADING) && !defined(PAGINATION) //XXX 
	delete execFile;
#endif
    if(addrSpace == NULL){
     printf("No se pudo crear un espacio de memoria\n");
     delete filePath;
     for(int i = 0; i < argc; i++){
        delete argv[i];
      }
     delete argv;
     machine -> WriteRegister(2, -1);
     return;
     }
     
     addrSpace -> SetArguments(argc, argv);  

    //Nuevo thread.
    Thread * thread = new Thread(filePath, true);

  if(thread == NULL){
   printf("No se pudo crear thread\n"); 
   delete filePath;
   for(int i = 0; i < argc; i++){
    delete argv[i];
   }
   delete argv;
   delete addrSpace;
   machine -> WriteRegister(2, -1);
   return;
 }

 thread -> space = addrSpace;

 //Agregamos thread a la tabla de procesos
 SpaceId id = processTable -> addProcess(thread);

 if (id < 0){
    printf("No se pudo agregar el thread a la tabla de procesos\n"); 
      delete filePath;
  for(int i = 0; i < argc; i++){
    delete argv[i];
   }
   delete argv;
   delete addrSpace;
   delete thread;
   machine -> WriteRegister(2, -1);
   return;
   }

 machine -> WriteRegister(2,id);
 //Fork del thread.
 thread -> Fork(processCreator,NULL);


 return;

}



//----------------------------------------------------------------------
// SysCall_Exec()  
//   	
//         Exec system call
//----------------------------------------------------------------------
void SysCall_Exec(){
  
  //printf("SysCall_Exec()\n");  
  int arg1 = machine -> ReadRegister(4); 
  
  int stringLength = getStrLengthFromUsr(arg1);
  char *filename = new char[stringLength +1];
  readStrFromUsr(arg1, filename);

  OpenFile *executable = fileSystem -> Open (filename);
  if (executable == NULL) {
   printf("No se pudo abrir el archivo\n");
   delete filename;
   machine -> WriteRegister(2,-1);
   return;
  } 

  AddrSpace *addrSpace;
  addrSpace = new AddrSpace(executable);
  delete executable;
  
  if (addrSpace == NULL){
    printf("No se pudo crear dirección de memoria\n");
    delete filename;
    machine -> WriteRegister(2,-1);
   return;
   }

 //Joinable thread.
 Thread *thread = new Thread(filename, true);

   if (thread == NULL){
    printf("No se pudo crear thread\n");
    delete filename;
    delete addrSpace;
    machine -> WriteRegister(2,-1);
    return;
    }
 
 //Asignamos espacio de memoria. 
 thread -> space = addrSpace;
 
 //Agregamos thread a la table de procesos
 SpaceId id = processTable -> addProcess(thread);
 

  if (id < 0){
    printf("No se pudo agregar el thread a la tabla de procesos\n");
    delete filename;
    delete addrSpace;
    delete thread;
    machine -> WriteRegister(2,-1);
    return;
   }
  
 machine -> WriteRegister(2,id);
 thread -> Fork(processCreator,NULL);
 
 return;

}


//----------------------------------------------------------------------
// SysCall_Exit()
//   	
//              Exit system call.
//----------------------------------------------------------------------

void SysCall_Exit(){
 //printf("SysCall_Exit()\n");

 int exitValue = machine -> ReadRegister(4);

 SpaceId id = processTable -> getSpaceId(currentThread);

 if (id >= 0) {
   processTable-> removeProcess(id, exitValue);
  }
 
  //
  if(id < 0) {
   interrupt -> Halt();
  }
 
  //Thread killer.
  currentThread -> Finish();

}

//----------------------------------------------------------------------
// SysCall_Join()
//   	
//              Join system call.
//----------------------------------------------------------------------
void SysCall_Join(){
 
 //printf("SysCall_Join()\n");
 SpaceId id = (SpaceId) machine -> ReadRegister(4);
 Thread* threadToJoin = processTable -> getThread(id);

 if(threadToJoin == NULL){
   //printf("Thread to join nulo"); 
   machine -> WriteRegister(2,-1);
   return;
 }

 threadToJoin -> Join();
 int exitValue;
 //Valor de retorno.
 bool opRes = processTable -> getExitValue(id, exitValue);


 if(opRes){
   machine -> WriteRegister(2,exitValue);
  }
 else{
   machine -> WriteRegister(2,-1); 
  }
 
 return;

} 


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{

/* Código Original

    printf("Entre en ExceptionHandler()\n");
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(false);
    }
*/


   int type = machine->ReadRegister(2);


   if (which == SyscallException) {
      switch (type) { 
          case SC_Halt:
            SysCall_Halt();
            break;
          case SC_Create:
            SysCall_Create(); 
            IncreaseProgramCounter(); 
            break;
          case SC_Open:
            SysCall_Open(); 
            IncreaseProgramCounter(); 
            break;
          case SC_Read:
            SysCall_Read(); 
            IncreaseProgramCounter(); 
            break;
           case SC_Write:
            SysCall_Write(); 
            IncreaseProgramCounter(); 
            break;
          case SC_Close:
            SysCall_Close(); 
            IncreaseProgramCounter(); 
            break;
          case SC_Exit:
            SysCall_Exit(); 
            IncreaseProgramCounter(); 
            break;
          case SC_Exec:
            //SysCall_Exec(); 
            SysCall_ExecNew(); //Exec c/ argumentos.
            IncreaseProgramCounter(); 
            break;
	  case SC_Join:
            SysCall_Join(); 
            IncreaseProgramCounter(); 
            break;
          default:
            printf("Unexpected user mode exception %d %d\n", which, type);
	    ASSERT(false);
            break;
       }
   }
    else if (which == PageFaultException){
    #ifdef USE_TLB
    //Handler
    //printf("PageFault\n"); 
      
   int badVrAddr = machine -> ReadRegister(BadVAddrReg);
   int badVrPage = badVrAddr / PageSize;


   tlbHandler-> TLBUpdater(badVrPage); 

   #endif
       }
     else if (which == ReadOnlyException){
    #ifdef USE_TLB
    ASSERT(false);
    #endif
       }
     else {
	printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(false); 
      }
}












