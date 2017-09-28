/*


    alloc_mem:   C program to allocate a specified amount of memory
                 or shared memory and then sleep.  Memory can
                 be allocated in any block size (megs).


     Notes: Failure to allocate a memory can be caused by few different things
                 - your out of memory and/or swap.  (Virtual memory)
                 - maxdsize, maxdsize_64
                 - shmmni, shmmax, shmmnu
                 - ulimit




@(#) Revision: 1.11
@(#)   $Date:2017/01


Rev history
2000/05  1.00  Jonathan Senkerik  Orig version for HP-UX
2016/01  1.10  Jonathan Senkerik  Ported to Linux
2017/01  1.11  Jonathan Senkerik  Reformatted to look cleaner 

*/


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>


void Handle_Sig (int);

int main (argc, argv)
int argc;
char *argv[255];

{
  int IterationCounter;
  int isUsingBlocks, isReleaseOnExit, isLockingMem, isUsingSHM, isVerbose, isHelpFlag;
  int Size, BlockSize, *Result, Result2;
  int shmidCount;
  int *shmid;
  long Count;
  char *shmptr;

/*

  init variables

*/

  isUsingBlocks = 0;            /*   Use Blocks flag        */
  isReleaseOnExit = 0;          /*   Exit & release allocated memory on exit flag   */
  isLockingMem = 0;             /*   Lock memory flag       */
  isUsingSHM = 0;               /*   Use shared memory          */
  isVerbose = 0;                /*   Verbose flag       */
  isHelpFlag = 0;               /*   Help flag              */
  Size = 0;                     /*   Total size to allocate */
  BlockSize = 0;                /*   Block size to allocate */
  shmidCount = 0;


  for (IterationCounter = 1; IterationCounter < argc; IterationCounter++)   /*   Go through command line args  */
    {
      if (!strcmp (argv[IterationCounter], "-b"))
        isUsingBlocks = 1;
      if (!strcmp (argv[IterationCounter], "-r"))
        isReleaseOnExit = 1;
      if (!strcmp (argv[IterationCounter], "-l"))
        isLockingMem = 1;
      if (!strcmp (argv[IterationCounter], "-s"))
        isUsingSHM = 1;
      if (!strcmp (argv[IterationCounter], "-v"))
        isVerbose = 1;
      if (!strcmp (argv[IterationCounter], "-?"))
        isHelpFlag = 1;

      if (Size != 0)
        {
          BlockSize = atol (argv[IterationCounter]);
          isUsingBlocks = 1;
        }
      if (Size == 0)
        Size = atol (argv[IterationCounter]);
    }


  if (Size <= 0)
    isHelpFlag = 1;

  if (BlockSize <= 0)
    {
      if (isUsingSHM == 1)
        BlockSize = 10;
      else
        BlockSize = 1;
    }
  if (BlockSize > Size)
    isHelpFlag = 1;



  if (isHelpFlag == 1)
    {
      printf ("usage: %s [-b] [-r] [-s] [-l] [-v] [size] [bsize]\n         -b allocate in blocks (default std=1meg, shm=10meg)\n", argv[0]);
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
      exit (1);
    }


  if (isVerbose == 1)
    {
      printf ("\n Version                   = 1.11\n");
      if (isUsingBlocks == 1)
        printf (" Allocate memory in blocks = True\n");
      else
        printf (" Allocate memory in blocks = False\n");
      if (isReleaseOnExit == 1)
        printf (" Release after allocated   = True\n");
      else
        printf (" Release after allocated   = False\n");
      if (isUsingSHM == 1)
        printf (" Use shared memory         = True\n");
      else
        printf (" Use shared memory         = False\n");
      if (isLockingMem == 1)
        printf (" Lock pages in memory      = True\n");
      else
        printf (" Lock pages in memory      = False\n");
      printf (" Total memory to allocate  = %d meg\n", Size);
      if (isUsingBlocks == 1)
        printf (" Allocated in block size   = %d meg\n", BlockSize);
    }


/*
     Are we creating a shared memory segment???
*/

  if (isUsingSHM == 1)
    {

      if (isLockingMem == 1)    /* lockpages if isLockingMem is set  */
        {
          Result2 = mlockall (MCL_CURRENT | MCL_FUTURE);

          if (Result2 == 0)
            printf ("Setting locked pages success.\n");
          else
            perror ("Setting locked pages failed ");
        }


      if (isUsingBlocks == 1)
        {                       /* in blocks */
          shmid = malloc (Size / BlockSize * sizeof (int));
          for (IterationCounter = BlockSize; IterationCounter <= Size; IterationCounter = IterationCounter + BlockSize)
            {
              *(shmid + shmidCount) = shmget (IPC_PRIVATE, BlockSize * 1048576, 0600);
              if (*(shmid + shmidCount) == -1)
                {
                  perror ("Allocated shared memory failed ");
                  Done (isReleaseOnExit, isLockingMem, isUsingSHM, isVerbose, shmid, shmidCount);
                }
              else
                {
                  shmptr = shmat (*(shmid + shmidCount), (char *) 0, 0);

                  /*
                     If we don't write to each (4k) page, then the kernel
                     will not create the page...
                   */
                  for (Count = 0; Count < (BlockSize * 1048576); Count = Count + 4096)
                    {
                      *(shmptr + Count) = 0;
                    }
                  printf ("Create shared segment, %i megs success. (shmid = %i)", IterationCounter, *(shmid + shmidCount));
                  if (isLockingMem == 1)
                    {
                      Result2 = shmctl (*(shmid + shmidCount), SHM_LOCK, 0);

                      if (Result2 == 0)
                        printf (" -locked success.\n");
                      else
                        printf (" -locked failed!!!\n");
                    }
                  else
                    printf ("\n");
                  shmdt (shmptr);
                }
              shmidCount++;
            }
        }

      else                      /* else create shared mem seg in one big chunk  */
        {
          shmid = malloc (sizeof (int));
          *(shmid + shmidCount) = shmget (IPC_PRIVATE, Size * 1048576, 0600);
          if (*(shmid + shmidCount) == -1)
            {
              perror ("Allocated shared memory failed ");
              exit (1);
            }
          else
            {
              shmptr = shmat ((*(shmid + shmidCount)), (char *) 0, 0);
              /*   
                 If we don't write to each (4k) page, then the kernel
                 will not create the page...
               */
              for (Count = 0; Count < (Size * 1048576); Count = Count + 4096)
                {
                  *(shmptr + Count) = 0;
                }
              printf ("Create shared segment, %i megs success. (shmid = %i)", Size, *(shmid + shmidCount));
            }
          if (isLockingMem == 1)
            {
              Result2 = shmctl (*(shmid + shmidCount), SHM_LOCK, 0);

              if (Result2 == 0)
                printf (" -locked success.\n");
              else
                printf (" -locked failed!!!\n");
            }
          else
            printf ("\n");

          shmdt (shmptr);
          shmidCount = 1;       /* here we're only allocating 1 block  */
        }



      Done (isReleaseOnExit, isLockingMem, isUsingSHM, isVerbose, shmid, shmidCount);
    }



/* 
    We must be allocating memory normally (data area).
    Using calloc (instead of malloc) to force the kernel
      to create the page.

*/
  if (isLockingMem == 1)        /* lock pages if isLockingMem is set  */
    {
      Result2 = mlockall (MCL_CURRENT | MCL_FUTURE);

      if (Result2 == 0)
        printf ("Setting locked pages success.\n");
      else
        perror ("Setting locked pages failed ");
    }

  if (isUsingBlocks == 1)       /* create in blocks */
    {
      for (IterationCounter = BlockSize; IterationCounter <= Size; IterationCounter = IterationCounter + BlockSize)
        {
          Result = calloc (262144 * BlockSize, sizeof (int));
          if (Result == 0)
            {
              printf ("Malloc %i meg failed.  Requested %i megs.\n", IterationCounter, Size);
              perror ("Error ");
              IterationCounter = Size;
            }
          else
            {
              printf ("Malloc %i meg success.  ", IterationCounter);
              printf ("Address 0x%x\n", Result);
            }
        }
      Done (isReleaseOnExit, isLockingMem, isUsingSHM, isVerbose, NULL, 0);
    }
  else
    {                           /* 1 big chunk  */
      Size = Size * 1024 * 256;
      Result = calloc (Size, sizeof (int));
      if (Result == 0)
        {
          printf ("Malloc %i megs failed.\n", Size / 1024 / 256);
          perror ("Error ");
          exit (1);
        }
      else
        printf ("Malloc %i megs success.\n", Size / 1024 / 256);

      Done (isReleaseOnExit, isLockingMem, isUsingSHM, isVerbose, NULL, 0);
    }
}




