#ifndef THREAD_HANDLER_H_
#define THREAD_HANDLER_H_

struct thread_args
{
    int num;
	char token[128];
	char metrics_publish_URL[256];
	char device_id[256];
};

/**
 * @brief Starts all threads registered
 */
int startThreads(const char *metrics_publish_URL, const char *token, const char *device_id);

#endif /* THREAD_HANDLER_H_ */
