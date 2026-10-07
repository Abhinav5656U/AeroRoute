/*
 *
 * FILE        : AssemblyEngine.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : COA Lab | Course Code: 2310221L
 * TASK        : Task 2 — 64-bit Assembly Logic
 * COURSE OUT  : CO2
 *
 */

#include "AssemblyEngine.h"
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <stdexcept>
using namespace std;

// ── Constructor ───────────────────────────────────────────────
AssemblyEngine::AssemblyEngine(DroneRegisters& registerFile)
    : regs(registerFile) {
    currentDisp = {0.0, 0.0, 0.0, "none"};
    cout << "[AssemblyEngine] Initialised. "
              << "Ready to execute instructions.\n";
}

// ── Private: log one instruction to the trace ─────────────────
// [COA CO2 - Task 2: Instruction trace logging]
void AssemblyEngine::logInstruction(
    const string& addr,
    const string& op,
    const string& args)
{
    instructionLog.push_back({ addr, op, args });
    // Also print to terminal
    cout << "  " << left << setw(14) << addr
              << setw(6) << op
              << args << "\n";
}

// ── Private: convert PC value to hex string ───────────────────
string AssemblyEngine::pcToHex(long long pc) const {
    ostringstream oss;
    oss << "0x" << hex << uppercase
        << setfill('0') << setw(4) << pc;
    return oss.str();
}

