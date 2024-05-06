#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>

#define MAX_PATH_LEN 256
// i have created a function that will check the process is in given tree or not 
//in this i have used if conditional statement to check the process exist or not 
int processBelongsToTree(pid_t processId, pid_t rootProcess) {
    char filename[1024];  // Buffer for file path
    char line[1024];    // Buffer for reading lines from files
    FILE *file;
    snprintf(filename, sizeof(filename), "/proc/%d/status", processId);
    file = fopen(filename, "r");
    //this condition will check if the process in not found in that tree
    if (file == NULL) {
        return 0; // Process does not exist than it will return o
    }

    // this condition will read process id of parent 
    while (fgets(line, sizeof(line), file)) {  //here i have used while condition to get the process id of parent 
        if (strncmp(line, "PPid:", 5) == 0) {
            pid_t ppid;
            sscanf(line + 5, "%d", &ppid);  //getting ppid 
            fclose(file);
            return (ppid == rootProcess || ppid == processId || ppid == 1);  //here it is checking 3 differnet conditions 
        }
    }

    fclose(file);  
    //if ppid is not found 
    return 0;
}

// Function to list immediate descendants of a process
void listImmediateDescendants(pid_t processId) {   //while is used to immplement this 
    DIR *dir;
    struct dirent *entry;
    char path[1024];

    snprintf(path, sizeof(path), "/proc");

    if ((dir = opendir(path)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                // here i am Checking if the name of the directory is number or not 
                char *endptr;
                long pid = strtol(entry->d_name, &endptr, 10);
                if (*endptr == '\0') {
                    // to get the pid it will read the status of the file
                    char status_path[1024];
                    snprintf(status_path, sizeof(status_path), "/proc/%ld/status", pid);
                    FILE *status_file = fopen(status_path, "r");
                    if (status_file != NULL) {
                        char line[1024];
                        while (fgets(line, sizeof(line), status_file)) {
                            if (strncmp(line, "PPid:", 5) == 0) {
                                long ppid;
                                sscanf(line + 5, "%ld", &ppid);
                                if (ppid == processId) {
                                    printf("%ld\n", pid);
                                }
                                break;  //breaks the loop
                            }
                        }
                        fclose(status_file); //it will cloase the status file 
                    }
                }
            }
        }
        closedir(dir);  //closing the dir 
    }
}

// this function is used Display all non-direct descen of a process
void listNonDirectDescendants(pid_t processId) {
    DIR *dir;
    struct dirent *entry;
    char path[1024];

    snprintf(path, sizeof(path), "/proc");

    if ((dir = opendir(path)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                // it checks the dir name id no. that is pr. id 
                char *endptr;
                long pid = strtol(entry->d_name, &endptr, 10);
                if (*endptr == '\0') {
                    // to get pid it will read the status of file 
                    char status_path[1024];
                    snprintf(status_path, sizeof(status_path), "/proc/%ld/status", pid);
                    FILE *status_file = fopen(status_path, "r");
                    if (status_file != NULL) {
                        char line[1024];
                        while (fgets(line, sizeof(line), status_file)) {
                            if (strncmp(line, "PPid:", 5) == 0) {
                                long ppid;
                                sscanf(line + 5, "%ld", &ppid);
                                if (ppid != processId && processBelongsToTree(ppid, processId)) {
                                    printf("%ld\n", pid);
                                }
                                break;
                            }
                        }
                        fclose(status_file);  //closing the status file 
                    }
                }
            }
        }
        closedir(dir); //clossing the dir 
    }
}

// Function to list all sibling processes of a process


