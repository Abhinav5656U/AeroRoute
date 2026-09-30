/*
 * ============================================================
 * FILE        : AssemblyEngine.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : COA Lab | Course Code: 2310221L
 * TASK        : Task 2 — 64-bit Assembly Logic
 * COURSE OUT  : CO2
 * ============================================================
 */

#include "AssemblyEngine.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

// ── Constructor ───────────────────────────────────────────────
AssemblyEngine::AssemblyEngine(DroneRegisters& registerFile)
    : regs(registerFile) {
    currentDisp = {0.0, 0.0, 0.0, "none"};
    std::cout << "[AssemblyEngine] Initialised. "
              << "Ready to execute instructions.\n";
}

// ── Private: log one instruction to the trace ─────────────────
// [COA CO2 - Task 2: Instruction trace logging]
void AssemblyEngine::logInstruction(
    const std::string& addr,
    const std::string& op,
    const std::string& args)
{
    instructionLog.push_back({ addr, op, args });
    // Also print to terminal
    std::cout << "  " << std::left << std::setw(14) << addr
              << std::setw(6) << op
              << args << "\n";
}

// ── Private: convert PC value to hex string ───────────────────
std::string AssemblyEngine::pcToHex(long long pc) const {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase
        << std::setfill('0') << std::setw(4) << pc;
    return oss.str();
}

// ════════════════════════════════════════════════════════════
// SIM_MOV
// [COA CO2 - Task 2: Simulated MOV instruction]
// Copies src into dest and logs the instruction.
// ════════════════════════════════════════════════════════════
void AssemblyEngine::SIM_MOV(
    long long& dest, long long src,
    const std::string& destReg,
    const std::string& srcOperand)
{
    dest = src;
    logInstruction(
        pcToHex(regs.getPC()),
        "MOV",
        destReg + ", " + srcOperand +
        "  ; " + destReg + " <- " + std::to_string(src)
    );
    regs.advancePC();
}

// ════════════════════════════════════════════════════════════
// SIM_ADD
// [COA CO2 - Task 2: Simulated ADD instruction]
// dest = dest + src, updates register flags
// ════════════════════════════════════════════════════════════
long long AssemblyEngine::SIM_ADD(
    long long dest, long long src,
    const std::string& destReg,
    const std::string& srcOperand)
{
    long long result = dest + src;
    logInstruction(
        pcToHex(regs.getPC()),
        "ADD",
        destReg + ", " + srcOperand +
        "  ; " + std::to_string(dest) +
        " + " + std::to_string(src) +
        " = " + std::to_string(result)
    );
    regs.advancePC();
    return result;
}

// ════════════════════════════════════════════════════════════
// SIM_SUB
// [COA CO2 - Task 2: Simulated SUB instruction]
// dest = dest - src
// ════════════════════════════════════════════════════════════
long long AssemblyEngine::SIM_SUB(
    long long dest, long long src,
    const std::string& destReg,
    const std::string& srcOperand)
{
    long long result = dest - src;
    logInstruction(
        pcToHex(regs.getPC()),
        "SUB",
        destReg + ", " + srcOperand +
        "  ; " + std::to_string(dest) +
        " - " + std::to_string(src) +
        " = " + std::to_string(result)
    );
    regs.advancePC();
    return result;
}

// ════════════════════════════════════════════════════════════
// SIM_CMP_JMP
// [COA CO2 - Task 2: CMP + conditional JMP simulation]
//
// jumpType options:
//   "JE"  — jump if equal         (ZF=1)
//   "JNE" — jump if not equal     (ZF=0)
//   "JL"  — jump if less than     (SF!=OF)
//   "JG"  — jump if greater than  (ZF=0 and SF=OF)
//   "JZ"  — jump if zero          (ZF=1)
//
// Returns true if jump should be taken (caller decides branch)
// ════════════════════════════════════════════════════════════
bool AssemblyEngine::SIM_CMP_JMP(
    long long op1, long long op2,
    const std::string& jumpType,
    const std::string& op1Name,
    const std::string& op2Name)
{
    // CMP sets flags based on op1 - op2
    long long result = op1 - op2;
    bool ZF = (result == 0);
    bool SF = (result < 0);
    bool CF = ((unsigned long long)op1 < (unsigned long long)op2);
    bool OF = ((op1 > 0 && op2 < 0 && result < 0) ||
               (op1 < 0 && op2 > 0 && result > 0));

    logInstruction(
        pcToHex(regs.getPC()),
        "CMP",
        op1Name + ", " + op2Name
    );
    regs.advancePC();

    // Determine if jump is taken
    bool taken = false;
    if      (jumpType == "JE"  || jumpType == "JZ")  taken = ZF;
    else if (jumpType == "JNE")                       taken = !ZF;
    else if (jumpType == "JL")                        taken = (SF != OF);
    else if (jumpType == "JG")                        taken = (!ZF && SF == OF);
    else if (jumpType == "JLE")                       taken = (ZF || SF != OF);
    else if (jumpType == "JGE")                       taken = (SF == OF);

    logInstruction(
        pcToHex(regs.getPC()),
        jumpType,
        std::string("target  ; Jump ") +
        (taken ? "TAKEN" : "NOT TAKEN")
    );
    regs.advancePC();

    return taken;
}

