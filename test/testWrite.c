#include "syscall.h"

int
main()
{
      
    //Write("Ejecutando exec\n", 17, ConsoleOutput);

    OpenFileId o = Open("../test/XXXXXX.txt");  
    Write("ABC",3,o);
    Close(o);
    /* not reached */
}
