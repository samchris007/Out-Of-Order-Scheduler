#ifndef REORDER_BUFFER_H   // Include guard to prevent multiple inclusions
#define REORDER_BUFFER_H

#include <queue>
#include <math.h>
#include "instructions_table.h"

using namespace std;

class ReorderBuffer : public InstructionsTable
{
    public:
        int tailIndex = -1;
        int lastRemovedElementIndex;
        ReorderBuffer(unsigned long queue_size) : InstructionsTable (queue_size) {}

        int CreateNewEntryAndGetRobValue(Instruction instruction, int elementsToBeAdded = 0)
        {
            int robValue;
            if (!InstructionsQueue.empty())
            {
                bool hasRobValue = InstructionsQueue.front().TryGetDestinationRegisterRobValue(robValue);
                if (hasRobValue && robValue != 0 && tailIndex == (InstructionsQueue.size() - 1))
                {
                    tailIndex = lastRemovedElementIndex;
                    instruction.robValue = lastRemovedElementIndex + elementsToBeAdded;
                    instruction.DestinationRegister.isReady = false;
                    InstructionsQueue.push(instruction);
                    // printf("robValue : %d\n", instruction.robValue); 
                    return instruction.robValue;
                }
            }
            
            tailIndex++;
            instruction.robValue = tailIndex;
            InstructionsQueue.push(instruction);
            // printf("robValue : %d\n", instruction.robValue);
            return instruction.robValue;
        }

        bool IsRobEntryReady(int robValue)
        {
            queue<Instruction> instructions = InstructionsQueue;
            while(!instructions.empty())
            {
                if(instructions.front().robValue == robValue && instructions.front().DestinationRegister.isReady)
                {
                    return true;
                }
                instructions.pop();
            } 
            return false;
        }

        void UpdateReadinessOfTheInstruction(int robValue, std::map<PipelineRegister, CycleInfo> registerCycles)
        {
            std::queue<Instruction> tempQueue;
            while (!InstructionsQueue.empty()) 
            {
                if (InstructionsQueue.front().robValue == robValue) 
                {
                    InstructionsQueue.front().DestinationRegister.isReady = true;
                    InstructionsQueue.front().registerCycles = registerCycles;
                }
                tempQueue.push(InstructionsQueue.front());
                InstructionsQueue.pop();
            }
            InstructionsQueue = tempQueue;
        }

        bool HasRobEntry(int robValue)
        {
            std::queue<Instruction> tempQueue = InstructionsQueue;
            while (!tempQueue.empty()) 
            {
                if (tempQueue.front().robValue == robValue) 
                {
                    return true;
                }
                tempQueue.pop();
            }
            return false;
        }

        // void RemoveRobEntriesByReadyBits()
        // {
        //     if (!InstructionsQueue.front().isInstructionReady)
        //     {
        //         return;
        //     }
        //     while (InstructionsQueue.front().isInstructionReady)
        //     {
        //         lastRemovedElementIndex = InstructionsQueue.front().DestinationRegister.Value;
        //         InstructionsQueue.pop();
        //     }
        // }
};

#endif