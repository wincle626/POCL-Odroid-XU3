#include <string.h>
#include <pthread.h>
#include "pocl-pthread_scheduler.h"
#include "pocl_cl.h"
#include "pocl-pthread.h"
#include "pocl-pthread_utils.h"
#include "utlist.h"
#include "pocl_util.h"
#include "common.h"
#include "pocl_mem_management.h" 

static void* pocl_pthread_driver_thread (void *p);

struct pool_thread_data
{
  pthread_t thread;
  int my_id;
  struct shared_data * sd;
  _cl_command_node *volatile work_queue;
  kernel_run_command *volatile kernel_queue;
  pthread_cond_t wakeup_cond;
  pthread_mutex_t lock;
  volatile int executed_commands;
  volatile int stolen_commands;
  volatile int stolen_wgs;
  volatile unsigned lock_counter;
  volatile uint64_t prev_wg_finish_time;
  pthread_mutex_t kernel_q_lock;
  volatile int kernel_counter;
};

typedef struct scheduler_data_
{
  struct pool_thread_data *volatile thread_pool;
  _cl_command_node *volatile work_queue;
  kernel_run_command *volatile kernel_queue;
  volatile int num_threads;
  volatile int round_robin_index;
  pthread_cond_t cq_finished_cond;
  pthread_cond_t wake_pool;
  pthread_mutex_t wq_lock;
  pthread_mutex_t cq_finished_lock;
  volatile int thread_pool_shutdown_requested;
  cl_device_id *volatile pool_devices;
} scheduler_data;

static scheduler_data scheduler;

void pthread_scheduler_init (size_t num_worker_threads)
{
  int i;
  pthread_mutex_init (&(scheduler.wq_lock), NULL);
  pthread_mutex_init (&(scheduler.cq_finished_lock), NULL);
  pthread_cond_init (&(scheduler.cq_finished_cond), NULL);
  pthread_cond_init (&(scheduler.wake_pool), NULL);

  scheduler.thread_pool = calloc
    (num_worker_threads, sizeof (struct pool_thread_data));
  scheduler.num_threads = num_worker_threads;

  for (i = 0; i < num_worker_threads; ++i)
    {
      scheduler.thread_pool[i].my_id = i;
      pthread_cond_init (&scheduler.thread_pool[i].wakeup_cond, NULL);
      pthread_mutex_init (&scheduler.thread_pool[i].lock, NULL);
      pthread_mutex_init (&scheduler.thread_pool[i].kernel_q_lock, NULL);
      pthread_create (&scheduler.thread_pool[i].thread, NULL,
                      pocl_pthread_driver_thread,
                      (void*)&scheduler.thread_pool[i]);
    }

}

void pthread_scheduler_uinit ()
{
  int i;
  scheduler.thread_pool_shutdown_requested = 1;

  pthread_mutex_lock (&scheduler.wq_lock);
  pthread_cond_broadcast (&scheduler.wake_pool);
  pthread_mutex_unlock (&scheduler.wq_lock);

  for (i = 0; i < scheduler.num_threads; ++i)
    {
      pthread_join (scheduler.thread_pool[i].thread, NULL);
    }
}

void pthread_scheduler_push_command (_cl_command_node *cmd)
{
  PTHREAD_LOCK (&scheduler.wq_lock, NULL);
  DL_APPEND (scheduler.work_queue, cmd);
  pthread_cond_broadcast (&scheduler.wake_pool);
  PTHREAD_UNLOCK (&scheduler.wq_lock);
}

void pthread_scheduler_push_kernel (kernel_run_command *run_cmd)
{
  PTHREAD_LOCK (&scheduler.wq_lock, NULL);
  LL_APPEND (scheduler.kernel_queue, run_cmd);
  pthread_cond_broadcast (&scheduler.wake_pool);
  PTHREAD_UNLOCK (&scheduler.wq_lock);
}

void pthread_scheduler_wait_cq (cl_command_queue cq)
{
  while (1)
    {
      pthread_mutex_lock (&scheduler.cq_finished_lock);
      POCL_LOCK_OBJ (cq);
      if (cq->command_count == 0)
        {
          POCL_UNLOCK_OBJ (cq);
          pthread_mutex_unlock (&scheduler.cq_finished_lock);
          return;
        }
      POCL_UNLOCK_OBJ (cq);
      pthread_cond_wait (&scheduler.cq_finished_cond,
                         &scheduler.cq_finished_lock);
      pthread_mutex_unlock (&scheduler.cq_finished_lock);
    }
}

