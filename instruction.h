#ifndef INSTRUCTION_H   // Include guard to prevent multiple inclusions
#define INSTRUCTION_H

#include <inttypes.h>
#include <string>

using namespace std;

class Register {
    public:
        int Value;
        bool HasRobValue;
        bool Exist = false;
        bool isReady = false;
};

struct CycleInfo {
    int beginCycle;
    int endCycle;
};

enum PipelineRegister {
    FE,
    DE,
    RN,
    RR,
    DI,
    IS,
    EX,
    WB,
    RT,
    ROB
};

class Instruction
{
    public:
        uint64_t ProgramCounter = 0;
        Register DestinationRegister;
        int OpType = -1;
        Register SourceRegister1;
        Register SourceRegister2;
        // bool isInstructionReady = false;
        bool InstructionValidInIQ = false;
        int robValue;
        unsigned long instructionSequenceNumber = -1;
        int latency = -1;

        // Register names:
        // FE, DE, RN, RR, DI, IS, EX, WB and RT
        std::map<PipelineRegister, CycleInfo> registerCycles;

        Instruction(uint64_t programCounter, 
                    int opType, 
                    Register destinationRegister, 
                    Register sourceRegister1, 
                    Register sourceRegister2, 
                    unsigned long sequenceNumber,
                    unsigned long initialCycle)
        {
            ProgramCounter = programCounter;
            DestinationRegister = destinationRegister;
            OpType = opType;
            SourceRegister1 = sourceRegister1;
            SourceRegister2 = sourceRegister2;
            instructionSequenceNumber = sequenceNumber;

            registerCycles[PipelineRegister::FE].beginCycle = initialCycle;
            registerCycles[PipelineRegister::FE].endCycle = -1;

            registerCycles[PipelineRegister::DE].beginCycle = -1;
            registerCycles[PipelineRegister::DE].endCycle = -1;

            registerCycles[PipelineRegister::RN].beginCycle = -1;
            registerCycles[PipelineRegister::RN].endCycle = -1;

            registerCycles[PipelineRegister::RR].beginCycle = -1;
            registerCycles[PipelineRegister::RR].endCycle = -1;

            registerCycles[PipelineRegister::DI].beginCycle = -1;
            registerCycles[PipelineRegister::DI].endCycle = -1;

            registerCycles[PipelineRegister::IS].beginCycle = -1;
            registerCycles[PipelineRegister::IS].endCycle = -1;

            registerCycles[PipelineRegister::EX].beginCycle = -1;
            registerCycles[PipelineRegister::EX].endCycle = -1;

            registerCycles[PipelineRegister::WB].beginCycle = -1;
            registerCycles[PipelineRegister::WB].endCycle = -1;

            registerCycles[PipelineRegister::RT].beginCycle = -1;
            registerCycles[PipelineRegister::RT].endCycle = -1;
        }

        Instruction() {};

        bool TryGetDestinationRegisterRobValue(int &robValue)
        {
            if (DestinationRegister.HasRobValue)
            {
                robValue = DestinationRegister.Value;
                return true;
            }
            return false;
        }

        void SetBeginCycleForRegister(PipelineRegister registerVal, int cycleValue)
        {
            registerCycles[registerVal].beginCycle = cycleValue;
        }
        
        void SetEndCycleForRegister(PipelineRegister registerVal, int cycleValue)
        {
            registerCycles[registerVal].endCycle = cycleValue;
        }

        int GetBeginCycleValueForRegister(PipelineRegister registerVal)
        {
            return registerCycles[registerVal].beginCycle;
        }
        
        int GetEndCycleValueForRegister(PipelineRegister registerVal)
        {
            return registerCycles[registerVal].endCycle;
        }
};

#endif