void listSiblingProcesses(pid_t process_id) {  //display all the siblings of process id provided 
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {    //condition for printing error 
        perror("Error opening /proc directory");
        return;
    }

    pid_t parent_pid = -1;  //setting id to -1 
    char status_path[MAX_PATH_LEN];  //using max_path_len which i have declared in starting 
    snprintf(status_path, sizeof(status_path), "/proc/%d/status", process_id);
    FILE *status_file = fopen(status_path, "r");   //opening file in read mode only
    if (status_file == NULL) {     //checking weather the status file is null 
        perror("Error opening status file");  //than it will print the error 
        closedir(proc_dir);  //closing proc_dir
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), status_file)) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line + 6, "%d", &parent_pid);  //getting parent pid 
            break;
        }
    }
    fclose(status_file);  //clossing the file that was opened 

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            pid_t pid = atoi(entry->d_name);
            if (pid > 0 && pid != process_id) {
                char process_status_path[MAX_PATH_LEN];
                snprintf(process_status_path, sizeof(process_status_path), "/proc/%d/status", pid);
                FILE *process_status_file = fopen(process_status_path, "r");
                if (process_status_file != NULL) {
                    pid_t process_parent_pid = -1;
                    while (fgets(line, sizeof(line), process_status_file)) {
                        if (strncmp(line, "PPid:", 5) == 0) {
                            sscanf(line + 6, "%d", &process_parent_pid);
                            break;
                        }
                    }
                    fclose(process_status_file);
                    if (process_parent_pid == parent_pid) {
                        printf("%d\n", pid);
                    }
                }
            }
        }
    }

    closedir(proc_dir);
}
// here i have write the Function to stop a particular process with process id 
void pauseProcess(pid_t processId) {
    if (kill(processId, SIGSTOP) == 0) {  //using SIGSTOP to pause the process 
        printf("Process %d paused\n", processId);
    } else {
        perror("Error pausing process");    //if it does not pause the process than it will show an error 
    }
}

// here i am implementing the Function which will continue all previously stopped  processes
void resumePausedProcesses() {   //defining a function 
    // i need to implement this function
    printf("Resuming paused processes\n");
}

// Function to list defunct (zombie) processes
void listDefunctProcesses(pid_t processId) {  //defining the function that will print the process that are kiled 
    DIR *dir;
    struct dirent *entry;
    char path[1024];

    snprintf(path, sizeof(path), "/proc");

    if ((dir = opendir(path)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                // Check if the directory name is a number (process ID)
                char *endptr;
                long pid = strtol(entry->d_name, &endptr, 10);
                if (*endptr == '\0') {
                    // Read the status file to get the process state
                    char status_path[1024];
                    snprintf(status_path, sizeof(status_path), "/proc/%ld/status", pid);
                    FILE *status_file = fopen(status_path, "r");
                    if (status_file != NULL) {
                        char line[1024];
                        while (fgets(line, sizeof(line), status_file)) {
                            if (strncmp(line, "State:", 6) == 0) {
                                if (strstr(line, "defunct") != NULL && processBelongsToTree(pid, processId)) {
                                    printf("%ld\n", pid);
                                }
                                break;
                            }
                        }
                        fclose(status_file);
                    }
                }
            }
        }
        closedir(dir);
    }
}

