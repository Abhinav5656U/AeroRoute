#include "Co4.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include "AssemblyEngine.h"
#include "DroneRegisters.h"

using namespace std;

void runCoaCo4() {
    cout << "\n--- COA CO4: Integrated processor-level mission workflow ---\n";
    DroneRegisters registers;
    AssemblyEngine assembly(registers);
    assembly.clearLog();

    registers.loadTelemetry(120, 18.5, 45.0, 87.0);
    cout << "  Application telemetry loaded into CPU registers:\n";
    registers.displayRegisters();
    const DisplacementData displacement =
        assembly.computeDisplacement(0, 0, 7, 5);
    const long long altitude = assembly.SIM_ADD(
        registers.getRAX(), 15, "RAX", "15");
    registers.MOV_RAX(altitude);
    const bool dispatchAllowed = assembly.SIM_CMP_JMP(
        registers.getRDX(), 2000, "JG", "RDX", "2000");
    const long long multipliedRouteCost =
        assembly.SIM_MUL_SUCCESSIVE(9, 6);
    string missionText = "AR-DRONE";
    const size_t originalLength = assembly.asmStringLength(missionText);
    assembly.asmStringAppend(missionText, "-READY");

    if (displacement.dx != 5.0 || displacement.dy != 7.0 ||
        altitude != 135 || !dispatchAllowed ||
        multipliedRouteCost != 54 || originalLength != 8 ||
        missionText != "AR-DRONE-READY") {
        throw runtime_error("COA CO4 integrated mission workflow failed.");
    }

    cout << "  Application coordinates -> displacement: ("
         << displacement.dx << ", " << displacement.dy
         << "), distance=" << displacement.total << "\n"
         << "  ALU/register update: altitude RAX=" << registers.getRAX()
         << " m; conditional branch: "
         << (dispatchAllowed ? "dispatch allowed" : "hold for charging")
         << "\n"
         << "  Arithmetic result: 9 x 6 = " << multipliedRouteCost << "\n"
         << "  64-bit string processing: " << missionText << "\n";
    assembly.displayInstructionLog();
    cout << "COA CO4 checks passed: application logic is mapped to registers, "
            "ALU instructions, control flow, assembly and string processing.\n";
    registers.exportToJson(assembly.getInstructionLog(), displacement);
}
