#include <sys/ipc.h>
#include <sys/shm.h>

#define KEY 12345

int shmid = 0;
void *shm = NULL;

unsigned long *lookup_count;

int initshm(int create)
{
	if (create)
		shmid = shmget(KEY, sizeof(unsigned long), 0777 | IPC_CREAT);
	else
		shmid = shmget(KEY, sizeof(unsigned long), 0777);
	if (shmid < 0) {
		perror("shmget");
		return -1;
	}
	shm = shmat(shmid, 0, 0);
	if (shm < 0) {
		perror("shmat");
		return -1;
	}
	if (create)
		memset(shm, 0, sizeof(unsigned long));
	lookup_count = shm;
	return 0;
}

void inc_lookup_count()
{
	if (lookup_count)
		(*lookup_count)++;
	if (debug)
		printf("lookup_count=%lu\n", *lookup_count);
}

unsigned long get_lookup_count()
{
	if (lookup_count)
		return *lookup_count;
	return 0;
}