// ════════════════════════════════════════════════════════════
// REAL INLINE x86-64 ASSEMBLY — Operation 1
// [COA CO2 - Task 2: Actual ADD instruction in assembly]
//
// This uses real x86-64 assembly via GCC inline ASM.
// AT&T syntax: source on left, destination on right.
// "r" constraint: compiler picks any 64-bit register.
// ════════════════════════════════════════════════════════════
long long AssemblyEngine::asmAddCoordinates(
    long long coordA, long long coordB)
{
    long long result = 0;

#ifdef __GNUC__
    // Real x86-64 inline assembly
    // Computes: result = coordA + coordB
    // Using actual ADD machine instruction
    __asm__ volatile (
        "movq %1, %0\n\t"   // MOV coordA into result register
        "addq %2, %0\n\t"   // ADD coordB to result register
        : "=r" (result)     // output: result in any register
        : "r"  (coordA),    // input 1: coordA
          "r"  (coordB)     // input 2: coordB
        :                   // no clobbered registers
    );

    std::cout << "\n  [REAL ASM] Inline x86-64 executed:\n";
    std::cout << "    movq " << coordA << ", %%rax\n";
    std::cout << "    addq " << coordB << ", %%rax\n";
    std::cout << "    Result in register: " << result << "\n\n";

    logInstruction(
        pcToHex(regs.getPC()),
        "MOV",
        "RAX, " + std::to_string(coordA) + "  [REAL ASM]"
    );
    regs.advancePC();
    logInstruction(
        pcToHex(regs.getPC()),
        "ADD",
        "RAX, " + std::to_string(coordB) +
        " = " + std::to_string(result) + "  [REAL ASM]"
    );
    regs.advancePC();

#else
    // Fallback for non-GCC compilers
    result = coordA + coordB;
    std::cout << "  [ASM SIM] ADD: "
              << coordA << " + " << coordB
              << " = " << result << "\n";
    logInstruction(pcToHex(regs.getPC()), "ADD",
        std::to_string(coordA) + ", " + std::to_string(coordB));
    regs.advancePC();
#endif

    return result;
}

// ════════════════════════════════════════════════════════════
// REAL INLINE x86-64 ASSEMBLY — Operation 2
// [COA CO2 - Task 2: Actual SUB instruction in assembly]
// Computes coordinate delta using real SUB machine instruction
// ════════════════════════════════════════════════════════════
long long AssemblyEngine::asmSubCoordinates(
    long long coordA, long long coordB)
{
    long long result = 0;

#ifdef __GNUC__
    // Real x86-64 inline assembly
    // Computes: result = coordA - coordB
    __asm__ volatile (
        "movq %1, %0\n\t"   // MOV coordA into result
        "subq %2, %0\n\t"   // SUB coordB from result
        : "=r" (result)
        : "r"  (coordA),
          "r"  (coordB)
        :
    );

    std::cout << "\n  [REAL ASM] Inline x86-64 executed:\n";
    std::cout << "    movq " << coordA << ", %%rax\n";
    std::cout << "    subq " << coordB << ", %%rax\n";
    std::cout << "    Result in register: " << result << "\n\n";

    logInstruction(
        pcToHex(regs.getPC()),
        "MOV",
        "RAX, " + std::to_string(coordA) + "  [REAL ASM]"
    );
    regs.advancePC();
    logInstruction(
        pcToHex(regs.getPC()),
        "SUB",
        "RAX, " + std::to_string(coordB) +
        " = " + std::to_string(result) + "  [REAL ASM]"
    );
    regs.advancePC();

#else
    result = coordA - coordB;
    std::cout << "  [ASM SIM] SUB: "
              << coordA << " - " << coordB
              << " = " << result << "\n";
    logInstruction(pcToHex(regs.getPC()), "SUB",
        std::to_string(coordA) + ", " + std::to_string(coordB));
    regs.advancePC();
#endif

    return result;
}

