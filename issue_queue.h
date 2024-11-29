#ifndef ISSUE_QUEUE_H   // Include guard to prevent multiple inclusions
#define ISSUE_QUEUE_H

#include <queue>
#include <string>
#include <limits>
#include "instruction.h"
#include "instructions_table.h"

using namespace std;

/// @class IssueQueue
/// @brief Provides an abstract layer for IssueQueue
class IssueQueue
{
    public:
        Instruction* issueQueue;
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

        /// @brief Gets free issue queue entries
        int GetFreeIssueQueueEntries()
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

        /// @brief Checks and sets the readiness of the source registers
        /// @param robValue Reorder buffer value
        void CheckAndWakeupSourceOperandsInInstructions(int robValue)
        {
            for (int i = 0; i < size; i++)
            {
                if (issueQueue[i].SourceRegister1.HasRobValue 
                    && issueQueue[i].SourceRegister1.Value == robValue) 
                {
                    issueQueue[i].SourceRegister1.IsReady = true;
                }
                if (issueQueue[i].SourceRegister2.HasRobValue 
                    && issueQueue[i].SourceRegister2.Value == robValue) 
                {
                    issueQueue[i].SourceRegister2.IsReady = true;
                }
            }
        }

        /// @brief Finds whether the issue queue is empty or not
        bool IsEmpty()
        {
            return GetFreeIssueQueueEntries() == size;
        }

        /// @brief Sets the validity of the element by index
        /// @param index Index of the issue queue
        void RemoveElementAtIndex(int index)
        {
            issueQueue[index].InstructionValidInIQ = false;
        }
};

#endif