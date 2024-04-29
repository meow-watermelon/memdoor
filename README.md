# memdoor

## Intro

When a Linux system encounters an Out-of-Memory (OOM) situation, it becomes challenging to gather crucial information such as memory allocation mappings or network connection details of the OOM-killed process before it terminates. This information is vital for users to debug the process behavior and understand why the Linux OOM killer terminated the process.

memdoor is a tool designed to provide real-time process information for OOM debugging. It offers the following features:

* Process RSS and PSS memory usage

* Process OOM score and OOM score adjustment value

* Process memory mappings

* Process network connection information(IPv4 TCP + UDP)

`memdoor can operate in infinite loop mode or for a specified number of loops. It starts collecting process information when the process RSS memory usage ratio reaches or exceeds a specified memory pressure threshold.

## Compilation

To compile the memdoor binary, users can choose between two targets in the Makefile: the default target for compiling a dynamically linked binary, and the static target for compiling a statically linked binary.

To clean up the compiled runtime files, please use `make clean` to clean up the environment.

## Usage

```
$ ./memdoor 
memdoor version 1.0.0
usage: memdoor -p|--pid <target process id>
               -e|--exename <full path of target process>
               -m|--memory-pressure-threshold <percentage integer>
               -i|--interval <second(s)>
               [-c|--count <count(s)>]
```

`-p` or `--pid`: the target process ID

`-e` or `--exename`: full absolute path of the target process executable file

`-m` or `--memory-pressure-threshold`: process memory usage percentage ratio. the formula is `process_rss_usage / total_memory_usage * 100`. the valid range is from 1 to 99 integer only

`-i` or `--interval`: second(s) between each process information collection

`-c` or `--count`: number of cycles would be used for process information collection. `memdoor` will go to an infinite loop mode if this option is not used

`memdoor` will quit or stop running if it detects the command path of the target process ID does not match the full absolute path of the target process executable file. This will ensure `memdoor` is always tracking the correct process ID.

## Example

In this example, the program `oom` will keep allocating 1 MB memory in each iteration in an infinite loop and never free the memory region. The process ID is `8893`, the executable file full path is `/home/ericlee/oom`, the specified time period between each run is `1` second and the data collection is triggered when the process RSS memory usage is equal to or over `50%`. Because the program `oom` does not utilize any network resources so there's no network connection details.

At `Sun Apr 28 22:44:37 2024`, the process `8893` has been killed so the sanity check for process ID and executable file full path is failed then `memdoor` exited.

```
$ ./memdoor --pid 8893 --exename /home/ericlee/oom --interval 1 --memory-pressure-threshold 50
Report Time: Sun Apr 28 22:44:24 2024
##### PROCESS BASIC INFORMATION #####
PID: 8893
Executable Absolute Path: /home/ericlee/oom

Process memory usage is not equal to or greater than input memory pressure threshold

<... skip ...>

Report Time: Sun Apr 28 22:44:33 2024
##### PROCESS BASIC INFORMATION #####
PID: 8893
Executable Absolute Path: /home/ericlee/oom

##### PROCESS MEMORY INFORMATION #####
Total System Memory: 6790004 kB
Process RSS Memory Usage: 3615104 kB
Process PSS Memory Usage: 3615100 kB
Process OOM Score: 1238
Process OOM Score Adjustment Value: 0

##### PROCESS MEMORY MAPPING INFORMATION #####
START ADDRESS     SIZE                PERM  DEV    INODE      FILE PATH
000055e87e594000  4               kB  r--p  00:2e  330129     /home/ericlee/oom
000055e87e595000  4               kB  r-xp  00:2e  330129     /home/ericlee/oom
000055e87e596000  4               kB  r--p  00:2e  330129     /home/ericlee/oom
000055e87e597000  4               kB  r--p  00:2e  330129     /home/ericlee/oom
000055e87e598000  4               kB  rw-p  00:2e  330129     /home/ericlee/oom
000055e87fb51000  926224312       kB  rw-p  00:00  0          [heap]
00007fda8ec5b000  67371008        kB  rw-p  00:00  0          
00007fea9ec5b000  136             kB  r--p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ec7d000  1380            kB  r-xp  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9edd6000  316             kB  r--p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ee25000  16              kB  r--p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ee29000  8               kB  rw-p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ee2b000  24              kB  rw-p  00:00  0          
00007fea9ee3b000  4               kB  r--p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee3c000  128             kB  r-xp  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee5c000  32              kB  r--p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee65000  4               kB  r--p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee66000  4               kB  rw-p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee67000  4               kB  rw-p  00:00  0          
00007fffdd66b000  132             kB  rw-p  00:00  0          [stack]
00007fffdd783000  16              kB  r--p  00:00  0          [vvar]
00007fffdd787000  8               kB  r-xp  00:00  0          [vdso]
ffffffffff600000  4               kB  --xp  00:00  0          [vsyscall]

##### PROCESS NETWORK CONNECTION INFORMATION #####
PROT STATE        LOCAL ADDRESS         REMOTE ADDRESS        TX QUEUE    RX QUEUE    


Report Time: Sun Apr 28 22:44:35 2024
##### PROCESS BASIC INFORMATION #####
PID: 8893
Executable Absolute Path: /home/ericlee/oom

##### PROCESS MEMORY INFORMATION #####
Total System Memory: 6790004 kB
Process RSS Memory Usage: 3942664 kB
Process PSS Memory Usage: 3942660 kB
Process OOM Score: 1286
Process OOM Score Adjustment Value: 0

##### PROCESS MEMORY MAPPING INFORMATION #####
START ADDRESS     SIZE                PERM  DEV    INODE      FILE PATH
000055e87e594000  4               kB  r--p  00:2e  330129     /home/ericlee/oom
000055e87e595000  4               kB  r-xp  00:2e  330129     /home/ericlee/oom
000055e87e596000  4               kB  r--p  00:2e  330129     /home/ericlee/oom
000055e87e597000  4               kB  r--p  00:2e  330129     /home/ericlee/oom
000055e87e598000  4               kB  rw-p  00:2e  330129     /home/ericlee/oom
000055e87fb51000  1009338540      kB  rw-p  00:00  0          [heap]
00007fda8ec5b000  67371008        kB  rw-p  00:00  0          
00007fea9ec5b000  136             kB  r--p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ec7d000  1380            kB  r-xp  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9edd6000  316             kB  r--p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ee25000  16              kB  r--p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ee29000  8               kB  rw-p  00:2e  318324     /usr/lib/x86_64-linux-gnu/libc-2.31.so
00007fea9ee2b000  24              kB  rw-p  00:00  0          
00007fea9ee3b000  4               kB  r--p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee3c000  128             kB  r-xp  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee5c000  32              kB  r--p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee65000  4               kB  r--p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee66000  4               kB  rw-p  00:2e  318320     /usr/lib/x86_64-linux-gnu/ld-2.31.so
00007fea9ee67000  4               kB  rw-p  00:00  0          
00007fffdd66b000  132             kB  rw-p  00:00  0          [stack]
00007fffdd783000  16              kB  r--p  00:00  0          [vvar]
00007fffdd787000  8               kB  r-xp  00:00  0          [vdso]
ffffffffff600000  4               kB  --xp  00:00  0          [vsyscall]

##### PROCESS NETWORK CONNECTION INFORMATION #####
PROT STATE        LOCAL ADDRESS         REMOTE ADDRESS        TX QUEUE    RX QUEUE    


Report Time: Sun Apr 28 22:44:37 2024
ERROR: PID 8893 does not match the executable name /home/ericlee/oom
```

## ChangeLog

```
[04/29/2024] 1.0.0 - initial commit
```
