#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_REF 100
#define MAX_FRAMES 10

void print_simulation(const int ref[], int n, int frames_history[MAX_REF][MAX_FRAMES], const bool faults[], int f, const char* algo_name) {
    printf("\n============================================================\n");
    printf("  %s Simulation (Frames = %d)\n", algo_name, f);
    printf("============================================================\n");
    
    printf("Ref string : ");
    for (int i = 0; i < n; i++) {
        printf("%2d ", ref[i]);
    }
    printf("\n");

    for (int r = 0; r < f; r++) {
        printf("Frame %d    : ", r + 1);
        for (int c = 0; c < n; c++) {
            if (frames_history[c][r] == -1) {
                printf(" - ");
            } else {
                printf("%2d ", frames_history[c][r]);
            }
        }
        printf("\n");
    }

    printf("Fault/Hit  : ");
    int total_faults = 0;
    for (int i = 0; i < n; i++) {
        if (faults[i]) {
            printf(" F ");
            total_faults++;
        } else {
            printf(" H ");
        }
    }
    printf("\n");
    printf("------------------------------------------------------------\n");
    printf("Total Page Faults: %d | Total Hits: %d\n", total_faults, n - total_faults);
    printf("Page Fault Rate  : %.2f%% | Page Hit Rate: %.2f%%\n", 
           ((double)total_faults / n) * 100.0, 
           ((double)(n - total_faults) / n) * 100.0);
    printf("============================================================\n");
}

void simulate_fifo(const int ref[], int n, int f) {
    int frames[MAX_FRAMES];
    int frames_history[MAX_REF][MAX_FRAMES];
    bool faults[MAX_REF];
    
    for (int i = 0; i < f; i++) frames[i] = -1;
    
    int oldest_idx = 0;
    
    for (int i = 0; i < n; i++) {
        int page = ref[i];
        bool found = false;
        
        for (int j = 0; j < f; j++) {
            if (frames[j] == page) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            faults[i] = true;
            frames[oldest_idx] = page;
            oldest_idx = (oldest_idx + 1) % f;
        } else {
            faults[i] = false;
        }
        
        for (int j = 0; j < f; j++) {
            frames_history[i][j] = frames[j];
        }
    }
    
    print_simulation(ref, n, frames_history, faults, f, "FIFO (First-In, First-Out)");
}

void simulate_lru(const int ref[], int n, int f) {
    int frames[MAX_FRAMES];
    int frames_history[MAX_REF][MAX_FRAMES];
    bool faults[MAX_REF];
    int last_used[MAX_FRAMES]; // Tracks the timestamp (step) of last access
    
    for (int i = 0; i < f; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }
    
    for (int i = 0; i < n; i++) {
        int page = ref[i];
        bool found = false;
        int hit_idx = -1;
        
        for (int j = 0; j < f; j++) {
            if (frames[j] == page) {
                found = true;
                hit_idx = j;
                break;
            }
        }
        
        if (found) {
            faults[i] = false;
            last_used[hit_idx] = i;
        } else {
            faults[i] = true;
            // Find empty frame if any
            int replace_idx = -1;
            for (int j = 0; j < f; j++) {
                if (frames[j] == -1) {
                    replace_idx = j;
                    break;
                }
            }
            
            // If all frames full, find least recently used
            if (replace_idx == -1) {
                int min_time = i;
                replace_idx = 0;
                for (int j = 0; j < f; j++) {
                    if (last_used[j] < min_time) {
                        min_time = last_used[j];
                        replace_idx = j;
                    }
                }
            }
            
            frames[replace_idx] = page;
            last_used[replace_idx] = i;
        }
        
        for (int j = 0; j < f; j++) {
            frames_history[i][j] = frames[j];
        }
    }
    
    print_simulation(ref, n, frames_history, faults, f, "LRU (Least Recently Used)");
}

void simulate_optimal(const int ref[], int n, int f) {
    int frames[MAX_FRAMES];
    int frames_history[MAX_REF][MAX_FRAMES];
    bool faults[MAX_REF];
    
    for (int i = 0; i < f; i++) frames[i] = -1;
    
    for (int i = 0; i < n; i++) {
        int page = ref[i];
        bool found = false;
        
        for (int j = 0; j < f; j++) {
            if (frames[j] == page) {
                found = true;
                break;
            }
        }
        
        if (found) {
            faults[i] = false;
        } else {
            faults[i] = true;
            // Find empty frame if any
            int replace_idx = -1;
            for (int j = 0; j < f; j++) {
                if (frames[j] == -1) {
                    replace_idx = j;
                    break;
                }
            }
            
            // If all frames full, look into future requests
            if (replace_idx == -1) {
                int farthest = -1;
                replace_idx = 0;
                for (int j = 0; j < f; j++) {
                    int next_request = -1;
                    // Look forward in the reference string
                    for (int k = i + 1; k < n; k++) {
                        if (ref[k] == frames[j]) {
                            next_request = k;
                            break;
                        }
                    }
                    
                    // If a page is never requested in future, it's the best candidate
                    if (next_request == -1) {
                        replace_idx = j;
                        break;
                    }
                    
                    if (next_request > farthest) {
                        farthest = next_request;
                        replace_idx = j;
                    }
                }
            }
            
            frames[replace_idx] = page;
        }
        
        for (int j = 0; j < f; j++) {
            frames_history[i][j] = frames[j];
        }
    }
    
    print_simulation(ref, n, frames_history, faults, f, "Optimal (OPT)");
}

int main() {
    // Default reference string (example from standard textbooks)
    int ref[MAX_REF] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
    int n = 20;
    int f = 3;

    char input[256];
    printf("=== Page Replacement Algorithms Simulator ===\n");
    printf("Enter number of frames (default is 3, max %d): ", MAX_FRAMES);
    if (fgets(input, sizeof(input), stdin) && strcmp(input, "\n") != 0) {
        f = atoi(input);
        if (f <= 0 || f > MAX_FRAMES) f = 3;
    }

    printf("Enter reference string separated by spaces (e.g. 7 0 1 2 0 3)\n");
    printf("Press [Enter] to use default: ");
    for (int i = 0; i < n; i++) printf("%d ", ref[i]);
    printf("\n> ");
    
    if (fgets(input, sizeof(input), stdin) && strcmp(input, "\n") != 0) {
        char *token = strtok(input, " \t\n");
        int idx = 0;
        while (token != NULL && idx < MAX_REF) {
            ref[idx++] = atoi(token);
            token = strtok(NULL, " \t\n");
        }
        n = idx;
    }

    simulate_fifo(ref, n, f);
    simulate_lru(ref, n, f);
    simulate_optimal(ref, n, f);

    return 0;
}

