#ifndef POCL_PTHREAD_UTILS_H
#define POCL_PTHREAD_UTILS_H

#include "pocl_cl.h"

/* locking macros */
#define PTHREAD_LOCK(__lock , __counter)        \
  do {                                          \
    if ((__counter))                            \
      ++(*((unsigned*)(__counter)));            \
  }while (pthread_mutex_trylock((__lock)))

//#define PTHREAD_LOCK(__lock, __counter) do {pthread_mutex_lock((__lock__));} while (0)

#define PTHREAD_UNLOCK(__lock) do { pthread_mutex_unlock((__lock)); }while(0)



typedef struct kernel_run_command kernel_run_command;
struct kernel_run_command
{
  void *data;
  cl_kernel kernel;
  cl_device_id device;
  struct pocl_context pc;
  _cl_command_node *cmd;
  pthread_mutex_t lock;
  unsigned lock_counter;
  volatile unsigned group_idx[3];
  volatile unsigned remaining_wgs;
  volatile unsigned wgs_dealt;
  pocl_workgroup workgroup;
  struct pocl_argument *kernel_args;
  volatile int ref_count;
  kernel_run_command *volatile next;
#ifdef POCL_PTHREAD_CACHE_MONITORING
  pocl_cache_data cache_data;
#endif
};

void pocl_init_kernel_run_command_manager (void);
void pocl_init_thread_argument_manager ();
kernel_run_command* new_kernel_run_command ();
void free_kernel_run_command (kernel_run_command *k);
void setup_kernel_arg_array(void **arguments, kernel_run_command *k);
void free_kernel_arg_array (void **arguments, kernel_run_command *k);

#endif
