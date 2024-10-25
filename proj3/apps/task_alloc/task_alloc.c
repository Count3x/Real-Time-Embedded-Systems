/*
 *
 * CS 251/EE 255: Real-Time Embedded Systems
 * HW Project #3: Task Allocation and Virtual Memory Management
 * Group 16
 * Hengshuo Zhang
 * Zhaoze Sun
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


/*
 * This struct represents a task with a name, CPU time, and deadline.
 * It is used to store the tasks read from the input file and to allocate
 * them to CPUs using different allocation policies.
 */
typedef struct {
    char name[21];
    int C;
    int T;
} task_t;


/*
 * This function uses Best-Fit Decreasing (BFD) algorithm to allocate tasks to CPUs.
 *
 * The function first sorts the tasks in non-increasing order of deadline and then allocates
 * the tasks to CPUs using BFD algorithm. If a task cannot be allocated to any CPU, the function
 * tries to re-allocate the task to another CPU that can accommodate it. If re-allocation is not
 * possible, the function returns "Failure". Otherwise, it returns "Success" and prints the
 * CPU assignments.
 */
void allocate_tasks_bfd(int B, task_t *tasks, int N) {
    int *cpu_assignments = (int *)calloc(N, sizeof(int));
    int *bin_capacities = (int *)calloc(B, sizeof(int));
    for (int i = 0; i < B; i++) bin_capacities[i] = 10;

    // Sort tasks in non-increasing order of deadline
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            double ratio_i = (double)tasks[i].C / tasks[i].T;
            double ratio_j = (double)tasks[j].C / tasks[j].T;
            if (ratio_i < ratio_j || (ratio_i == ratio_j && tasks[i].C > tasks[j].C)) {
                task_t temp = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = temp;
            }
        }
    }

    // Allocate tasks to CPUs using Best-Fit Decreasing (BFD)
    for (int i = 0; i < N; i++) {
        int j, min_cap = INT_MAX, min_cap_index = -1;
        for (j = 0; j < B; j++) {
            if (tasks[i].C <= bin_capacities[j] && bin_capacities[j] - tasks[i].C < min_cap) {
                min_cap = bin_capacities[j] - tasks[i].C;
                min_cap_index = j;
            }
        }
        if (min_cap_index == -1) {
            // Try to re-allocate the task to another CPU
            int max_execution_time = 0;
            int max_execution_time_task_index = -1;
            for (int k = i; k < N; k++) {
                if (cpu_assignments[k] != -1 && tasks[k].C > max_execution_time) {
                    max_execution_time = tasks[k].C;
                    max_execution_time_task_index = k;
                }
            }
            if (max_execution_time_task_index == -1) {
                printf("Failure\n");
                free(cpu_assignments);
                free(bin_capacities);
                return;
            }
            int old_bin = cpu_assignments[max_execution_time_task_index];
            bin_capacities[old_bin] += tasks[max_execution_time_task_index].C;
            int reallocated = 0;
            for (int j = 0; j < B; j++) {
                if (j != old_bin && tasks[max_execution_time_task_index].C <= bin_capacities[j]) {
                    cpu_assignments[max_execution_time_task_index] = j;
                    bin_capacities[j] -= tasks[max_execution_time_task_index].C;
                    reallocated = 1;
                    break;
                }
            }
            if (!reallocated) {
                printf("Failure\n");
                free(cpu_assignments);
                free(bin_capacities);
                return;
            }
        } else {
            cpu_assignments[i] = min_cap_index;
            bin_capacities[min_cap_index] -= tasks[i].C;
        }
    }

// Print the CPU assignments
    printf("Success\n");
    for (int j = 0; j < B; j++) {
        printf("CPU%d", j);
        int allocated = 0;
        for (int i = 0; i < N; i++) {
            if (cpu_assignments[i] == j) {
                printf(",%s", tasks[i].name);
                allocated = 1;
            }
        }
        if (!allocated) {
            printf(" ");
        }
        printf("\n");
    }

    // Free memory
    free(cpu_assignments);
    free(bin_capacities);
}


/*
 * This function allocates tasks to CPUs using the Worst-Fit Decreasing (WFD) algorithm.
 *
 * It first sorts the tasks in non-increasing order of execution time, and then initializes
 * the CPU assignments and bin capacities arrays. It then allocates tasks to CPUs using WFD,
 * which involves selecting the bin with the largest capacity that can accommodate the task.
 * If there is no such bin, it returns "Failure". Next, it checks if any CPU is overloaded
 * and re-allocates tasks if necessary. If there is still an overloaded CPU after re-allocation,
 * it returns "Failure". Finally, it prints the CPU assignments and frees the allocated memory.
 */

