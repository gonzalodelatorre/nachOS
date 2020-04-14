#include "syscall.h"

int
main()
{
      

    OpenFileId o = Open("../test/XXXXXX.txt");  
    Write("3\n",3,o);
    Close(o);
    /* not reached */
}
