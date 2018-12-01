#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#define NUMBER_ENTRIES  (1001)
#define FALSE           (0)
#define TRUE            (1)
#define DONE            (2)

struct request{
  int is_req;
  int is_allocated;
  int size;
  int match_alloc;
  int base_adr;
  int next_boundary_adr;
  int memory_left;
  int largest_chunk;
  int elements_on_free_list;
}req_array[NUMBER_ENTRIES];

struct free_list{
  struct free_list *next;
  struct free_list *previous;
  int block_size;
  int block_adr;
  int adjacent_adr;
}list_head, *top;

int total_free_space;
int free_list_length = 0, total_free;


void first_fit(std::string policy, std::string mem_pool, std::string fname);
void best_fit(std::string policy, std::string mem_pool, std::string fname);
void buddy_system(std::string policy, std::string mem_pool, std::string fname);
int allocate_memory(struct request * request);
int update_list(int index);


int main(int argc, char* argv[])
{
  std::string user_policy = argv[1];
  std::string tmfps = argv[2]; //total memory free pool size
  std::string file_name = argv[3];
  std::ifstream user_file;



 //test free pool

  if(!(tmfps == "1MB" || tmfps == "512KB"))
    {
      std::cout << "Error, the free pool size is unrecognized." << std::endl
		<< std::endl;
      std::cout << "Please enter # of KB (case sensitive): " << std::endl;
      std::cout << "1000" << std::endl;
      std::cout << "512" << std::endl << std::endl;
      exit(1);
    }


  //test file

  user_file.open(file_name.c_str());
  if(!user_file)
    {
      std::cout << "File not found" << std::endl;
      exit(1);
    }
  user_file.close();



  //test policies

  if(user_policy == "First_fit")
    {
      first_fit(user_policy, tmfps, file_name);
    }
  else if(user_policy == "Best_fit")
    {
      best_fit(user_policy, tmfps, file_name);
    }
  else if(user_policy == "Buddy_system")
    {
      buddy_system(user_policy, tmfps, file_name);
    }
  else
    {
      std::cout << "Error, the policy entered is unrecognized." << std::endl
		<< std::endl;
      std::cout << "Please enter (case sensitive): " << std::endl;
      std::cout << "First_fit" << std::endl;
      std::cout << "Best_fit" << std::endl;
      std::cout << "Buddy_system" << std::endl << std::endl;
      exit(1);
    }

 

  return 0;
}

void first_fit(std::string policy, std::string mem_pool, std::string fname)
{
  //std::cout << "First fit" << std::endl;
  int i, seq_num, type_val, tot_alloc = 0;
  char type[20];
  FILE *fp;
  struct free_list *p;

  total_free_space = total_free = (atoi((mem_pool).c_str())*1024);

  for(i = 0; i < NUMBER_ENTRIES; i++)
    {
      req_array[i].is_req = FALSE;
      req_array[i].is_allocated = FALSE;
    }
  if((top = new free_list) == NULL)
    {
      std::cout << std::endl << "Malloc error, exiting..." << std::endl;
      exit(1);
    }

  top->next = NULL;
  top->previous = &list_head;
  top->block_size = total_free_space;
  top->block_adr = 0;
  top->adjacent_adr = total_free_space;

  list_head.next = top;
  list_head.previous = NULL;
  list_head.block_size = -1;
  list_head.block_adr = -1;
  list_head.adjacent_adr = -1;

  fp = fopen("a5_data.txt", "r");
  while(fscanf(fp, "%d %s %d", &seq_num, type, &type_val) != EOF)
    {
      if(strcmp(type, "alloc") == 0)
	{
	  req_array[seq_num].is_req = TRUE;
	  req_array[seq_num].size = type_val;
	  allocate_memory(&req_array[seq_num]);
	  req_array[seq_num].elements_on_free_list = 0;
	  req_array[seq_num].largest_chunk = 0;
	  for(p = list_head.next; p; p=p->next)
	    {
	      ++req_array[seq_num].elements_on_free_list;
	      if(p->block_size > req_array[seq_num].largest_chunk)
		{
		  req_array[seq_num].largest_chunk = p->block_size;
		}
	    }
	}
      else
	{
	  req_array[seq_num].match_alloc = type_val;
	  req_array[seq_num].size = req_array[type_val].size;
	  req_array[seq_num].is_allocated = req_array[type_val].is_allocated;
	  update_list(type_val);
	  req_array[seq_num].memory_left = total_free;
	  req_array[seq_num].elements_on_free_list = 0;
	  req_array[seq_num].largest_chunk = 0;
	  for(p=list_head.next; p; p=p->next)
	    {
	      ++req_array[seq_num].elements_on_free_list;
	      if(p->block_size > req_array[seq_num].largest_chunk)
		{
		  req_array[seq_num].largest_chunk = p->block_size;
		}
	    }
	}
    }

  for(i = 0; i < NUMBER_ENTRIES; i++)
    {
      if(req_array[i].is_req && req_array[i].is_allocated)
	{
	  ++tot_alloc;
	}
      else
	{
	  if(req_array[i].is_req)
	    {
	      std::cout << "Failed Req " << i << " is size "
			<< req_array[i].size << std::endl;
	    }
	}
    }

  std::cout << "Total allocations: " << tot_alloc << std::endl;

  //write to file
  
  
  delete top;
}
void best_fit(std::string policy, std::string mem_pool, std::string fname)
{
  //std::cout << "Best fit" << std::endl;
 int i, seq_num, type_val, tot_alloc = 0;
  char type[20];
  FILE *fp;
  struct free_list *p;

  total_free_space = total_free = (atoi((mem_pool).c_str())*1024);

  for(i = 0; i < NUMBER_ENTRIES; i++)
    {
      req_array[i].is_req = FALSE;
      req_array[i].is_allocated = FALSE;
    }
  if((top = new free_list) == NULL)
    {
      std::cout << std::endl << "Malloc error, exiting..." << std::endl;
      exit(1);
    }

  top->next = NULL;
  top->previous = &list_head;
  top->block_size = total_free_space;
  top->block_adr = 0;
  top->adjacent_adr = total_free_space;

  list_head.next = top;
  list_head.previous = NULL;
  list_head.block_size = -1;
  list_head.block_adr = -1;
  list_head.adjacent_adr = -1;

  fp = fopen("a5_data.txt", "r");
  while(fscanf(fp, "%d %s %d", &seq_num, type, &type_val) != EOF)
    {
      if(strcmp(type, "alloc") == 0)
	{
	  req_array[seq_num].is_req = TRUE;
	  req_array[seq_num].size = type_val;
	  allocate_memory(&req_array[seq_num]);
	  req_array[seq_num].elements_on_free_list = 0;
	  req_array[seq_num].largest_chunk = 0;
	  for(p = list_head.next; p; p=p->next)
	    {
	      ++req_array[seq_num].elements_on_free_list;
	      if(p->block_size > req_array[seq_num].largest_chunk)
		{
		  req_array[seq_num].largest_chunk = p->block_size;
		}
	    }
	}
      else
	{
	  req_array[seq_num].match_alloc = type_val;
	  req_array[seq_num].size = req_array[type_val].size;
	  req_array[seq_num].is_allocated = req_array[type_val].is_allocated;
	  update_list(type_val);
	  req_array[seq_num].memory_left = total_free;
	  req_array[seq_num].elements_on_free_list = 0;
	  req_array[seq_num].largest_chunk = 0;
	  for(p=list_head.next; p; p=p->next)
	    {
	      ++req_array[seq_num].elements_on_free_list;
	      if(p->block_size > req_array[seq_num].largest_chunk)
		{
		  req_array[seq_num].largest_chunk = p->block_size;
		}
	    }
	}
    }

  for(i = 0; i < NUMBER_ENTRIES; i++)
    {
      if(req_array[i].is_req && req_array[i].is_allocated)
	{
	  ++tot_alloc;
	}
      else
	{
	  if(req_array[i].is_req)
	    {
	      std::cout << "Failed Req " << i << " is size "
			<< req_array[i].size << std::endl;
	    }
	}
    }

  std::cout << "Total allocations: " << tot_alloc << std::endl;

  //write to file
  
  
  delete top;
}
void buddy_system(std::string policy, std::string mem_pool, std::string fname)
{
  std::cout << "Buddy system" << std::endl;
}


