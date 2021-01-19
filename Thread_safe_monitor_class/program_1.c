#include <stdio.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>


void panic_on_failure(int e, const char* file, int line) {
  if (e) {
    fprintf(stderr, "Failure at %s:%d\n", file, line);
    exit(-1);
  }
}

#define try(expr) panic_on_failure((expr), __FILE__, __LINE__)


int n_cats = 0;
int n_dogs = 0;
int n_birds = 0;

int cat_time = 0;
int dog_time = 0;
int bird_time = 0;

struct Monitor {
  pthread_mutex_t mutex;
  pthread_cond_t ticket;
  bool awake;
  bool timeout;

  int cat_count;
  int dog_count;
  int bird_count;
};

// Create an instance of struct Monitor called "monitor".
struct Monitor monitor = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, false, 0 , 0 ,0 };

void play(void) {
  for (int i=0; i<10; i++) {
    assert(monitor.cat_count >= 0 && monitor.cat_count <= n_cats);
    assert(monitor.dog_count >= 0 && monitor.dog_count <= n_dogs);
    assert(monitor.bird_count >= 0 && monitor.bird_count <= n_birds);
    assert(monitor.cat_count == 0 || monitor.dog_count == 0);
    assert(monitor.cat_count == 0 || monitor.bird_count == 0);
   }
}

void cat_exit(void) {

    if (monitor.awake) {
       return;
    }

    try(pthread_mutex_lock(&monitor.mutex));
    monitor.cat_count--;
  

    try(pthread_mutex_unlock(&monitor.mutex));

    //printf("Cat exited  %d\n", monitor.cat_count);
}

void cat_enter(void) {

    try(pthread_mutex_lock(&monitor.mutex));

    if (monitor.awake) {
        try(pthread_mutex_unlock(&monitor.mutex));
        return;
    }

    cat_time++;
    monitor.cat_count++;
    try(pthread_mutex_unlock(&monitor.mutex));
    //printf("Cat entered %d\n", monitor.cat_count);
    play();
    cat_exit();
}


void dog_exit(void) {

    if (!monitor.awake) {
       return;
    }

    try(pthread_mutex_lock(&monitor.mutex));
    monitor.dog_count--;

    try(pthread_mutex_unlock(&monitor.mutex));

    //printf("dog exited %d\n", monitor.dog_count);
}

void dog_enter(void) {
  
 
    try(pthread_mutex_lock(&monitor.mutex));

    if (!monitor.awake) {
  
       try(pthread_mutex_unlock(&monitor.mutex));
       return;
    }
    dog_time++;  
    monitor.dog_count++;
    try(pthread_mutex_unlock(&monitor.mutex));

    //printf("dog entered %d\n", monitor.dog_count);
    play();
    dog_exit();
}


void bird_exit(void) {

    if (!monitor.awake) {
       return;
    }

    try(pthread_mutex_lock(&monitor.mutex));
    monitor.bird_count--;


    try(pthread_mutex_unlock(&monitor.mutex));

    //printf("bird exited %d\n", monitor.bird_count);
}

void bird_enter(void) {


    try(pthread_mutex_lock(&monitor.mutex));
    
    if (!monitor.awake)
    {    
         try(pthread_mutex_unlock(&monitor.mutex));
         return;
    }

    bird_time++;
    monitor.bird_count++;
    try(pthread_mutex_unlock(&monitor.mutex));

    //printf("bird entered %d\n", monitor.bird_count);
    play();
    bird_exit();
}



// Method on struct Monitor monitor.
// Blocks calling thread until Monitor value is at least num.
void cats_compete_untill_timeout(void) {
 
  while(1) {
     try(pthread_mutex_lock(&monitor.mutex));

     while (monitor.awake != 0 && !monitor.timeout)
        try(pthread_cond_wait(&monitor.ticket, &monitor.mutex));

     try(pthread_mutex_unlock(&monitor.mutex));

     if(monitor.timeout)
         return;
      
     cat_enter();
 }
}


