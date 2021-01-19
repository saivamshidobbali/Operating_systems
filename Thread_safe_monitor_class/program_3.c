#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "crc.h"
#include <pthread.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <regex.h>

void panic_on_failure(int e, const char* file, int line) {
  if (e) {
    fprintf(stderr, "Failure at %s:%d\n", file, line);
    exit(-1);
  }
}

#define try(expr) panic_on_failure((expr), __FILE__, __LINE__)

#define INITIAL_CAPACITY 10

struct filename {
char* name;
char* printname;
uint32_t CRC;
struct filename* next;
};


struct Monitor {
  pthread_mutex_t mutex;
  pthread_cond_t ticket;

  char** file_list;
  int size;
  int capacity;
  bool done;
};

struct filename* head = NULL;
char * pathname = NULL;

// Create an instance of struct Monitor called "monitor".
struct Monitor monitor = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0 , 2,  false };

void print(void) {

    if (!head)
       return;
    
    struct filename* delete_node = NULL;
    struct filename* temp = head;
    struct filename* cmp = NULL;

  while(head != NULL) {
    temp = head;

    if (temp->next == NULL)
    {
        if (temp->CRC == UINT_MAX)
               printf("%s %s \n", temp->printname, "ACCESS ERROR");
        else
               printf("%s %08X \n", temp->printname, temp->CRC);
        free(head);
        break;
    }


    if (delete_node == temp && delete_node != NULL)
    {
        head = temp->next;
        free(temp);
        temp = head;
        delete_node = NULL;
    }

    while(temp != NULL && temp->next != NULL)
    {
       if (temp->next == delete_node && delete_node != NULL)
       {
           
              temp->next = temp->next->next;
              free(delete_node);
              delete_node = NULL;

       }

        if (!cmp) {
            cmp = temp;
        } else {
            if (strcmp(cmp->name, temp->name) > 0)
            {
                cmp = temp;
            }
        }
       temp = temp->next;
    }
    if (cmp) {
        if (cmp->CRC == UINT_MAX)
               printf("%s %s \n", cmp->printname, "ACCESS ERROR");
        else
               printf("%s %08X \n", cmp->printname, cmp->CRC);
    }
     delete_node = cmp;
     cmp = NULL;
    }
}


uint32_t compute_checksum(char * file_name)
 {
   FILE *f = fopen(file_name, "rb");
 
   if(!f)
      return UINT_MAX;  

   fseek(f, 0, SEEK_END);
   long fsize = ftell(f);
   fseek(f, 0, SEEK_SET);                 
   char *string = malloc(fsize + 1);
   fread(string, 1, fsize, f);
   fclose(f);

   string[fsize] = 0;
 
  uint32_t ret = crc32(0, (void*)string, fsize);
  free(string);

  return ret;
}

void* worker_thread(void* _) {
      char * file;
      while(1) {
         try(pthread_mutex_lock(&monitor.mutex));
 
         while (monitor.size <= 0 && !monitor.done) {
            try(pthread_cond_broadcast(&monitor.ticket));
            try(pthread_cond_wait(&monitor.ticket, &monitor.mutex));
         }
    
         file = monitor.file_list[monitor.size-1];
         monitor.size = monitor.size-1;         
         try(pthread_mutex_unlock(&monitor.mutex));

         if(monitor.done && monitor.size <= 0)
               pthread_exit(NULL);
 
         try(pthread_mutex_lock(&monitor.mutex));
              struct filename* temp;
              if (!head)
               {
                   head = (struct filename*)malloc(sizeof(struct filename));
                   head->next = NULL;
                   temp = head;
               } else {
                  temp = head;
                  
                  while(temp->next != NULL)
                       temp = temp->next;

                  temp->next = (struct filename*)malloc(sizeof(struct filename));
                  temp = temp->next;
                  temp->next = NULL;
               }               

               if (pathname[strlen(pathname)-1] != '/')
               {
                   temp->name = (char*)malloc(sizeof(char) * (strlen(pathname) + strlen(file) + 2));
                   strcpy(temp->name, pathname);
                   temp->name[strlen(pathname)] = '/';           
                   temp->name = strcat(temp->name, file);           
               } else {

                   temp->name = (char*)malloc(sizeof(char) * (strlen(pathname) + strlen(file) + 1));
                   strcpy(temp->name, pathname);
                   temp->name = strcat(temp->name, file);           
               }

               
               temp->printname = (char*)malloc(sizeof(char) * (strlen(file) + 1));
               strcpy(temp->printname, file);

         try(pthread_mutex_unlock(&monitor.mutex));


         temp->CRC = compute_checksum(temp->name);
     }
     return NULL;
 }
       
int main(int argc , char ** argv)
{

   if (argc != 3)
   {
      printf("Incorrect number of arguments passed, pass directory name and number of threads to spawn");
      exit(0);
   }

   regex_t regex;
   regcomp(&regex, "^[1-9][0-9]?$", REG_EXTENDED);

   if ( regexec(&regex, argv[2], 0, NULL, 0)  == REG_NOMATCH )
   {
      printf("Incorrect type of Parameters entered, enter number of threads to be spawned\n");
      exit(0);
   }

    DIR *dir;
    struct dirent *dp;
    dir = opendir(argv[1]);

    pathname = (char*)malloc(sizeof(char) * (strlen(argv[1])+1));
    strcpy(pathname, argv[1]);

    pthread_t thrd[atoi(argv[2])];

    monitor.file_list = malloc(INITIAL_CAPACITY * sizeof(char*));

    int i = 0;
    while (i < atoi(argv[2]))
        try(pthread_create(&thrd[i++], NULL, worker_thread, NULL));

    while ((dp=readdir(dir)) != NULL) {
        switch (dp->d_type) {
            case DT_REG:
                try(pthread_mutex_lock(&monitor.mutex));
                     while (monitor.size >= INITIAL_CAPACITY)
                                   try(pthread_cond_wait(&monitor.ticket, &monitor.mutex));
                           
                monitor.file_list[monitor.size++] = dp->d_name;
                try(pthread_mutex_unlock(&monitor.mutex));
                try(pthread_cond_broadcast(&monitor.ticket));
                break;
            case DT_DIR:
                break;
            default:
                break;
        }
    }


    monitor.done = true; 
    try(pthread_cond_broadcast(&monitor.ticket));
 
    i = 0;
    while (i < atoi(argv[2]))
    {
        try(pthread_cond_broadcast(&monitor.ticket));
        try(pthread_join(thrd[i++], NULL));
    }
    
    print();
    closedir(dir);
    return 0;
}
