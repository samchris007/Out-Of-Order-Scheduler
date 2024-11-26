#ifndef OUT_OF_ORDER_SCHEDULER_H   // Include guard to prevent multiple inclusions
#define OUT_OF_ORDER_SCHEDULER_H

#include <string>
#include <math.h>
#include <queue>
#include <vector>
#include <algorithm>
#include "sim.h"
#include "instruction.h"
#include "instructions_table.h"
#include "rename_map_table.h"
#include "reorder_buffer.h"
#include "issue_queue.h"

using namespace std;

class Scheduler {
    public:
        InstructionsTable Decoder;
        InstructionsTable RenameRegister;
        RenameMapTable RMT;
        InstructionsTable ReadRegisterTable;
        InstructionsTable DispatchRegister;
        InstructionsTable ExecutionList;
        InstructionsTable WriteBackBuffer;
        ReorderBuffer ReorderBufferQueue;
        IssueQueue IssueBuffer;
        unsigned long &CurrentCyclesCount;
        bool FileReadComplete = false;

        vector<Instruction> FinalInstructions;

    public:
        Scheduler(FILE* file,
                unsigned long width, 
                unsigned long robSize,
                unsigned long iqSize,
                unsigned long &currentCycleCount) : Decoder(width),
                                        RenameRegister(width), 
                                        RMT(), ReadRegisterTable(width), 
                                        ReorderBufferQueue(robSize), 
                                        DispatchRegister(width),
                                        IssueBuffer(iqSize),
                                        ExecutionList(width*5),
                                        WriteBackBuffer(width*5),
                                        CurrentCyclesCount(currentCycleCount)
        {
            traceFile = file;
            tableWidth = width;
            reorderBufferSize = robSize;
            IqSize = iqSize;
        }

        // Do nothing if either (1) there are no
        // more instructions in the trace file or
        // (2) DE is not empty (cannot accept a new
        // decode bundle).
        //
        // If there are more instructions in the
        // trace file and if DE is empty (can accept
        // a new decode bundle), then fetch up to
        // WIDTH instructions from the trace file
        // into DE. Fewer than WIDTH instructions
        // will be fetched only if the trace file
        // has fewer than WIDTH instructions left. 
        void FetchInstruction(unsigned long &fetchedInstructionsCount)
        {
            uint64_t pc;
            int op_type, dest, src1, src2;
            if (!Decoder.IsEmpty())
            {
                return;
            }
            
            int currentReadLines = 0;
            while(fscanf(traceFile, "%lx %d %d %d %d", &pc, &op_type, &dest, &src1, &src2) != EOF)
            {
                currentReadLines++;
                Register sourceReg1 = {src1, false, src1 != -1};
                Register sourceReg2 = {src2, false, src2 != -1};
                Register destinationReg = {dest, false, dest != -1};
                Instruction instruction = Instruction(pc, 
                                                    op_type, 
                                                    destinationReg, 
                                                    sourceReg1,
                                                    sourceReg2,
                                                    fetchedInstructionsCount,
                                                    CurrentCyclesCount);
                
                instruction.SetEndCycleForRegister(PipelineRegister::FE, CurrentCyclesCount);
                instruction.SetEndCycleForRegister(PipelineRegister::FE, CurrentCyclesCount - instruction.registerCycles[PipelineRegister::FE].beginCycle + 1);
                instruction.SetBeginCycleForRegister(PipelineRegister::DE, CurrentCyclesCount + 1);
                Decoder.PushInstruction(instruction);

                // printf("Fetching Instruction \n");
                // printf("PC: %lx; OP Type: %d; Destination: %d; Source1: %d; Source2: %d\n", pc, op_type, dest, src1, src2);
                fetchedInstructionsCount++;
                if (currentReadLines == tableWidth)
                {
                    break;
                }
            }
        }

        // If DE contains a decode bundle:
        // If RN is not empty (cannot accept a new
        // rename bundle), then do nothing.
        // If RN is empty (can accept a new rename
        // bundle), then advance the decode bundle
        // from DE to RN.
        void DecodeInstruction()
        {
            if (Decoder.IsEmpty() || RenameRegister.IsFull())
            {
                return;
            }
            while(!Decoder.IsEmpty() && !RenameRegister.IsFull())
            {
                Instruction instruction = Decoder.Front();
                instruction.SetEndCycleForRegister(PipelineRegister::DE, CurrentCyclesCount - instruction.registerCycles[PipelineRegister::DE].beginCycle + 1);
                instruction.SetBeginCycleForRegister(PipelineRegister::RN, CurrentCyclesCount+1);
                
                RenameRegister.PushInstruction(instruction);
                Decoder.PopInstruction();
            }
        }

