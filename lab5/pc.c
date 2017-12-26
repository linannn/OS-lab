#define __LIBRARY__
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

int main()
{
	int NUM = 500;
	int BUFFER = 10;
	int pidNum = 5;
	int inPos = 0, outPos = 0;
	int i, j, getNum;
	sem_t *empty, *full, *mutex;
	FILE *fp = NULL;
	
	empty = sem_open("empty", O_CREAT, 0064, 10);
	full = sem_open("full", O_CREAT, 0064, 0);
	mutex = sem_open("mutex", O_CREAT, 0064, 1);
	fp = fopen("buffer.txt", "wb+");
	
	if (!fork())
	{
		for (i=0; i<NUM; i++)
		{
			sem_wait(empty);
			sem_wait(mutex);
			fseek(fp, inPos*sizeof(int), SEEK_SET);
			fwrite(&i, sizeof(int), 1, fp);
			fflush(fp);
			inPos = (inPos+1)%BUFFER;
			sem_post(mutex);
			sem_post(full);
		}
		exit(0);
	}

	for (j=0; j<pidNum;j++)
	{
		if (!fork())
		{
			for (i=0; i<NUM; i++)
			{
				sem_wait(full);
				sem_wait(mutex);
				fseek(fp, 10*sizeof(int), SEEK_SET);
				fread(&outPos, sizeof(int), 1, fp);
				fseek(fp, outPos*sizeof(int), SEEK_SET);
				fread(&getNum, sizeof(int), 1, fp);
				printf("%d:%d\n", getpid(), getNum);
				outPos = (outPos + 1) % BUFFER;
				fseek(fp, 10*sizeof(int), SEEK_SET);
				fwrite(&outPos, sizeof(int), 1, fp);
				fflush(fp);
				sem_post(mutex);
				sem_post(empty);
			}
			exit(0);
		}
	}

	wait(NULL);
	fclose(fp);
	sem_unlink("empty");
	sem_unlink("full");
	sem_unlink("mutex");
	return 0;

}
