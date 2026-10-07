/*
 * ============================================================
 * FILE        : JsonWriter.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * DESCRIPTION : Lightweight JSON file writer.
 *               No external libraries needed.
 *               Every subject uses this to write output
 *               that the website dashboard reads.
 *
 *               HOW IT WORKS:
 *               Each write function opens a file, writes
 *               formatted JSON, and closes it.
 *               The website polls these files every 1 second.
 *
 * ============================================================
 */

#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include "Constants.h"
using namespace std;

// ── Forward declarations of structs used in writes ────────────

struct WaypointData {
    int         row;
    int         col;
    string label;
};

struct EntityData {
    string type;    // "warehouse" "customer" "charging" "drone" "package"
    string label;
    int         row;
    int         col;
    double      weight_kg; // used for package only
};

struct InstructionData {
    string addr;
    string op;
    string args;
};

struct DisplacementData {
    double      dx;
    double      dy;
    double      total;
    string asm_op;
};

struct GraphicsData {
    double translate_x;
    double translate_y;
    double rotate_deg;
    double scale;
    int    clipped_segments;
};

// ── Helper: open file safely ──────────────────────────────────
inline bool openFile(ofstream& f, const string& path) {
    f.open(path);
    if (!f.is_open()) {
        // Try current directory as fallback
        // (useful when running from different working dirs)
        string name = path.substr(path.find_last_of("/\\") + 1);
        f.open(name);
        if (!f.is_open()) {
            return false;
        }
    }
    return true;
}

// ════════════════════════════════════════════════════════════
// writeGridJson
// Called by: Subject 1 CityGrid
// Writes:    data/grid.json
// ════════════════════════════════════════════════════════════
inline void writeGridJson(
    int grid[GRID_ROWS][GRID_COLS],
    const string labels[GRID_ROWS][GRID_COLS],
    int nodeCount,
    int rows = GRID_ROWS,
    int cols = GRID_COLS
) {
    ofstream f;
    if (!openFile(f, JSON_GRID)) return;

    f << "{\n";
    f << "  \"source\": \"cpp_subject1\",\n";
    f << "  \"rows\": " << rows << ",\n";
    f << "  \"cols\": " << cols << ",\n";
    f << "  \"node_count\": " << nodeCount << ",\n";

    // Write 2D cells array
    f << "  \"cells\": [\n";
    for (int r = 0; r < rows; r++) {
        f << "    [";
        for (int c = 0; c < cols; c++) {
            f << grid[r][c];
            if (c < cols - 1) f << ",";
        }
        f << "]";
        if (r < rows - 1) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Write labels object — only non-empty labels
    f << "  \"labels\": {\n";
    bool firstLabel = true;
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (!labels[r][c].empty()) {
                if (!firstLabel) f << ",\n";
                f << "    \"" << r << "," << c
                  << "\": \"" << labels[r][c] << "\"";
                firstLabel = false;
            }
        }
    }
    f << "\n  }\n";
    f << "}\n";
    f.close();
}

// ════════════════════════════════════════════════════════════
// writeRouteJson
// Called by: Subject 1 RouteTracker
// Writes:    data/route.json
// ════════════════════════════════════════════════════════════
inline void writeRouteJson(
    const string&              routeType,
    const vector<WaypointData>& waypoints,
    int                             currentIndex
) {
    ofstream f;
    if (!openFile(f, JSON_ROUTE)) return;

    f << "{\n";
    f << "  \"source\": \"cpp_subject1\",\n";
    f << "  \"type\": \"" << routeType << "\",\n";
    f << "  \"node_count\": " << (int)waypoints.size() << ",\n";
    f << "  \"current_index\": " << currentIndex << ",\n";
    f << "  \"waypoints\": [\n";

    for (int i = 0; i < (int)waypoints.size(); i++) {
        f << "    {\n";
        f << "      \"row\": "   << waypoints[i].row   << ",\n";
        f << "      \"col\": "   << waypoints[i].col   << ",\n";
        f << "      \"label\": \"" << waypoints[i].label << "\"\n";
        f << "    }";
        if (i < (int)waypoints.size() - 1) f << ",";
        f << "\n";
    }

    f << "  ]\n";
    f << "}\n";
    f.close();
}

// ════════════════════════════════════════════════════════════
// writeTelemetryJson
// Called by: Subject 1 + 2 together in main.cpp
// Writes:    data/telemetry.json
// ════════════════════════════════════════════════════════════
inline void writeTelemetryJson(
    int         gridRow,
    int         gridCol,
    double      speedMs,
    int         altitudeM,
    double      batteryPct,
    int         headingDeg,
    int         waypointCurrent,
    int         waypointTotal,
    string droneModel,
    string routeType,
    string missionStatus
) {
    ofstream f;
    if (!openFile(f, JSON_TELEMETRY)) return;

    f << fixed << setprecision(2);
    f << "{\n";
    f << "  \"source\": \"cpp_subject1_2\",\n";
    f << "  \"mission_status\": \"" << missionStatus << "\",\n";
    f << "  \"grid_row\": "         << gridRow        << ",\n";
    f << "  \"grid_col\": "         << gridCol        << ",\n";
    f << "  \"speed_ms\": "         << speedMs        << ",\n";
    f << "  \"altitude_m\": "       << altitudeM      << ",\n";
    f << "  \"battery_pct\": "      << batteryPct     << ",\n";
    f << "  \"heading_deg\": "      << headingDeg     << ",\n";
    f << "  \"waypoint_current\": " << waypointCurrent<< ",\n";
    f << "  \"waypoint_total\": "   << waypointTotal  << ",\n";
    f << "  \"drone_model\": \""    << droneModel     << "\",\n";
    f << "  \"route_type\": \""     << routeType      << "\"\n";
    f << "}\n";
    f.close();
}