// ════════════════════════════════════════════════════════════
// computeDisplacement
// [COA CO2 - Task 2: Main displacement computation]
//
// Given two grid positions, computes:
//   deltaX = toCol - fromCol    (using real ASM SUB)
//   deltaY = toRow - fromRow    (using real ASM SUB)
//   displacement = sqrt(dX^2 + dY^2)  (Euclidean distance)
//
// Assembly instruction sequence used:
//   MOV R1, fromCol
//   MOV R2, toCol
//   SUB R2, R1       → deltaX  [REAL ASM]
//   MOV R3, fromRow
//   MOV R4, toRow
//   SUB R4, R3       → deltaY  [REAL ASM]
//   IMUL R2, R2      → deltaX^2 (simulated)
//   IMUL R4, R4      → deltaY^2 (simulated)
//   ADD R2, R4       → sum of squares (simulated)
//   (sqrt computed in C, no x86 scalar sqrt in basic model)
// ════════════════════════════════════════════════════════════
DisplacementData AssemblyEngine::computeDisplacement(
    int fromRow, int fromCol,
    int toRow,   int toCol)
{
    std::cout << "\n[AssemblyEngine] Computing displacement:\n";
    std::cout << "  From: (" << fromRow << "," << fromCol << ")\n";
    std::cout << "  To:   (" << toRow   << "," << toCol   << ")\n";
    std::cout << "\n  Instruction trace:\n";

    long long R1, R2, R3, R4;

    // Step 1: Load source column coordinate
    SIM_MOV(R1, (long long)fromCol, "R1", std::to_string(fromCol));

    // Step 2: Load destination column coordinate
    SIM_MOV(R2, (long long)toCol,   "R2", std::to_string(toCol));

    // Step 3: deltaX = toCol - fromCol  [REAL x86-64 ASM]
    long long deltaX = asmSubCoordinates(R2, R1);

    // Step 4: Load source row coordinate
    SIM_MOV(R3, (long long)fromRow, "R3", std::to_string(fromRow));

    // Step 5: Load destination row coordinate
    SIM_MOV(R4, (long long)toRow,   "R4", std::to_string(toRow));

    // Step 6: deltaY = toRow - fromRow  [REAL x86-64 ASM]
    long long deltaY = asmSubCoordinates(R4, R3);

    // Step 7: Square deltaX (simulated IMUL)
    long long dX2 = SIM_ADD(deltaX * deltaX, 0LL,
                             "R2", "R2*R2  ; IMUL (squared)");

    // Step 8: Square deltaY (simulated IMUL)
    long long dY2 = SIM_ADD(deltaY * deltaY, 0LL,
                             "R4", "R4*R4  ; IMUL (squared)");

    // Step 9: Sum of squares
    long long sumSq = SIM_ADD(dX2, dY2, "R2", "R4");

    // Step 10: CMP — check if displacement is zero
    bool isZero = SIM_CMP_JMP(sumSq, 0LL, "JZ", "R2", "0");
    if (isZero) {
        std::cout << "  [JZ taken] Source = Destination. "
                  << "No movement needed.\n";
    }

    // Euclidean displacement
    double displacement = std::sqrt((double)sumSq);

    // Determine dominant direction for ASM operation label
    std::string asmOp;
    if      (std::abs(deltaX) > std::abs(deltaY))
        asmOp = "SUB R2, R1 (deltaX dominant)";
    else if (std::abs(deltaY) > std::abs(deltaX))
        asmOp = "SUB R4, R3 (deltaY dominant)";
    else
        asmOp = "SUB R2,R1 + SUB R4,R3 (diagonal)";

    currentDisp = {
        (double)deltaX,
        (double)deltaY,
        displacement,
        asmOp
    };

    std::cout << "\n  Results:\n";
    std::cout << "    deltaX       = " << deltaX << "\n";
    std::cout << "    deltaY       = " << deltaY << "\n";
    std::cout << "    displacement = "
              << std::fixed << std::setprecision(4)
              << displacement << " grid units\n\n";

    return currentDisp;
}

// ── Display instruction log ───────────────────────────────────
void AssemblyEngine::displayInstructionLog() const {
    std::cout << "\n  Instruction Trace Log:\n";
    std::cout << "  "
              << std::left << std::setw(14) << "Address"
              << std::setw(8) << "Opcode"
              << "Operands\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    for (const auto& ins : instructionLog) {
        std::cout << "  "
                  << std::left << std::setw(14) << ins.addr
                  << std::setw(8) << ins.op
                  << ins.args << "\n";
    }
    std::cout << "\n";
}

// ── Display displacement results ──────────────────────────────
void AssemblyEngine::displayDisplacement() const {
    std::cout << "\n  Displacement Computation Result:\n";
    std::cout << "    deltaX       = " << currentDisp.dx << "\n";
    std::cout << "    deltaY       = " << currentDisp.dy << "\n";
    std::cout << "    displacement = "
              << std::fixed << std::setprecision(4)
              << currentDisp.total << " grid units\n";
    std::cout << "    ASM op       = "
              << currentDisp.asm_op << "\n\n";
}

// ── Accessors ─────────────────────────────────────────────────
const std::vector<InstructionData>&
AssemblyEngine::getInstructionLog() const {
    return instructionLog;
}

const DisplacementData&
AssemblyEngine::getDisplacement() const {
    return currentDisp;
}

void AssemblyEngine::clearLog() {
    instructionLog.clear();
    currentDisp = {0.0, 0.0, 0.0, "none"};
}