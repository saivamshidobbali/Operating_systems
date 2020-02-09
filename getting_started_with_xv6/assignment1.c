#include "assignment1.h"
#include <fcntl.h>
#include <stdbool.h>

/*
B1
Value of cr4  at the start of _start before PSE enabled is 0x00000000
Value of cr4 after PSE enabled is 0x00000010

B2
Size of kernel stack in bytes is 0x00001000 (4096) bytes.

B3
name of process is "initcode"

B4
64
If there are too many processes, then context switch overhead increases for schedular and that will slow down the system
so there is a need for the limit. And also to protect system from attacks like fork bomb there is a need for limit on 
number of processes runnable on the system.

B5 
every command on the shell is executed by forking a new "sh" process. when the command is done, process is moved to zombie
state by calling exit function. So I did a breakpoint at exit function and ran kill command on the shell to hit the 
break point.

B6
After triggering exit running process will move into ZOMBIE STATE.

B7:
Thread 1 hit Breakpoint 5, myproc () at proc.c:64
64        popcli();
(gdb) p *p                                                                                 
$1 = {sz = 49152, pgdir = 0x8dfbc000, kstack = 0x8dfbe000 "", state = RUNNING, pid = 3,     
  parent = 0x80112dd0 <ptable+176>, tf = 0x8dfbefb4, context = 0x8dfbef9c, chan = 0x0,      
  killed = 0, ofile = {0x8010fff4 <ftable+52>, 0x8010fff4 <ftable+52>,                      
    0x8010fff4 <ftable+52>, 0x0 <repeats 13 times>}, cwd = 0x80110a14 <icache+52>
  name = "sh", '\000' <repeats 13 times>} 


(gdb) p *p.parent                                                                               
$3 = {sz = 16384, pgdir = 0x8df73000, kstack = 0x8dffe000 "", state = SLEEPING, pid = 2,    
  parent = 0x80112d54 <ptable+52>, tf = 0x8dffefb4, context = 0x8dffeebc,                   
  chan = 0x80112dd0 <ptable+176>, killed = 0, ofile = {0x8010fff4 <ftable+52>,              
    0x8010fff4 <ftable+52>, 0x8010fff4 <ftable+52>, 0x0 <repeats 13 times>},                
  cwd = 0x80110a14 <icache+52>, name = "sh", '\000' <repeats 13 times>}     

pid = 3, process name = sh
ppid = 2, process name = sh

Every command is run by forking a new child sh process, so both parent and child are sh processes.
*/

uint64_t byte_sort(uint64_t arg) {
 
    uint64_t mask = 0xFF;
    int num_bytes = sizeof(arg);
    uint8_t result[num_bytes];
    uint64_t temp = arg;

    for (int i = 0; i < num_bytes; i++)
    {
      result[i] = temp & mask;
      temp = temp >> 8;
    }

    int swap;
    for (int i = 0; i < num_bytes; i++) {
      for (int j = 0; j < num_bytes-1-i; j++) {
              if (result[j] < result[j+1])
               {
                    swap = result[j];
                    result[j] = result[j+1];
                    result[j+1] = swap;
               }
      }
    }

    uint64_t resulta = 0;
    for (int i = 0; i < num_bytes; i++)
    {
      resulta = resulta << 8; 
      resulta = result[i] | resulta;
    }  

    return resulta;
}

uint64_t nibble_sort(uint64_t arg) {
 
    uint64_t mask = 0xF;
    int num_bytes = sizeof(arg) * 2;
    uint8_t result[num_bytes];
    uint64_t temp = arg;

    for (int i = 0; i < num_bytes; i++)
    {
      result[i] = temp & mask;
      temp = temp >> 4 ;
    }

    int swap;
    for (int i = 0; i < num_bytes; i++) {
      for (int j = 0; j < num_bytes-1-i; j++) {
              if (result[j] < result[j+1])
               {
                    swap = result[j];
                    result[j] = result[j+1];
                    result[j+1] = swap;
               }
      }
    }

    uint64_t resulta = 0;
    for (int i = 0; i < num_bytes; i++)
    {
      resulta = resulta << 4; 
      resulta = result[i] | resulta;
    }  
    
    return resulta;
}

struct elt* circular_list(const char* str) {
     
    int i = 1;
    struct elt* head = NULL, *prev = NULL, *next =NULL;

    head = (struct elt*)malloc(sizeof(struct elt));
    
    if (head == NULL) 
         return 0;


    head->val = str[0];
    prev = head;
 
    while (str[i]) {
       
       struct elt* new_node = (struct elt*)malloc(sizeof(struct elt));

       if(new_node == NULL) {
         
         while (head != NULL)
         {
           next = head->link;
           free(head);
           head = next;
         }
         return 0;
       }
     
       new_node->link = NULL;
       new_node->val = str[i];
       prev->link = new_node; 
       prev = new_node;
       i++;
    }

    prev->link = head;
    return head;
}


int stringlength(const char* s) {
  int i = 0;
  while (s[i] != '\0') {
    i++;
  }
  return i;
}

bool findchar(char* target, char character) {

  int i = 0;
  while (target[i] != '\0') {
    if (target[i] == character) {
      return true;
    }
    i++;
  }

  return false;
}

// Used strchr is it valid ??
// Will there be null at the end of string str??
int convert(enum format_t mode, const char* str, uint64_t* out) {

       if (out == NULL || str == NULL) {
         return -1;
       }


       if (mode != 66 && mode != 67 && mode != 68) {
          return -1;
       }

      // empty string
      if (!stringlength(str)) {
          return -1;
      }

      int i = 0;
      char benchmark[3][17] = {
                                "01234567\0",
                                "01\0",
                                "0123456789abcdef\0"
                               };
  
      int multiplier[3] = {8, 2, 16};
      uint64_t n = 0, num = 0;

      // validity of chars
      while(str[i])
      {
        bool found = findchar(benchmark[mode-66], str[i]);
        
        if(!found)
            return -1;
       
        switch(str[i]) {
              case 'a':
                 num = 10;
                 break;
              case 'b':
                 num = 11;
                 break;
              case 'c':
                 num = 12;
                 break;

              case 'd':
                 num = 13;
                 break;

              case 'e':
                 num = 14;
                 break;

              case 'f':  
                 num = 15;
                 break;
                  
              default:
                 num = (int)(str[i]) - 48;   
            }

        if (((18446744073709551615ULL - num) / multiplier[mode-66]) < n) {
             return -1;
        }

        n = n * multiplier[mode-66] + num;
        i++;
      }

      *out = n;
       return 0;    
}

const char * convertInt2Char(int pid) {

    int temp = pid;
    int counter = 0;

    // count num of digits
    while(temp != 0) {
      counter++;
      temp = temp / 10;
    }

    counter++;

    char* output = (char*)malloc(counter);
    if (output == NULL)
        return NULL;

    output[--counter] = '\n';
    while(pid != 0)
    {
            output[--counter] = (char)(pid % 10 + 48);
            pid = pid/10;
    }
 
  return output;
}

void log_pid() {
    int pid = syscall(39);
    
    if (pid == -1)
        return;

    const char* buf = convertInt2Char(pid);
    
    if (!buf)
       return;

    int count = stringlength(buf);

    int create_fd = syscall(2, "log.pid", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (create_fd == -1) {
          syscall(87, "log.pid");
          return;
    }

    int ret = syscall(SYS_write, create_fd, buf, count);
    syscall(3, create_fd);
    
    if (ret == -1) {
      syscall(87, "log.pid");
    }

    free((char*)buf);
    return;
}