// Function to list all grandchildren of a process
void listGrandchildren(pid_t processId) { // i have define a function to get info display of grandchildrens of particulare process id 
     DIR *dir;
    struct dirent *entry;

    dir = opendir("/proc");
    if (dir == NULL) {    //condition for checking an error 
        perror("Error opening /proc directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type != DT_DIR || !isdigit(*entry->d_name))
            continue;

        pid_t pid = atoi(entry->d_name);

        if (pid == getpid() || getppid() == pid)
            continue;

        char status_path[MAX_PATH_LEN];
        snprintf(status_path, sizeof(status_path), "/proc/%s/status", entry->d_name);
        FILE *status_file = fopen(status_path, "r");
        if (status_file == NULL)
            continue;

        char line[256];
        pid_t ppid = -1;
        while (fgets(line, sizeof(line), status_file)) {
            if (strncmp(line, "PPid:", 5) == 0) {
                sscanf(line + 6, "%d", &ppid);
                break;
            }
        }
        fclose(status_file);

        if (ppid == processId) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dir);
}

// i have created this function to get the status of the process that it is defunct or not 
void printProcessStatus(pid_t processId) {  //defining the function that will show that the id is zombie or not 
    DIR *dir;
    struct dirent *entry;
    char path[1024];

    snprintf(path, sizeof(path), "/proc");

    if ((dir = opendir(path)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                // Check if the directory name is a number (process ID)
                char *endptr;
                long pid = strtol(entry->d_name, &endptr, 10);
                if (*endptr == '\0' && pid == processId) {
                    // Read the status file to get the process state
                    char status_path[1024];
                    snprintf(status_path, sizeof(status_path), "/proc/%ld/status", pid);
                    FILE *status_file = fopen(status_path, "r");
                    if (status_file != NULL) {
                        char line[1024];
                        while (fgets(line, sizeof(line), status_file)) {
                            if (strncmp(line, "State:", 6) == 0) {
                                if (strstr(line, "defunct") != NULL) {
                                    printf("Defunct\n");
                                } else {
                                    printf("Not Defunct\n");
                                }
                                fclose(status_file);
                                return;
                            }
                        }
                        fclose(status_file);
                    }
                }
            }
        }
        closedir(dir);
    }
    printf("Process not found\n");
}

// i have created one Function which will handle all kind of options that i will check 
void handleOption(pid_t processId, pid_t rootProcess, const char *option) {  //defining function that will alow different otpions 
    if (strcmp(option, "-xd") == 0) { // display immediate descendants of processId
        listImmediateDescendants(processId);
    } else if (strcmp(option, "-xn") == 0) {
        // show all non-direct descendants of processId
        listNonDirectDescendants(processId);
    } else if (strcmp(option, "-xs") == 0) {
        // Lists all sibling processes of processId
        listSiblingProcesses(processId);
    } else if (strcmp(option, "-xt") == 0) {
        // Pauses  process
        pauseProcess(processId);
    } else if (strcmp(option, "-xc") == 0) {
        // continue all previously paused processes
        resumePausedProcesses();
    } else if (strcmp(option, "-xz") == 0) {
        // Display all defunct (zombie) processes
        listDefunctProcesses(processId);
    } else if (strcmp(option, "-xg") == 0) {
        // shows all grandchildren of processId
        listGrandchildren(processId);
    } else if (strcmp(option, "-zs") == 0) {
        // it wil Print the process status of (Defunct/Not Defunct)
        printProcessStatus(processId);
    } else if (strcmp(option, "-rp") == 0) {
        // it will Kill the  processId if it belongs to the process tree rooted at rootProcess
        if (processBelongsToTree(processId, rootProcess)) {
            if (kill(processId, SIGKILL) == 0) {
                printf("Process %d killed\n", processId);
            } else {
                perror("Error");
            }
        } else {
            printf("Process does not belong to the process tree\n");
        }
    } else if (strcmp(option, "-pr") == 0) {
        // Kill the root process
        if (kill(rootProcess, SIGKILL) == 0) {
            printf("Root process %d killed\n", rootProcess);
        } else {
            perror("Error killing root process");
        }
    } else {
        printf("Invalid option\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 4) {   //it will check the number of arguments 
        fprintf(stderr, "Usage: %s [process_id] [root_process] [OPTION]\n", argv[0]);
        exit(EXIT_FAILURE);  //or it will exit 
    }

    // Extract process_id, root_process, and option from command-line arguments
    pid_t processId = atoi(argv[1]);  //extracting th id's 
    pid_t rootProcess = atoi(argv[2]);
    const char *option = argv[3]; //created a pointer 

    // Check if processId belongs to the process tree rooted at rootProcess
    if (!processBelongsToTree(processId, rootProcess)) {
        printf("Does not belong to the process tree\n");
        exit(EXIT_SUCCESS);
    }

    // it will handle the all kind of options that i will check in implementation 
    handleOption(processId, rootProcess, option);

    return 0;
}
