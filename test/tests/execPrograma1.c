#include "syscall.h"

  main(){

  Write("Ejecutando programa1.c\n",22,ConsoleOutput);

  SpaceId progrId = Exec("../test/tests/programa1");

   Write("Esperando que termine programa1.c\n",33,ConsoleOutput);
   Join(progrId);
     Write("programa1.c finalizado\n",22,ConsoleOutput);
Halt();
  
}


