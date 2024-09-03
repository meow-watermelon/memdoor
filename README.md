# memdoor

## Intro

When a Linux system encounters an Out-of-Memory (OOM) situation, it becomes challenging to gather crucial information such as memory allocation mappings or network connection details of the OOM-killed process before it terminates. This information is vital for users to debug the process behavior and understand why the Linux OOM killer terminated the process.

`memdoor` is a tool designed to provide real-time process information for OOM debugging. It offers the following features:

* Process RSS and PSS memory usage

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
memdoor version 1.4.0
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

In this example, the program `oom` will keep allocating 1 MB memory in each iteration in an infinite loop and never free the memory region. The process ID is `20175`, the executable file full path is `/home/ericlee/oom`, the specified time period between each run is `1` second and the data collection is triggered when the process RSS memory usage is equal to or over `50%`. Because the program `oom` does not utilize any network resources so there's no network connection details.

At `Mon Sep  2 21:28:47 2024`, the process `20175` has been killed so the sanity check for process ID and executable file full path is failed then `memdoor` exited.

```
$ ./memdoor --pid 20175 --exename /home/ericlee/oom --interval 1 --memory-pressure-threshold 50
Report Time: Mon Sep  2 21:28:38 2024
##### PROCESS BASIC INFORMATION #####
PID: 20175
Executable Absolute Path: /home/ericlee/oom

Process memory usage is not equal to or greater than input memory pressure threshold

<... skip ...>

Report Time: Mon Sep  2 21:28:44 2024
##### PROCESS BASIC INFORMATION #####
PID: 20175
Executable Absolute Path: /home/ericlee/oom

##### PROCESS MEMORY INFORMATION #####
Total System Memory: 6791036 kB
Process RSS Memory Usage: 3705172 kB
Process PSS Memory Usage: 3705168 kB
Process OOM Score: 1251
Process OOM Score Adjustment Value: 0

##### PROCESS TREE INFORMATION #####
20175 (oom) - OOM score: 1251 - OOM adjustment score: 0 - RSS: 3706188 kB - PSS: 3706184 kB
9361 (bash) - OOM score: 666 - OOM adjustment score: 0 - RSS: 1896 kB - PSS: 1892 kB
9359 (ld-linux-x86-64) - OOM score: 666 - OOM adjustment score: 0 - RSS: 1508 kB - PSS: 1504 kB
535 (ld-linux-x86-64) - OOM score: 666 - OOM adjustment score: 0 - RSS: 4596 kB - PSS: 4454 kB
176 (systemd) - OOM score: 666 - OOM adjustment score: 0 - RSS: 1984 kB - PSS: 1980 kB
1 (systemd) - OOM score: 666 - OOM adjustment score: 0 - RSS: -1 kB - PSS: -1 kB

##### PROCESS MEMORY MAPPING INFORMATION #####
START ADDRESS     SIZE                PERM  DEV    INODE        FILE PATH
000055cd5b47a000  4               kB  r--p  00:2e  330129       /home/ericlee/oom
000055cd5b47b000  4               kB  r-xp  00:2e  330129       /home/ericlee/oom
000055cd5b47c000  4               kB  r--p  00:2e  330129       /home/ericlee/oom
000055cd5b47d000  4               kB  r--p  00:2e  330129       /home/ericlee/oom
000055cd5b47e000  4               kB  rw-p  00:2e  330129       /home/ericlee/oom
000055cd5d464000  948955412       kB  rw-p  00:00  0            [heap]
00007f24b5fa6000  67371008        kB  rw-p  00:00  0            
00007f34c5fa6000  136             kB  r--p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f34c5fc8000  1380            kB  r-xp  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f34c6121000  316             kB  r--p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f34c6170000  16              kB  r--p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f34c6174000  8               kB  rw-p  00:2e  332162       /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007f34c6176000  24              kB  rw-p  00:00  0            
00007f34c6187000  4               kB  r--p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f34c6188000  128             kB  r-xp  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f34c61a8000  32              kB  r--p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f34c61b1000  4               kB  r--p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f34c61b2000  4               kB  rw-p  00:2e  332158       /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007f34c61b3000  4               kB  rw-p  00:00  0            
00007ffd51a0e000  132             kB  rw-p  00:00  0            [stack]
00007ffd51b05000  16              kB  r--p  00:00  0            [vvar]
00007ffd51b09000  8               kB  r-xp  00:00  0            [vdso]
ffffffffff600000  4               kB  --xp  00:00  0            [vsyscall]

##### PROCESS NETWORK CONNECTION INFORMATION #####
PROT  STATE        L.ADDR                                       L.PORT  R.ADDR                                       R.PORT  TX QUEUE  RX QUEUE  


Report Time: Mon Sep  2 21:28:47 2024
ERROR: PID 20175 does not match the executable name /home/ericlee/oom
```

Please note that RSS / PSS information in process tree is not retrievable sometime under normal user. In that case, the values of those items would be `-1`.

## ChangeLog

```
[04/29/2024] 1.0.0 - initial commit

[05/05/2024] 1.0.1 - bug issue#1 - low performance when dealing with a large number of network connections

[06/01/2024] 1.1.0 - fearure issue#4 - add ipv6 connection information

[06/14/2024] 1.2.0 - feature issue#6 - allow memdoor to dump output without specifying memory usage threshold

[07/15/2024] 1.3.0 - feature issue#8 - convert all atoi and atol functions to strtol-family functions

[09/02/2024] 1.4.0 - feature issue#10 - add process tree information
```
