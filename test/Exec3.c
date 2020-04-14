#include "syscall.h"

int
main()
{
 
   
   SpaceId id1 = Exec("../test/testWrite1");
   Join(id1);
   SpaceId id2 = Exec("../test/testWrite2");
   Join(id2);
   SpaceId id3 = Exec("../test/testWrite3");
   Join(id3);
    
  
    
    /* not reached */
}