void pthread_scheduler_release_host ()
{
  PTHREAD_LOCK (&scheduler.cq_finished_lock, NULL);
  pthread_cond_signal (&scheduler.cq_finished_cond);
  PTHREAD_UNLOCK (&scheduler.cq_finished_lock);
}

static int
work_group_scheduler (kernel_run_command *k,
                      struct pool_thread_data *thread_data);

void finalize_kernel_command (thread_data *thread_data,
                              kernel_run_command *k);

int pthread_scheduler_get_work (thread_data *td, _cl_command_node **cmd_ptr)
{
  _cl_command_node *cmd;
  kernel_run_command *run_cmd;
  // execute kernel if available
  PTHREAD_LOCK (&scheduler.wq_lock, NULL);
  if ((run_cmd = scheduler.kernel_queue))
    {
      ++run_cmd->ref_count;
      PTHREAD_UNLOCK (&scheduler.wq_lock);

      work_group_scheduler (run_cmd, td);

      PTHREAD_LOCK (&scheduler.wq_lock, NULL);
      if (!(--run_cmd->ref_count))
        {
          PTHREAD_UNLOCK (&scheduler.wq_lock);
          finalize_kernel_command (td, run_cmd);
        }
      else
        PTHREAD_UNLOCK (&scheduler.wq_lock);
    }
  else
    PTHREAD_UNLOCK (&scheduler.wq_lock);

  // execute a command if available
  PTHREAD_LOCK (&scheduler.wq_lock, NULL);
  if ((cmd = scheduler.work_queue))
    {
      DL_DELETE (scheduler.work_queue, cmd);
      PTHREAD_UNLOCK (&scheduler.wq_lock);
      *cmd_ptr = cmd;
      return 0;
    }
  PTHREAD_UNLOCK (&scheduler.wq_lock);
  *cmd_ptr = NULL;
  return 1;
}

void pthread_scheduler_sleep()
{
  static struct timespec time_to_wait = {0, 0};
  time_to_wait.tv_sec = time(NULL) + 5;

  PTHREAD_LOCK (&scheduler.wq_lock, NULL);
  if (scheduler.work_queue == NULL && scheduler.kernel_queue == 0)
    pthread_cond_timedwait (&scheduler.wake_pool, &scheduler.wq_lock, &time_to_wait);
  PTHREAD_UNLOCK (&scheduler.wq_lock);
}

#define POCL_PTHREAD_MAX_WGS 256
static int get_wg_index_range (kernel_run_command *k, unsigned *start_index,
                               unsigned *end_index, char *last_wgs)
{
  int max_wgs;
  *last_wgs = 0;
  PTHREAD_LOCK (&k->lock, NULL);
  if (k->remaining_wgs == 0)
    {
      PTHREAD_UNLOCK (&k->lock);
      return 0;
    }
  max_wgs = min (POCL_PTHREAD_MAX_WGS,
                 (1 + k->remaining_wgs / scheduler.num_threads));
  max_wgs = min (max_wgs, k->remaining_wgs);

  *start_index = k->wgs_dealt;
  *end_index = k->wgs_dealt + max_wgs-1;
  k->remaining_wgs -= max_wgs;
  k->wgs_dealt += max_wgs;
  if (k->remaining_wgs == 0)
    *last_wgs = 1;
  PTHREAD_UNLOCK (&k->lock);

  return 1;
}

inline static void translate_wg_index_to_3d_index (kernel_run_command *k,
                                                   unsigned index,
                                                   size_t *index_3d)
{
  unsigned xy_slice = k->pc.num_groups[0] * k->pc.num_groups[1];
  index_3d[2] = index / xy_slice;
  index_3d[1] = (index % xy_slice) / k->pc.num_groups[0];
  index_3d[0] = (index % xy_slice) % k->pc.num_groups[0];
}

static int
work_group_scheduler (kernel_run_command *k,
                      struct pool_thread_data *thread_data)
{
  void *arguments[k->kernel->num_args + k->kernel->num_locals];
  struct pocl_context pc;
  int i;
  unsigned start_index;
  unsigned end_index;
  char last_wgs = 0;

