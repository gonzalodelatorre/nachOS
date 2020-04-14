#include "syscall.h"

int
main()
{
      

    OpenFileId o = Open("../test/XXXXXX.txt");  
    Write("1\n",3,o);
    Close(o);
    /* not reached */
}
