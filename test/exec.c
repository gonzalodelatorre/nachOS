#include "syscall.h"

int
main()
{
 
   //Write("Ejecutando exec\n", 17, ConsoleOutput);
   //Write("...\n", 5, ConsoleOutput);
   Exec("../test/testWrite");
   //Write("Despues de exec\n", 17, ConsoleOutput);
   Halt();
    
  
    
    /* not reached */
}
