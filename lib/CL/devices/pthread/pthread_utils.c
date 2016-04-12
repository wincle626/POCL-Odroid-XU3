
#include <string.h>
#include "pocl-pthread_utils.h"
#include "utlist.h"
#include "common.h"
#include "pocl-pthread.h"
#include "pocl_mem_management.h"

static kernel_run_command *volatile kernel_pool = 0;
static int kernel_pool_initialized = 0;
pocl_lock_t kernel_pool_lock;


void pocl_init_kernel_run_command_manager (void)
{
  if (!kernel_pool_initialized)
    {
      kernel_pool_initialized = 1;
      POCL_INIT_LOCK (kernel_pool_lock);
    }
}

void pocl_init_thread_argument_manager (void)
{
  if (!kernel_pool_initialized)
    {
      kernel_pool_initialized = 1;
      POCL_INIT_LOCK (kernel_pool_lock);
    }
}

kernel_run_command* new_kernel_run_command ()
{
  kernel_run_command *volatile k = NULL;
  POCL_LOCK (kernel_pool_lock);
  if ((k = kernel_pool))
    {
      LL_DELETE (kernel_pool, k);
      memset (k, 0, sizeof(kernel_run_command));
      pthread_mutex_init(&k->lock, NULL);
      POCL_UNLOCK (kernel_pool_lock);
      return k;
    }

  POCL_UNLOCK (kernel_pool_lock);
  k = (kernel_run_command*)calloc (1, sizeof (kernel_run_command));
  pthread_mutex_init (&k->lock, NULL);
  return k;
}

void free_kernel_run_command (kernel_run_command *k)
{
  POCL_LOCK (kernel_pool_lock);
  pthread_mutex_destroy (&k->lock);
  LL_PREPEND (kernel_pool, k);
  POCL_UNLOCK (kernel_pool_lock);
}

void setup_kernel_arg_array(void **arguments, kernel_run_command *k)
{
  struct pocl_argument *al;  
  cl_kernel kernel = k->kernel;
  int i;

  for (i = 0; i < kernel->num_args; ++i)
    {
      al = &(k->kernel_args[i]);
      if (kernel->arg_info[i].is_local)
        {
          arguments[i] = malloc (sizeof (void *));
          *(void **)(arguments[i]) = pocl_memalign_alloc(MAX_EXTENDED_ALIGNMENT, al->size);
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_POINTER)
      {
        /* It's legal to pass a NULL pointer to clSetKernelArguments. In 
           that case we must pass the same NULL forward to the kernel.
           Otherwise, the user must have created a buffer with per device
           pointers stored in the cl_mem. */
        if (al->value == NULL) 
          {
            arguments[i] = malloc (sizeof (void *));
            *(void **)arguments[i] = NULL;
          }
        else
          {
            cl_mem m = *(cl_mem *)al->value;
            if (m->device_ptrs)
              arguments[i] = &(m->device_ptrs[k->device->dev_id].mem_ptr);
            else
              arguments[i] = &(m->mem_host_ptr);
          }
      }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_IMAGE)
        {
          dev_image_t di;
          fill_dev_image_t(&di, al, k->device);
          void* devptr = pocl_memalign_alloc(MAX_EXTENDED_ALIGNMENT, 
                                             sizeof(dev_image_t));
          arguments[i] = malloc (sizeof (void *));
          *(void **)(arguments[i]) = devptr;
          pocl_pthread_write (k->data, &di, devptr, 0, sizeof(dev_image_t));
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_SAMPLER)
        {
          dev_sampler_t ds;
          fill_dev_sampler_t(&ds, al);

          void* devptr = pocl_memalign_alloc(MAX_EXTENDED_ALIGNMENT, 
                                             sizeof(dev_sampler_t));
          arguments[i] = malloc (sizeof (void *));
          *(void **)(arguments[i]) = devptr;
          pocl_pthread_write (k->data, &ds, *(void**)arguments[i], 0,
                              sizeof(dev_sampler_t));
        }
      else
        arguments[i] = al->value;
    }

  /* Allocate the automatic local buffers which are implemented as implicit
     extra arguments at the end of the kernel argument list. */
  for (i = kernel->num_args;
       i < kernel->num_args + kernel->num_locals;
       ++i)
    {
      al = &(k->kernel_args[i]);
      arguments[i] = malloc (sizeof (void *));
      *(void **)(arguments[i]) = pocl_memalign_alloc (MAX_EXTENDED_ALIGNMENT, al->size);
    }

}

void free_kernel_arg_array (void **arguments, kernel_run_command *k)
{
  int i;
  cl_kernel kernel = k->kernel;
  for (i = 0; i < kernel->num_args; ++i)
    {
      if (kernel->arg_info[i].is_local )
        {
          POCL_MEM_FREE(*(void **)(arguments[i]));
          POCL_MEM_FREE(arguments[i]);
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_IMAGE ||
                kernel->arg_info[i].type == POCL_ARG_TYPE_SAMPLER)
        {
          POCL_MEM_FREE(*(void **)(arguments[i]));
          POCL_MEM_FREE(arguments[i]);
        }
      else if (kernel->arg_info[i].type == POCL_ARG_TYPE_POINTER && *(void**)arguments[i] == NULL)
        {
          POCL_MEM_FREE(arguments[i]);
        }
    }
  for (i = kernel->num_args;
       i < kernel->num_args + kernel->num_locals;
       ++i)
    {
      POCL_MEM_FREE(*(void **)(arguments[i]));
      POCL_MEM_FREE(arguments[i]);
    }
}

