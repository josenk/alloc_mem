Introduction
------------

  This project is a simple C lang tool to allocate memory to test for memory limits of your Linux server.   You can allocate memory using calloc (a malloc call, but clears the page), or shared memory segments (shm).  Either type can be "locked into physical memory" to prevent the pages to be swapped out.  Allocating in one large chunk vs many small chunks is a way to check if you have memory fragmentation.  

Compiling
--------

  64bit kernels
```
make 64bit
```

  32bit kernels, or to test 32bit applications on 64bit kernel
```
make 32bit
```

Command Line Args
-----------------

  ./alloc_mem [-b] [-r] [-s] [-l] [-v] [size] [bsize]
         -b allocate in blocks (default std=1meg, shm=10meg)
         -r Release memory after it has been allocated
         -s Allocate using shared memory
         -l Lock pages in memory  (need root or MLOCK privs)
         -v Verbose
         size = Allocate specified in megs (default 10 megs)
        bsize = Specify block size (requires -b option)


        Example : ./alloc_mem -b 200 10       (Allocate 200 megs memory in 10 meg chunks)
                  ./alloc_mem -b -s 500       (Allocate 500 megs shared memory in 10 meg chunks)
                  ./alloc_mem -r -l 2000 100  (Allocate 2 gig memory in 100 meg chunks)



Memory allocation failures
--------------------------

 Notes: Failure to allocate a memory can be caused by few different things
              - your out of memory and/or swap.  (Virtual memory)
              - maxdsize, maxdsize_64
              - shmmni, shmmax, shmmnu
              - ulimit




Examples
--------


  Locked pages in memory, allocating 4GB of memory in 500MB segments.  Ultimately fails because there is not enough physical memory.
```
[root@nas alloc_mem]# ./alloc_mem -b -l 4000 500
Setting locked pages success.
Malloc 500 meg success.  Address 0x4ccab010
Malloc 1000 meg success.  Address 0x2d8aa010
Malloc 1500 meg success.  Address 0xe4a9010
Malloc 2000 meg success.  Address 0xef0a8010
Malloc 2500 meg success.  Address 0xcfca7010
Malloc 3000 meg failed.  Requested 4000 megs.
Error : Cannot allocate memory
 Waiting for BREAK signal ( [CTRL]-C )...
```

  Unlocked pages (pages allowed to be swapped out), allocating 4GB of memory in 500MB segments.  Passes because there is enough physical memory plus swap to allocate the pages.

```
[root@nas alloc_mem]# ./alloc_mem -b 4000 500
Malloc 500 meg success.  Address 0xdd83b010
Malloc 1000 meg success.  Address 0xbe43a010
Malloc 1500 meg success.  Address 0x9f039010
Malloc 2000 meg success.  Address 0x7fc38010
Malloc 2500 meg success.  Address 0x60837010
Malloc 3000 meg success.  Address 0x41436010
Malloc 3500 meg success.  Address 0x22035010
Malloc 4000 meg success.  Address 0x2c34010
 Waiting for BREAK signal ( [CTRL]-C )...
```

License
-------

Copyright (C) 2017 Jonathan Senkerik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.


Support
-------
  Website : http://www.jintegrate.co

  github  : http://github.com/josenk/alloc_mem

