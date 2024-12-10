# memdoor

## Intro

When a Linux system encounters an Out-of-Memory (OOM) situation, it becomes challenging to gather crucial information such as memory allocation mappings or network connection details of the OOM-killed process before it terminates. This information is vital for users to debug the process behavior and understand why the Linux OOM killer terminated the process.

`memdoor` is a tool designed to provide real-time process information for OOM debugging. It offers the following features:

* Process RSS, PSS and USS memory usage

* Process page tables usage

* Process OOM score and OOM score adjustment value

* Process tree information in reverse order

* Process memory mappings

* Process network connection information(IPv4 and IPv6 TCP + UDP)

`memdoor` can operate in infinite loop mode or for a specified number of loops. It starts collecting process information when the process RSS memory usage ratio reaches or exceeds a specified memory pressure threshold.

## Compilation

To compile the `memdoor` binary, users can choose between two targets in the Makefile: the default target for compiling a dynamically linked binary, and the static target for compiling a statically linked binary.

To clean up the compiled runtime files, please use `make clean` to clean up the environment.

## Usage

```
$ ./memdoor 
memdoor version 1.7.0
usage: memdoor -p|--pid <target process id>
               -e|--exename <full path of target process>
               -i|--interval <second(s)>
               [-m|--memory-pressure-threshold <percentage integer>]
               [-c|--count <count(s)>]
               [-l|--lock-memory]
```

`-p` or `--pid`: the target process ID

`-e` or `--exename`: full absolute path of the target process executable file

`-i` or `--interval`: second(s) between each process information collection

`-m` or `--memory-pressure-threshold`: process memory usage percentage ratio. the formula is `process_rss_usage / total_memory_usage * 100`. the valid range is from 1 to 99 integer only. if this option is omitted, `memdoor` will still print the process information anyway

`-c` or `--count`: number of cycles would be used for process information collection. `memdoor` will go to an infinite loop mode if this option is not used

`-l` or `--lock-memory`: an option to enable the memory locking feature, preventing `memdoor`'s memory from being swapped out. Please note that enabling this feature may introduce additional overhead

`memdoor` will quit or stop running if it detects the command path of the target process ID does not match the full absolute path of the target process executable file. This will ensure `memdoor` is always tracking the correct process ID.

## Example

In this example, the program `oom` will keep allocating 1 MB memory in each iteration in an infinite loop and never free the memory region. The process ID is `31768`, the executable file full path is `/home/ericlee/oom`, the specified time period between each run is `1` second and the data collection is triggered when the process RSS memory usage is equal to or over `50%`. Because the program `oom` does not utilize any network resources so there's no network connection details.

At `Sun Nov 17 15:15:58 2024`, the process `31768` has been killed so the sanity check for process ID and executable file full path is failed then `memdoor` exited.

