#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int LATEST_ARRIVAL_TIME=0;

// Defining the structs and enum
typedef struct {
    char name[20];
    int duration;
    int executed_time;
} Instruction;

typedef enum { SILVER, GOLD, PLATINUM }
ProcessType ;

// Defining the process fields
typedef struct {
    char name[20];
    int priority;
    int arrival_time;
    int num_instructions;
    ProcessType type;
    Instruction current_instruction;
    int current_instruction_index;
    Instruction instructions[20];
    int turnaround_time;
    int waiting_time;
    int total_exec_time;
    int remaining_quantums;
    int is_ended;
    int gone;
    int time_quantum_expired;
    int time_quantum;
    int turn ;
} Process;

Process emptyProcess;

// Defining the scheduler fields
typedef struct {
    Instruction instructions[2000];  
    Process processes[10];        
    int num_instructions;
    int num_processes;
    int initial_number_of_processes;
    Process ready_queue[10];   
    int num_ready_processes;
    Process current_process;
    int time;
} Scheduler;



// Reading the instructions form the Instructions.txt
void read_instructions(Instruction instructions[], int *num_instructions, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    *num_instructions = 0;
    while (fscanf(file, "%s %d", 
                instructions[*num_instructions].name,
                &instructions[*num_instructions].duration) == 2) {
        instructions[*num_instructions].executed_time=0;
        (*num_instructions)++;
    }


    fclose(file);
}

// Reading the Processes from the definition file
void read_processes(Process processes[] , int *num_processes , const char *filename){
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    *num_processes = 0;
    char process_type[20];
    while (fscanf(file, "%s %d %d %s",
                  processes[*num_processes].name,
                  &processes[*num_processes].priority,
                  &processes[*num_processes].arrival_time,
                  process_type) == 4) {
        processes[*num_processes].is_ended=0;
        processes[*num_processes].time_quantum_expired=0;
        processes[*num_processes].current_instruction_index=0;
        processes[*num_processes].turn=0;
        processes[*num_processes].gone=0;
        processes[*num_processes].total_exec_time = 0;
        processes[*num_processes].turnaround_time = 0;
        processes[*num_processes].waiting_time = 0;
        if(strcmp(process_type,"SILVER")==0){
            processes[*num_processes].type = SILVER;
            processes[*num_processes].time_quantum=80;
        }
        else{
            processes[*num_processes].time_quantum=120;
            if(strcmp(process_type,"GOLD")==0){
                processes[*num_processes].type = GOLD;
            }
            else{
                processes[*num_processes].type = PLATINUM;
            }
        }


       if (processes[*num_processes].arrival_time > LATEST_ARRIVAL_TIME) {
            LATEST_ARRIVAL_TIME = processes[*num_processes].arrival_time;
        }
        processes[*num_processes].remaining_quantums=processes[*num_processes].time_quantum;




        (*num_processes)++;
    }

    fclose(file);

}

// Reading the instructions of the projects 
void read_process(Instruction instructions [],  int *num_instructions , const char *filename){
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    *num_instructions = 0;
    while (fscanf(file, "%s", 
                instructions[*num_instructions].name) == 1) {
        (*num_instructions)++;
    }
}

const char* processTypeToString(ProcessType type) {
    switch(type) {
        case SILVER:
            return "SILVER";
        case GOLD:
            return "GOLD";
        case PLATINUM:
            return "PLATINUM";
    }
}


