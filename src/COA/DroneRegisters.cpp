/*
 *
 * FILE        : DroneRegisters.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : COA Lab | Course Code: 2310221L
 * TASK        : Task 1 — CPU Register Simulation
 * COURSE OUT  : CO1
 *
 */

#include "DroneRegisters.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <bitset>
using namespace std;

// ── FLAGS display ─────────────────────────────────────────────
void FLAGS_Register::display() const {
    cout << "  FLAGS: ";
    cout << "ZF=" << ZF << " ";
    cout << "CF=" << CF << " ";
    cout << "SF=" << SF << " ";
    cout << "OF=" << OF << " ";
    cout << "PF=" << PF << "\n";
}

// ════════════════════════════════════════════════════════════
// Constructor — Power-on register state
// [COA CO1 - Task 1: Register Initialisation]
// All general purpose registers start at 0.
// PC starts at PC_START (0x1000).
// SP starts at top of user-space stack.
// ════════════════════════════════════════════════════════════
DroneRegisters::DroneRegisters()
    : RAX(0), RBX(0), RCX(0), RDX(0),
      PC(PC_START), IR(0),
      SP(0x00007FFFFFFFFFFF),
      instructionCount(0) {
    FLAGS.clear();
    cout << "[Registers] Power-on state initialised.\n";
    cout << "[Registers] PC = 0x"
              << hex << PC << dec << "\n";
}

// ── Private: update FLAGS after arithmetic result ─────────────
// [COA CO1 - Task 1: FLAGS update logic]
void DroneRegisters::updateFlags(long long result) {
    FLAGS.ZF = (result == 0);
    FLAGS.SF = (result < 0);
    FLAGS.PF = checkParity(result);
    // CF and OF require context (set by caller if needed)
}

// ── Private: parity check (even number of 1-bits) ────────────
bool DroneRegisters::checkParity(long long value) {
    // Count 1-bits in lowest 8 bits
    unsigned char low8 = (unsigned char)(value & 0xFF);
    int ones = 0;
    while (low8) { ones += (low8 & 1); low8 >>= 1; }
    return (ones % 2 == 0); // PF=1 if even
}

// ════════════════════════════════════════════════════════════
// MOV Operations
// [COA CO1 - Task 1: Register Transfer — MOV]
// Loads an immediate 64-bit value into a register.
// Advances PC by 4 bytes (one instruction word).
// ════════════════════════════════════════════════════════════

void DroneRegisters::MOV_RAX(long long value) {
    RAX = value;
    IR  = 0x48C7C0; // encoding for MOV RAX, imm64
    advancePC();
    cout << "[REG] MOV RAX, " << value
              << "  | RAX = " << RAX << "\n";
}

void DroneRegisters::MOV_RBX(long long value) {
    RBX = value;
    IR  = 0x48C7C3; // MOV RBX, imm64
    advancePC();
    cout << "[REG] MOV RBX, " << value
              << "  | RBX = " << RBX << "\n";
}

void DroneRegisters::MOV_RCX(long long value) {
    RCX = value;
    IR  = 0x48C7C1; // MOV RCX, imm64
    advancePC();
    cout << "[REG] MOV RCX, " << value
              << "  | RCX = " << RCX << "\n";
}

void DroneRegisters::MOV_RDX(long long value) {
    RDX = value;
    IR  = 0x48C7C2; // MOV RDX, imm64
    advancePC();
    cout << "[REG] MOV RDX, " << value
              << "  | RDX = " << RDX << "\n";
}

// [COA CO1 - Task 1: Register to Register Transfer]
void DroneRegisters::MOV_REG_TO_REG(
    long long& dest, long long src,
    const string& destName,
    const string& srcName)
{
    dest = src;
    IR   = 0x4889C0; // MOV reg, reg (generalised)
    advancePC();
    cout << "[REG] MOV " << destName
              << ", " << srcName
              << "  | " << destName << " = " << dest << "\n";
}

// ════════════════════════════════════════════════════════════
// ADD Operation
// [COA CO1 - Task 1: ADD — updates FLAGS]
// dest = dest + src
// Returns new value (caller must store it back)
// ════════════════════════════════════════════════════════════
long long DroneRegisters::ADD(long long dest, long long src,
                               const string& destName) {
    long long result = dest + src;

    // Carry flag: unsigned overflow (result < either operand)
    FLAGS.CF = ((unsigned long long)result <
                (unsigned long long)dest);

    // Overflow flag: signed overflow
    FLAGS.OF = ((dest > 0 && src > 0 && result < 0) ||
                (dest < 0 && src < 0 && result > 0));

    updateFlags(result);
    IR = 0x4801C0; // ADD reg, reg
    advancePC();

    cout << "[REG] ADD " << destName
              << ", " << src
              << "  | " << destName << " = "
              << dest << " + " << src << " = " << result << "\n";
    FLAGS.display();
    return result;
}

// ════════════════════════════════════════════════════════════
// SUB Operation
// [COA CO1 - Task 1: SUB — updates FLAGS]
// dest = dest - src
// ════════════════════════════════════════════════════════════
long long DroneRegisters::SUB(long long dest, long long src,
                               const string& destName) {
    long long result = dest - src;

    // Carry flag: borrow occurred (dest < src in unsigned)
    FLAGS.CF = ((unsigned long long)dest <
                (unsigned long long)src);

    // Overflow flag
    FLAGS.OF = ((dest > 0 && src < 0 && result < 0) ||
                (dest < 0 && src > 0 && result > 0));

    updateFlags(result);
    IR = 0x4829C0; // SUB reg, reg
    advancePC();

    cout << "[REG] SUB " << destName
              << ", " << src
              << "  | " << destName << " = "
              << dest << " - " << src << " = " << result << "\n";
    FLAGS.display();
    return result;
}

