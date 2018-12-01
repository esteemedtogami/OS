#include "A4.hpp"

DONUT_SHOP shared_ring;

pthread_mutex_t prod[NUMFLAVORS];
pthread_mutex_t cons[NUMFLAVORS];
pthread_cond_t prod_cond[NUMFLAVORS];
pthread_cond_t cons_cond[NUMFLAVORS];
pthread_t thread_id[NUMCONSUMERS+NUMPRODUCERS];
pthread_t sig_wait_id;
int thread_number = 0;

int main(int argc, char* argv[])
{
  int i, j, nsigs;
  struct timeval first_time, last_time;
  struct sigaction new_act;
  int arg_array[NUMCONSUMERS];
  sigset_t all_signals;
  int sigs[] = {SIGBUS, SIGSEGV, SIGFPE};

  pthread_attr_t thread_attr;
  struct sched_param sched_struct;
  int cn;
  float etime;

  gettimeofday(&first_time, (struct timezone *) 0);

  for(i = 0; i < NUMCONSUMERS; i++)
    {
      arg_array[i] = i + 1;
    }

  for(i = 0; i < NUMFLAVORS; i++)
    {
      pthread_mutex_init(&prod[i], NULL);
      pthread_mutex_init(&cons[i], NULL);
      pthread_cond_init(&prod_cond[i], NULL);
      pthread_cond_init(&cons_cond[i], NULL);
      shared_ring.outptr[i] = 0;
      shared_ring.in_ptr[i] = 0;
      shared_ring.serial[i] = 0;
      shared_ring.spaces[i] = NUMSLOTS;
      shared_ring.donuts[i] = 0;
    }

  sigfillset(&all_signals);
  nsigs = sizeof(sigs)/sizeof(int);
  for(i = 0; i < nsigs; i++)
    {
      sigdelset(&all_signals, sigs[i]);
    }

  sigprocmask(SIG_BLOCK, &all_signals, NULL);
  sigfillset(&all_signals);
  for(i = 0; i < nsigs; i++)
    {
      new_act.sa_handler = sig_handler;
      new_act.sa_mask = all_signals;
      new_act.sa_flags = 0;
      if(sigaction(sigs[i], &new_act, NULL) == -1)
	{
	  perror("Can't set signals: ");
	  exit(1);
	}
    }
  // std::cout<< "just before threads created" << std::endl;

  if(pthread_create (&sig_wait_id, NULL, sig_waiter, NULL) != 0)
    {
      std::cout << "pthread_create failed" << std::endl;
      exit(3);
    }

  pthread_attr_init(&thread_attr);
  pthread_attr_setinheritsched(&thread_attr, PTHREAD_INHERIT_SCHED);

  #ifndef GLOBAL

  pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&thread_attr, SCHED_OTHER);
  sched_struct.sched_priority = sched_get_priority_max(SCHED_OTHER);
  pthread_attr_setschedparam(&thread_attr, &sched_struct);
  pthread_attr_setscope(&thread_attr, PTHREAD_SCOPE_SYSTEM);
  
  #endif
  
   for(i=0; i < NUMCONSUMERS; i++, j++)
   {
      if(pthread_create(&thread_id[i], &thread_attr, consumer,
			(void*)&arg_array[i]) != 0)
	{
	  std::cout << "pthread_create failed" << std::endl;
	  exit(3);
	}     
   }

   
   for(; i < NUMPRODUCERS + NUMCONSUMERS; i++)
    {
      if(pthread_create(&thread_id[i], &thread_attr, producer, NULL) != 0)
	{
	  std::cout << "pthread_create failed " << std::endl;
	  exit(3);
	}
    }
   



  // std::cout << "Just after threads created" << std::endl;


  for(i = 1; i < NUMCONSUMERS + 1; i++)
    {
      pthread_join(thread_id[i], NULL);
    }

  gettimeofday(&last_time, (struct timezone *)0);
  if((i=last_time.tv_sec - first_time.tv_sec) == 0)
    {
      j=last_time.tv_usec - first_time.tv_usec;
    }
  else
    {
      if(last_time.tv_usec - first_time.tv_usec < 0)
	{
	  i--;
	  j = 1000000 + (last_time.tv_usec - first_time.tv_usec);
	}
      else
	{
	  j = last_time.tv_usec - first_time.tv_usec;
	}
    }
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Elapsed time consumer time is " << i
	    << " sec and " << j << " usec, or "
	    << (etime = i + (float)j/1000000) << " sec" << std::endl;
  if((cn = open("./run_times", O_WRONLY|O_CREAT|O_APPEND, 0644)) == -1)
    {
      perror("Cannot open sys time file ");
      exit(1);
    }

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "ALL CONSUMERS FINISHED, KILLING PROCESS" << std::endl;
  exit(0);

  return 0;
}

