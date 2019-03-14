typedef struct {
	pthread_t thread;
	mpf_t rop;
	unsigned long iterStart;	// interval: [iterStart; iterFinish[
	unsigned long iterFinish;
} worker_t;

typedef struct {
	unsigned long precision;
	worker_t *workers;
} workInfo_t;

// workInfo_t workInfo;

void chudnovsky(mpf_t rop, unsigned long precision, unsigned long iterStart, unsigned long iterFinish);

unsigned long roundDiv(unsigned long q, unsigned long r);