int allocate_memory(struct request * request)
{
  struct free_list* freeList = NULL;

  
  for(freeList = list_head.next; freeList; freeList = freeList->next)
    {
      if(request->size <= freeList->block_size){
	
      request->is_allocated = TRUE;
      request->base_adr = freeList->block_adr;
      request->next_boundary_adr = request->base_adr + request->size;

      total_free = total_free - request->size;
      request->memory_left = total_free;

      if((freeList->block_size = freeList->block_size - request->size) == 0){
	freeList->previous->next = freeList->next;
	freeList->next->previous = freeList->previous;

	delete[] freeList;       
	return 0;
      } 

      freeList->block_adr = freeList->block_adr + request->size; 
      return 0; 
    }
  }

  request->memory_left = total_free;
  return 0;
}

int update_list(int index)
{
  
  struct free_list* freeList;
  struct free_list* newBlock;  
  struct free_list* temp;
 
  if(req_array[index].is_allocated == FALSE)
    {
      return 0;
    }

  total_free += req_array[index].size;

  for(freeList = list_head.next; freeList; freeList = freeList->next)
    {
    if(req_array[index].base_adr > freeList -> block_adr)
      {
	
      }

    newBlock = new free_list;    
    newBlock->block_size = req_array[index].size;
    newBlock->block_adr = req_array[index].base_adr;
    newBlock->adjacent_adr = newBlock->block_adr + newBlock->block_size;

    newBlock->next = freeList;
    freeList->previous->next = newBlock;
    newBlock->previous = freeList->previous;
    freeList->previous = newBlock;
    
    if(newBlock->adjacent_adr == newBlock->next->block_adr)
      {
	temp = newBlock->next;
	newBlock->block_size = newBlock->block_size+newBlock->next->block_size;
	newBlock->adjacent_adr = newBlock->next->adjacent_adr;
	newBlock->next = newBlock->next->next;
      
	if(newBlock->next)
	  {
	    newBlock->next->previous = newBlock;
	  }

	delete[] temp;
    }

    newBlock = newBlock->previous;
    if((newBlock != NULL) && (newBlock->adjacent_adr != 0))
      {
	if(newBlock -> adjacent_adr == newBlock -> next -> block_adr)
	  {
	    temp = newBlock->next;
	    newBlock->block_size = newBlock->block_size+newBlock->next->block_size;
	    newBlock->adjacent_adr = newBlock->next->adjacent_adr;
	    newBlock->next = newBlock->next->next;

        if(newBlock->next){
          newBlock->next->previous = newBlock;
	}

        delete[] temp;

      }
    }
  }

    delete newBlock;
    return 0;
}
