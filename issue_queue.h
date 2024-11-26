#ifndef ISSUE_QUEUE_H   // Include guard to prevent multiple inclusions
#define ISSUE_QUEUE_H

#include <queue>
#include <string>
#include <limits>
#include "instruction.h"
#include "instructions_table.h"

using namespace std;

class IssueQueue
{
    public:
        Instruction* issueQueue;
        int lastlyAddedElementIndex = 0;
        unsigned long size;

        IssueQueue(int iqSize)
        {
            size = iqSize;
            issueQueue = new Instruction[iqSize];

            for (int i = 0; i < size; i++)
            {
                Instruction instruction = Instruction();
                instruction.InstructionValidInIQ = false;
                issueQueue[i] = instruction;
            }
        }
        
        void AddElements(InstructionsTable &instructions)
        {
            for (int i = 0; i < size; i++)
            {
                if (instructions.IsEmpty())
                {
                    return;
                }

                if (issueQueue[i].InstructionValidInIQ == false)
                {
                    Instruction instruction = instructions.Front();
                    instruction.InstructionValidInIQ = true;
                    issueQueue[i] = instruction;
                    instructions.PopInstruction();
                }
            }
        }

        int FreeIQEntries()
        {
            int totalFreeEntries = 0;
            for (int i = 0; i < size; i++)
            {
                if (issueQueue[i].InstructionValidInIQ == false)
                {
                    totalFreeEntries++;
                }
            }
            return totalFreeEntries;
        }

        Instruction GetOldestSequenceInstruction(int &oldestInstructionIndex)
        {
            unsigned long min_seq_no = numeric_limits<unsigned long>::max();
            Instruction minEntry;

            for (int i = 0; i < size; i++) 
            {
                if (issueQueue[i].InstructionValidInIQ 
                    && issueQueue[i].instructionSequenceNumber < min_seq_no
                    && issueQueue[i].instructionSequenceNumber != -1) 
                {
                    min_seq_no = issueQueue[i].instructionSequenceNumber;
                    minEntry = issueQueue[i];
                    oldestInstructionIndex = i;
                }
            }
            return minEntry;
        }

        void CheckAndWakeupSourceOperandsInInstructions(int destinationRobValue)
        {
            for (int i = 0; i < size; i++)
            {
                if (issueQueue[i].SourceRegister1.HasRobValue 
                    && issueQueue[i].SourceRegister1.Value == destinationRobValue) 
                {
                    issueQueue[i].SourceRegister1.isReady = true;
                }
                if (issueQueue[i].SourceRegister2.HasRobValue 
                    && issueQueue[i].SourceRegister2.Value == destinationRobValue) 
                {
                    issueQueue[i].SourceRegister2.isReady = true;
                }
            }
        }

        bool IsEmpty()
        {
            return FreeIQEntries() == size;
        }

        void RemoveElementAtIndex(int index)
        {
            issueQueue[index].InstructionValidInIQ = false;
        }
};

#endif