// ════════════════════════════════════════════════════════════
// writeRegistersJson
// Called by: Subject 2 DroneRegisters + AssemblyEngine
// Writes:    data/registers.json
// ════════════════════════════════════════════════════════════
inline void writeRegistersJson(
    long long                        RAX,
    long long                        RBX,
    long long                        RCX,
    long long                        RDX,
    long long                        PC,
    long long                        IR,
    long long                        SP,
    bool                             ZF,
    bool                             CF,
    bool                             SF,
    bool                             OF,
    bool                             PF,
    const vector<InstructionData>& instructions,
    const DisplacementData&          disp
) {
    ofstream f;
    if (!openFile(f, JSON_REGISTERS)) return;

    f << "{\n";
    f << "  \"source\": \"cpp_subject2\",\n";
    f << "  \"RAX\": " << RAX << ",\n";
    f << "  \"RBX\": " << RBX << ",\n";
    f << "  \"RCX\": " << RCX << ",\n";
    f << "  \"RDX\": " << RDX << ",\n";
    f << "  \"PC\": "  << PC  << ",\n";
    f << "  \"IR\": "  << IR  << ",\n";
    f << "  \"SP\": "  << SP  << ",\n";

    // FLAGS
    f << "  \"flags\": {\n";
    f << "    \"ZF\": " << (ZF ? 1 : 0) << ",\n";
    f << "    \"CF\": " << (CF ? 1 : 0) << ",\n";
    f << "    \"SF\": " << (SF ? 1 : 0) << ",\n";
    f << "    \"OF\": " << (OF ? 1 : 0) << ",\n";
    f << "    \"PF\": " << (PF ? 1 : 0) << "\n";
    f << "  },\n";

    // Instruction trace
    f << "  \"instructions\": [\n";
    // Show last 12 only to keep JSON small
    int start = (int)instructions.size() > 12
                ? (int)instructions.size() - 12 : 0;
    for (int i = start; i < (int)instructions.size(); i++) {
        f << "    {\n";
        f << "      \"addr\": \""  << instructions[i].addr << "\",\n";
        f << "      \"op\": \""    << instructions[i].op   << "\",\n";
        f << "      \"args\": \"" << instructions[i].args  << "\"\n";
        f << "    }";
        if (i < (int)instructions.size() - 1) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Displacement
    f << "  \"displacement\": {\n";
    f << fixed << setprecision(4);
    f << "    \"dx\": "      << disp.dx     << ",\n";
    f << "    \"dy\": "      << disp.dy     << ",\n";
    f << "    \"total\": "   << disp.total  << ",\n";
    f << "    \"asm_op\": \"" << disp.asm_op << "\"\n";
    f << "  }\n";
    f << "}\n";
    f.close();
}

// ════════════════════════════════════════════════════════════
// writeMissionJson
// Called by: Subject 4 OOP + Subject 3 Graphics
// Writes:    data/mission.json
// ════════════════════════════════════════════════════════════
inline void writeMissionJson(
    const vector<EntityData>& entities,
    const GraphicsData&            graphics
) {
    ofstream f;
    if (!openFile(f, JSON_MISSION)) return;

    f << "{\n";
    f << "  \"source\": \"cpp_subject4_3\",\n";

    // Entities list
    f << "  \"entities\": [\n";
    for (int i = 0; i < (int)entities.size(); i++) {
        const EntityData& e = entities[i];
        f << "    {\n";
        f << "      \"type\": \""  << e.type  << "\",\n";
        f << "      \"label\": \"" << e.label << "\",\n";
        f << "      \"row\": "     << e.row   << ",\n";
        f << "      \"col\": "     << e.col;
        // Package has weight, no row/col in the usual sense
        if (e.type == "package") {
            f << ",\n      \"weight_kg\": " << e.weight_kg << "\n";
        } else {
            f << "\n";
        }
        f << "    }";
        if (i < (int)entities.size() - 1) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Graphics data
    f << fixed << setprecision(4);
    f << "  \"graphics\": {\n";
    f << "    \"translate_x\": "      << graphics.translate_x      << ",\n";
    f << "    \"translate_y\": "      << graphics.translate_y      << ",\n";
    f << "    \"rotate_deg\": "       << graphics.rotate_deg       << ",\n";
    f << "    \"scale\": "            << graphics.scale            << ",\n";
    f << "    \"clipped_segments\": " << graphics.clipped_segments << "\n";
    f << "  }\n";
    f << "}\n";
    f.close();
}

#endif // JSON_WRITER_H