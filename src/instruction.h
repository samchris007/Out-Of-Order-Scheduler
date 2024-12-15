#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <inttypes.h>
#include <string>

using namespace std;

/// @class Register
/// @brief Register class for source and destination addresses
class Register {
    public:
        int Value;
        bool HasRobValue = false;
        bool Exist = false;
        bool IsReady = false;
};

/// @class Instruction
/// @brief Class that contains information related to a specific instruction
class Instruction
{
    public:
        int OpType = -1;
        int RobValue; // Rob value of an instruction. Specific to Reorder buffer
        int Latency = -1;
        bool InstructionValidInIQ = false;
        unsigned long InstructionSequenceNumber = -1;

        uint64_t ProgramCounter = 0;
        Register SourceRegister1;
        Register SourceRegister2;
        Register DestinationRegister;

        /// Contains cycle information for each pipeline stages
        // Pipeline stages: FE, DE, RN, RR, DI, IS, EX, WB and RT
        std::map<PipelineRegister, CycleInfo> registerCycles;

        /// @brief Constructs Instruction class which is used in each piepline stages
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
            InstructionSequenceNumber = sequenceNumber;

            registerCycles[PipelineRegister::FE].start = initialCycle;
            registerCycles[PipelineRegister::FE].finish = -1;

            registerCycles[PipelineRegister::DE].start = -1;
            registerCycles[PipelineRegister::DE].finish = -1;

            registerCycles[PipelineRegister::RN].start = -1;
            registerCycles[PipelineRegister::RN].finish = -1;

            registerCycles[PipelineRegister::RR].start = -1;
            registerCycles[PipelineRegister::RR].finish = -1;

            registerCycles[PipelineRegister::DI].start = -1;
            registerCycles[PipelineRegister::DI].finish = -1;

            registerCycles[PipelineRegister::IS].start = -1;
            registerCycles[PipelineRegister::IS].finish = -1;

            registerCycles[PipelineRegister::EX].start = -1;
            registerCycles[PipelineRegister::EX].finish = -1;

            registerCycles[PipelineRegister::WB].start = -1;
            registerCycles[PipelineRegister::WB].finish = -1;

            registerCycles[PipelineRegister::RT].start = -1;
            registerCycles[PipelineRegister::RT].finish = -1;
        }

        Instruction() {};

        /// @brief Attempts to get the ROB value for the destination register.
        /// @param robValue [out] Reference to an integer where the ROB value will be stored if available.
        /// @return `true` if the destination register has a valid ROB value, `false` otherwise.
        bool TryGetDestinationRegisterRobValue(int &robValue)
        {
            if (DestinationRegister.HasRobValue)
            {
                robValue = DestinationRegister.Value;
                return true;
            }
            return false;
        }

        /// @brief Sets the start cycle for a specific pipeline register.
        /// @param registerVal The pipeline register for which the start cycle is to be set.
        /// @param cycleValue The cycle value to set as the start cycle.
        void SetBeginCycleForRegister(PipelineRegister registerVal, int cycleValue)
        {
            registerCycles[registerVal].start = cycleValue;
        }
        
        /// @brief Sets the end cycle for a specific pipeline register.
        /// @param registerVal The pipeline register for which the end cycle is to be set.
        /// @param cycleValue The cycle value to set as the end cycle.
        void SetEndCycleForRegister(PipelineRegister registerVal, int cycleValue)
        {
            registerCycles[registerVal].finish = cycleValue;
        }

        /// @brief Gets the start cycle for a specific pipeline register.
        /// @param registerVal The pipeline register for which the start cycle is to be obtained.
        int GetBeginCycleValueForRegister(PipelineRegister registerVal)
        {
            return registerCycles[registerVal].start;
        }
        
        /// @brief Gets the end cycle for a specific pipeline register.
        /// @param registerVal The pipeline register for which the end cycle is to be obtained.
        int GetEndCycleValueForRegister(PipelineRegister registerVal)
        {
            return registerCycles[registerVal].finish;
        }
};

#endif