#ifndef INSTRUCTIONS_TABLE_H   // Include guard to prevent multiple inclusions
#define INSTRUCTIONS_TABLE_H

#include <queue>
#include "instruction.h"

using namespace std;

/// @class InstructionsTable
/// @brief Base class that contains queue of instructions.
class InstructionsTable
{
    public:
        std::queue<Instruction> InstructionsQueue;

        InstructionsTable(unsigned long queue_size)
        {
            size = queue_size;
        }
        
        /// @brief Pushes instruction at the end of the queue.
        /// @param instruction Instruction to be pushed
        void PushInstruction(Instruction instruction)
        {
            if (InstructionsQueue.size() >= size)
            {
                return;
            }
            InstructionsQueue.push(instruction);
        }
        
        /// @brief Removes instruction at the head of the queue.
        /// @param instruction Instruction to be popped
        void PopInstruction()
        {
            InstructionsQueue.pop();
        }

        /// @brief Gets the head instruction of the queue.
        Instruction Front()
        {
            return InstructionsQueue.front();
        }
        
        /// @brief Gets the size of the queue.
        unsigned long GetSize()
        {
            return InstructionsQueue.size();
        }

        /// @brief Gets whether the queue is empty or not.
        bool IsEmpty()
        {
            return InstructionsQueue.empty();
        }

        /// @brief Gets whether the queue is full or not.
        bool IsFull()
        {
            return InstructionsQueue.size() == size;
        }
        
        /// @brief Gets the free entries of the queue.
        unsigned long GetFreeEntries()
        {
            return size - InstructionsQueue.size();
        }

        /// @brief Checks and sets the readiness of the source registers
        /// @param robValue Reorder buffer value
        void CheckAndWakeupSourceOperandsInInstructions(int robValue)
        {
            std::queue<Instruction> tempQueue;

            // Iterate through the original queue
            while (!InstructionsQueue.empty()) 
            {
                Instruction instruction = InstructionsQueue.front();
                if (instruction.SourceRegister1.HasRobValue 
                    && instruction.SourceRegister1.Value == robValue) 
                {
                    instruction.SourceRegister1.IsReady = true;
                }
                if (instruction.SourceRegister2.HasRobValue 
                    && instruction.SourceRegister2.Value == robValue) 
                {
                    instruction.SourceRegister2.IsReady = true;
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