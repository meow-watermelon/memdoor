# memdoor

## Intro

When a Linux system encounters an Out-of-Memory (OOM) situation, it becomes challenging to gather crucial information such as memory allocation mappings or network connection details of the OOM-killed process before it terminates. This information is vital for users to debug the process behavior and understand why the Linux OOM killer terminated the process.

`memdoor` is a tool designed to provide real-time process information for OOM debugging. It offers the following features:

* Process RSS and PSS memory usage

* Process OOM score and OOM score adjustment value

* Process memory mappings

* Process network connection information(IPv4 and IPv6 TCP + UDP)

`memdoor` can operate in infinite loop mode or for a specified number of loops. It starts collecting process information when the process RSS memory usage ratio reaches or exceeds a specified memory pressure threshold.

## Compilation

To compile the `memdoor` binary, users can choose between two targets in the Makefile: the default target for compiling a dynamically linked binary, and the static target for compiling a statically linked binary.

To clean up the compiled runtime files, please use `make clean` to clean up the environment.

## Usage

```
$ ./memdoor 
memdoor version 1.3.0
usage: memdoor -p|--pid <target process id>
               -e|--exename <full path of target process>
               -i|--interval <second(s)>
               [-m|--memory-pressure-threshold <percentage integer>]
               [-c|--count <count(s)>]
```

`-p` or `--pid`: the target process ID

`-e` or `--exename`: full absolute path of the target process executable file

`-i` or `--interval`: second(s) between each process information collection

`-m` or `--memory-pressure-threshold`: process memory usage percentage ratio. the formula is `process_rss_usage / total_memory_usage * 100`. the valid range is from 1 to 99 integer only. if this option is omitted, `memdoor` will still print the process information anyway

`-c` or `--count`: number of cycles would be used for process information collection. `memdoor` will go to an infinite loop mode if this option is not used

`memdoor` will quit or stop running if it detects the command path of the target process ID does not match the full absolute path of the target process executable file. This will ensure `memdoor` is always tracking the correct process ID.

## Example

In this example, the program `oom` will keep allocating 1 MB memory in each iteration in an infinite loop and never free the memory region. The process ID is `18584`, the executable file full path is `/home/ericlee/oom`, the specified time period between each run is `1` second and the data collection is triggered when the process RSS memory usage is equal to or over `50%`. Because the program `oom` does not utilize any network resources so there's no network connection details.

At `Sun May  5 15:52:02 2024`, the process `18584` has been killed so the sanity check for process ID and executable file full path is failed then `memdoor` exited.

```
$ ./memdoor --pid 18584 --exename /home/ericlee/oom --interval 1 --memory-pressure-threshold 50
Report Time: Sun May  5 15:51:02 2024
##### PROCESS BASIC INFORMATION #####
PID: 18584
Executable Absolute Path: /home/ericlee/oom

Process memory usage is not equal to or greater than input memory pressure threshold

<... skip ...>

Report Time: Sun May  5 15:52:02 2024
##### PROCESS BASIC INFORMATION #####
PID: 18584
Executable Absolute Path: /home/ericlee/oom

##### PROCESS MEMORY INFORMATION #####
Total System Memory: 6790004 kB
Process RSS Memory Usage: 3937320 kB
Process PSS Memory Usage: 3937316 kB
Process OOM Score: 1285
Process OOM Score Adjustment Value: 0

##### PROCESS MEMORY MAPPING INFORMATION #####
START ADDRESS     SIZE                PERM  DEV    INODE        FILE PATH
000055f7952d6000  4               kB  r--p  00:2e  330129       /home/ericlee/oom
000055f7952d7000  4               kB  r-xp  00:2e  330129       /home/ericlee/oom
000055f7952d8000  4               kB  r--p  00:2e  330129       /home/ericlee/oom
000055f7952d9000  4               kB  r--p  00:2e  330129       /home/ericlee/oom
000055f7952da000  4               kB  rw-p  00:2e  330129       /home/ericlee/oom
000055f795a28000  1008056472      kB  rw-p  00:00  0            [heap]
00007f80937c5000  67371008        kB  rw-p  00:00  0            
00007f90a37c5000  136             kB  r--p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f90a37e7000  1380            kB  r-xp  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f90a3940000  316             kB  r--p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f90a398f000  16              kB  r--p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f90a3993000  8               kB  rw-p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f90a3995000  24              kB  rw-p  00:00  0            
00007f90a39a5000  4               kB  r--p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f90a39a6000  128             kB  r-xp  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f90a39c6000  32              kB  r--p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f90a39cf000  4               kB  r--p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f90a39d0000  4               kB  rw-p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f90a39d1000  4               kB  rw-p  00:00  0            
00007ffffc39d000  132             kB  rw-p  00:00  0            [stack]
00007ffffc3ed000  16              kB  r--p  00:00  0            [vvar]
00007ffffc3f1000  8               kB  r-xp  00:00  0            [vdso]
ffffffffff600000  4               kB  --xp  00:00  0            [vsyscall]

##### PROCESS NETWORK CONNECTION INFORMATION #####
PROT  STATE        L.ADDR                                       L.PORT  R.ADDR                                       R.PORT  TX QUEUE  RX QUEUE  


Report Time: Sun May  5 15:52:04 2024
ERROR: PID 18584 is not accessible: No such process
```

## ChangeLog

```
[04/29/2024] 1.0.0 - initial commit

[05/05/2024] 1.0.1 - bug issue#1 - low performance when dealing with a large number of network connections

[06/01/2024] 1.1.0 - fearure issue#4 - add ipv6 connection information

[06/14/2024] 1.2.0 - feature issue#6 - allow memdoor to dump output without specifying memory usage threshold

[07/15/2024] 1.3.0 - feature issue#8 - convert all atoi and atol functions to strtol-family functions
```
