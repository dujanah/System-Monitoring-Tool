#include "stats_functions.h"

#define NUM_FUNCTIONS 3
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define MEMORY_ID 0
#define USER_ID 1
#define CPU_ID 2

#define TRUE 1
#define FALSE 0

// HELPER FUNCTIONS
int set_rusage(struct rusage *ru);
int set_sysinfo(struct sysinfo *si);
static void quit(int signo);
static void ignore(int signo);

int main(int argc, char **argv){	
	///////////////////// SET VALUES OF BOOLEANS ////////////////////////
	// default values
	int samples = 10;
	int tdelay = 1;
	int bool_memory = 1;
	int bool_user = 1;
	int bool_cpu = 1;
	int bool_system = 1;
	int bool_sequential = 0;
	int bool_graphics = 0;
		
	int command_system = 0;
	int command_user = 0;
	int command_samples = 0;
	int command_tdelay = 0;
	
	// temp var
	int temp;
	// behavior based on arguments
	for (int i = 1; i < argc; i++){
		if (strcmp(argv[i], "--system") == 0){
			command_system = 1;
		} else if (strcmp(argv[i], "--user") == 0){
			command_user = 1;
		} else if (strcmp(argv[i], "--graphics") == 0){
			bool_graphics = 1;
		} else if (strcmp(argv[i], "--sequential") == 0){
			bool_sequential = 1;
		} else if (strncmp(argv[i], "--samples=xx", 10) == 0){
			command_samples = 1;
			if (sscanf(argv[i], "--samples=%d", &temp) == 1){
				samples = temp;
			}
		} else if (strncmp(argv[i], "--tdelay=xx", 9) == 0){
			command_tdelay = 1;
			if (sscanf(argv[i], "--tdelay=%d", &temp) == 1){
				tdelay = temp;
			}
		} else {
			int num = 4;
			if (sscanf(argv[i], "%d", &num) != 1){
				continue;
			}
			
			if (command_samples == 0){
					command_samples = 1;
					samples = num;
			} else if (command_tdelay == 0){
				command_tdelay = 1;
				tdelay = num;
			}
		}
	}
	
	// behavior based on commands (that are already not specified)
	if (command_system && !(command_user)){
		bool_user = 0;
	} else if (command_user && !(command_system)){
		bool_cpu = 0;
		bool_memory = 0;
		bool_system = 0;
	} 
	
	// memory variables
	struct rusage ru;
	if(set_rusage(&ru)){
		return 1;
	}
	
	struct sysinfo si;
	if(set_sysinfo(&si)){
		return 1;
	}
	
	struct utsname un;
	if (uname(&un)){
		return 1;
	}
	
	///////////////////////////// FINISH ////////////////////////////////
	
	// Return value of fork() functions
	int r[NUM_FUNCTIONS];
	
	// Declare array for file descriptors of pipes for each child process
	int fd[NUM_FUNCTIONS][2];
	
	for (int i = 0; i < NUM_FUNCTIONS; i++){		
		// Create a pipe for ith function
		if (pipe(fd[i]) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		r[i] = fork();
		if (r[i] == -1){
			perror("fork");
			exit(EXIT_FAILURE);
		}
		
		// Let child process write to file
		// Let main process read to file
		if (r[i] == 0){ // Operations for corresponding function
			// Close reading end to pipe from child process
			if (close(fd[i][0]) == -1){
				perror("close");
				exit(EXIT_FAILURE);
			}
			
			if (i == MEMORY_ID){
				// Update array of Memory structs containing memory info reports
				for (int sample_number = 1; sample_number <= samples; sample_number++){
					if (bool_memory && i == MEMORY_ID){
						// Collect sample_number's memory info
						Memory* memory = report_memory(si, sample_number);
						
						// Write to pipe
						write(fd[i][1], memory, sizeof(Memory));
						
						// Free memory allocated 
						free(memory);
					}
				}
			} else if (i == USER_ID){
				// Collect all user info in a string
				char *ui = user_info();
				
				// Write to pipe
				write(fd[i][1], ui, strlen(ui));

				// Free memory allocated 
				free(ui);
			} else if (i == CPU_ID){
				for (int sample_number = 1; sample_number <= samples; sample_number++){
					// Collect cpu info for current sample with two samples
					double* utilization = malloc(sizeof(double));
					*utilization = cpu_util(sample_number);
					
					// Write to pipe
					write(fd[i][1], utilization, sizeof(double));
					
					// Free memory allocation
					free(utilization);
				}
			}
		
			// Close writing end to pipe from child process once
			// done writing to pipe
			if (close(fd[i][1]) == -1){
				perror("close");
				exit(EXIT_FAILURE);
			}
			
			// Terminate process successfully
			exit(EXIT_SUCCESS);
		} else if (r > 0){ // Parent process operations
			// Close writing end to pipe from main process
			if (close(fd[i][1]) == -1){
				perror("close");
				exit(EXIT_FAILURE);
			}
			
		} else if (r[i] == -1){ // Error checking
			// Terminate process unsuccessfully
			exit(EXIT_FAILURE);
		}
	}
	
	
	
	// Set up signal handler for CTRL-C
	struct sigaction ctrl_c;
	ctrl_c.sa_handler = quit;
	ctrl_c.sa_flags = 0;
	
	if (sigaction(SIGINT, &ctrl_c, NULL) == -1){
		exit(1);
	}
	
	// Set up signal handler for CTRL-Z
	struct sigaction ctrl_z;
	ctrl_z.sa_handler = ignore;
	ctrl_z.sa_flags = 0;
	
	if (sigaction(SIGTSTP, &ctrl_z, NULL) == -1){
		exit(1);
	}
	
	// Read user data from pipe
	int result;
	char* user_data = malloc(sizeof(char) * MAX_STRLEN);
	result = read(fd[USER_ID][0], user_data, sizeof(char) * MAX_STRLEN);
	
	if (result == -1){
		perror("read");
	}
	
	// Read data from pipe and print according
	Memory* mem_data = malloc(sizeof(Memory) * samples);
	double* cpu_data = malloc(sizeof(double) * samples);
	
	// print info
	for (int sample_number = 1; sample_number <= samples; sample_number++){
		
		if (!(bool_sequential)){
			refresh_terminal();
		}
		
		print_basic_info(samples, tdelay, ru.ru_maxrss);
		
		if (bool_memory){
			result = read(fd[MEMORY_ID][0], mem_data + sample_number - 1, sizeof(Memory));
			print_memory(samples, sample_number, mem_data, bool_graphics, bool_sequential);
		}
		
		if (bool_user){
			printf("%s", user_data);
			
		}
		
		if (bool_cpu){
			result = read(fd[CPU_ID][0], cpu_data + sample_number - 1, sizeof(double));
			print_cpu(bool_graphics, bool_sequential, sample_number, samples, cpu_data);
		} 
		
		if (bool_system){
			print_sysinfo(un);
			print_time(si);
		}
		
		if (result == -1){
			perror("read");
		}
		
		printf("---------------------------------------\n");
		sleep(tdelay);
	}
	
	free(mem_data);
	free(user_data);
	free(cpu_data);
	
	// Close reading end to pipe from main process
	for (int i = 0; i < NUM_FUNCTIONS; i++){
		if (close(fd[i][0]) == -1){
				perror("close");
				exit(EXIT_FAILURE);
			}
	}
	
	// Wait for all child processes to terminate
	for (int i = 0; i < NUM_FUNCTIONS; i++){
		int status;
		if (wait(&status) == -1)  {
			perror("wait");
		}
	}
	
	return 0;
}

int set_rusage(struct rusage *ru){
	return (getrusage(RUSAGE_SELF, ru));
}

int set_sysinfo(struct sysinfo *si){
	return (sysinfo(si));
}

static void quit(int signo){
	char buffer[MAX_STRLEN];
	
	puts("\nWould you like to quit? yes/no : ");
	
	while (TRUE){
		scanf("%s", buffer);
		
		if (strcmp(buffer, "yes") == 0){
			exit(EXIT_SUCCESS);
		} else if (strcmp(buffer, "no") == 0){
			return;
		} else {
			printf("\nInvalid answer. Try again.\n");
		}
	}
}

static void ignore(int signo){
	return;
}