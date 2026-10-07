/*
 *
 * FILE        : AssemblyEngine.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : COA Lab | Course Code: 2310221L
 * TASK        : Task 2 — 64-bit Assembly Logic
 * COURSE OUT  : CO2 — Implement displacement calculation and
 *               coordinate delta using 64-bit assembly logic.
 *               x86-64 instructions: MOV ADD SUB CMP JMP
 *               and conditional jumps.
 *               Includes 2 real inline ASM operations.
 * DATE        : 2026-2027
 *
 */

#ifndef ASSEMBLY_ENGINE_H
#define ASSEMBLY_ENGINE_H

#include <cstddef>
#include <string>
#include <vector>
#include <cmath>
#include "DroneRegisters.h"
#include "../shared/JsonWriter.h"
using namespace std;

// ════════════════════════════════════════════════════════════
// [COA CO2 - Task 2: AssemblyEngine Class]
//
// Simulates an x86-64 instruction pipeline for drone
// coordinate and displacement computation.
//
// Two operations use REAL inline x86-64 assembly:
//   1. asmAddCoordinates  — uses actual ADD instruction
//   2. asmSubCoordinates  — uses actual SUB instruction
//
// All other operations are C++ simulations of assembly
// logic with matching register semantics.
// ════════════════════════════════════════════════════════════
class AssemblyEngine {
private:

    // Reference to the register file
    DroneRegisters& regs;

    // Instruction trace log (sent to JSON and displayed)
    vector<InstructionData> instructionLog;

    // Current displacement computation results
    DisplacementData currentDisp;

    // ── Private helpers ──────────────────────────────────────
    void logInstruction(const string& addr,
                        const string& op,
                        const string& args);

    string pcToHex(long long pc) const;

public:
    // Constructor — requires a DroneRegisters instance
    explicit AssemblyEngine(DroneRegisters& registerFile);

    // ── Simulated Assembly Instructions [COA CO2] ─────────────

    // MOV simulation — loads value into named register slot
    void SIM_MOV(long long& dest, long long src,
                 const string& destReg,
                 const string& srcOperand);

    // ADD simulation — dest = dest + src, logs instruction
    long long SIM_ADD(long long dest, long long src,
                      const string& destReg,
                      const string& srcOperand);

    // SUB simulation — dest = dest - src, logs instruction
    long long SIM_SUB(long long dest, long long src,
                      const string& destReg,
                      const string& srcOperand);

    // [COA CO3 - Task 3] Multiplication using successive addition.
    long long SIM_MUL_SUCCESSIVE(long long multiplicand,
                                 long long multiplier);

    // [COA CO3 - Task 3] String operations using 64-bit string instructions.
    size_t asmStringLength(const string& text);
    void asmStringAppend(string& destination, const string& source);

    // CMP + conditional JMP simulation
    // Returns true if jump should be taken
    bool SIM_CMP_JMP(long long op1, long long op2,
                     const string& jumpType,
                     const string& op1Name,
                     const string& op2Name);

    // ── REAL Inline x86-64 Assembly [COA CO2] ─────────────────

    // Real ASM Operation 1:
    // Adds two 64-bit coordinate values using actual ADD
    // Uses __asm__ volatile with x86-64 AT&T syntax
    long long asmAddCoordinates(long long coordA, long long coordB);

    // Real ASM Operation 2:
    // Subtracts two coordinates using actual SUB
    long long asmSubCoordinates(long long coordA, long long coordB);

    // ── Displacement Computation [COA CO2] ────────────────────

    // Main computation: given two grid positions,
    // compute delta-X, delta-Y, and Euclidean displacement.
    // Uses simulated assembly instruction sequence:
    // MOV, SUB, IMUL (squared), ADD (sum), computes sqrt.
    DisplacementData computeDisplacement(
        int fromRow, int fromCol,
        int toRow,   int toCol);

    // ── Display ──────────────────────────────────────────────
    void displayInstructionLog()    const;
    void displayDisplacement()      const;

    // ── Accessors ────────────────────────────────────────────
    const vector<InstructionData>& getInstructionLog() const;
    const DisplacementData&             getDisplacement()   const;

    // Clear log between missions
    void clearLog();
};

#endif // ASSEMBLY_ENGINE_H