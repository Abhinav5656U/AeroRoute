/*
 *
 * FILE        : Subject2_Main.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : COA Lab | Course Code: 2310221L
 * TASK        : Task 1 (CO1) + Task 2 (CO2) — Full Demo
 * DESCRIPTION : Standalone entry point for Subject 2.
 *               Demonstrates register simulation and
 *               assembly instruction execution.
 *               Writes real register data to registers.json.
 * HOW TO RUN  :
 *   cd src/subject2
 *   g++ Subject2_Main.cpp DroneRegisters.cpp AssemblyEngine.cpp DisplacementAsm.S -o ..\..\build\subject2.exe -std=c++17
 *   ..\..\build\subject2.exe
 *
 */

#include <iostream>
#include <string>
#include <limits>
#include "DroneRegisters.h"
#include "AssemblyEngine.h"
#include "../shared/Constants.h"
using namespace std;

void showMenu();
void runTask1Demo(DroneRegisters& regs);
void runTask2Demo(DroneRegisters& regs, AssemblyEngine& engine);
void runTask3Demo(DroneRegisters& regs, AssemblyEngine& engine);
void runInteractiveMode(DroneRegisters& regs,
                        AssemblyEngine& engine);
void clearInput();

// ── Main ──────────────────────────────────────────────────────
int main() {

    cout << "\n";
    cout << "  ================================================\n";
    cout << "   AeroRoute — Subject 2: COA Lab\n";
    cout << "   Course Code: 2310221L\n";
    cout << "   CO1: CPU Register Simulation\n";
    cout << "   CO2: 64-bit Assembly Logic\n";
    cout << "   MIT Academy of Engineering | Sem III 2026-27\n";
    cout << "  ================================================\n\n";

    DroneRegisters regs;
    AssemblyEngine engine(regs);

    int choice = 0;
    do {
        showMenu();
        if (!(cin >> choice)) {
            if (cin.eof()) {
                cout << "\n  Input closed; exiting Subject 2.\n\n";
                break;
            }
            clearInput();
            cout << "\n  Invalid menu input.\n\n";
            continue;
        }
        clearInput();

        switch(choice) {
            case 1: runTask1Demo(regs);               break;
            case 2: runTask2Demo(regs, engine);        break;
            case 3: runTask3Demo(regs, engine);        break;
            case 4: runInteractiveMode(regs, engine);  break;
            case 5:
                regs.exportToJson(
                    engine.getInstructionLog(),
                    engine.getDisplacement()
                );
                cout << "\n  registers.json updated.\n"
                          << "  Check website dashboard.\n\n";
                break;
            case 0:
                cout << "\n  Exiting Subject 2.\n\n";
                break;
            default:
                cout << "\n  Invalid choice.\n\n";
        }
    } while(choice != 0);

    return 0;
}

void showMenu() {
    cout << "  ------------------------------------------------\n";
    cout << "   SUBJECT 2 — MAIN MENU\n";
    cout << "  ------------------------------------------------\n";
    cout << "   [1] Task 1 Demo — Register Simulation (CO1)\n";
    cout << "       MOV ADD SUB CMP with FLAGS\n";
    cout << "   [2] Task 2 Demo — Assembly Engine (CO2)\n";
    cout << "       Displacement computation + real ASM\n";
    cout << "   [3] Task 3 — Arithmetic and String Processing (CO3)\n";
    cout << "       Successive addition + 64-bit string instructions\n";
    cout << "   [4] Interactive Mode\n";
    cout << "       Enter coordinates, see computation\n";
    cout << "   [5] Export to JSON (update website)\n";
    cout << "   [0] Exit\n";
    cout << "  ------------------------------------------------\n";
    cout << "   Choice: ";
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(),'\n');
}

// [COA CO3 - Task 3: arithmetic algorithm and assembly string processing]
void runTask3Demo(DroneRegisters& regs, AssemblyEngine& engine) {
    cout << "\n  --- TASK 3: Arithmetic and String Processing ---\n";
    cout << "  Enter two integers from -100 to 100 for multiplication: ";
    int multiplicand = 0;
    int multiplier = 0;
    if (!(cin >> multiplicand >> multiplier)) {
        if (cin.eof()) return;
        clearInput();
        cout << "  Invalid numeric input; Task 3 cancelled safely.\n\n";
        return;
    }
    clearInput();
    if (multiplicand < -100 || multiplicand > 100 ||
        multiplier < -100 || multiplier > 100) {
        cout << "  Values must be between -100 and 100; Task 3 cancelled.\n\n";
        return;
    }

    string source;
    string suffix;
    cout << "  String to process: ";
    if (!getline(cin, source)) return;
    cout << "  String to append: ";
    if (!getline(cin, suffix)) return;

    engine.clearLog();
    const long long product =
        engine.SIM_MUL_SUCCESSIVE(multiplicand, multiplier);
    const size_t sourceLength = engine.asmStringLength(source);
    engine.asmStringAppend(source, suffix);

    cout << "  Processed string: \"" << source << "\"\n"
         << "  Original string length: " << sourceLength << "\n"
         << "  Arithmetic result: " << product << "\n";
    engine.displayInstructionLog();
    regs.exportToJson(engine.getInstructionLog(),
                      engine.getDisplacement());
}

