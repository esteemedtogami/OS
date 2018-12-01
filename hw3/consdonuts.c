#include "donuts.h"
#include <string.h>

int shmid, semid[3];

int get_cpu_id()
{
  int i;
  char* line;
  FILE* procfile = fopen("/proc/self/stat", "r");
  long to_read = 8192;
  char buffer[to_read];
  int read = fread(buffer, sizeof(char), to_read, procfile);
  fclose(procfile);

  line = strtok(buffer, " ");
  for(i = 1; i < 38; i++)
    {
      line = strtok(NULL, " ");
    }
  line = strtok(NULL, " ");
  int cpu_id = atoi(line);
  return cpu_id;

}



int main(int argc, char* argv[])
{
  int i,j,k,l,m,donut;
  struct donut_ring *shared_ring;
  struct timeval randtime;
  unsigned short xsub[3];
  char *dtype[] = {"jelly", "coconut", "plain", "glazed"};
  key_t memkey, semkey;
  time_t rawtime;
  struct tm * timeinfo;

  memkey = semkey = MEMKEY + getuid();

  if((shmid=shmget(memkey, sizeof(struct donut_ring), 0)) == -1)
    {
      perror("shared get failed: ");
      exit(1);
    }
  if((shared_ring=(struct donut_ring*)shmat(shmid, NULL, 0)) == (void*) -1)
    {
      perror("shared attach failed: ");
      exit(1);
    }


  for(i=0; i < NUMSEMIDS; i++)
    {
	if((semid[i]=semget(semkey+i, NUMFLAVORS, 0)) == -1)
	  {
	    perror("semaphore alloc failed");
	    exit(1);
	  }
    }


	for(i = 0; i < 10; i++)
	  {
	    int jel = 0, coc = 0, pla = 0, gla = 0;
	    
	    for(m = 0; m < 12; m++)
	      {
		j=nrand48(xsub) & 3;

		if( p(semid[CONSUMER], j) == -1)
		  {
		    perror("p op failed");
		    exit(3);
		  }
		if(p(semid[OUTPTR], j) == -1)
		  {
		    perror("p op failed");
		    exit(3);
		  }
		donut = shared_ring->flavor[j][shared_ring->outptr[j]];

		shared_ring->outptr[j] =
		  (shared_ring->outptr[j] + 1) % NUMSLOTS;
		
	        if(j == 0)
		  {
		    jel ++;
		    shared_ring->flavor[j][m] = jel;
		  }
		else if(j == 1)
		  {
		    coc ++;
		    shared_ring->flavor[j][m] = coc;
		  }
		else if(j == 2)
		  {
		    pla ++;
		    shared_ring->flavor[j][m] = pla;
		  }
		else if(j == 3)
		  {
		    gla ++;
		    shared_ring->flavor[j][m] = gla;
		  }
		
		if (v(semid[PROD], j) == -1)
		  {
		    perror("v op failed");
		    exit(3);
		  }
		if (v(semid[OUTPTR], j) == -1)
		  {
		    perror("v op failed");
		    exit(3);
		  }

		
	      } //end 1 dozen

	    time(&rawtime);
	    timeinfo = localtime(&rawtime);

	    
	    printf("Process PID: %d   Dozen: %d   Time: %s\n",
		   getpid(), i+1, asctime(timeinfo));
	    printf("\n");
	    printf("plain\tjelly\tcoconut\thoney dip\n\n");


     
	    for(k = 0; k < 12; k++)
	      {
		printf("%d\t%d\t%d\t%d\n", 
		shared_ring->flavor[0][k], shared_ring->flavor[1][k],
		shared_ring->flavor[2][k], shared_ring->flavor[3][k]);
	      }

	    for(k = 0; k < NUMFLAVORS; k++)
	      {
		for(l = 0; l < 12; l++)
		  {
		    shared_ring->flavor[k][l] = 0;
		  } 
	      }

	    printf("\n");

	    
	    usleep(10000);
	    
	  } //end 10 dozen

	fprintf(stderr, "Consumer %s done\n", argv[1]);

  return 0;
}