        // If RN contains a rename bundle:
        // If either RR is not empty (cannot accept
        // a new register-read bundle) or the ROB
        // does not have enough free entries to
        // accept the entire rename bundle, then do
        // nothing.
        // If RR is empty (can accept a new
        // register-read bundle) and the ROB has 10
        // enough free entries to accept the entire
        // rename bundle, then process (see below)
        // the rename bundle and advance it from
        // RN to RR.
        //
        // Apply your learning from the class
        // lectures/notes on the steps for renaming:
        // (1) allocate an entry in the ROB for the
        // instruction, (2) rename its source
        // registers, and (3) rename its destination
        // register (if it has one). Note that the
        // rename bundle must be renamed in program
        // order (fortunately the instructions in
        // the rename bundle are in program order).
        void Rename()
        {
            if (RenameRegister.IsEmpty())
            {
                return;
            }
            if (!ReadRegisterTable.IsEmpty() 
                || ReorderBufferQueue.FreeEntries() < RenameRegister.GetSize())
            {
                return;
            }
            int totalElementsAdded = 1;
            while(!ReorderBufferQueue.IsFull() && !RenameRegister.IsEmpty())
            {
                Instruction instruction = RenameRegister.Front();
                int robValue = ReorderBufferQueue.CreateNewEntryAndGetRobValue(instruction, totalElementsAdded);
                instruction.robValue = robValue;
                CheckIfSourceOperandsHasRMTValues(instruction);

                if (instruction.DestinationRegister.Exist)
                {
                    RMT.AddElement(true, robValue, instruction.DestinationRegister.Value);
                }

                instruction.DestinationRegister.Value = robValue;
                instruction.DestinationRegister.Exist = true;
                instruction.DestinationRegister.HasRobValue = true;

                instruction.SetEndCycleForRegister(PipelineRegister::RN, CurrentCyclesCount+1 - instruction.registerCycles[PipelineRegister::RN].beginCycle);
                instruction.SetBeginCycleForRegister(PipelineRegister::RR, CurrentCyclesCount+1);
                
                ReadRegisterTable.PushInstruction(instruction);
                RenameRegister.PopInstruction();
                totalElementsAdded++;
            }


            return;
        }

        // If RR contains a register-read bundle:
        // If DI is not empty (cannot accept a
        // new dispatch bundle), then do nothing.
        // If DI is empty (can accept a new dispatch
        // bundle), then process (see below) the
        // register-read bundle and advance it from
        // RR to DI.
        //
        // Since values are not explicitly modeled,
        // the sole purpose of the Register Read
        // stage is to ascertain the readiness of
        // the renamed source operands. Apply your
        // learning from the class lectures/notes on
        // this topic.
        //
        // Also take care that producers in their
        // last cycle of execution wakeup dependent
        // operands not just in the IQ, but also in
        // two other stages including RegRead()
        // (this is required to avoid de
        void ReadRegister()
        {
            if (ReadRegisterTable.IsEmpty() || DispatchRegister.IsFull())
            {
                return;
            }

            // Tag Wakeup rob values are replaced with actual register values

            while(!ReadRegisterTable.IsEmpty() && !DispatchRegister.IsFull())
            {
                Instruction instruction  = ReadRegisterTable.Front();
                instruction.SourceRegister1.isReady = IsRegisterReady(instruction.SourceRegister1);
                instruction.SourceRegister2.isReady = IsRegisterReady(instruction.SourceRegister2);

                instruction.SetEndCycleForRegister(PipelineRegister::RR, CurrentCyclesCount+1 - instruction.registerCycles[PipelineRegister::RR].beginCycle);
                instruction.SetBeginCycleForRegister(PipelineRegister::DI, CurrentCyclesCount+1);

                DispatchRegister.PushInstruction(instruction);
                ReadRegisterTable.PopInstruction();
            }
        }

