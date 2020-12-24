#include "types.h"
#include "user.h"
#include <stddef.h>
#include "pstat.h"
#include "param.h"

int main(int argc, char** argv) {

    int loop = 0;


    if (argc > 2) {
      printf(1, "%s", "Incorrect Input");
      return -1;
    }

   
    if (argc ==2)
    {
        if(!strcmp(argv[1], "-r")) {
          loop = 1;
        } else {
          printf(1, "%s", "Incorrect Input");
        }
    }

    struct pstat p;
    int retVal = getpinfo(&p);

    if (retVal != 0)
          return -1;

    int i;

    printf(1, "%s \n" , "PID TICKETS TICKS\n");
    for (i = 0; i < NPROC; i++ )
    {
         if (p.inuse[i] != 0)
            printf(1, "%d %d %d \n" , p.pid[i], p.tickets[i],  p.ticks[i]);
    }
    sleep(100);

   while(loop) {

      getpinfo(&p);
      printf(1, "%s \n" , "PID TICKETS TICKS\n");
      for (i = 0; i < NPROC; i++ )
      {
         if (p.inuse[i] != 0) {
            printf(1, "%d %d %d \n" , p.pid[i], p.tickets[i],  p.ticks[i]);
          }
      }
      sleep(100);
   }
   exit();
   return 0;
}
