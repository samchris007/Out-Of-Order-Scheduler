#ifndef INSTRUCTIONS_TABLE_H   // Include guard to prevent multiple inclusions
#define INSTRUCTIONS_TABLE_H

#include <queue>
#include "instruction.h"

using namespace std;

class InstructionsTable
{
    public:
        std::queue<Instruction> InstructionsQueue;

        InstructionsTable(unsigned long queue_size)
        {
            size = queue_size;
        }
        
        void PushInstruction(Instruction instruction)
        {
            if (InstructionsQueue.size() >= size)
            {
                return;
            }
            InstructionsQueue.push(instruction);
        }
        
        void PopInstruction()
        {
            InstructionsQueue.pop();
        }

        Instruction Front()
        {
            return InstructionsQueue.front();
        }

        unsigned long GetSize()
        {
            return InstructionsQueue.size();
        }

        bool IsEmpty()
        {
            return InstructionsQueue.size() == 0;
        }

        bool IsFull()
        {
            return InstructionsQueue.size() == size;
        }

        unsigned long FreeEntries()
        {
            return size - InstructionsQueue.size();
        }

        void CheckAndWakeupSourceOperandsInInstructions(int destinationRobValue)
        {
            std::queue<Instruction> tempQueue;

            // Iterate through the original queue
            while (!InstructionsQueue.empty()) 
            {
                Instruction instruction = InstructionsQueue.front();
                if (instruction.SourceRegister1.HasRobValue 
                    && instruction.SourceRegister1.Value == destinationRobValue) 
                {
                    instruction.SourceRegister1.isReady = true;
                }
                if (instruction.SourceRegister2.HasRobValue 
                    && instruction.SourceRegister2.Value == destinationRobValue) 
                {
                    instruction.SourceRegister2.isReady = true;
                }
                tempQueue.push(instruction);
                InstructionsQueue.pop();
            }

            InstructionsQueue = tempQueue;
        }
    
    public:
        unsigned long int size = 0;
};

#endif