// ════════════════════════════════════════════════════════════
// TASK 1 DEMO — CPU Register Simulation
// [COA CO1 - Task 1: All register operations demonstrated]
// ════════════════════════════════════════════════════════════
void runTask1Demo(DroneRegisters& regs) {

    cout << "\n  --- TASK 1: CPU Register Simulation ---\n\n";

    // Load real drone telemetry into registers
    cout << "  Loading drone telemetry into registers:\n";
    cout << "  Altitude=120m Speed=18.5m/s "
              << "Heading=45deg Battery=87%\n\n";

    regs.loadTelemetry(120, 18.5, 45.0, 87.0);
    regs.displayRegisters();

    // Demonstrate ADD — altitude increases during climb
    cout << "  Drone climbing +15m (ADD RAX, 15):\n";
    long long newAlt = regs.ADD(regs.getRAX(), 15, "RAX");
    regs.MOV_RAX(newAlt);
    regs.displayRegisterState("after climb ADD");

    // Demonstrate SUB — battery drains
    cout << "  Battery drain -500 (SUB RDX, 500):\n";
    long long newBat = regs.SUB(regs.getRDX(), 500, "RDX");
    regs.MOV_RDX(newBat);
    regs.displayRegisterState("after battery SUB");

    // Demonstrate CMP — check if battery is critical
    cout << "  Checking battery level (CMP RDX, 1000):\n";
    cout << "  (Battery < 10% = 1000 in register units)\n";
    regs.CMP(regs.getRDX(), 1000LL, "RDX", "1000");

    // Demonstrate MOV reg to reg
    cout << "\n  Copying altitude to RBX (MOV RBX, RAX):\n";
    long long tempRBX = regs.getRBX();
    regs.MOV_REG_TO_REG(tempRBX, regs.getRAX(), "RBX", "RAX");
    regs.MOV_RBX(tempRBX);

    regs.displayRegisters();
    cout << "  Task 1 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// TASK 2 DEMO — 64-bit Assembly Engine
// [COA CO2 - Task 2: Displacement + real ASM operations]
// ════════════════════════════════════════════════════════════
void runTask2Demo(DroneRegisters& regs, AssemblyEngine& engine) {

    cout << "\n  --- TASK 2: 64-bit Assembly Logic ---\n\n";
    engine.clearLog();

    // Example 1: short hop delivery
    cout << "  === Mission 1: Short delivery hop ===\n";
    DisplacementData d1 = engine.computeDisplacement(0,0, 3,4);
    regs.exportToJson(engine.getInstructionLog(), d1);

    // Example 2: diagonal cross-city delivery
    cout << "  === Mission 2: Cross-city diagonal ===\n";
    engine.clearLog();
    DisplacementData d2 = engine.computeDisplacement(0,0, 9,9);
    engine.displayInstructionLog();
    regs.exportToJson(engine.getInstructionLog(), d2);

    // Example 3: same position (JZ taken)
    cout << "  === Mission 3: Same position (JZ test) ===\n";
    engine.clearLog();
    DisplacementData d3 = engine.computeDisplacement(5,5, 5,5);
    regs.exportToJson(engine.getInstructionLog(), d3);

    cout << "  Task 2 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// INTERACTIVE MODE — user enters coordinates
// ════════════════════════════════════════════════════════════
void runInteractiveMode(DroneRegisters& regs,
                        AssemblyEngine& engine) {

    cout << "\n  --- Interactive Mode ---\n\n";

    int subChoice = 0;
    do {
        cout << "  [1] Load custom telemetry into registers\n";
        cout << "  [2] Compute displacement between two points\n";
        cout << "  [3] View register state\n";
        cout << "  [4] View instruction log\n";
        cout << "  [5] Export to JSON\n";
        cout << "  [0] Back\n";
        cout << "  Choice: ";
        cin >> subChoice;
        clearInput();

        if (subChoice == 1) {
            int    alt;
            double spd, hdg, bat;
            cout << "  Altitude (m):      "; cin >> alt;
            cout << "  Speed (m/s):       "; cin >> spd;
            cout << "  Heading (degrees): "; cin >> hdg;
            cout << "  Battery (%):       "; cin >> bat;
            regs.loadTelemetry(alt, spd, hdg, bat);
            regs.displayRegisters();
            regs.exportToJson(engine.getInstructionLog(),
                              engine.getDisplacement());
        }
        else if (subChoice == 2) {
            int fr, fc, tr, tc;
            cout << "  From row (0-9): "; cin >> fr;
            cout << "  From col (0-9): "; cin >> fc;
            cout << "  To   row (0-9): "; cin >> tr;
            cout << "  To   col (0-9): "; cin >> tc;
            engine.clearLog();
            DisplacementData d = engine.computeDisplacement(
                fr, fc, tr, tc);
            engine.displayInstructionLog();
            regs.exportToJson(engine.getInstructionLog(), d);
            cout << "  Exported to registers.json\n";
        }
        else if (subChoice == 3) {
            regs.displayRegisters();
        }
        else if (subChoice == 4) {
            engine.displayInstructionLog();
        }
        else if (subChoice == 5) {
            regs.exportToJson(engine.getInstructionLog(),
                              engine.getDisplacement());
            cout << "  registers.json updated.\n";
        }

    } while(subChoice != 0);
}