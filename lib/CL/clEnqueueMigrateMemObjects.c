/* OpenCL runtime library: clEnqueueMigrateMemObjects()

   Copyright (c) 2013 Ville Korhonen / Tampere Univ. of Tech.
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "pocl_cl.h"
#include "pocl_util.h"
#include <string.h>

CL_API_ENTRY cl_int CL_API_CALL
POname(clEnqueueMigrateMemObjects) (cl_command_queue command_queue,
                                    cl_uint num_mem_objects,
                                    const cl_mem *mem_objects,
                                    cl_mem_migration_flags flags,
                                    cl_uint num_events_in_wait_list,
                                    const cl_event *event_wait_list,
                                    cl_event *event)CL_API_SUFFIX__VERSION_1_0
{
  unsigned i;
  int errcode;
  _cl_command_node *cmd = NULL;

  POCL_RETURN_ERROR_COND((event_wait_list == NULL && num_events_in_wait_list > 0),
    CL_INVALID_EVENT_WAIT_LIST);

  POCL_RETURN_ERROR_COND((event_wait_list != NULL && num_events_in_wait_list == 0),
    CL_INVALID_EVENT_WAIT_LIST);

  POCL_RETURN_ERROR_COND((command_queue == NULL), CL_INVALID_COMMAND_QUEUE);

  POCL_RETURN_ERROR_COND((num_mem_objects == 0), CL_INVALID_VALUE);
  POCL_RETURN_ERROR_COND((mem_objects == NULL), CL_INVALID_VALUE);

  for (i = 0; i < num_mem_objects; ++i)
    {
      POCL_RETURN_ERROR_COND((mem_objects[i] == NULL), CL_INVALID_MEM_OBJECT);

      POCL_RETURN_ERROR_COND((mem_objects[i]->context != command_queue->context),
        CL_INVALID_CONTEXT);
    }

  for (i = 0; i < num_events_in_wait_list; i++)
    POCL_RETURN_ERROR_COND((event_wait_list[i] == NULL), CL_INVALID_EVENT_WAIT_LIST);

  errcode = pocl_create_command (&cmd, command_queue,
                                 CL_COMMAND_MIGRATE_MEM_OBJECTS,
                                 event, num_events_in_wait_list,
                                 event_wait_list, num_mem_objects, mem_objects);

  if (errcode != CL_SUCCESS)
    goto ERROR;

  cmd->command.migrate.data = command_queue->device->data;
  cmd->command.migrate.num_mem_objects = num_mem_objects;
  cmd->command.migrate.mem_objects = malloc (sizeof (cl_mem) * num_mem_objects);
  cmd->command.migrate.source_devices = malloc
    (num_mem_objects * sizeof (cl_device_id));
  memcpy (cmd->command.migrate.mem_objects, mem_objects,
          num_mem_objects * sizeof (cl_mem));

  for (i = 0; i < num_mem_objects; ++i)
    {
      POname(clRetainMemObject) (mem_objects[i]);
      cmd->command.migrate.source_devices[i] = mem_objects[i]->owning_device;
      mem_objects[i]->owning_device = command_queue->device;
    }

  pocl_command_enqueue (command_queue, cmd);

  return CL_SUCCESS;

 ERROR:
  free (cmd);
  free (event);
  return errcode;
}
POsym(clEnqueueMigrateMemObjects)
