# Prettify-Child-Resource-Usage
An improvement over /usr/bin/time, to print child resource usage in pretty table

I was studying con-currency in various languages. The programming languages that i choose to compare concurrency are Go & C. 

The [/usr/bin/time](http://www.tutorialspoint.com/unix_commands/time.htm) command was the preffered one. Other way is to use the APIs provided by language/library to get resource usage. I choose to use /usr/bin/time.

But it has its limitation. For example the default resource usage provided by time is

```
$ /usr/bin/time ls
........
0.00user 0.00system 0:00.00elapsed 0%CPU (0avgtext+0avgdata 2488maxresident)k
0inputs+0outputs (0major+114minor)pagefaults 0swaps
```

Since it isn't readable, wanted to do something about it - also had time to kill. So wanted to implement pretty output of resource usage in two methods.

- One is developing a program similar to /usr/bin/time. Only that here i have better control over the output format. 
- Two is capturing the output of /usr/bin/time & presenting in a pretty format.

### 1. Implementing a program similar to /usr/bin/time, child_ru.c

  This was done using the [getrusage()](http://man7.org/linux/man-pages/man2/getrusage.2.html) system call in Linux. Man page of getrusage() provides following fields.

```
           struct rusage {
               struct timeval ru_utime; /* user CPU time used */
               struct timeval ru_stime; /* system CPU time used */
               long   ru_maxrss;        /* maximum resident set size */
               long   ru_ixrss;         /* integral shared memory size */       # not-maintained
               long   ru_idrss;         /* integral unshared data size */       # not-maintained
               long   ru_isrss;         /* integral unshared stack size */      # not-maintained
               long   ru_minflt;        /* page reclaims (soft page faults) */
               long   ru_majflt;        /* page faults (hard page faults) */
               long   ru_nswap;         /* swaps */ # not-maintained
               long   ru_inblock;       /* block input operations */
               long   ru_oublock;       /* block output operations */
               long   ru_msgsnd;        /* IPC messages sent */                 # not-maintained
               long   ru_msgrcv;        /* IPC messages received */             # not-maintained
               long   ru_nsignals;      /* signals received */                  # not-maintained
               long   ru_nvcsw;         /* voluntary context switches */
               long   ru_nivcsw;        /* involuntary context switches */
           };
```
  Of these some of the fields aren't implemented in Linux. In this implementation the tool outputs all the fields, that are implemented, along with these it also calculates CPU usage in percentage. This is achieved lby 

- End Time - Start Time = (1) # this is the clock time between child invocation and waiting on child. Obtained using gettimeofday()
- rusage.ru_utime + rusage.ru_stime = (2)
- (2)/(1) * 100 = (3)


Sample output looks thus.

```
$ ./child_ru ../amicable >/dev/null
+--------------+-----------+--------------+------+--------+---------------+---------------+--------+---------+-----------+--------------+
|Usr CPU       |Sys CPU    |Elap Time     |CPU % |RSS Max |Soft PgFaults  |Hard PgFaults  |Read FS |Write FS |Vol Switch |InVol Switch  |
+--------------+-----------+--------------+------+--------+---------------+---------------+--------+---------+-----------+--------------+
|53(s).77(us)  |0(s).0(us) |24(s).39(us)  |220.4 |2292KB  |102            |0              |0       |0        |71         |4614          |
+--------------+-----------+--------------+------+--------+---------------+---------------+--------+---------+-----------+--------------+

```
*The program makes use of the COLUMNS  environment variable, otherwise it defaults to 80 columns. Ensure COLUMNS is exported to child program.*

Building child_ru
```
  $ make -f Makefile.stat child_ru
  gcc -Wall -c -I. -ggdb  child_ru.c
  gcc -Wall -c -I. -ggdb  child_ru_print.c
  gcc -o child_ru child_ru.o child_ru_print.o -lm
```

### 2. Capturing the output of /usr/bin/time , child_ru_time.c

This uses /usr/bin/time command and its output. Since the tool has to parse time's output, the output needs to be in specific format. time command can take **-f FORMAT** option or use **TIME environment variable**. I have used TIME variable here.


- Block separator is {#}
- Field separator is ||


This is my sample TIME variable

```
$ echo $TIME
\n{#}\n||User||System||Elapse||CPU||(Data||Max)K||Ip+Op||Kern-Swtch||Vol-Swtch||\n{#}\n||%U||%S||%E||%P||(%D||%M)K||%I+%O||%c||%w||\n{#}
```

As you can see the header starts with Block separator, ends with block separator. And the value row too ends with block separator. The individual column heders & values are separated by field separator.** If the variable isn't configured as stated above, the program will fail.**

Building child_ru_time
```
$ make -f Makefile.stat child_ru_time
gcc -Wall -c -I. -ggdb  child_ru_time.c
gcc -Wall -c -I. -ggdb  parse_time.c
gcc -o child_ru_time child_ru_time.o parse_time.o -lm
```

Running child_ru_time
```
$ echo $TIME
\n{#}\n||User||System||Elapse||CPU||(Data||Max)K||Ip+Op||Kern-Swtch||Vol-Swtch||\n{#}\n||%U||%S||%E||%P||(%D||%M)K||%I+%O||%c||%w||\n{#}


$./child_ru_time /usr/bin/time ../amicable >/dev/null
+-------------+----------------+------------------+----------+-------------+----------------+-------------+--------------------------+
|User         |System          |Elapse            |CPU       |(Data        |Max)K           |Ip+Op        |Kern-Swtch                |
+-------------+----------------+------------------+----------+-------------+----------------+-------------+--------------------------+
|53.74        |0.06            |0:24.42           |220%      |(0           |2372)K          |0+0          |4907                      |
+-------------+----------------+------------------+----------+-------------+----------------+-------------+--------------------------+

```