// ════════════════════════════════════════════════════════════
// SIM_MOV
// [COA CO2 - Task 2: Simulated MOV instruction]
// Copies src into dest and logs the instruction.
// ════════════════════════════════════════════════════════════
void AssemblyEngine::SIM_MOV(
    long long& dest, long long src,
    const string& destReg,
    const string& srcOperand)
{
    dest = src;
    logInstruction(
        pcToHex(regs.getPC()),
        "MOV",
        destReg + ", " + srcOperand +
        "  ; " + destReg + " <- " + to_string(src)
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
    const string& destReg,
    const string& srcOperand)
{
    long long result = dest + src;
    logInstruction(
        pcToHex(regs.getPC()),
        "ADD",
        destReg + ", " + srcOperand +
        "  ; " + to_string(dest) +
        " + " + to_string(src) +
        " = " + to_string(result)
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
    const string& destReg,
    const string& srcOperand)
{
    long long result = dest - src;
    logInstruction(
        pcToHex(regs.getPC()),
        "SUB",
        destReg + ", " + srcOperand +
        "  ; " + to_string(dest) +
        " - " + to_string(src) +
        " = " + to_string(result)
    );
    regs.advancePC();
    return result;
}

// [COA CO3 - Task 3: arithmetic algorithm using successive addition]
long long AssemblyEngine::SIM_MUL_SUCCESSIVE(
    long long multiplicand, long long multiplier)
{
    if (multiplicand < -1000000LL || multiplicand > 1000000LL ||
        multiplier < -1000000LL || multiplier > 1000000LL) {
        throw out_of_range(
            "Successive-addition multiplication requires operands in [-1000000, 1000000].");
    }
    const bool negative = (multiplicand < 0) != (multiplier < 0);
    const unsigned long long left = multiplicand < 0
        ? 0ULL - static_cast<unsigned long long>(multiplicand)
        : static_cast<unsigned long long>(multiplicand);
    const unsigned long long right = multiplier < 0
        ? 0ULL - static_cast<unsigned long long>(multiplier)
        : static_cast<unsigned long long>(multiplier);
    const unsigned long long additions = min(left, right);
    const long long addend = static_cast<long long>(max(left, right));
    long long result = 0;

    cout << "\n[COA CO3 - Task 3] Multiplication by successive addition:\n";
    for (unsigned long long index = 0; index < additions; ++index) {
        result = SIM_ADD(result, addend, "RAX", to_string(addend));
    }
    if (negative) result = -result;
    cout << "  " << multiplicand << " x " << multiplier
         << " = " << result << " ("
         << additions << " successive additions)\n";
    return result;
}

// [COA CO3 - Task 3: 64-bit REPNE SCASB string-length operation]
size_t AssemblyEngine::asmStringLength(const string& text) {
    size_t length = 0;
#if defined(__GNUC__) && defined(__x86_64__)
    const char* cursor = text.c_str();
    unsigned long long remaining =
        static_cast<unsigned long long>(text.size()) + 1ULL;
    __asm__ volatile (
        "repne scasb"
        : "+D"(cursor), "+c"(remaining)
        : "a"(0)
        : "memory"
    );
    length = static_cast<size_t>(text.size() + 1ULL - remaining - 1ULL);
#else
    length = text.size();
#endif
    logInstruction(pcToHex(regs.getPC()), "SCASB",
                   "REPNE SCASB ; input length = " +
                       to_string(length));
    regs.advancePC();
    return length;
}

// [COA CO3 - Task 3: 64-bit REP MOVSB string-concatenation operation]
void AssemblyEngine::asmStringAppend(
    string& destination, const string& source)
{
    const string aliasCopy = &destination == &source ? source : string();
    const string& inputText = &destination == &source ? aliasCopy : source;
    const size_t oldLength = destination.size();
    const size_t sourceLength = inputText.size();
    if (sourceLength > destination.max_size() - oldLength) {
        throw length_error("Concatenated string exceeds the supported size.");
    }
#if defined(__GNUC__) && defined(__x86_64__)
    destination.resize(oldLength + sourceLength);
    char* output = destination.data() + oldLength;
    const char* input = inputText.data();
    size_t remaining = sourceLength;
    __asm__ volatile (
        "rep movsb"
        : "+D"(output), "+S"(input), "+c"(remaining)
        :
        : "memory"
    );
#else
    destination.append(inputText);
#endif
    logInstruction(pcToHex(regs.getPC()), "MOVSB",
                   "REP MOVSB ; appended " + to_string(sourceLength) +
                       " bytes");
    regs.advancePC();
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
    const string& jumpType,
    const string& op1Name,
    const string& op2Name)
{
    // CMP sets flags based on op1 - op2
    long long result = op1 - op2;
    bool ZF = (result == 0);
    bool SF = (result < 0);
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
        string("target  ; Jump ") +
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

    cout << "\n  [REAL ASM] Inline x86-64 executed:\n";
    cout << "    movq " << coordA << ", %%rax\n";
    cout << "    addq " << coordB << ", %%rax\n";
    cout << "    Result in register: " << result << "\n\n";

    logInstruction(
        pcToHex(regs.getPC()),
        "MOV",
        "RAX, " + to_string(coordA) + "  [REAL ASM]"
    );
    regs.advancePC();
    logInstruction(
        pcToHex(regs.getPC()),
        "ADD",
        "RAX, " + to_string(coordB) +
        " = " + to_string(result) + "  [REAL ASM]"
    );
    regs.advancePC();

#else
    // Fallback for non-GCC compilers
    result = coordA + coordB;
    cout << "  [ASM SIM] ADD: "
              << coordA << " + " << coordB
              << " = " << result << "\n";
    logInstruction(pcToHex(regs.getPC()), "ADD",
        to_string(coordA) + ", " + to_string(coordB));
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

    cout << "\n  [REAL ASM] Inline x86-64 executed:\n";
    cout << "    movq " << coordA << ", %%rax\n";
    cout << "    subq " << coordB << ", %%rax\n";
    cout << "    Result in register: " << result << "\n\n";

    logInstruction(
        pcToHex(regs.getPC()),
        "MOV",
        "RAX, " + to_string(coordA) + "  [REAL ASM]"
    );
    regs.advancePC();
    logInstruction(
        pcToHex(regs.getPC()),
        "SUB",
        "RAX, " + to_string(coordB) +
        " = " + to_string(result) + "  [REAL ASM]"
    );
    regs.advancePC();

#else
    result = coordA - coordB;
    cout << "  [ASM SIM] SUB: "
              << coordA << " - " << coordB
              << " = " << result << "\n";
    logInstruction(pcToHex(regs.getPC()), "SUB",
        to_string(coordA) + ", " + to_string(coordB));
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
    cout << "\n[AssemblyEngine] Computing displacement:\n";
    cout << "  From: (" << fromRow << "," << fromCol << ")\n";
    cout << "  To:   (" << toRow   << "," << toCol   << ")\n";
    cout << "\n  Instruction trace:\n";

    long long R1, R2, R3, R4;

    // Step 1: Load source column coordinate
    SIM_MOV(R1, (long long)fromCol, "R1", to_string(fromCol));

    // Step 2: Load destination column coordinate
    SIM_MOV(R2, (long long)toCol,   "R2", to_string(toCol));

    // Step 3: deltaX = toCol - fromCol  [REAL x86-64 ASM]
    long long deltaX = asmSubCoordinates(R2, R1);

    // Step 4: Load source row coordinate
    SIM_MOV(R3, (long long)fromRow, "R3", to_string(fromRow));

    // Step 5: Load destination row coordinate
    SIM_MOV(R4, (long long)toRow,   "R4", to_string(toRow));

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
        cout << "  [JZ taken] Source = Destination. "
                  << "No movement needed.\n";
    }

    // Euclidean displacement
    double displacement = sqrt((double)sumSq);

    // Determine dominant direction for ASM operation label
    string asmOp;
    if      (abs(deltaX) > abs(deltaY))
        asmOp = "SUB R2, R1 (deltaX dominant)";
    else if (abs(deltaY) > abs(deltaX))
        asmOp = "SUB R4, R3 (deltaY dominant)";
    else
        asmOp = "SUB R2,R1 + SUB R4,R3 (diagonal)";

    currentDisp = {
        (double)deltaX,
        (double)deltaY,
        displacement,
        asmOp
    };

    cout << "\n  Results:\n";
    cout << "    deltaX       = " << deltaX << "\n";
    cout << "    deltaY       = " << deltaY << "\n";
    cout << "    displacement = "
              << fixed << setprecision(4)
              << displacement << " grid units\n\n";

    return currentDisp;
}

// ── Display instruction log ───────────────────────────────────
void AssemblyEngine::displayInstructionLog() const {
    cout << "\n  Instruction Trace Log:\n";
    cout << "  "
              << left << setw(14) << "Address"
              << setw(8) << "Opcode"
              << "Operands\n";
    cout << "  " << string(60, '-') << "\n";
    for (const auto& ins : instructionLog) {
        cout << "  "
                  << left << setw(14) << ins.addr
                  << setw(8) << ins.op
                  << ins.args << "\n";
    }
    cout << "\n";
}

// ── Display displacement results ──────────────────────────────
void AssemblyEngine::displayDisplacement() const {
    cout << "\n  Displacement Computation Result:\n";
    cout << "    deltaX       = " << currentDisp.dx << "\n";
    cout << "    deltaY       = " << currentDisp.dy << "\n";
    cout << "    displacement = "
              << fixed << setprecision(4)
              << currentDisp.total << " grid units\n";
    cout << "    ASM op       = "
              << currentDisp.asm_op << "\n\n";
}

// ── Accessors ─────────────────────────────────────────────────
const vector<InstructionData>&
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