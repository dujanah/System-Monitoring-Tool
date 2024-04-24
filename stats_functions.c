#include "stats_functions.h"

// print info that would be same whole time
void print_basic_info(int samples, int tdelay, int memory_kb){
	printf("Nbr of samples: %d -- every %d secs\n", samples, tdelay);
	printf("\tMemory usage: %d kilobytes\n", memory_kb);
	printf("---------------------------------------\n");
}

void refresh_terminal(){
	printf("\033[2J\033[H");
}

Memory* report_memory(struct sysinfo si, int sample_number){
	// memory values
	Memory* this = malloc(sizeof(Memory));
	this -> phys_total = si.totalram / 1000000000.0;
	this -> phys_used = (si.totalram - si.freeram) / 1000000000.0;
	this -> virt_total = (si.totalswap + si.totalram) / 1000000000.0;
	this -> virt_used = ((si.totalswap + si.totalram) - (si.freeram + si.freeswap)) / 1000000000.0;
	
	return this;
}

// print memory data upto required based on the second; "update" it
void print_memory(int samples, int sample_number, Memory memory_data[],
					int bool_graphics, int bool_sequential){
	// title
	printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");
	
	// print already collected data
	for (int i = 0; i < sample_number; i++){
		printf("\n");
		if (bool_sequential && i != sample_number - 1){
			continue;
		}
		
		printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB", memory_data[i].phys_used,
															memory_data[i].phys_total,
															memory_data[i].virt_used,
															memory_data[i].virt_total);
															
		if (bool_graphics){
			printf("|");
			if (i == 0){
				printf("o 0.00 (%.2f)", memory_data[i].virt_used);
			} else {
				double difference = memory_data[i].virt_used - memory_data[i-1].virt_used;
				difference *= 10;
				if (difference >= 0){
					for (int i = 0; i < difference; i++){
						printf("#");
					}
					printf("* %.2f (%.2f)", difference, memory_data[i].virt_used);
				} else if (difference < 0){
					difference *= -1;
					for (int i = 0; i < difference; i++){
						printf(":");
					}
					printf("@ %.2f (%.2f)", difference, memory_data[i].virt_used);
				}
			}
		} 
	}
	
	// print empty lines for uncollected data
	for (int i = sample_number; i < samples; i++){
		printf("\n");
	}
	
	// prnt divider
	printf("\n---------------------------------------\n");
}

char* non_string(char *s, int length){
	char* str = malloc(length + 1);
	
	int i;
	for (i = 0; i < length; i++){
		str[i] = s[i];
	}
	
	str[i] = '\0';
	
	return str;
}

char* user_info(){
	char* buffer = malloc(sizeof(char) * MAX_STRLEN);
	
	// add title 
	char title[] = "### Sessions/users ### \n";
	strncpy(buffer, title , MAX_STRLEN);
	buffer[strlen(title)] = '\0';
	
	
	// open file
	FILE *user_info = fopen("/var/run/utmp", "r");
	if (user_info == NULL){
		// add divider
		strncat(buffer, "---------------------------------------\n", MAX_STRLEN - strlen(buffer));
		return buffer;
	}
	
	printf("buffer is %s", buffer);
	
	
	// initialize user
	struct utmp user;
	
	
	while ((fread(&user, sizeof(struct utmp), 1, user_info)) == 1){
		if (((strcmp(user.ut_user, "")) == 0) || ((strcmp(user.ut_user, "reboot")) == 0) || ((strcmp(user.ut_user, "runlevel")) == 0)){
			continue;
		}
		
		// add users
		strncat(buffer, non_string(user.ut_user, UT_NAMESIZE), MAX_STRLEN - strlen(buffer));
		strncat(buffer, "\t", MAX_STRLEN - strlen(buffer));
		strncat(buffer, non_string(user.ut_line, UT_LINESIZE), MAX_STRLEN - strlen(buffer));
		strncat(buffer, "\t", MAX_STRLEN - strlen(buffer));
		strncat(buffer, non_string(user.ut_host, UT_HOSTSIZE), MAX_STRLEN - strlen(buffer));
		strncat(buffer, "\t\n", MAX_STRLEN - strlen(buffer));
	}
	
	// close file
	fclose(user_info);
	
	
	// addd divider
	strncat(buffer, "---------------------------------------\n", MAX_STRLEN - strlen(buffer));
	return buffer;
}

cpuInfo get_cpu_info(){
	// declare buffer values
	char buff[1024];
	int user;
	int nice;
	int sys;
	int idle;
	int iow;
	int irq;
	int sirq;
	
	// open file
	FILE *proc = fopen("/proc/stat", "r");
	
	// read values
	fscanf(proc, "%s %d %d %d %d %d %d %d", buff, &user, &nice, &sys, &idle, &iow, &irq, &sirq);
	fclose(proc);
	
	// create cpuInfo variable and store corresponding values
	cpuInfo cpu_info;
	cpu_info.t_time = user + nice + sys + idle + iow + irq + sirq + 0.0;
	cpu_info.u_time = cpu_info.t_time - idle;
	
	return cpu_info;
}

double cpu_util(int sample_number){
	cpuInfo s1 = get_cpu_info();
	sleep(1);
	cpuInfo s2 = get_cpu_info();
	
	return (s2.u_time - s1.u_time) / (s2.t_time - s1.t_time);
}

void print_cpu(int bool_graphics, int bool_sequential, int sample_number, int samples, double utilizations[]){	// print number of cores
	long cores = sysconf(_SC_NPROCESSORS_CONF);
	printf("Number of cores: %ld\n", cores);
	
	// print utilization
	printf("\ttotal cpu use: %.2f%%", utilizations[sample_number-1] * 100);
	
	////////////////////////////////////////////////////
	// print already collected data
	if (bool_graphics){
		for (int i = 0; i < sample_number; i++){
			printf("\n");
			if (bool_sequential && i != sample_number - 1){
				continue;
			}
			
			// prints a bar for each FULL  percent
			printf("\t");
			int bars = utilizations[i] * 100;
			for (int i = 0; i < bars; i++){
				printf("|");
			}
			printf(" %.2f", utilizations[i]*100);
		}
		
		// print empty lines for uncollected data
		for (int i = sample_number; i < samples; i++){
			printf("\n");
		}
	}
	//////////////////////////////////////////////////////
	
	// prnt divider
	printf("\n---------------------------------------\n");
}

void print_sysinfo(struct utsname un){
	printf("### System Information ###\n");
	printf("System Name = %s\n", un.sysname);
	printf("Machine Name = %s\n", un.nodename);
	printf("Version = %s\n", un.version);
	printf("Release = %s\n", un.release);
	printf("Architecture = %s\n", un.machine);
}

void print_time(struct sysinfo si){
	long seconds = si.uptime;
	
	long days = seconds / 86400;
	seconds -= days * 86400;
	long hours = seconds / 3600;
	seconds -= hours * 3600;
	long minutes = seconds / 60;
	seconds -= minutes * 60;
	
	printf("System running since last reboot: %ld days %02ld:%02ld:%02ld (%02ld:%02ld:%02ld)\n", days, hours, minutes, seconds,
																				 days*24+hours, minutes, seconds);
}