/*
 * ============================================================
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
 *   g++ Subject2_Main.cpp DroneRegisters.cpp AssemblyEngine.cpp -o ..\..\build\subject2.exe -std=c++17
 *   ..\..\build\subject2.exe
 * ============================================================
 */

#include <iostream>
#include <string>
#include <limits>
#include "DroneRegisters.h"
#include "AssemblyEngine.h"
#include "../shared/Constants.h"

void showMenu();
void runTask1Demo(DroneRegisters& regs);
void runTask2Demo(DroneRegisters& regs, AssemblyEngine& engine);
void runInteractiveMode(DroneRegisters& regs,
                        AssemblyEngine& engine);
void clearInput();

// ── Main ──────────────────────────────────────────────────────
int main() {

    std::cout << "\n";
    std::cout << "  ================================================\n";
    std::cout << "   AeroRoute — Subject 2: COA Lab\n";
    std::cout << "   Course Code: 2310221L\n";
    std::cout << "   CO1: CPU Register Simulation\n";
    std::cout << "   CO2: 64-bit Assembly Logic\n";
    std::cout << "   MIT Academy of Engineering | Sem III 2026-27\n";
    std::cout << "  ================================================\n\n";

    DroneRegisters regs;
    AssemblyEngine engine(regs);

    int choice = 0;
    do {
        showMenu();
        std::cin >> choice;
        clearInput();

        switch(choice) {
            case 1: runTask1Demo(regs);               break;
            case 2: runTask2Demo(regs, engine);        break;
            case 3: runInteractiveMode(regs, engine);  break;
            case 4:
                regs.exportToJson(
                    engine.getInstructionLog(),
                    engine.getDisplacement()
                );
                std::cout << "\n  registers.json updated.\n"
                          << "  Check website dashboard.\n\n";
                break;
            case 0:
                std::cout << "\n  Exiting Subject 2.\n\n";
                break;
            default:
                std::cout << "\n  Invalid choice.\n\n";
        }
    } while(choice != 0);

    return 0;
}

void showMenu() {
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   SUBJECT 2 — MAIN MENU\n";
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   [1] Task 1 Demo — Register Simulation (CO1)\n";
    std::cout << "       MOV ADD SUB CMP with FLAGS\n";
    std::cout << "   [2] Task 2 Demo — Assembly Engine (CO2)\n";
    std::cout << "       Displacement computation + real ASM\n";
    std::cout << "   [3] Interactive Mode\n";
    std::cout << "       Enter coordinates, see computation\n";
    std::cout << "   [4] Export to JSON (update website)\n";
    std::cout << "   [0] Exit\n";
    std::cout << "  ------------------------------------------------\n";
    std::cout << "   Choice: ";
}

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
}