// ════════════════════════════════════════════════════════════
// CMP Operation
// [COA CO1 - Task 1: CMP — sets FLAGS without storing result]
// Performs SUB internally but discards the result.
// Only FLAGS are affected.
// ════════════════════════════════════════════════════════════
void DroneRegisters::CMP(long long op1, long long op2,
                          const string& op1Name,
                          const string& op2Name) {
    long long result = op1 - op2;

    FLAGS.CF = ((unsigned long long)op1 <
                (unsigned long long)op2);
    FLAGS.OF = ((op1 > 0 && op2 < 0 && result < 0) ||
                (op1 < 0 && op2 > 0 && result > 0));

    updateFlags(result);
    IR = 0x483BC0; // CMP reg, reg
    advancePC();

    cout << "[REG] CMP " << op1Name
              << ", " << op2Name
              << "  | " << op1 << " vs " << op2;

    if (FLAGS.ZF) cout << "  -> EQUAL";
    else if (FLAGS.CF) cout << "  -> LESS THAN";
    else cout << "  -> GREATER THAN";
    cout << "\n";
    FLAGS.display();
}

// ── Advance PC ────────────────────────────────────────────────
// [COA CO1 - Task 1: Program Counter advancement]
// Each instruction is 4 bytes (simplified model)
void DroneRegisters::advancePC() {
    PC += 4;
    instructionCount++;
}

// ════════════════════════════════════════════════════════════
// loadTelemetry
// [COA CO1 - Task 1: Load real drone data into registers]
// Converts human-readable units to integer register values
// ════════════════════════════════════════════════════════════
void DroneRegisters::loadTelemetry(
    int    altitudeM,
    double speedMs,
    double headingDeg,
    double batteryPct)
{
    cout << "\n[Registers] Loading telemetry into registers:\n";

    // RAX = altitude in metres (direct integer)
    MOV_RAX((long long)altitudeM);

    // RBX = speed in cm/s (multiply by 100 to keep integer)
    MOV_RBX((long long)(speedMs * 100));

    // RCX = heading in degrees * 100
    MOV_RCX((long long)(headingDeg * 100));

    // RDX = battery percentage * 100
    MOV_RDX((long long)(batteryPct * 100));

    cout << "[Registers] Telemetry loaded.\n\n";
}

// ── Accessors ─────────────────────────────────────────────────
long long      DroneRegisters::getRAX()   const { return RAX; }
long long      DroneRegisters::getRBX()   const { return RBX; }
long long      DroneRegisters::getRCX()   const { return RCX; }
long long      DroneRegisters::getRDX()   const { return RDX; }
long long      DroneRegisters::getPC()    const { return PC;  }
long long      DroneRegisters::getIR()    const { return IR;  }
long long      DroneRegisters::getSP()    const { return SP;  }
FLAGS_Register DroneRegisters::getFLAGS() const { return FLAGS; }

int    DroneRegisters::getAltitudeM()  const { return (int)RAX; }
double DroneRegisters::getSpeedMs()    const { return RBX / 100.0; }
double DroneRegisters::getHeadingDeg() const { return RCX / 100.0; }
double DroneRegisters::getBatteryPct() const { return RDX / 100.0; }

// ════════════════════════════════════════════════════════════
// displayRegisters
// [COA CO1 - Task 1: Register State Display]
// ════════════════════════════════════════════════════════════
void DroneRegisters::displayRegisters() const {
    cout << "\n";
    cout << "  ============================================\n";
    cout << "   x86-64 Register State — AeroRoute Drone\n";
    cout << "  ============================================\n";

    cout << hex << uppercase;

    auto showReg = [](const string& name, long long val,
                      const string& meaning) {
        cout << "  " << left << setw(6) << name
                  << "0x" << setfill('0')
                  << setw(16) << (unsigned long long)val
                  << setfill(' ')
                  << "  (" << dec << val << ")"
                  << "  [" << meaning << "]\n"
                  << hex;
    };

    showReg("RAX", RAX, "Altitude " + to_string((int)RAX) + " m");
    showReg("RBX", RBX, "Speed " + to_string(RBX/100) + "." +
            to_string(RBX%100) + " m/s");
    showReg("RCX", RCX, "Heading " + to_string((int)(RCX/100)) + " deg");
    showReg("RDX", RDX, "Battery " + to_string((int)(RDX/100)) + "%");
    showReg("PC",  PC,  "Program Counter");
    showReg("IR",  IR,  "Instruction Register");
    showReg("SP",  SP,  "Stack Pointer");

    cout << dec;
    FLAGS.display();
    cout << "  ============================================\n\n";
}

void DroneRegisters::displayRegisterState(
    const string& context) const {
    cout << "\n  --- Register state after: "
              << context << " ---\n";
    displayRegisters();
}

// ════════════════════════════════════════════════════════════
// exportToJson
// Writes register state to website/data/registers.json
// ════════════════════════════════════════════════════════════
void DroneRegisters::exportToJson(
    const vector<InstructionData>& instructions,
    const DisplacementData& disp) const
{
    FLAGS_Register f = FLAGS;
    writeRegistersJson(
        RAX, RBX, RCX, RDX, PC, IR, SP,
        f.ZF, f.CF, f.SF, f.OF, f.PF,
        instructions, disp
    );
    cout << "[Registers] Exported to registers.json\n";
}