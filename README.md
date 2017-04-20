## Semaphore in Minix

### Instructions to run the program

* First implement system calls
    * Add pm server table entry
      * Open file `/usr/src/servers/pm/table.c` and replace no_sys with syscall given below. Any unused entry can be used. I have used 56, 58, 70, 79, 108, 109 and 110.
      ```c
        ...
        do_psem_init, /* 56 = psem_init */
        ...
        do_psem_wait, /* 58 = psem_wait */
        ...
        do_psem_signal, /* 70 = psem_signal */
        ...
        do_psem_destroy, /* 79 = psem_destroy */
        ...
        do_proc_no, /* 108 = proc_no */
        do_psem_block, /* 109 = psem_block */
        do_psem_wakeup, /* 110 = psem_wakeup */
        ...

      ```
    * Define system call number in `/usr/src/include/minix/callnr.h`
      ```c
        ...
        #define PSEM_INIT 56
        ...
        #define PSEM_WAIT 58
        ...
        #define PSEM_SIGNAL 70
        ...
        #define PSEM_DESTROY 79
        ...
        #define PROC_NO 108
        #define PSEM_BLOCK 109
        #define PSEM_WAKEUP 110
        ...
      ```
    * Define prototypes in `/usr/src/servers/pm/proto.h` under `/* misc.c */` list of definitions
      ```c
        /* misc.c */
        ...
        void* do_psem_init(void);
        int do_psem_wait(void);
        int do_psem_signal(void);
        int do_psem_destroy(void);
        int do_proc_no(void);
        int do_psem_block(void);
        int do_psem_wakeup(void);
        ...
      ```
    * Implement the syscalls. Add `psem.c` file in `/usr/src/servers/pm/`
    * Add this filename in `/usr/src/servers/pm/Makefile`
      ```
        SRCS= ... \
              ... psem.c
      ```
    * Compile and install
      * cd to `/usr/src/releasetools/` and run
      ```shell
        $ make services
        $ make install
      ```
    * Restart
* Comoile and run
  * Compile (clang is used to compile)
  ```shell
    $ cc prod_cons.c -lm
    $ cc producer.c -o producer -lm
    $ cc consumer.c -o consumer -lm
  ```
  * Run
  ```shell
    $ ./a.out 100 10 15 15 10 2 5
  ```
    Input is given as arguments
  ```
    $ ./a.out capacity np nc cntp cntc up uc
  ```
  * Log files are produced after the program has successfully run
