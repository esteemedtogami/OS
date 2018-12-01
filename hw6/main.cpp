#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

extern int errno;
void print_details(struct stat);

int main(int argc, char* argv[])
{
  struct stat stat_buf;
  int i;
  DIR* dirptr;
  struct dirent * entptr;

  if(argc > 1)
    {
      for(i = 1; i < argc; i++)
	{
	  if(lstat(argv[i], &stat_buf) == -1)
	    {
	      perror("Bad file name for stat\n");
	      exit(1);
	    }
	  printf("FILENAME:\t\t\t%s\n", argv[i]);
	  print_details(stat_buf);
	}
    }
  else
    {
      if((dirptr = opendir(".")) == NULL)
	{
	  perror("Failed to open current directory\n");
	  exit(2);
	}
      while ((entptr = readdir(dirptr)) != NULL)
	{
	  if(lstat(entptr->d_name, &stat_buf) == -1)
	    {
	      perror("Bad file name for stat\n");
	      exit(1);
	    }
	  printf("FILENAME:\t\t\t%s\n", entptr->d_name);
          print_details(stat_buf);
	}
    }

  return 0;
}

void print_details(struct stat stat_buf)
{
  char prot_buf[10];
  struct passwd * pwdptr;
  struct group * grptr;
  int j;

  prot_buf[9] = '\0';
  if(S_ISDIR(stat_buf.st_mode))printf("FILE TYPE:\t\t\tdirectory\n");
  if(S_ISREG(stat_buf.st_mode))printf("FILE TYPE:\t\t\tordinary\n");
  if(S_ISCHR(stat_buf.st_mode))printf("FILE TYPE:\t\t\tchar dev\n");
  if(S_ISBLK(stat_buf.st_mode))printf("FILE TYPE:\t\t\tblock dev\n");
  if(S_ISFIFO(stat_buf.st_mode))printf("FILE TYPE:\t\t\tpipe\n");
  if(S_ISSOCK(stat_buf.st_mode))printf("FILE TYPE:\t\t\tUD socket\n");
  if(S_ISLNK(stat_buf.st_mode))printf("FILE TYPE:\t\t\tsymbolic link\n");
  for(j = 0; j < 9; j++) prot_buf[j] = '-';
  if(stat_buf.st_mode & S_IRUSR) prot_buf[0] = 'r';
  if(stat_buf.st_mode & S_IWUSR) prot_buf[1] = 'w';
  if(stat_buf.st_mode & S_IXUSR) prot_buf[2] = 'x';
  if(stat_buf.st_mode & S_IRGRP) prot_buf[3] = 'r';
  if(stat_buf.st_mode & S_IWGRP) prot_buf[4] = 'w';
  if(stat_buf.st_mode & S_IXGRP) prot_buf[5] = 'x';
  if(stat_buf.st_mode & S_IROTH) prot_buf[6] = 'r';
  if(stat_buf.st_mode & S_IWOTH) prot_buf[7] = 'w';
  if(stat_buf.st_mode & S_IXOTH) prot_buf[8] = 'x';
  printf("PERMISSIONS:\t\t\t%s\n", prot_buf);
  if((pwdptr = getpwuid(stat_buf.st_uid)) == NULL)
    {
      printf("OWNER_NAME:\t\t\t%d\n", stat_buf.st_uid);
    }
  else
    {
      printf("OWNER_NAME:\t\t\t%s\n", pwdptr->pw_name);
    }
  if((grptr = getgrgid(stat_buf.st_gid)) == NULL)
    {
      printf("GROUP_NAME:\t\t\t%d\n", stat_buf.st_gid);
    }
  else
    {
      printf("GROUP_NAME:\t\t\t%s\n", grptr->gr_name);
    }
  printf("DATE_OF_LAST_MODIFICATION:\t%s", ctime(&(stat_buf.st_mtime)));
  printf("NLINK SIZEOF:\t\t\t%d bytes\n", sizeof(stat_buf.st_nlink));
  printf("LINK COUNT:\t\t\t%d\n", stat_buf.st_nlink);
  if((S_ISCHR(stat_buf.st_mode)) || (S_ISBLK(stat_buf.st_mode)))
    {
      printf("MAJOR - MINOR CODES:\t\t%d, %d\n", major(stat_buf.st_rdev), minor(stat_buf.st_rdev));
    }
  else
    {
      printf("FILE SIZEOF:\t\t\t%d bytes\n", sizeof(stat_buf.st_size));
      printf("FILE SIZE:\t\t\t%d\n", stat_buf.st_size);
    }
  printf("INUM SIZE:\t\t\t%d bytes\n", sizeof(stat_buf.st_ino));
  printf("INODE NUMBER:\t\t\t%llu\n\n", stat_buf.st_ino);
  return;
}




