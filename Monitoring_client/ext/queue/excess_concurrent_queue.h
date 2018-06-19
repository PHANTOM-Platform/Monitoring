// EXCESS data structure framework concurrent queue C adapter
// Anders Gidenstam
#ifndef EXCESS_CONCURRENT_QUEUE
#define EXCESS_CONCURRENT_QUEUE

#ifdef __cplusplus
extern "C" {
#endif

typedef void* EXCESS_concurrent_queue_t;
typedef void* EXCESS_concurrent_queue_handle_t;

/*
 * Creates and initializes a queue instance.
 *   queue_implementation    selects the implementation. Currently ignored.
 */ 
EXCESS_concurrent_queue_t ECQ_create(int queue_implementation);

/*
 * Frees a queue instance.
 */
void ECQ_free(EXCESS_concurrent_queue_t queue);

/*
 * Get a thread local handle to the queue.
 */
EXCESS_concurrent_queue_handle_t ECQ_get_handle(EXCESS_concurrent_queue_t queue);

/*
 * Free a thread local handle to the queue.
 */
void ECQ_free_handle(EXCESS_concurrent_queue_handle_t handle);

/*
 * Enqueue an item in the queue using a thread local handle.
 */
void ECQ_enqueue(EXCESS_concurrent_queue_handle_t handle,
                 void*                            item);

int ECQ_try_dequeue(EXCESS_concurrent_queue_handle_t handle,
                    void**                           item_ptr);

int ECQ_is_empty(EXCESS_concurrent_queue_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
