# System Monotoring Tool

System Monotoring Tool is a program that displays user and system stats on a Linux terminal, with varying displays based on user command arguments.

## How to Run
```bash
./smt command arguments ...
```

## How to Compile
```bash
make      # to compile source code into executable smt
clean     # remove all generated object and exe files
help      # for more information
```

## How to Use
```bash
--system      # if you only want to see system usage information
--user        # if you only want to see user usage information
--graphics    # if you want a graphical representation of memory and cpu utilization
--sequential  # if you prefer an independent report of each time interval
--samples=N   # if you would like to indicate your preferred number of samples
--tdelay=T    # if you would like to indicate your preferred delay after each sample

# NOTES
   # multiple commands can be used together
   # using both --system and --user will result in the same output
   # samples and tdelay can be positional arguments, in respective order
   # flag commands for samples and tdelay have precedence to respective positional arguments
   # the first solo integer after --samples=N will be consider tdelay
```
## Code
```c
	for (int i = 0; i < NUM_FUNCTIONS; i++){		
		// Create a pipe for corresponding process,
        // then fork processes
		pipe(fd[i]);
		r[i] = fork();
		
		// Let child process write to file
		// Let main process read to file
		if (r[i] == 0){ // Operations for child process
			// Close reading end to pipe from child process

			if (i == MEMORY_ID){
				// Write memory to pipe
			} else if (i == USER_ID){
				// Write user information to pipe
			} else if (i == CPU_ID){
				// Write CPU information to pipe
			}
		
			// Close writing end to pipe from child process once
			// done writing to pipe

		} else if (r > 0){ // Parent process operations
			// Close writing end to pipe from main process
			}
			
		}
```
This code forks three new processes for reporting memory, user information and CPU utilization. Each process terminates once done its work within the loop. Therefore, only the parent process forks new processes.

### Memory
```c
typedef struct Memory {
   float phys_used;
   float phys_total;
   float virt_used;
   float virt_total;
} Memory;

// Write to pipe
write(fd[i][1], memory, sizeof(Memory));
```
### Users
```c
// Write to pipe
write(fd[i][1], ui, strlen(ui));
```

### CPU Utilization
```c
typedef struct cpuInfo {
   float u_time;
   float t_time;
} cpuInfo;

for (int sample_number = 1; sample_number <= samples; sample_number++){	
// Collect cpu info for current sample with two samples
	*utilization = cpu_util(sample_number);

	// Write to pipe
	write(fd[i][1], utilization, sizeof(double));
}
```
### Printing
Reading from memory was done prior to printing so that main process did not print before information was collected.

### Terminating
All child processes were terminated after their work is done, with no processes forked in the child processes. Parent waits for all child processes before terminating.

### Handling Interruptions
```c
/* Handles CTRL_C */
static void quit(int signo){
	puts("\nWould you like to quit? yes/no : ");
	
	while (TRUE){
		// Takes user input until valid input is input
		// Returns back to call if input "no"
		// Exits process if input "yes"
	}
}

/* Handles CTRL_Z */
static void ignore(int signo){
	return;
}
```
Placed after forking processes loop so that only main receives these signals.

## How I Solved These Problems
### I used libraries:
  - sys/wait.h
  - signal.h
  - sys/utsname.h
  - utmp.h
  - unistd.h
  - sys/resource.h
  - sys/sysinfo.h

### I also used files:
  - /proc/stat to collect CPU utilization report