void allocate_tasks_wfd(int B, task_t *tasks, int N) {
    // Sort tasks in non-increasing order of execution time
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            double ratio_i = (double)tasks[i].C / tasks[i].T;
            double ratio_j = (double)tasks[j].C / tasks[j].T;
            if (ratio_i < ratio_j || (ratio_i == ratio_j && tasks[i].C > tasks[j].C)) {
                task_t temp = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = temp;
            }
        }
    }

    // Initialize the CPU assignments array
    int *cpu_assignments = (int *) malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        cpu_assignments[i] = -1;
    }

    // Initialize the bin capacities array
    int *bin_capacities = (int *) malloc(B * sizeof(int));
    for (int i = 0; i < B; i++) {
        bin_capacities[i] = 10;
    }

    // Allocate tasks to CPUs using Worst-Fit Decreasing (WFD)
    for (int i = 0; i < N; i++) {
        int worst_fit_bin = -1;
        int worst_fit_capacity = -1;
        for (int j = 0; j < B; j++) {
            if (tasks[i].C <= bin_capacities[j] && bin_capacities[j] > worst_fit_capacity) {
                worst_fit_bin = j;
                worst_fit_capacity = bin_capacities[j];
            }
        }
        if (worst_fit_bin == -1) {
            printf("Failure\n");
            free(cpu_assignments);
            free(bin_capacities);
            return;
        } else {
            cpu_assignments[i] = worst_fit_bin;
            bin_capacities[worst_fit_bin] -= tasks[i].C;
        }
    }

    // Check if any CPU is overloaded and re-allocate tasks if necessary
    int *cpu_execution_times = (int *) calloc(B, sizeof(int));
    for (int i = 0; i < N; i++) {
        cpu_execution_times[cpu_assignments[i]] += tasks[i].C;
    }
    int reallocated = 0;
    for (int i = 0; i < B; i++) {
        if (cpu_execution_times[i] > 10) {
            // Find the task with the largest execution time assigned to this CPU
            int max_execution_time = -1;
            int max_execution_time_task_index = -1;
            for (int j = 0; j < N; j++) {
                if (cpu_assignments[j] == i && tasks[j].C > max_execution_time) {
                    max_execution_time = tasks[j].C;
                    max_execution_time_task_index = j;
                }
            }
            if (max_execution_time_task_index == -1) {
                printf("Failure\n");
                free(cpu_assignments);
                free(bin_capacities);
                free(cpu_execution_times);
                return;
            } else {
                // Re-allocate the task to another CPU
                int min_capacity = INT_MAX;
                int min_capacity_bin = -1;
                for (int j = 0; j < B; j++) {
                    if (tasks[max_execution_time_task_index].C <= bin_capacities[j] &&
                        bin_capacities[j] < min_capacity) {
                        min_capacity = bin_capacities[j];
                        min_capacity_bin = j;
                    }
                }
                if (min_capacity_bin != -1) {
                    int old_bin = cpu_assignments[max_execution_time_task_index];
                    cpu_assignments[max_execution_time_task_index] = min_capacity_bin;
                    bin_capacities[old_bin] += tasks[max_execution_time_task_index].C;
                    bin_capacities[min_capacity_bin] -= tasks[max_execution_time_task_index].C;
                    reallocated = 1;
                    break;
                }
            }
            if (!reallocated) {
                printf("Failure\n");
                free(cpu_assignments);
                free(bin_capacities);
                free(cpu_execution_times);
                return;
            }
        }
    }
    // Print the CPU assignments
    printf("Success\n");
    for (int i = 0; i < B; i++) {
        printf("CPU%d", i);
        int cpu_time = 0;
        for (int j = 0; j < N; j++) {
            if (cpu_assignments[j] == i) {
                printf(",%s", tasks[j].name);
                cpu_time += tasks[j].C;
            }
        }
        if (cpu_time == 0) {
            printf(" ");
        }
        printf("\n");
    }

    // Free memory
    free(cpu_assignments);
    free(bin_capacities);
    free(cpu_execution_times);
}

/*
 * This function allocates a set of tasks to a given number of CPUs using the First-Fit Decreasing (FFD) heuristic.
 *
 * The FFD heuristic sorts the tasks in non-increasing order of their execution time to ensure that larger tasks are
 * assigned to CPUs with sufficient capacity. Then, it assigns each task to the first CPU with enough capacity to
 * execute the task. If no such CPU exists, the function returns "Failure".
 */