void dogs_compete_untill_timeout(void) {

  while(1) {
     try(pthread_mutex_lock(&monitor.mutex));

     while (monitor.awake != 1 && !monitor.timeout)
        try(pthread_cond_wait(&monitor.ticket, &monitor.mutex));

     try(pthread_mutex_unlock(&monitor.mutex));

     if(monitor.timeout)
         return;

     dog_enter();
 }

}


void birds_compete_untill_timeout(void) {

  while(1) {
     try(pthread_mutex_lock(&monitor.mutex));

     while (monitor.awake != 1 && !monitor.timeout)
        try(pthread_cond_wait(&monitor.ticket, &monitor.mutex));

     try(pthread_mutex_unlock(&monitor.mutex));

     if(monitor.timeout)
         return;
     
     bird_enter();
 }

}
// -- End Monitor monitor --
void* bird_thread(void* _) {
  birds_compete_untill_timeout();
  return NULL;
}

void* cat_thread(void* _) {
  cats_compete_untill_timeout();
  return NULL;
}

void* dog_thread(void* _) {

  dogs_compete_untill_timeout();
  return NULL;
}

int main(int argc, char* argv[]) {

  if (argc != 4) {
     printf("Incorrect number of arguments passed\n");
  }

  regex_t regex;
  regcomp(&regex, "^[0-9][0-9]?$", REG_EXTENDED);
  
  if ( regexec(&regex, argv[1], 0, NULL, 0)  == REG_NOMATCH || regexec(&regex, argv[2], 0, NULL, 0) == REG_NOMATCH 
           || regexec(&regex, argv[3], 0, NULL, 0) == REG_NOMATCH )
  {
      printf("Incorrect type of Parameters entered, enter number of cats, dogs, birds\n");
      exit(0);
  } 
  
  n_cats = atoi(argv[1]);
  n_dogs = atoi(argv[2]);
  n_birds = atoi(argv[3]); 
 
  pthread_t cat_thrd[n_cats];
  pthread_t dog_thrd[n_dogs];
  pthread_t bird_thrd[n_birds];
  
  int i = 0;
  while (i < n_cats) 
        try(pthread_create(&cat_thrd[i++], NULL, cat_thread, NULL));

  i = 0;
  while (i < n_dogs) 
        try(pthread_create(&dog_thrd[i++], NULL, dog_thread, NULL));

  i = 0;
  while (i < n_birds) 
        try(pthread_create(&bird_thrd[i++], NULL, bird_thread, NULL));

  // sleep(10);
  monitor.timeout = false;
  time_t begin = time(NULL);

while(1) {

     if ((time(NULL) - begin) >= 10)
     {
         monitor.timeout = true;
         try(pthread_cond_broadcast(&monitor.ticket));
         break;
     }

     try(pthread_mutex_lock(&monitor.mutex));

    if (monitor.awake && !monitor.dog_count && !monitor.bird_count) {
          monitor.awake = 0;

          try(pthread_mutex_unlock(&monitor.mutex));
          try(pthread_cond_broadcast(&monitor.ticket));

          //printf("last dog n bird  exited %d %d\n", monitor.dog_count, monitor.bird_count);
    } else if (!monitor.awake && !monitor.cat_count) {
          monitor.awake = 1;
          try(pthread_mutex_unlock(&monitor.mutex));
          try(pthread_cond_broadcast(&monitor.ticket));

          //printf("last cat exited %d\n", monitor.cat_count);
    } else {

          try(pthread_mutex_unlock(&monitor.mutex));
    }
}

  i = 0;
  while (i < n_cats) 
    try(pthread_join(cat_thrd[i++], NULL));
  
  i = 0;
  while (i < n_dogs) 
    try(pthread_join(dog_thrd[i++], NULL));
  
  i = 0;
  while (i < n_birds) 
    try(pthread_join(bird_thrd[i++], NULL));

 
  printf("cat play = %d, dog play = %d, bird play = %d \n\n", cat_time, dog_time, bird_time);
  return 0;
}
