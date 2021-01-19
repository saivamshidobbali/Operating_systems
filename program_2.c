#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "crc.h"

struct filename {
char* name;
char* printname;
uint32_t CRC;
struct filename* next;
};


struct filename* head = NULL;

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
        if (temp->CRC == UINT_MAX) {
               printf("%s %s \n", temp->printname, "ACCESS ERROR"); 
        }  else {
               printf("%s %08X \n", temp->printname, temp->CRC);
        }

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
        if (cmp->CRC == UINT_MAX) {
               printf("%s %s \n", cmp->printname, "ACCESS ERROR");
        } else {
               printf("%s %08X \n", cmp->printname, cmp->CRC);
        }
     }

     delete_node = cmp;
     cmp = NULL;
    }
}

int main(int argc , char ** argv)
{

   if (argc != 2)
   {
      printf("Incorrect number of arguments passed, pass directory name alone");
      exit(0);
   } 


    DIR *dir;
    struct dirent *dp;
    struct filename * temp;
    dir = opendir(argv[1]);

    while ((dp=readdir(dir)) != NULL) {
        switch (dp->d_type) {
            case DT_REG: 
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


             
               if (argv[1][strlen(argv[1])-1] != '/')
               {
            
                   temp->name = (char*)malloc(sizeof(char) * (strlen(argv[1]) + strlen(dp->d_name) + 2));

                   strcpy(temp->name, argv[1]);
                   temp->name[strlen(argv[1])] = '/';           
                   temp->name = strcat(temp->name, dp->d_name);           
               } else {

                   temp->name = (char*)malloc(sizeof(char) * (strlen(argv[1]) + strlen(dp->d_name) + 1));
                   strcpy(temp->name, argv[1]);
                   temp->name = strcat(temp->name, dp->d_name);           
               }


               temp->printname = (char*)malloc(sizeof(char) * (strlen(dp->d_name) + 1));
               strcpy(temp->printname, dp->d_name);

                FILE *f = fopen(temp->name, "r");

                if(!f) {
                    printf("ACCESS ERROR\n ");
                    temp->CRC  = UINT_MAX;
                    continue;
                }

                // Read file contents
                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                fseek(f, 0, SEEK_SET);                 
                char *string = malloc(fsize + 1);
                fread(string, 1, fsize, f);
                fclose(f);
                string[fsize] = 0;
             
                temp->CRC = crc32(0, (void*)string, fsize);

                free(string);
                break;

            case DT_DIR:
                break;
            default:
                break;
        }
    }

    closedir(dir);


    print();
    return 0;
}