void allocate_tasks_ffd(int B, task_t *tasks, int N) {
    // Sort tasks according to the FFD heuristic
    for (int i = 0; i < N - 1; i++) {
        for (int j = i + 1; j < N; j++) {
            double ratio_i = (double)tasks[i].C / tasks[i].T;
            double ratio_j = (double)tasks[j].C / tasks[j].T;
            if (ratio_i < ratio_j || (ratio_i == ratio_j && tasks[i].C > tasks[j].C)) {
                task_t temp = tasks[i];
                tasks[i] = tasks[j];
                tasks[j] = temp;
            }
        }
    }

    // Initialize the CPU assignments array
    int *cpu_assignments = (int *)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        cpu_assignments[i] = -1;
    }

    // Initialize the bin capacities array
    int *bin_capacities = (int *)malloc(B * sizeof(int));
    for (int i = 0; i < B; i++) {
        bin_capacities[i] = 10;
    }

    // Allocate tasks to CPUs using the First-Fit Decreasing (FFD) heuristic
    for (int i = 0; i < N; i++) {
        int assigned_bin = -1;
        for (int j = 0; j < B; j++) {
            if (tasks[i].C <= bin_capacities[j]) {
                assigned_bin = j;
                break;
            }
        }
        if (assigned_bin == -1) {
            printf("Failure\n");
            free(cpu_assignments);
            free(bin_capacities);
            return;
        } else {
            cpu_assignments[i] = assigned_bin;
            bin_capacities[assigned_bin] -= tasks[i].C;
        }
    }

    // Print the CPU assignments
    printf("Success\n");
    for (int j = 0; j < B; j++) {
        printf("CPU%d", j);
        int allocated = 0;
        for (int i = 0; i < N; i++) {
            if (cpu_assignments[i] == j) {
                printf(",%s", tasks[i].name);
                allocated = 1;
            }
        }
        if (!allocated) {
            printf(" ");
        }
        printf("\n");
    }

    // Free memory
    free(cpu_assignments);
    free(bin_capacities);
}


/* 
 * Main function that reads input from a file and calls the appropriate task allocation function 
 * based on the policy specified in the file.
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: ./task_alloc input_file\n");
        return 1;
    }
    
    // Read input file and allocate memory for tasks
    char *filename = argv[1];	// Get filename from command line argument
    
    // Open the file for reading
    FILE *input_file = fopen(filename, "r");	
    if (input_file == NULL) {
        printf("Could not open input file %s\n", filename);
        return 1;
    }

    // Read the bin capacity and allocation policy from the first line
    int B, n;
    char policy[4];
    fscanf(input_file, "%d,%s\n", &B, policy);

    // Count the number of tasks
    int N = 0;
    char task_name[21];
    int C, T;
    while (fscanf(input_file, "%20[^,],%d,%d\n", task_name, &C, &T) == 3) {
        N++;
    }

    // Allocate memory for the tasks
    task_t *tasks = (task_t *)malloc(N * sizeof(task_t));
    if (tasks == NULL) {
        printf("Failed to allocate memory for tasks\n");
        return 1;
    }

    fseek(input_file, 0, SEEK_SET);	// Move the file pointer to the beginning of the input file
    fscanf(input_file, "%d,%s\n", &B, policy);	// get the number of CPUs and the allocation policy
    fscanf(input_file, "%d\n", &n);	// get the number of tasks
    // Read the remaining lines of the input file to get the details of each task
    for (int i = 0; i < n; i++) {
        // Read the task name, execution time, and deadline from the input file
        fscanf(input_file, "%20[^,],%d,%d\n", task_name, &C, &T);
        
        // Create a new task object and store its details
        task_t task = {0};
        strncpy(task.name, task_name, 20);
        task.C = C;
        task.T = T;
        tasks[i] = task;
    }

    fclose(input_file);	// Close the input file

    // This block of code chooses the allocation policy based on the input policy string, 
    // and calls the corresponding function.
    if (strcmp(policy, "BFD") == 0) {
        allocate_tasks_bfd(B, tasks, N);
    } else if (strcmp(policy, "WFD") == 0) {
        allocate_tasks_wfd(B, tasks, N);
    } else if (strcmp(policy, "FFD") == 0) {
        allocate_tasks_ffd(B, tasks, N);
    } else {
        printf("Invalid policy: %s\n", policy);
        free(tasks);
        return 1;
    }

    free(tasks);

    return 0;
}