  if (!get_wg_index_range (k, &start_index, &end_index,  &last_wgs))
    return 0;

  setup_kernel_arg_array ((void**)&arguments, k);
  memcpy (&pc, &k->pc, sizeof (struct pocl_context));
  do
    {
      if (last_wgs)
        {
          PTHREAD_LOCK (&scheduler.wq_lock, NULL);
          LL_DELETE (scheduler.kernel_queue, k);
          PTHREAD_UNLOCK (&scheduler.wq_lock);
        }
      for (i = start_index; i <= end_index; ++i)
        {
          translate_wg_index_to_3d_index (k, i, (size_t*)&pc.group_id);
#ifdef DEBUG_MT
          printf("### exec_wg: gid_x %d, gid_y %d, gid_z %d\n",
                 pc.group_id[0],
                 pc.group_id[1], pc.group_id[2]);
#endif
          k->workgroup (arguments, &pc);
        }

    }while (get_wg_index_range (k, &start_index, &end_index,  &last_wgs));


  free_kernel_arg_array (arguments, k);

  return 1;
}

void finalize_kernel_command (struct pool_thread_data *thread_data,
                              kernel_run_command *k)
{
#ifdef DEBUG_MT
  printf("### kernel %s finished\n", k->cmd->command.run.kernel->name);
#endif

  pocl_ndrange_node_cleanup (k->cmd);
  POCL_UPDATE_EVENT_COMPLETE (&k->cmd->event);

  pocl_mem_manager_free_command (k->cmd);

  free_kernel_run_command (k);
}

void
pocl_pthread_prepare_kernel
(void *data, 
 _cl_command_node* cmd)
{
  kernel_run_command *run_cmd;
  unsigned i;
  cl_kernel kernel = cmd->command.run.kernel;
  struct pocl_context *pc = &cmd->command.run.pc;
  cl_device_id device;

  cmd->device->ops->compile_kernel (cmd, NULL, NULL);

  /* Find which device number within the context correspond
     to current device.  */
  for (i = 0; i < kernel->context->num_devices; ++i)
    {
      if (kernel->context->devices[i]->data == data)
        {
          device = kernel->context->devices[i];
          break;
        }
    }

  int num_groups = pc->num_groups[0] * pc->num_groups[1] * pc->num_groups[2];

  run_cmd = new_kernel_run_command ();
  run_cmd->data = data;
  run_cmd->kernel = kernel;
  run_cmd->device = device;
  run_cmd->pc = *pc;
  run_cmd->cmd = cmd;
  run_cmd->group_idx[0] = 0;
  run_cmd->group_idx[1] = 0;
  run_cmd->group_idx[2] = 0;
  run_cmd->pc.local_size[0] = cmd->command.run.local_x;
  run_cmd->pc.local_size[1] = cmd->command.run.local_y;
  run_cmd->pc.local_size[2] = cmd->command.run.local_z;
  run_cmd->remaining_wgs = num_groups;
  run_cmd->workgroup = cmd->command.run.wg;
  run_cmd->kernel_args = cmd->command.run.arguments;
  run_cmd->next = NULL;

  pthread_scheduler_push_kernel (run_cmd);  

}

void pocl_pthread_exec_command (_cl_command_node * volatile cmd,
                                struct pool_thread_data *td)
{
  if(cmd->type == CL_COMMAND_NDRANGE_KERNEL)
    {
      POCL_UPDATE_EVENT_RUNNING(&(cmd->event));
      pocl_pthread_prepare_kernel (cmd->command.run.data, cmd);
    }
  else
    {
      pocl_exec_command(cmd);
    }
}


static
void*
pocl_pthread_driver_thread (void *p)
{
  struct pool_thread_data *td = (struct pool_thread_data*)p;
  _cl_command_node *cmd = NULL;

  while (1)
    {
      if (scheduler.thread_pool_shutdown_requested)
        {
          pthread_exit (NULL);
        }

      pthread_scheduler_get_work (td, &cmd);
      if (cmd)
        {
          assert (pocl_command_is_ready(cmd->event));
          assert (cmd->event->status == CL_SUBMITTED);
          pocl_pthread_exec_command (cmd, td);
          cmd = NULL;
          ++td->executed_commands;
        }
      // check if its time to sleep
      pthread_scheduler_sleep();
    }
}
