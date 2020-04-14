#include "syscall.h"

int
main()
{
      

    OpenFileId o = Open("../test/XXXXXX.txt");  
    Write("2\n",3,o);
    Close(o);
    /* not reached */
}
