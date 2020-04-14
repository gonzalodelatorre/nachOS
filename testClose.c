#include "syscall.h"

int
main()
{
 
  Create("../test/testClose.txt");
  OpenFileId o = Open("../test/testClose.txt");
  Close(o);
    /* not reached */
}
