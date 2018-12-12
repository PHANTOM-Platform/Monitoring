#ifndef THREAD_HANDLER_H_
#define THREAD_HANDLER_H_

struct thread_args
{
    int num;
	char *token;
};

/**
 * @brief Starts all threads registered
 */
int startThreads(void);

#endif /* THREAD_HANDLER_H_ */
