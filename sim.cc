#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <inttypes.h>
#include <algorithm>
#include "sim.h"
#include "out_of_order_scheduler.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim 256 32 4 gcc_trace.txt
    argc = 5
    argv[0] = "sim"
    argv[1] = "256"
    argv[2] = "32"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    proc_params params;       // look at sim_bp.h header file for the the definition of struct proc_params
    int op_type, dest, src1, src2;  // Variables are read from trace file
    uint64_t pc; // Variable holds the pc read from input file
    
    
    std::map<int, int> opTypeByLatency;
    opTypeByLatency[0] = 1;
    opTypeByLatency[1] = 2;
    opTypeByLatency[2] = 5;
    if (argc != 5)
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.rob_size     = strtoul(argv[1], NULL, 10);
    params.iq_size      = strtoul(argv[2], NULL, 10);
    params.width        = strtoul(argv[3], NULL, 10);
    trace_file          = argv[4];
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
    unsigned long currentCycleCount = 0;
    unsigned long fetchedInstructions = 0;
    Scheduler outOfOrderScheduler = Scheduler(FP, params.width, params.rob_size, params.iq_size, currentCycleCount);
    do
    {
        outOfOrderScheduler.RetireInstructions();
        outOfOrderScheduler.WritebackToRegister();
        outOfOrderScheduler.Execute();
        outOfOrderScheduler.IssueInstruction(opTypeByLatency);
        outOfOrderScheduler.DispatchInstruction();
        outOfOrderScheduler.ReadRegister();
        outOfOrderScheduler.Rename();
        outOfOrderScheduler.DecodeInstruction();
        outOfOrderScheduler.FetchInstruction(fetchedInstructions);
        currentCycleCount++;
    } while (outOfOrderScheduler.AdvanceToNextCycle());

    sort(outOfOrderScheduler.FinalInstructions.begin(), outOfOrderScheduler.FinalInstructions.end(), [](const Instruction& a, const Instruction& b) {
        return a.instructionSequenceNumber < b.instructionSequenceNumber;
    });
    
    for (auto& instruction : outOfOrderScheduler.FinalInstructions) 
    {
        printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n", 
            instruction.instructionSequenceNumber, 
            instruction.OpType, 
            instruction.SourceRegister1.Value,
            instruction.SourceRegister2.Value,
            instruction.DestinationRegister.Value,
            instruction.GetBeginCycleValueForRegister(PipelineRegister::FE),
            instruction.GetEndCycleValueForRegister(PipelineRegister::FE),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::DE),
            instruction.GetEndCycleValueForRegister(PipelineRegister::DE),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::RN),
            instruction.GetEndCycleValueForRegister(PipelineRegister::RN),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::RR),
            instruction.GetEndCycleValueForRegister(PipelineRegister::RR),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::DI),
            instruction.GetEndCycleValueForRegister(PipelineRegister::DI),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::IS),
            instruction.GetEndCycleValueForRegister(PipelineRegister::IS),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::EX),
            instruction.GetEndCycleValueForRegister(PipelineRegister::EX),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::WB),
            instruction.GetEndCycleValueForRegister(PipelineRegister::WB),
            instruction.GetBeginCycleValueForRegister(PipelineRegister::RT),
            instruction.GetEndCycleValueForRegister(PipelineRegister::RT)
            );
    }

    printf("# === Simulator Command =========\n");
    printf("# %s "
            "%lu "
            "%lu "
            "%lu "
            "%s\n", argv[0], params.rob_size, params.iq_size, params.width, trace_file);
    printf("# === Processor Configuration ===\n");
    printf("# ROB_SIZE = %lu\n", params.rob_size);
    printf("# IQ_SIZE = %lu\n", params.iq_size);
    printf("# WIDTH = %lu\n", params.width);
    printf("# === Simulation Results ========\n");
    printf("# Dynamic Instruction Count    = %lu\n", fetchedInstructions);
    printf("# Cycles                       = %lu\n", currentCycleCount);
    printf("# Instructions Per Cycle (IPC) = %1.2f\n", (float)fetchedInstructions/currentCycleCount);
    return 0;
}
