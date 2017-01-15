/*


    alloc_mem:   C program to allocate a specified amount of memory
                 or shared memory and then sleep.  Memory can
                 be allocated in any block size (megs).


     Notes: Failure to allocate a memory can be caused by few different things
                 - your out of memory and/or swap.  (Virtual memory)
                 - maxdsize, maxdsize_64
                 - shmmni, shmmax, shmmnu
                 - ulimit




@(#) Revision: 1.10
@(#)   $Date:2016/01


Rev history
2000/05	 1.00  Jonathan Senkerik  Orig version for HP-UX
2016/01  1.10  Jonathan Senkerik  Ported to Linux

*/


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>


void Handle_Sig(int); 

int main(argc,argv)
int argc;
char *argv[255];

{
        int i,bflag,rflag,lflag,sflag,vflag,hflag;
        int Size,BSize,*Result,Result2;
        int shmidCount;
        int *shmid;
        long Count;
        char *shmptr;

/*

  init variables

*/

        bflag = 0;		/*   Use Blocks flag	    */
        rflag = 0;		/*   Exit & release allocated memory on exit flag   */
        lflag = 0;		/*   Lock memory flag	    */
        sflag = 0;		/*   Use shared memory	    */
        vflag = 0;		/*   Verbose flag	    */
        hflag = 0;		/*   Help flag              */
        Size = 0;		/*   Total size to allocate */
        BSize = 0;		/*   Block size to allocate */
        shmidCount=0;


  for (i=1;i<argc;i++)  /*   Go through command line args  */
  {
    if (!strcmp(argv[i], "-b") )
      bflag = 1;
    if (!strcmp(argv[i], "-r") )
      rflag = 1;
    if (!strcmp(argv[i], "-l") )
      lflag = 1;
    if (!strcmp(argv[i], "-s") )
      sflag = 1;
    if (!strcmp(argv[i], "-v") )
      vflag = 1;
    if (!strcmp(argv[i], "-?") )
      hflag = 1;
   
    if ( Size != 0 )
    {
      BSize = atol(argv[i]);
      bflag = 1;
    }
    if ( Size == 0 )
      Size = atol(argv[i]);
  }

  
  if (Size <= 0)
    hflag = 1;

  if (BSize <= 0)
  {
    if (sflag == 1)
      BSize = 10;
    else 
      BSize = 1;
  }
  if (BSize > Size)
    hflag = 1;



  if (hflag == 1)  
  {
    printf ("usage: %s [-b] [-r] [-s] [-l] [-v] [size] [bsize]\n         -b allocate in blocks (default std=1meg, shm=10meg)\n",argv[0]);
    printf ("         -r Release memory after it has been allocated\n");
    printf ("         -s Allocate using shared memory\n");
    printf ("         -l Lock pages in memory  (need root or MLOCK privs)\n");
    printf ("         -v Verbose\n");
    printf ("         size = Allocate specified in megs (default 10 megs)\n");
    printf ("        bsize = Specify block size (requires -b option)\n");
    printf ("\n\n        Example : ./alloc_mem -b 200 10       (Allocate 200 megs memory in 10 meg chunks)\n");
    printf ("                  ./alloc_mem -b -s 500       (Allocate 500 megs shared memory in 10 meg chunks)\n");
    printf ("                  ./alloc_mem -r -l 2000 100  (Allocate 2 gig memory in 100 meg chunks)\n");

    printf ("\n\n Notes: Failure to allocate a memory can be caused by few different things\n");
    printf ("              - your out of memory and/or swap.  (Virtual memory)\n");
    printf ("              - maxdsize, maxdsize_64\n");
    printf ("              - shmmni, shmmax, shmmnu\n");
    printf ("              - ulimit\n");
    exit(1);
  }


  if (vflag == 1)
  {
    printf ("\n Version                   = 1.10\n");
    if (bflag ==1)
      printf (" Allocate memory in blocks = True\n");
    else
      printf (" Allocate memory in blocks = False\n");
    if (rflag ==1)
      printf (" Release after allocated   = True\n");
    else
      printf (" Release after allocated   = False\n");
    if (sflag ==1)
      printf (" Use shared memory         = True\n");
    else
      printf (" Use shared memory         = False\n");
    if (lflag ==1)
      printf (" Lock pages in memory      = True\n");
    else
      printf (" Lock pages in memory      = False\n");
    printf (" Total memory to allocate  = %d meg\n",Size);
    if (bflag == 1)
      printf (" Allocated in block size   = %d meg\n",BSize);
  }


/*
     Are we creating a shared memory segment???
*/

  if (sflag == 1)
  {

    if (lflag == 1)              /* lockpages if lflag is set  */
    {
      Result2 = mlockall(MCL_CURRENT|MCL_FUTURE);
  
      if (Result2 == 0 )
        printf("Setting locked pages success.\n");
      else
        perror("Setting locked pages failed ");
    }


    if(bflag ==1)
    {                                                   /* in blocks */
      shmid = malloc(Size/BSize*sizeof(int));
      for (i=BSize;i<=Size;i=i+BSize)
      {
        *(shmid+shmidCount) = shmget (IPC_PRIVATE, BSize*1048576, 0600);
        if (*(shmid+shmidCount) == -1)
        {
          perror("Allocated shared memory failed ");
          Done(rflag,lflag,sflag,vflag,shmid,shmidCount);
        }
        else
        {
            shmptr = shmat(*(shmid+shmidCount), (char *)0, 0);
  
          /*
              If we don't write to each (4k) page, then the kernel
              will not create the page...
          */
          for(Count=0;Count<(BSize*1048576);Count=Count+4096)
          {
            *(shmptr+Count) = 0;
          }   
          printf("Create shared segment, %i megs success. (shmid = %i)",i,*(shmid+shmidCount));
          if (lflag == 1)
          {
            Result2 = shmctl (*(shmid+shmidCount), SHM_LOCK, 0);
  
            if (Result2 == 0 )
              printf(" -locked success.\n");
            else
              printf(" -locked failed!!!\n");
          }
          else
            printf("\n");
          shmdt(shmptr);
        }
        shmidCount++;
      }
    }

    else			/* else create shared mem seg in one big chunk  */
    {                                                 
      shmid = malloc(sizeof(int));
      *(shmid+shmidCount) = shmget (IPC_PRIVATE, Size*1048576, 0600);
      if (*(shmid+shmidCount) == -1)
      {
        perror("Allocated shared memory failed ");
        exit(1);
      }
      else
      {
        shmptr = shmat((*(shmid+shmidCount)), (char *)0, 0);
        /*   
            If we don't write to each (4k) page, then the kernel
            will not create the page...
        */
        for(Count=0;Count<(Size*1048576);Count=Count+4096)
        {
          *(shmptr+Count) = 0;
        } 
        printf("Create shared segment, %i megs success. (shmid = %i)",Size,*(shmid+shmidCount));
      }
      if (lflag == 1)
      {
        Result2 = shmctl (*(shmid+shmidCount), SHM_LOCK, 0);

        if (Result2 == 0 )
          printf(" -locked success.\n");
        else
          printf(" -locked failed!!!\n");
      }
      else
        printf("\n");

      shmdt(shmptr);
      shmidCount=1;                /* here we're only allocating 1 block  */
    }



    Done(rflag,lflag,sflag,vflag,shmid,shmidCount);
  }



/* 
	We must be allocating memory normally (data area).
	Using calloc (instead of malloc) to force the kernel
	  to create the page.

*/
  if (lflag == 1)              /* lock pages if lflag is set  */
  {
    Result2 = mlockall(MCL_CURRENT|MCL_FUTURE);

    if (Result2 == 0 )
      printf("Setting locked pages success.\n");
    else
      perror("Setting locked pages failed ");
  }

  if (bflag == 1)                            /* create in blocks */
  {
    for (i=BSize;i<=Size;i=i+BSize)
    {
      Result = calloc(262144*BSize,sizeof(int));  
      if (Result == 0 )
      {
        printf("Malloc %i meg failed.  Requested %i megs.\n",i,Size);
        perror("Error ");
        i=Size;
      }
      else {
        printf("Malloc %i meg success.  ",i);
        printf("Address 0x%x\n", Result);
      }
    }
    Done(rflag,lflag,sflag,vflag,NULL,0);
  }
  else
  {                                            /* 1 big chunk  */
    Size=Size * 1024 * 256;
    Result  = calloc(Size,sizeof(int));
    if (Result == 0 )
    {
      printf("Malloc %i megs failed.\n",Size/1024/256);
      perror("Error ");
      exit(1);
    }
    else
      printf("Malloc %i megs success.\n",Size/1024/256);

    Done(rflag,lflag,sflag,vflag,NULL,0);
  }
}