```
$ ./memdoor --pid 31768 --exename /home/ericlee/oom --interval 1 --memory-pressure-threshold 50
Report Time: Sun Nov 17 15:15:44 2024
##### PROCESS BASIC INFORMATION #####
PID: 31768
Executable Absolute Path: /home/ericlee/Projects/git/memdoor/oom

Process memory usage is not equal to or greater than input memory pressure threshold

<... skip ...>

Report Time: Sun Nov 17 15:15:55 2024
##### PROCESS BASIC INFORMATION #####
PID: 31768
Executable Absolute Path: /home/ericlee/Projects/git/memdoor/oom

##### PROCESS MEMORY INFORMATION #####
Total System Memory: 6786948 kB
Process RSS Memory Usage: 4305572 kB
Process PSS Memory Usage: 4305517 kB
Process USS Memory Usage: 4305488 kB
Process Page Tables Usage: 2016460 kB
Process OOM Score: 1301
Process OOM Score Adjustment Value: 0

##### PROCESS TREE INFORMATION #####
31768 (oom) - OOM score: 1301 - OOM adjustment score: 0 - RSS: 4305728 kB - PSS: 4305673 kB - USS: 4305644 kB
11922 (bash) - OOM score: 666 - OOM adjustment score: 0 - RSS: 1896 kB - PSS: 1892 kB - USS: 1892 kB
11920 (ld-linux-x86-64) - OOM score: 666 - OOM adjustment score: 0 - RSS: 988 kB - PSS: 984 kB - USS: 984 kB
521 (ld-linux-x86-64) - OOM score: 666 - OOM adjustment score: 0 - RSS: 4048 kB - PSS: 3842 kB - USS: 3640 kB
154 (systemd) - OOM score: 666 - OOM adjustment score: 0 - RSS: 1972 kB - PSS: 1968 kB - USS: 1968 kB
1 (systemd) - OOM score: 666 - OOM adjustment score: 0 - RSS: -1 kB - PSS: -1 kB - USS: -1 kB

##### PROCESS MEMORY MAPPING INFORMATION #####
START ADDRESS     SIZE                PERM  DEV    INODE        FILE PATH
0000564a761a7000  4               kB  r--p  00:2d  406032       /home/ericlee/Projects/git/memdoor/oom
0000564a761a8000  4               kB  r-xp  00:2d  406032       /home/ericlee/Projects/git/memdoor/oom
0000564a761a9000  4               kB  r--p  00:2d  406032       /home/ericlee/Projects/git/memdoor/oom
0000564a761aa000  4               kB  r--p  00:2d  406032       /home/ericlee/Projects/git/memdoor/oom
0000564a761ab000  4               kB  rw-p  00:2d  406032       /home/ericlee/Projects/git/memdoor/oom
0000564aa3e3b000  1035252280      kB  rw-p  00:00  0            [heap]
00007fab0c53c000  67371008        kB  rw-p  00:00  0            
00007fbb1c53c000  136             kB  r--p  00:2d  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fbb1c55e000  1380            kB  r-xp  00:2d  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fbb1c6b7000  316             kB  r--p  00:2d  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fbb1c706000  16              kB  r--p  00:2d  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fbb1c70a000  8               kB  rw-p  00:2d  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fbb1c70c000  24              kB  rw-p  00:00  0            
00007fbb1c71d000  4               kB  r--p  00:2d  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fbb1c71e000  128             kB  r-xp  00:2d  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fbb1c73e000  32              kB  r--p  00:2d  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fbb1c747000  4               kB  r--p  00:2d  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fbb1c748000  4               kB  rw-p  00:2d  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fbb1c749000  4               kB  rw-p  00:00  0            
00007ffe42225000  132             kB  rw-p  00:00  0            [stack]
00007ffe42252000  16              kB  r--p  00:00  0            [vvar]
00007ffe42256000  8               kB  r-xp  00:00  0            [vdso]
ffffffffff600000  4               kB  --xp  00:00  0            [vsyscall]

##### PROCESS NETWORK CONNECTION INFORMATION #####
PROT  STATE        L.ADDR                                       L.PORT  R.ADDR                                       R.PORT  TX QUEUE  RX QUEUE  


Report Time: Sun Nov 17 15:15:58 2024
ERROR: PID 31768 is not accessible: No such process
```

Please note that RSS, PSS and USS information in process tree is not retrievable sometime under normal user. In that case, the values of those items would be `-1`.

## ChangeLog

```
[04/29/2024] 1.0.0 - initial commit

[05/05/2024] 1.0.1 - bug issue#1 - low performance when dealing with a large number of network connections

[06/01/2024] 1.1.0 - fearure issue#4 - add ipv6 connection information

[06/14/2024] 1.2.0 - feature issue#6 - allow memdoor to dump output without specifying memory usage threshold

[07/15/2024] 1.3.0 - feature issue#8 - convert all atoi and atol functions to strtol-family functions

[09/02/2024] 1.4.0 - feature issue#10 - add process tree information

[11/17/2024] 1.5.0 - feature issue#12 - add uss memory usage information

[11/21/2024] 1.5.1 - bug issue#16 - optimize uss value calculation

[11/22/2024] 1.6.0 - feature issue#15 - add page tables usage information

[12/07/2024] 1.7.0 - feature issue#19 - add an option to enable memory locking
```