// finds the most appropiate index for preEmption
int check_for_preemption(Scheduler *myScheduler) {
    Process *current_process = &myScheduler->current_process;
    int higher_priority_index = -1;
    int platinum_found=0;
    // If current process is ended 
    if(current_process->is_ended && myScheduler->num_processes>0){
        // Initalize a candidate process
        Process *candidate_process = current_process;
        if(myScheduler->num_ready_processes>0){
            // Iterate through ready queue find a more appropiate process
            for (int i = 0; i < myScheduler->num_ready_processes; i++) {
                Process *new_candidate_process = &myScheduler->ready_queue[i];
                // If the candidate and the next index process is PLAT
                if(candidate_process->type ==PLATINUM && new_candidate_process->type==PLATINUM && !new_candidate_process->is_ended){
                    if(strcmp(candidate_process->name,current_process->name)==0){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                    if(new_candidate_process->arrival_time < candidate_process->arrival_time){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                    if((new_candidate_process->arrival_time == candidate_process->arrival_time) && 
                        strcmp(new_candidate_process->name,candidate_process->name) <0){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                }
                // If only the candidate process s PLAT
                else if(new_candidate_process->type != PLATINUM && candidate_process->type ==PLATINUM && !new_candidate_process->is_ended){
                    if(strcmp(candidate_process->name,current_process->name)==0){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                    continue;
                }
                
                // If only the next index process is PLAT
                else if(new_candidate_process->type == PLATINUM && candidate_process->type !=PLATINUM && !new_candidate_process->is_ended){
                    candidate_process=new_candidate_process;
                    higher_priority_index=i;
                    continue;
                }
                // If the current not the above situations check if the next index higher priority
                else if(new_candidate_process->priority > candidate_process->priority && !new_candidate_process->is_ended){
                    candidate_process=new_candidate_process;
                    higher_priority_index=i;
                    continue;
                }
                // If they are same priority choose the higher priorities according to rules defined in description
                else if(new_candidate_process->priority == candidate_process->priority && !new_candidate_process->is_ended){
                    int possible_higher_priority_process_arrived=0;
                    for(int k=i+1 ; k<myScheduler->num_ready_processes ; k++){
                        Process *possible_higher_priority_process = &myScheduler->ready_queue[k];
                        if(possible_higher_priority_process->priority > candidate_process->priority){
                            possible_higher_priority_process_arrived=1;
                        }
                    }
                    // If there is a more priority index at the end of the ready queue
                    if(!possible_higher_priority_process_arrived && strcmp(current_process->name,candidate_process->name)==0){ 
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        break;
                    }
                }
                // Else if choose the lower priority one if the candidate is current Process
                else if(new_candidate_process->priority < candidate_process->priority && !new_candidate_process->is_ended && strcmp(current_process->name,candidate_process->name)==0){
                    candidate_process=new_candidate_process;
                    higher_priority_index=i;
                    continue;
                }

            }
        }

    }

    // Same logic as above the only difference we do not tak the lower priority
    // This case if the process doesn't end but if time quantum expired
    // Only the different lines are commented
    else if ((current_process->type !=PLATINUM && !current_process->is_ended && current_process->time_quantum_expired))
    {
        Process *candidate_process = current_process;
        if(myScheduler->num_ready_processes>0){
            for (int i = 0; i < myScheduler->num_ready_processes; i++) {
                Process *new_candidate_process = &myScheduler->ready_queue[i];
                if(candidate_process->type ==PLATINUM && new_candidate_process->type==PLATINUM && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    if(new_candidate_process->arrival_time < candidate_process->arrival_time){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                    if((new_candidate_process->arrival_time == candidate_process->arrival_time) && 
                        strcmp(new_candidate_process->name,candidate_process->name) <0){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                }
                else if(new_candidate_process->type != PLATINUM && candidate_process->type ==PLATINUM && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    continue;
                }
                else if(new_candidate_process->type == PLATINUM && candidate_process->type !=PLATINUM && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    candidate_process=new_candidate_process;
                    higher_priority_index=i;
                    continue;
                }
                else if(new_candidate_process->priority > candidate_process->priority && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    candidate_process=new_candidate_process;
                    higher_priority_index=i;
                    continue;
                }
                else if(new_candidate_process->priority == candidate_process->priority && !new_candidate_process->is_ended && strcmp(current_process->name, new_candidate_process->name)!=0){
                    int possible_higher_priority_process_arrived=0;
                    for(int k=i+1 ; k<myScheduler->num_ready_processes ; k++){
                        Process *possible_higher_priority_process = &myScheduler->ready_queue[k];
                        if(possible_higher_priority_process->priority > candidate_process->priority){
                            possible_higher_priority_process_arrived=1;
                        }
                    }
                    // If the current process and same priority index arrived at the same time
                    if(!possible_higher_priority_process_arrived && 
                        strcmp(candidate_process->name,current_process->name)==0 &&
                        new_candidate_process->arrival_time == myScheduler->time ){
                        continue;
                    }


                    if(!possible_higher_priority_process_arrived ){ 
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        break;
                    }
                }
            }
        }
    }
    // Same logic as above 
    // This case if the process doesn't end also doesn't time quantum expired only checks if there is a better process
     else if ((current_process->type !=PLATINUM && !current_process->is_ended && !current_process->time_quantum_expired))
    {
        Process *candidate_process = current_process;
        if(myScheduler->num_ready_processes>0){
            for (int i = 0; i < myScheduler->num_ready_processes; i++) {
                Process *new_candidate_process = &myScheduler->ready_queue[i];
                if(candidate_process->type ==PLATINUM && new_candidate_process->type==PLATINUM && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    if(new_candidate_process->arrival_time < candidate_process->arrival_time){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                    if((new_candidate_process->arrival_time == candidate_process->arrival_time) && 
                        strcmp(new_candidate_process->name,candidate_process->name) <0){
                        candidate_process=new_candidate_process;
                        higher_priority_index=i;
                        continue;
                    }
                }
                else if(new_candidate_process->type != PLATINUM && candidate_process->type ==PLATINUM && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    continue;
                }
                else if(new_candidate_process->type == PLATINUM && candidate_process->type !=PLATINUM && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    candidate_process=new_candidate_process;
                    higher_priority_index=i;
                    continue;
                }
                else if(new_candidate_process->priority > candidate_process->priority && !new_candidate_process->is_ended  && strcmp(current_process->name, new_candidate_process->name)!=0){
                    candidate_process=new_candidate_process;
                    higher_priority_index=i;
                    continue;
                }
                //No need for check the same priority case since the process doesn't end or doesn't reach the time quantum
            }
        }
    }



    return higher_priority_index;

}

void check_for_arrival(Scheduler *myScheduler) {

    // Iterate throug the processes
    for (int i = 0; i < myScheduler->initial_number_of_processes; i++) {
        if (myScheduler->time >= myScheduler->processes[i].arrival_time) {
            int canbeAddedtoReadyQueue = 0;
            for (int j = 0; j < myScheduler->num_ready_processes; j++) {
                // If the current index is not in ready queue the also not the current process which is executed
                if (strcmp(myScheduler->ready_queue[j].name, myScheduler->processes[i].name) == 0 
                || strcmp(myScheduler->current_process.name, myScheduler->ready_queue[j].name) == 0 ) {
                    canbeAddedtoReadyQueue = 1;
                    break;
                }
            }
            if(strcmp(myScheduler->current_process.name, myScheduler->processes[i].name) == 0){
                canbeAddedtoReadyQueue=1;
            }
            // Adds the process to the ready queue
            if (canbeAddedtoReadyQueue==0 && myScheduler->processes[i].is_ended ==0 ) {
                // Move the platinum process to the front if it is PLATINUM
                if(strcmp("PLATINUM",processTypeToString(myScheduler->processes[i].type))==0){
                    for (int k = myScheduler->num_ready_processes; k > 0; k--) {
                        myScheduler->ready_queue[k] = myScheduler->ready_queue[k - 1];
                    }
                    myScheduler->ready_queue[0] = myScheduler->processes[i];
                    myScheduler->num_ready_processes++;
                }
                // Else add simply
                else{
                    myScheduler->ready_queue[myScheduler->num_ready_processes++] =  myScheduler->processes[i];
                }
            
            }
        }
    }
}

void context_switch(Scheduler *myScheduler) {
    Process *current_process = &myScheduler->current_process;
    // Get the higher priority index preempted 
    int higher_priority_index= check_for_preemption(myScheduler);
    if (higher_priority_index != -1) {
        // Add the time
        Process *higher_priority_process = &myScheduler->ready_queue[higher_priority_index];
        myScheduler->time += 10;
        myScheduler->current_process = *higher_priority_process;
        current_process->instructions[current_process->current_instruction_index].executed_time=0;
        // Remove the choosen process from the ready queue
        for (int i = higher_priority_index; i < myScheduler->num_ready_processes - 1; i++) {
            myScheduler->ready_queue[i] = myScheduler->ready_queue[i + 1];
        }
        // Substract the ready processes by one
        myScheduler->num_ready_processes--;
    }
    // If CPU is idle wait until an process arrives
    if(higher_priority_index==-1 && current_process->is_ended==1 && myScheduler->num_processes >0){
        while(myScheduler->num_ready_processes ==0){
            myScheduler->time++;
            check_for_arrival(myScheduler);
        }
        context_switch(myScheduler);
    }
}


void execute(Scheduler *myScheduler) {
    // Gets the current process and current instruction
    Process *current_process = &myScheduler->current_process;
    int time_quantum = current_process->time_quantum;
    current_process->current_instruction = current_process->instructions[current_process->current_instruction_index];
    int process_completed = 0;
    int time_quantum_expired = 0;
    int instruction_completed=1;
    int higher_priority_index=-1;
    int higher_priority_index_arrived = 0;
    int i=1;
    // If the process is not PLATINUM
    if(current_process->type != PLATINUM && myScheduler->num_processes>1){
        current_process->time_quantum_expired=0;
        Instruction *current_instruction = &current_process->instructions[current_process->current_instruction_index];
        // Check if the instruction is not completed and time quantum reached
        while (current_process->time_quantum >i || !instruction_completed) {
            current_instruction->executed_time++;
            instruction_completed=0;
            myScheduler->time++;
            // check if exit 
            if (strcmp(current_process->current_instruction.name, "exit") == 0 && current_instruction->duration == current_instruction->executed_time) {
                    myScheduler->current_process.total_exec_time += current_instruction->executed_time;
                    current_process->is_ended=1;
                    process_completed = 1;
                    break;
            }
            // If current instruction execution ends
            if (current_instruction->duration == current_instruction->executed_time) {
                instruction_completed=1;
                // Move the next instruction but doesn't start yet to execute
                current_process->current_instruction_index++;
                myScheduler->current_process.total_exec_time += current_instruction->executed_time;
                // Initialize the fiels of the next instructions
                current_process->current_instruction = current_process->instructions[current_process->current_instruction_index];
                current_process->current_instruction.executed_time=0;
                current_instruction = &current_process->current_instruction;
                current_instruction->executed_time=0;
                // Check if a process arrive at each of between two instructions
                check_for_arrival(myScheduler);
                // Check if the time quantum reached
                if (i>=current_process->time_quantum) {
                    current_process->time_quantum_expired=1;
                    i= 0;
                    // Add one to the turn if needed change it
                    current_process->turn++;
                    if (current_process->type == SILVER && current_process->turn == 3) {
                        current_process->type =GOLD;  
                        current_process->time_quantum=120;
                        current_process->turn = 0;
                    }
                    if (current_process->type ==GOLD && current_process->turn == 5) {
                        current_process->type=PLATINUM;
                        current_process->time_quantum=120;
                        current_process->turn = 0;
                    }
                }
                // Check if preemption neeced
                if (check_for_preemption(myScheduler) !=-1) {
                    higher_priority_index_arrived = 1;
                    break;
                }
                current_process->time_quantum_expired=0;
            }
            i++;
        }
    }
    else{
        // If the instruction is platınum
        Instruction *current_instruction = &current_process->instructions[current_process->current_instruction_index];
        // Do until the instruction is exi
        while(strcmp(current_process->current_instruction.name, "exit") != 0){
            // Same logic as above
            check_for_arrival(myScheduler);
            current_instruction->executed_time++;
            if (current_instruction->duration == current_instruction->executed_time) {
                current_process->current_instruction_index++;
                myScheduler->current_process.total_exec_time += current_instruction->executed_time;
                current_process->current_instruction = current_process->instructions[current_process->current_instruction_index];
                current_instruction = &current_process->current_instruction;
                current_instruction->executed_time=0;

            }
            myScheduler->time++;
        }
        // execute the exit
        while(current_instruction->duration > current_instruction->executed_time){
            current_instruction->executed_time++;
            myScheduler->time++;
        }
        myScheduler->current_process.total_exec_time += current_instruction->executed_time;
        process_completed=1;
    }
    // If process is completed
    if (process_completed) {
        if(myScheduler->current_process.gone ==0){
            myScheduler->num_processes--;
        }
        // Change the neccessary fields
        myScheduler->current_process.gone==1;
        myScheduler->current_process.is_ended =1;
        // Set the turnaround time and waiting times
        myScheduler->current_process.turnaround_time = myScheduler->time- myScheduler->current_process.arrival_time;
        myScheduler->current_process.waiting_time = myScheduler->current_process.turnaround_time - myScheduler->current_process.total_exec_time;
        for(int j=0 ; j< myScheduler->initial_number_of_processes ; j++){
            if(strcmp(myScheduler->current_process.name, myScheduler->processes[j].name)==0){
                myScheduler->processes[j].turnaround_time=myScheduler->current_process.turnaround_time;
                myScheduler->processes[j].waiting_time=myScheduler->current_process.waiting_time;
                myScheduler->processes[j].is_ended=1;
            }
        }
        // Context Switch if there is process left
        if (myScheduler->num_processes > 0) {
            context_switch(myScheduler);
        } 
        else {
            return ;
        }
    } 
    // Check if process doesn't end and other cases
    else if (higher_priority_index_arrived && !current_process->time_quantum_expired) {
        myScheduler->ready_queue[myScheduler->num_ready_processes++] = myScheduler->current_process;
        context_switch(myScheduler);
    } else if (current_process->time_quantum_expired && higher_priority_index_arrived) {
        myScheduler->ready_queue[myScheduler->num_ready_processes++] = myScheduler->current_process;
        context_switch(myScheduler);
    }
}

// Compare the two process according to arrival time and names
int compareStrings(const void *a, const void *b) {
    const Process *processA = (const Process *)a;
    const Process *processB = (const Process *)b;

    if(processA->arrival_time == processB->arrival_time){
        return strcmp(processA->name, processB->name);
    }

    return processA->arrival_time - processB->arrival_time;
}





int main(){
    // Intialize Scheduler fields
    Scheduler myScheduler;

    Process emptyProcess ;
    emptyProcess.type=SILVER;
    emptyProcess.priority=-1;
    emptyProcess.is_ended=0;
    strcpy(emptyProcess.name, "emptyProcess");

    myScheduler.num_instructions = 0;
    myScheduler.num_processes = 0;
    myScheduler.num_ready_processes = 0;
    myScheduler.current_process = emptyProcess;
    myScheduler.time = 0;


    // Take the input intialize the fields of structs
    read_instructions(myScheduler.instructions, &myScheduler.num_instructions, "instructions.txt");
    read_processes(myScheduler.processes, &myScheduler.num_processes, "definition.txt");

    for (int i = 0; i < myScheduler.num_processes; i++) {
        myScheduler.processes[i].num_instructions = 0;
        char filename[25];
        snprintf(filename, sizeof(filename), "%s.txt", myScheduler.processes[i].name);
        read_process(myScheduler.processes[i].instructions, &myScheduler.processes[i].num_instructions, filename);
        for (int j=0 ; j< myScheduler.processes[i].num_instructions ;j++){
            for(int k=0 ; k< myScheduler.num_instructions ; k++){
                if (strcmp(myScheduler.processes[i].instructions[j].name, myScheduler.instructions[k].name)==0){
                   myScheduler.processes[i].instructions[j].duration= myScheduler.instructions[k].duration;
                   break; 
                }
            }
        }
    }
    myScheduler.initial_number_of_processes=myScheduler.num_processes;
    qsort(myScheduler.processes, myScheduler.initial_number_of_processes, sizeof(myScheduler.processes[0]), compareStrings);
    check_for_arrival(&myScheduler);


    // Execute while loop
    while (myScheduler.num_processes > 0) {
        if (strcmp(myScheduler.current_process.name, "emptyProcess") == 0) {
            context_switch(&myScheduler);
        }
        execute(&myScheduler);
    }

    // Take the average waiting times and turnaround times
    int total_turnaround_time = 0;
    int total_waiting_time = 0;


    for (int i = 0; i < myScheduler.initial_number_of_processes; i++) {
        total_turnaround_time += myScheduler.processes[i].turnaround_time;
        total_waiting_time += myScheduler.processes[i].waiting_time;
    }

    double avg_turnaround_time = (double)total_turnaround_time / myScheduler.initial_number_of_processes;
    double avg_waiting_time = (double)total_waiting_time / myScheduler.initial_number_of_processes;

    printf("%.1lf\n", avg_waiting_time);
    printf("%.1lf\n", avg_turnaround_time);


}