        // If DI contains a dispatch bundle:
        // If the number of free IQ entries is less
        // than the size of the dispatch bundle in
        // DI, then do nothing. If the number of
        // free IQ entries is greater than or equal
        // to the size of the dispatch bundle in DI,
        // then dispatch all instructions from DI to
        // the IQ.
        void DispatchInstruction()
        {
            if (DispatchRegister.IsEmpty() || IssueBuffer.FreeIQEntries() < DispatchRegister.GetSize())
            {
                return;
            }
            for (int i = 0; i < IssueBuffer.size; i++)
            {
                if (IssueBuffer.issueQueue[i].InstructionValidInIQ == false && !DispatchRegister.IsEmpty())
                {
                    Instruction instruction = DispatchRegister.Front();
                    instruction.SetEndCycleForRegister(PipelineRegister::DI, CurrentCyclesCount+1 - instruction.registerCycles[PipelineRegister::DI].beginCycle);
                    instruction.SetBeginCycleForRegister(PipelineRegister::IS, CurrentCyclesCount+1);

                    instruction.InstructionValidInIQ = true;
                    IssueBuffer.issueQueue[i] = instruction;
                    DispatchRegister.PopInstruction();
                }
            }
        }
        
        // Issue up to WIDTH oldest instructions
        // from the IQ. (One approach to implement
        // oldest-first issuing, is to make multiple
        // passes through the IQ, each time finding
        // the next oldest ready instruction and
        // then issuing it. One way to annotate the
        // age of an instruction is to assign an
        // incrementing sequence number to each
        // instruction as it is fetched from the
        // trace file.)
        // To issue an instruction:
        // 1) Remove the instruction from the IQ.
        // 2) Add the instruction to the
        // execute_list. Set a timer for the
        // instruction in the execute_list that
        // will allow you to model its execution
        // latency.
        void IssueInstruction(std::map<int, int> opTypeByLatency)
        {
            sort(IssueBuffer.issueQueue, IssueBuffer.issueQueue + IqSize, [this](const Instruction& a, const Instruction& b) 
            {
            return compareInstructions(a, b);
            });
            
            if (IssueBuffer.IsEmpty())
            {
                return;
            }
            int issuedInstructions = 0;
            for (int i = 0; i < IqSize; i++)
            {
                if (issuedInstructions >= tableWidth)
                {
                    break;
                }
                
                if (IssueBuffer.issueQueue[i].instructionSequenceNumber < 0 
                || !IssueBuffer.issueQueue[i].InstructionValidInIQ 
                || !IssueBuffer.issueQueue[i].SourceRegister1.isReady
                || !IssueBuffer.issueQueue[i].SourceRegister2.isReady)
                {
                    continue;
                }

                Instruction instruction = IssueBuffer.issueQueue[i];
                instruction.SetEndCycleForRegister(PipelineRegister::IS, CurrentCyclesCount+1 - instruction.registerCycles[PipelineRegister::IS].beginCycle);
                instruction.SetBeginCycleForRegister(PipelineRegister::EX, CurrentCyclesCount+1);
                
                IssueBuffer.RemoveElementAtIndex(i);
                instruction.latency = opTypeByLatency[instruction.OpType];
                ExecutionList.PushInstruction(instruction);
                issuedInstructions++;
            }
        }

        // From the execute_list, check for
        // instructions that are finishing
        // execution this cycle, and:
        // 1) Remove the instruction from
        // the execute_list.
        // 2) Add the instruction to WB.
        // 3) Wakeup dependent instructions (set
        // their source operand ready flags) in
        // the IQ, DI (the dispatch bundle), and
        // RR (the register-read bundle).
        void Execute()
        {
            InstructionsTable tempTable = InstructionsTable(tableWidth*5);
            while (!ExecutionList.IsEmpty())
            {
                if (ExecutionList.Front().latency == 1)
                {
                    Instruction instruction = ExecutionList.Front();
                    ReadRegisterTable.CheckAndWakeupSourceOperandsInInstructions(instruction.robValue);
                    DispatchRegister.CheckAndWakeupSourceOperandsInInstructions(instruction.robValue);
                    IssueBuffer.CheckAndWakeupSourceOperandsInInstructions(instruction.robValue);
                instruction.SetEndCycleForRegister(PipelineRegister::EX, 
                                CurrentCyclesCount+1 - instruction.registerCycles[PipelineRegister::EX].beginCycle);
                instruction.SetBeginCycleForRegister(PipelineRegister::WB, CurrentCyclesCount+1);
                    WriteBackBuffer.PushInstruction(instruction);
                }
                else
                {
                    Instruction instruction = ExecutionList.Front();
                    instruction.latency--;
                    tempTable.PushInstruction(instruction);
                }
                ExecutionList.PopInstruction();
            }
            ExecutionList = tempTable;
        }

