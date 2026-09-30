/*
 * ============================================================
 * FILE        : DroneRegisters.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : COA Lab | Course Code: 2310221L
 * TASK        : Task 1 — CPU Register Simulation
 * COURSE OUT  : CO1 — Simulate internal CPU hardware registers
 *               RAX RBX RCX RDX PC IR SP FLAGS and implement
 *               basic register transfer operations for drone
 *               telemetry processing.
 * AUTHOR      : [Your Name] | MIT Academy of Engineering
 * DATE        : 2026-2027
 * ============================================================
 */

#ifndef DRONE_REGISTERS_H
#define DRONE_REGISTERS_H

#include <string>
#include <vector>
#include "../shared/Constants.h"
#include "../shared/JsonWriter.h"

// ════════════════════════════════════════════════════════════
// [COA CO1 - Task 1: FLAGS Register Structure]
// Models the x86-64 FLAGS/RFLAGS register.
// Each bit represents a condition after an arithmetic op.
// ════════════════════════════════════════════════════════════
struct FLAGS_Register {
    bool ZF;  // Zero Flag    — result was zero
    bool CF;  // Carry Flag   — unsigned overflow occurred
    bool SF;  // Sign Flag    — result was negative
    bool OF;  // Overflow Flag — signed overflow occurred
    bool PF;  // Parity Flag  — result has even number of 1-bits

    // Constructor — all flags clear on power-on
    FLAGS_Register() : ZF(false), CF(false),
                       SF(false), OF(false), PF(false) {}

    // Reset all flags to 0
    void clear() {
        ZF = CF = SF = OF = PF = false;
    }

    // Display flags in terminal
    void display() const;
};

// ════════════════════════════════════════════════════════════
// [COA CO1 - Task 1: DroneRegisters Class]
//
// Models the complete x86-64 register file for AeroRoute.
// Register-to-telemetry mapping:
//
//   RAX — Altitude in metres
//   RBX — Speed in cm/s  (divide by 100 for m/s)
//   RCX — Heading in degrees x 100 (divide by 100 for degrees)
//   RDX — Battery percentage x 100 (divide by 100 for %)
//   PC  — Program Counter (instruction address)
//   IR  — Instruction Register (current instruction opcode)
//   SP  — Stack Pointer (call stack base)
//   FLAGS — Condition flags updated after each operation
// ════════════════════════════════════════════════════════════
class DroneRegisters {
private:

    // [COA CO1 - Task 1: General Purpose Registers — 64-bit]
    long long RAX;   // altitude (metres)
    long long RBX;   // speed    (cm/s)
    long long RCX;   // heading  (degrees * 100)
    long long RDX;   // battery  (percent * 100)

    // [COA CO1 - Task 1: Special Purpose Registers]
    long long PC;    // Program Counter — address of next instr
    long long IR;    // Instruction Register — current instr
    long long SP;    // Stack Pointer

    // [COA CO1 - Task 1: FLAGS Register]
    FLAGS_Register FLAGS;

    // Instruction counter for PC advancement
    int instructionCount;

    // ── Private helpers ──────────────────────────────────────
    void updateFlags(long long result);
    bool checkParity(long long value);

public:
    // Constructor — power-on state
    DroneRegisters();

    // ── Register Transfer Operations [COA CO1] ───────────────

    // MOV: load immediate value into register
    // Equivalent: MOV RAX, value
    void MOV_RAX(long long value);
    void MOV_RBX(long long value);
    void MOV_RCX(long long value);
    void MOV_RDX(long long value);

    // MOV register to register
    // Equivalent: MOV RBX, RAX
    void MOV_REG_TO_REG(long long& dest, long long src,
                        const std::string& destName,
                        const std::string& srcName);

    // ADD: dest = dest + src, updates FLAGS
    // Equivalent: ADD RAX, RBX
    long long ADD(long long dest, long long src,
                  const std::string& destName);

    // SUB: dest = dest - src, updates FLAGS
    // Equivalent: SUB RAX, RBX
    long long SUB(long long dest, long long src,
                  const std::string& destName);

    // CMP: subtract without storing, only update FLAGS
    // Equivalent: CMP RAX, RBX
    void CMP(long long op1, long long op2,
             const std::string& op1Name,
             const std::string& op2Name);

    // Advance PC by 4 bytes (one instruction)
    void advancePC();

    // ── Telemetry load operations ─────────────────────────────
    // These load real drone data into registers
    void loadTelemetry(int altitudeM, double speedMs,
                       double headingDeg, double batteryPct);

    // ── Accessors ────────────────────────────────────────────
    long long getRAX()   const;
    long long getRBX()   const;
    long long getRCX()   const;
    long long getRDX()   const;
    long long getPC()    const;
    long long getIR()    const;
    long long getSP()    const;
    FLAGS_Register getFLAGS() const;

    // Decoded telemetry values
    int    getAltitudeM()   const;
    double getSpeedMs()     const;
    double getHeadingDeg()  const;
    double getBatteryPct()  const;

    // ── Display ──────────────────────────────────────────────
    void displayRegisters() const;
    void displayRegisterState(const std::string& context) const;

    // ── JSON export ──────────────────────────────────────────
    void exportToJson(
        const std::vector<InstructionData>& instructions,
        const DisplacementData& disp) const;
};

#endif // DRONE_REGISTERS_H