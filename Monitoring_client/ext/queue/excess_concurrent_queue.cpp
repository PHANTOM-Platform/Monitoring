// EXCESS data structure framework concurrent queue C adapter
// Anders Gidenstam

#include <excess_concurrent_queue.h>

#include <EXCESS/concurrent_queue>

// Prepare the types for a queue storing void* pointers.
typedef excess::concurrent_queue<void> concurrent_queue_t;
typedef excess::concurrent_queue<void>::handle handle_t;

extern "C" {

EXCESS_concurrent_queue_t ECQ_create(int queue_implementation)
{
  concurrent_queue_t* queue;

  switch (queue_implementation) {
  case 7:
#ifdef USE_TBB
    // The concurrent_queue from Intel Threading Building Blocks.
    queue = new excess::concurrent_queue_TBBQueue<void>();
#endif
    break;
  default:
    // An implementation of the Michael and Scott two-lock queue.
    queue = new excess::concurrent_queue_MSTLB<void>();
  }
  return (EXCESS_concurrent_queue_t)queue;
}

void ECQ_free(EXCESS_concurrent_queue_t queue)
{
  delete (concurrent_queue_t*)queue;
}

EXCESS_concurrent_queue_handle_t ECQ_get_handle(EXCESS_concurrent_queue_t queue)
{
  return
    (EXCESS_concurrent_queue_handle_t)((concurrent_queue_t*)queue)->get_handle();
}

void ECQ_free_handle(EXCESS_concurrent_queue_handle_t handle)
{
  delete (handle_t*)handle;
}

void ECQ_enqueue(EXCESS_concurrent_queue_handle_t handle,
                 void*                            item)
{
  ((handle_t*)handle)->enqueue(item);
}

int ECQ_try_dequeue(EXCESS_concurrent_queue_handle_t handle,
                    void**                           item_ptr)
{
  return ((handle_t*)handle)->try_dequeue(*item_ptr);
}

int ECQ_is_empty(EXCESS_concurrent_queue_handle_t handle)
{
  return ((handle_t*)handle)->empty();
}

}