/*
     We are all done...  Do we wait or exit?  (isReleaseOnExit)
*/
Done (isReleaseOnExit, isLockingMem, isUsingSHM, isVerbose, shmid, shmidCount)
int isReleaseOnExit, isLockingMem, isUsingSHM, isVerbose, *shmid, shmidCount;

{
  int IterationCounter;

  if (isReleaseOnExit == 1)
    {
      if (isUsingSHM == 1)      /* Remove shared memory segments */
        {
          for (IterationCounter = 0; IterationCounter < shmidCount; IterationCounter++)
            {
              if (isVerbose == 1 && *(shmid + IterationCounter) != -1)
                printf ("   Removing shared memory segment %d \n", *(shmid + IterationCounter));
              shmctl (*(shmid + IterationCounter), IPC_RMID, 0);
            }
        }
      exit (0);
    }
  else
    {
      printf (" Waiting for BREAK signal ( [CTRL]-C )...\n");
      signal (SIGINT, Handle_Sig);  /* Wait for BREAK to exit */
      pause ();
      if (isVerbose == 1)
        printf ("    OK, received BREAK signal...");
      printf ("\n");

      if (isUsingSHM == 1)      /* Remove shared memory segments */
        {
          for (IterationCounter = 0; IterationCounter < shmidCount; IterationCounter++)
            {
              if (isVerbose == 1 && *(shmid + IterationCounter) != -1)
                printf ("   Removing shared memory segment %d \n", *(shmid + IterationCounter));
              shmctl (*(shmid + IterationCounter), IPC_RMID, 0);
            }

        }
      exit (0);
    }
}


void Handle_Sig (Junk)
int Junk;
{
  //   Do nothing...
}