void *producer(void *arg)
{
  int j;
  unsigned short xsub[3];
  struct timeval randtime;
  gettimeofday(&randtime, (struct timezone*) 0);
  xsub[0] = (ushort) randtime.tv_usec;
  xsub[1] = (ushort) (randtime.tv_usec >> 16);
  xsub[2] = (ushort) (pthread_self());

  while(1)
    {
      j = nrand48 (xsub) & 3;
      pthread_mutex_lock (&prod[j]);
      while(shared_ring.spaces[j] == 0)
	{
	  pthread_cond_wait(&prod_cond[j], &prod[j]);
	}

      shared_ring.flavor[j][shared_ring.in_ptr[j]]=shared_ring.serial[j];
      shared_ring.in_ptr[j] = (shared_ring.in_ptr[j]+1) % NUMSLOTS;
      shared_ring.serial[j]++;
      shared_ring.spaces[j]--;
      
      pthread_mutex_unlock(&prod[j]);

      pthread_mutex_lock(&cons[j]);
      shared_ring.donuts[j]++;
      
      pthread_mutex_unlock(&cons[j]);

      pthread_cond_signal(&cons_cond[j]);

      
      
    }
  return NULL;

}

void *consumer(void *arg)
{
  thread_number++;
  int i,j,k,m;
  unsigned short xsub[3];
  struct timeval randtime;
  //id = *(int*) arg;
  gettimeofday(&randtime, (struct timezone *) 0);
  xsub[0] = (ushort) randtime.tv_usec;
  xsub[1] = (ushort) (randtime.tv_usec >> 16);
  xsub[2] = (ushort) (pthread_self());
  time_t rawtime;
  struct tm * timeinfo;


  FILE* fp = fopen("output.txt", "w");

  for(i = 0; i < 200; i++)
    {
      int plain = 0, jelly = 0, coconut = 0, honey_dip = 0;
      for(m = 0; m < 12; m++)
	{
	  j = nrand48(xsub)&3;

	  pthread_mutex_lock(&cons[j]);

	  while(shared_ring.donuts[j] == 0)
	    {
	      pthread_cond_wait(&cons_cond[j], &cons[j]);
	    }

	  k = shared_ring.flavor[j][shared_ring.outptr[j]];

	  if(k == 0)
	    {
	      plain++;
	    }
	  else if(k == 1)
	    {
	      jelly++;
	    }
	  else if(k == 2)
	    {
	      coconut++;
	    }
	  else if(k == 3)
	    {
	      honey_dip++;
	    }
	  
	  shared_ring.outptr[j] = (shared_ring.outptr[j] + 1) % NUMSLOTS;
	  shared_ring.donuts[j]--;
	  
	  pthread_mutex_unlock(&cons[j]);

	  pthread_mutex_lock(&prod[j]);

	  shared_ring.spaces[j]++;
      
	  pthread_mutex_unlock(&prod[j]);

	  pthread_cond_signal(&prod_cond[j]);
	  
	}
      time(&rawtime);
      timeinfo = localtime(&rawtime);

      if(i < 10)
	{
	  fprintf(fp, "Consumer#: %d  Dozen#: %d  Time: %s", thread_number,
	      i+1, asctime(timeinfo));
	  fprintf(fp, "\n");
	  fprintf(fp, "plain\tjelly\tcoconut\thoney dip\n\n");
	  fprintf(fp, "%d\t%d\t%d\t%d\t\n\n", plain, jelly, coconut, honey_dip);
	}
      
      
      usleep(1000);
    }
  fclose(fp);
  return NULL;
}

void *sig_waiter(void *arg)
{
  sigset_t sigterm_signal;
  int signo;

  sigemptyset(&sigterm_signal);
  sigaddset(&sigterm_signal, SIGTERM);
  sigaddset(&sigterm_signal, SIGINT);

  if (sigwait(&sigterm_signal, &signo) != 0)
    {
      std::cout << std::endl;
      std::cout << "sigwait() failed, exiting" << std::endl;
      std::cout << std::endl;
      exit(2);
    }
  std::cout << "Process exits on SIGNAL " << signo << std::endl << std::endl;
  exit(1);
  return NULL;
}

void sig_handler(int sig)
{
  pthread_t signaled_thread_id;
  int i, thread_index;

  signaled_thread_id = pthread_self();

  for(i = 0; i < (NUMCONSUMERS); i++)
    {
      if(signaled_thread_id == thread_id[i])
	{
	  thread_index = i + 1;
	  break;
	}
    }
  std::cout << std::endl << "Thread " << thread_index << " took signal # "
	    << sig << ", PROCESS HALT" << std::endl;
  exit(1);
}