// ════════════════════════════════════════════════════════════
// TASK 1 DEMO — CPU Register Simulation
// [COA CO1 - Task 1: All register operations demonstrated]
// ════════════════════════════════════════════════════════════
void runTask1Demo(DroneRegisters& regs) {

    std::cout << "\n  --- TASK 1: CPU Register Simulation ---\n\n";

    // Load real drone telemetry into registers
    std::cout << "  Loading drone telemetry into registers:\n";
    std::cout << "  Altitude=120m Speed=18.5m/s "
              << "Heading=45deg Battery=87%\n\n";

    regs.loadTelemetry(120, 18.5, 45.0, 87.0);
    regs.displayRegisters();

    // Demonstrate ADD — altitude increases during climb
    std::cout << "  Drone climbing +15m (ADD RAX, 15):\n";
    long long newAlt = regs.ADD(regs.getRAX(), 15, "RAX");
    regs.MOV_RAX(newAlt);
    regs.displayRegisterState("after climb ADD");

    // Demonstrate SUB — battery drains
    std::cout << "  Battery drain -500 (SUB RDX, 500):\n";
    long long newBat = regs.SUB(regs.getRDX(), 500, "RDX");
    regs.MOV_RDX(newBat);
    regs.displayRegisterState("after battery SUB");

    // Demonstrate CMP — check if battery is critical
    std::cout << "  Checking battery level (CMP RDX, 1000):\n";
    std::cout << "  (Battery < 10% = 1000 in register units)\n";
    regs.CMP(regs.getRDX(), 1000LL, "RDX", "1000");

    // Demonstrate MOV reg to reg
    std::cout << "\n  Copying altitude to RBX (MOV RBX, RAX):\n";
    long long tempRBX = regs.getRBX();
    regs.MOV_REG_TO_REG(tempRBX, regs.getRAX(), "RBX", "RAX");
    regs.MOV_RBX(tempRBX);

    regs.displayRegisters();
    std::cout << "  Task 1 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// TASK 2 DEMO — 64-bit Assembly Engine
// [COA CO2 - Task 2: Displacement + real ASM operations]
// ════════════════════════════════════════════════════════════
void runTask2Demo(DroneRegisters& regs, AssemblyEngine& engine) {

    std::cout << "\n  --- TASK 2: 64-bit Assembly Logic ---\n\n";
    engine.clearLog();

    // Example 1: short hop delivery
    std::cout << "  === Mission 1: Short delivery hop ===\n";
    DisplacementData d1 = engine.computeDisplacement(0,0, 3,4);
    regs.exportToJson(engine.getInstructionLog(), d1);

    // Example 2: diagonal cross-city delivery
    std::cout << "  === Mission 2: Cross-city diagonal ===\n";
    engine.clearLog();
    DisplacementData d2 = engine.computeDisplacement(0,0, 9,9);
    engine.displayInstructionLog();
    regs.exportToJson(engine.getInstructionLog(), d2);

    // Example 3: same position (JZ taken)
    std::cout << "  === Mission 3: Same position (JZ test) ===\n";
    engine.clearLog();
    DisplacementData d3 = engine.computeDisplacement(5,5, 5,5);
    regs.exportToJson(engine.getInstructionLog(), d3);

    std::cout << "  Task 2 demo complete.\n\n";
}

// ════════════════════════════════════════════════════════════
// INTERACTIVE MODE — user enters coordinates
// ════════════════════════════════════════════════════════════
void runInteractiveMode(DroneRegisters& regs,
                        AssemblyEngine& engine) {

    std::cout << "\n  --- Interactive Mode ---\n\n";

    int subChoice = 0;
    do {
        std::cout << "  [1] Load custom telemetry into registers\n";
        std::cout << "  [2] Compute displacement between two points\n";
        std::cout << "  [3] View register state\n";
        std::cout << "  [4] View instruction log\n";
        std::cout << "  [5] Export to JSON\n";
        std::cout << "  [0] Back\n";
        std::cout << "  Choice: ";
        std::cin >> subChoice;
        clearInput();

        if (subChoice == 1) {
            int    alt;
            double spd, hdg, bat;
            std::cout << "  Altitude (m):      "; std::cin >> alt;
            std::cout << "  Speed (m/s):       "; std::cin >> spd;
            std::cout << "  Heading (degrees): "; std::cin >> hdg;
            std::cout << "  Battery (%):       "; std::cin >> bat;
            regs.loadTelemetry(alt, spd, hdg, bat);
            regs.displayRegisters();
            regs.exportToJson(engine.getInstructionLog(),
                              engine.getDisplacement());
        }
        else if (subChoice == 2) {
            int fr, fc, tr, tc;
            std::cout << "  From row (0-9): "; std::cin >> fr;
            std::cout << "  From col (0-9): "; std::cin >> fc;
            std::cout << "  To   row (0-9): "; std::cin >> tr;
            std::cout << "  To   col (0-9): "; std::cin >> tc;
            engine.clearLog();
            DisplacementData d = engine.computeDisplacement(
                fr, fc, tr, tc);
            engine.displayInstructionLog();
            regs.exportToJson(engine.getInstructionLog(), d);
            std::cout << "  Exported to registers.json\n";
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
            std::cout << "  registers.json updated.\n";
        }

    } while(subChoice != 0);
}