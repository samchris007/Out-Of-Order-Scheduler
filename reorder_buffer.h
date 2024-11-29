#ifndef REORDER_BUFFER_H   // Include guard to prevent multiple inclusions
#define REORDER_BUFFER_H

#include <queue>
#include <math.h>
#include "instructions_table.h"

using namespace std;

/// @class ReorderBuffer
/// @brief The class that implements ROB queue
class ReorderBuffer : public InstructionsTable
{
    public:
        int tailIndex = -1;
        int lastRemovedElementIndex;
        unsigned long queueSize;
        
        ReorderBuffer(unsigned long queue_size) : InstructionsTable (queue_size) 
        {
            queueSize = queue_size;
        }
        
        /// @brief Creates a new entry in the ROB and returns the ROB index value.
        /// @param instruction The instruction in which a new entry is to be created.
        int CreateNewEntryAndGetRobValue(Instruction instruction)
        {
            if (tailIndex >= queueSize)
            {
                tailIndex = -1;
            }
            tailIndex++;
            instruction.RobValue = tailIndex;
            InstructionsQueue.push(instruction);
            return instruction.RobValue;
        }

        /// @brief Gets whether the rob entry is ready or not.
        /// @param robValue Rob value.
        bool IsRobEntryReady(int robValue)
        {
            queue<Instruction> instructions = InstructionsQueue;
            while(!instructions.empty())
            {
                if(instructions.front().RobValue == robValue 
                && instructions.front().DestinationRegister.IsReady)
                {
                    return true;
                }
                instructions.pop();
            }
            return false;
        }

        /// @brief Sets the source registers as ready based on the rob value.
        /// @param robValue Rob value.
        /// @param registerCycles Register cycles of each pipeline stages.
        void UpdateReadinessOfTheInstruction(int robValue, std::map<PipelineRegister, CycleInfo> registerCycles)
        {
            std::queue<Instruction> tempQueue;
            while (!InstructionsQueue.empty()) 
            {
                if (InstructionsQueue.front().RobValue == robValue) 
                {
                    InstructionsQueue.front().DestinationRegister.IsReady = true;
                    InstructionsQueue.front().registerCycles = registerCycles;
                }
                tempQueue.push(InstructionsQueue.front());
                InstructionsQueue.pop();
            }
            InstructionsQueue = tempQueue;
        }

        /// @brief Gets whether the rob entry is present or not.
        /// @param robValue Rob value.
        bool HasRobEntry(int robValue)
        {
            std::queue<Instruction> tempQueue = InstructionsQueue;
            while (!tempQueue.empty()) 
            {
                if (tempQueue.front().RobValue == robValue) 
                {
                    return true;
                }
                tempQueue.pop();
            }
            return false;
        }
};

#endif