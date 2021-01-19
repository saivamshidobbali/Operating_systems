#include "types.h"
#include "user.h"
#include <stddef.h>

int main(int argc, char** argv) {

   if (argc <= 3)
   {      
      char* x[] = {0, 0};
      int retVal = settickets(atoi(argv[1]));

      if (retVal != 0) {
            printf(1, "%s", "Errored");
            return -1;
      }

      x[0] = argv[2];
      exec(argv[2], x);

  }
  exit();
}
