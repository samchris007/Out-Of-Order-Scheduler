# Dynamic Instruction Scheduling Simulator

This project implements a simulator for an out-of-order superscalar processor that dynamically schedules instructions. The simulator models core aspects of processor architecture, including:
- Dynamic scheduling of instructions.
- Pipeline stages: Fetch, Decode, Rename, Register Read, Dispatch, Issue, Execute, Writeback, and Retire.
- Key architectural components like the Reorder Buffer (ROB), Issue Queue (IQ), and Rename Map Table (RMT).

The simulator assumes:
- **Perfect branch prediction**.
- **Perfect caches** (no memory dependencies are modeled).
- Execution is based on a MIPS-like ISA.

## Key Features
- Models data dependencies (through registers).
- Simulates structural hazards in the IQ and ROB.
- Outputs detailed per-instruction timing and simulation statistics.

---

## Usage Instructions

### Run the Simulator
Use the following command to run the simulator:
```bash
./sim <ROB_SIZE> <IQ_SIZE> <WIDTH> <tracefile>
```
- `<ROB_SIZE>`: Number of entries in the Reorder Buffer.
- `<IQ_SIZE>`: Number of entries in the Issue Queue.
- `<WIDTH>`: Pipeline width (number of instructions processed in parallel).
- `<tracefile>`: Path to the input trace file.

Example:
```bash
./sim 32 16 4 sample.trace.txt
```

## Input Trace File Format

Each line in the trace file represents an instruction in the following format:

```text
<PC> <operation type> <dest reg #> <src1 reg #> <src2 reg #>
```

Where:

- `<PC>`: Program counter (in hex).
- `<operation type>`: `0`, `1`, or `2` (determines execution latency).
- `<dest reg #>`: Destination register (-1 if none).
- `<src1 reg #>` and `<src2 reg #>`: Source registers (`-1` if none).

## Output Format
The simulator produces:

1. Per-instruction timing information:

```text 
<seq_no> fu{<op_type>} src{<src1>,<src2>} dst{<dst>} FE{<begin-cycle>,<duration>} ...
```

- `<seq_no>`: Instruction sequence number.
- `<op_type>`: Operation type (`0`, `1`, or `2`).
- `<begin-cycle>` and `<duration>`: Cycle timing information for each pipeline stage.

2. Final summary:

    - **Dynamic instruction count**: Total number of retired instructions.
    - **Cycles**: Total cycles taken to retire all instructions.
    - **Instructions per cycle (IPC)**: Total retired instructions divided by total cycles.

## Development Notes
### Pipeline Configuration
The pipeline operates with the following registers:

- `DE`: Between Fetch and Decode.
- `RN`: Between Decode and Rename.
- `RR`: Between Rename and Register Read.
- `DI`: Between Register Read and Dispatch.
- `IQ`: Issue Queue.
- `ROB`: Reorder Buffer.

### Execution Latencies
- Type 0: 1 cycle.
- Type 1: 2 cycles.
- Type 2: 5 cycles.

