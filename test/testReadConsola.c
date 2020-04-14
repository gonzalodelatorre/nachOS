#include "syscall.h"

int
main()
{
   char ch;
   ch = '\0';
   OpenFileId o = Open("../test/testConsole.txt");
   // Waits until user writes something...
   int charsReaded = Read(&ch,3,ConsoleInput);
   Write(&ch,3,o);
   Halt();

    
    /* not reached */
}
