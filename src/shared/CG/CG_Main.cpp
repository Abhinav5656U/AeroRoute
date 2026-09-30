/*
 * ============================================================
 * FILE        : Subject3_Main.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : CG — Computer Graphics Lab
 * COURSE CODE : 2310218L
 * TASK        : Task 1 (CO1 CO2) + Task 2 (CO3 CO5)
 * DESCRIPTION : Standalone entry point for CG subject.
 *               Demonstrates all OpenGL primitives,
 *               affine transformations, and Cohen-Sutherland
 *               line clipping.
 * HOW TO RUN  :
 *   cd src/CG
 *   g++ Subject3_Main.cpp GraphicsRenderer.cpp -o ..\..\build\CG.exe -std=c++17 -lfreeglut -lopengl32 -lglu32
 *   ..\..\build\CG.exe
 * ============================================================
 */

#include <iostream>
#include <vector>
#include "GraphicsRenderer.h"
#include "../Constants.h"

int main(int argc, char** argv) {

    std::cout << "\n";
    std::cout << "  ================================================\n";
    std::cout << "   AeroRoute — CG: Computer Graphics Lab\n";
    std::cout << "   Course Code: 2310218L\n";
    std::cout << "   CO1 CO2: OpenGL Primitives\n";
    std::cout << "   CO3:     Affine Transformations\n";
    std::cout << "   CO5:     Cohen-Sutherland Clipping\n";
    std::cout << "   MIT Academy of Engineering | Sem III 2026-27\n";
    std::cout << "  ================================================\n\n";

    GraphicsRenderer renderer;

    // ── Load a sample city grid ───────────────────────────────
    int sampleGrid[GRID_ROWS][GRID_COLS] = {
        {1,0,0,4,0,0,0,2,0,0},
        {0,0,4,0,0,2,0,0,0,0},
        {0,0,0,0,3,0,0,0,4,0},
        {4,0,0,2,0,0,0,0,0,2},
        {0,0,3,0,0,0,4,0,0,0},
        {0,2,0,0,0,1,0,0,3,0},
        {0,0,0,4,0,0,0,2,0,0},
        {2,0,0,0,0,0,3,0,0,4},
        {0,0,4,0,2,0,0,0,0,0},
        {0,2,0,0,0,0,4,0,3,1},
    };

    std::string sampleLabels[GRID_ROWS][GRID_COLS];
    sampleLabels[0][0] = "WH-1";
    sampleLabels[0][7] = "CS-A";
    sampleLabels[5][5] = "WH-2";
    sampleLabels[9][9] = "CS-B";
    sampleLabels[2][4] = "CP-1";
    sampleLabels[5][8] = "CP-2";

    renderer.loadGrid(sampleGrid, sampleLabels);

    // ── Load a sample route ───────────────────────────────────
    // Convert grid positions to GL coordinates manually
    // (renderer does this internally but we show the concept)
    std::vector<Point2D> route;
    // Route: WH-1 (0,0) -> midpoints -> CS-B (9,9)
    float orig_x = -230.0f;
    float orig_y =  230.0f;
    float cell   =   50.0f;

    auto gp = [&](int r, int c) -> Point2D {
        return { orig_x + c*cell + cell*0.5f,
                 orig_y - r*cell - cell*0.5f };
    };

    route.push_back(gp(0, 0));
    route.push_back(gp(0, 3));
    route.push_back(gp(2, 5));
    route.push_back(gp(4, 7));
    route.push_back(gp(7, 7));
    route.push_back(gp(9, 9));

    renderer.loadRoute(route, "SLL");
    renderer.setDronePosition(0, 0, 0.0f);

    // ── Demo: Transformation matrices in terminal ─────────────
    std::cout << "[CG CO3] Demonstrating transformation matrices:\n";
    renderer.makeTranslation(50.0f, -30.0f);
    renderer.makeRotation(45.0f);
    renderer.makeScaling(1.5f, 1.5f);

    // ── Set clipping viewport ─────────────────────────────────
    std::cout << "\n[CG CO5] Setting clipping viewport:\n";
    renderer.setViewport(-200.0f, 200.0f, -180.0f, 180.0f);

    // ── Export initial state to JSON ──────────────────────────
    renderer.exportToJson();

    // ── Launch OpenGL window ──────────────────────────────────
    renderer.launchWindow(argc, argv,
        "AeroRoute CG — Drone Delivery Grid | "
        "WASD=Move Q/E=Rotate +/-=Zoom C/V=Clip");

    return 0;
}