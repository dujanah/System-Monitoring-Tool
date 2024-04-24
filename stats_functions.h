#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>

#define MAX_STRLEN 2048

typedef struct Memory {
   float phys_used;
   float phys_total;
   float virt_used;
   float virt_total;
} Memory;

typedef struct cpuInfo {
   float u_time;
   float t_time;
} cpuInfo;

void print_basic_info(int samples, int tdelay, int memory_kb);
void refresh_terminal();
Memory* report_memory(struct sysinfo si, int sample_number);
void print_memory(int samples, int sample_number,
					Memory memory_data[], int bool_graphics, int bool_sequential);
char* user_info();
double cpu_util(int sample_number);
void print_cpu(int bool_graphics, int bool_sequential, int sample_number, int samples, double utilization[]);
void print_sysinfo(struct utsname si);
void print_time(struct sysinfo si);