/*
     We are all done...  Do we wait or exit?  (rflag)
*/
Done(rflag,lflag,sflag,vflag,shmid,shmidCount)
int rflag,lflag,sflag,vflag,*shmid,shmidCount;

{
  int i;                                    /* i = counter  */

  if (rflag == 1)
  {
    if (sflag == 1)                         /* Remove shared memory segments */
    {
      for(i=0;i<shmidCount;i++)
      {
        if (vflag == 1 && *(shmid+i) != -1)
          printf("   Removing shared memory segment %d \n",*(shmid+i));
        shmctl (*(shmid+i), IPC_RMID, 0);
      }
    }
    exit(0);
  }
  else
  {
    printf(" Waiting for BREAK signal ( [CTRL]-C )...\n");
    signal(SIGINT, Handle_Sig );                  /* Wait for BREAK to exit */
    pause();
    if (vflag == 1)
      printf ("    OK, received BREAK signal...");
    printf ("\n");

    if (sflag == 1)                        /* Remove shared memory segments */
    {
      for(i=0;i<shmidCount;i++)
      {
        if (vflag == 1 && *(shmid+i) != -1)
          printf("   Removing shared memory segment %d \n",*(shmid+i));
        shmctl (*(shmid+i), IPC_RMID, 0);   
      }

    }
    exit(0);
  }
}


void Handle_Sig(Junk)
int Junk;
{
     //   Do nothing...
}