        // From the execute_list, check for
        // instructions that are finishing
        // execution this cycle, and:
        // 1) Remove the instruction from
        // the execute_list.
        // 2) Add the instruction to WB.
        // 3) Wakeup dependent instructions (set
        // their source operand ready flags) in
        // the IQ, DI (the dispatch bundle), and
        // RR (the register-read bundle).
        void WritebackToRegister()
        {
            if (WriteBackBuffer.IsEmpty())
            {
                return;
            }
            while(!WriteBackBuffer.IsEmpty())
            {
                Instruction instruction = WriteBackBuffer.Front();
                instruction.SetEndCycleForRegister(PipelineRegister::WB, 
                                CurrentCyclesCount+1 - instruction.registerCycles[PipelineRegister::WB].beginCycle);
                instruction.SetBeginCycleForRegister(PipelineRegister::RT, CurrentCyclesCount+1);
                ReorderBufferQueue.UpdateReadinessOfTheInstruction(instruction.DestinationRegister.Value, 
                    instruction.registerCycles);
                WriteBackBuffer.PopInstruction();
            }
        }

        // Retire up to WIDTH consecutive
        // “ready” instructions from the head of
        // the ROB.
        void RetireInstructions()
        {
            int poppedInstructions = 0;

            if (ReorderBufferQueue.IsEmpty() 
                || !ReorderBufferQueue.Front().DestinationRegister.isReady)
            {
                return;
            }
            while(!ReorderBufferQueue.IsEmpty() 
                && ReorderBufferQueue.Front().DestinationRegister.isReady
                && poppedInstructions < tableWidth)
            {
                Instruction instruction = ReorderBufferQueue.Front();
                instruction.SetEndCycleForRegister(PipelineRegister::RT, 
                                CurrentCyclesCount+1 - instruction.registerCycles[PipelineRegister::RT].beginCycle);
                FinalInstructions.push_back(instruction);

                RenameMapElement mapElement;
                if (RMT.TryGetElement(instruction.DestinationRegister.Value, mapElement)
                    && instruction.robValue == mapElement.RobIndex)
                {
                    RMT.RemoveElement(instruction.DestinationRegister.Value);
                }
                ReorderBufferQueue.PopInstruction();
                poppedInstructions++;
            }
        }

        bool AdvanceToNextCycle()
        {
            if (feof(traceFile) && ReorderBufferQueue.IsEmpty()) 
            {
                return false;
            }
            return true;
        }

    private:
        FILE* traceFile;
        unsigned long tableWidth = 0;
        unsigned long reorderBufferSize = 0;
        unsigned long IqSize = 0; 

        void CheckIfSourceOperandsHasRMTValues(Instruction& instruction)
        {
            RenameMapElement sourceRegister1, sourceRegister2, destinationRegister;
            Register sourceReg1, destReg, sourceReg2;

            if (instruction.SourceRegister1.Exist && RMT.TryGetElement(instruction.SourceRegister1.Value, sourceRegister1))
            {
                sourceReg1 = {sourceRegister1.RobIndex, true, true};
                instruction.SourceRegister1 = sourceReg1;
            }
            if (instruction.SourceRegister2.Exist && RMT.TryGetElement(instruction.SourceRegister2.Value, sourceRegister2)) 
            {
                sourceReg2= {sourceRegister2.RobIndex, true, true};
                instruction.SourceRegister2 = sourceReg2;
            }
        }

        bool IsRegisterReady(Register registerVal)
        {
            if (registerVal.HasRobValue && ReorderBufferQueue.IsRobEntryReady(registerVal.Value)
                || !registerVal.Exist
                || registerVal.isReady)
            {
                return true;
            }
            else if (!registerVal.HasRobValue && registerVal.Value != -1)
            {
                // ARF register
                return true;
            }
            else if (registerVal.HasRobValue && !ReorderBufferQueue.HasRobEntry(registerVal.Value))
            {
                return true;
            }
            return false;
        }

        bool compareInstructions(const Instruction& a, const Instruction& b) 
        {
            return a.instructionSequenceNumber>=0 && b.instructionSequenceNumber >= 0 && a.instructionSequenceNumber < b.instructionSequenceNumber;
        }
};

#endif  // End of include guard