@echo off
REM ============================================================
REM  AeroRoute — Project Setup Script
REM  MIT Academy of Engineering · SY B.Tech Sem III · 2026-27
REM  Run this ONCE to create all folders and starter files
REM ============================================================

echo.
echo  ================================================
echo   AeroRoute — Project Setup
echo   MIT Academy of Engineering
echo  ================================================
echo.

REM ── Create folder structure ──────────────────────────────────
echo [1/5] Creating folder structure...

mkdir website\data
mkdir website\input
mkdir src\shared
mkdir src\subject1
mkdir src\subject2
mkdir src\subject3
mkdir src\subject4
mkdir build
mkdir docs

echo       Done.

REM ── Create starter JSON files ────────────────────────────────
echo [2/5] Creating starter JSON files...

REM data/grid.json
(
echo {
echo   "source": "awaiting_cpp",
echo   "rows": 10,
echo   "cols": 10,
echo   "node_count": 0,
echo   "cells": [
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0],
echo     [0,0,0,0,0,0,0,0,0,0]
echo   ],
echo   "labels": {}
echo }
) > website\data\grid.json

REM data/route.json
(
echo {
echo   "source": "awaiting_cpp",
echo   "type": "SLL",
echo   "node_count": 0,
echo   "current_index": 0,
echo   "waypoints": []
echo }
) > website\data\route.json

REM data/telemetry.json
(
echo {
echo   "source": "awaiting_cpp",
echo   "mission_status": "IDLE",
echo   "grid_row": null,
echo   "grid_col": null,
echo   "speed_ms": null,
echo   "altitude_m": null,
echo   "battery_pct": null,
echo   "heading_deg": null,
echo   "waypoint_current": null,
echo   "waypoint_total": null,
echo   "drone_model": null,
echo   "route_type": null
echo }
) > website\data\telemetry.json

REM data/registers.json
(
echo {
echo   "source": "awaiting_cpp",
echo   "RAX": 0,
echo   "RBX": 0,
echo   "RCX": 0,
echo   "RDX": 0,
echo   "PC": 4096,
echo   "IR": 0,
echo   "SP": 140737488355327,
echo   "flags": {
echo     "ZF": 0,
echo     "CF": 0,
echo     "SF": 0,
echo     "OF": 0,
echo     "PF": 0
echo   },
echo   "instructions": [],
echo   "displacement": {
echo     "dx": 0,
echo     "dy": 0,
echo     "total": 0.0,
echo     "asm_op": "none"
echo   }
echo }
) > website\data\registers.json

REM data/mission.json
(
echo {
echo   "source": "awaiting_cpp",
echo   "entities": [],
echo   "graphics": {
echo     "translate_x": 0,
echo     "translate_y": 0,
echo     "rotate_deg": 0,
echo     "scale": 1.0,
echo     "clipped_segments": 0
echo   }
echo }
) > website\data\mission.json

REM input/user_input.json
(
echo {
echo   "command": "NONE",
echo   "timestamp": 0
echo }
) > website\input\user_input.json

echo       Done.

REM ── Check g++ ────────────────────────────────────────────────
echo [3/5] Checking g++ compiler...
g++ --version >nul 2>&1
if %errorlevel% == 0 (
    echo       g++ found. You are ready to compile.
) else (
    echo       WARNING: g++ not found in PATH.
    echo       Install MinGW from: https://winlibs.com
    echo       Then add C:\mingw64\bin to your PATH.
)

REM ── Create README ────────────────────────────────────────────
echo [4/5] Creating README...
(
echo # AeroRoute — Autonomous Drone Delivery Simulator
echo MIT Academy of Engineering ^| SY B.Tech Sem III ^| 2026-27
echo.
echo ## How to Run
echo.
echo ### Subject 1 — Programming Lab
echo ```
echo cd src/subject1
echo g++ Subject1_Main.cpp CityGrid.cpp RouteTracker.cpp -o ../../build/subject1.exe
echo ../../build/subject1.exe
echo ```
echo.
echo ### Subject 2 — COA Lab
echo ```
echo cd src/subject2
echo g++ Subject2_Main.cpp DroneRegisters.cpp AssemblyEngine.cpp -o ../../build/subject2.exe
echo ../../build/subject2.exe
echo ```
echo.
echo ### Subject 3 — Graphics Lab
echo ```
echo cd src/subject3
echo g++ Subject3_Main.cpp GraphicsRenderer.cpp -o ../../build/subject3.exe -lfreeglut -lopengl32 -lglu32
echo ../../build/subject3.exe
echo ```
echo.
echo ### Subject 4 — OOP Lab
echo ```
echo cd src/subject4
echo g++ Subject4_Main.cpp -o ../../build/subject4.exe
echo ../../build/subject4.exe
echo ```
echo.
echo ### Full Integrated System
echo ```
echo cd src
echo g++ main.cpp subject1/CityGrid.cpp subject1/RouteTracker.cpp subject2/DroneRegisters.cpp subject2/AssemblyEngine.cpp subject4/Subject4_Main.cpp -o ../build/aeroroutemain.exe
echo ../build/aeroroutemain.exe
echo ```
echo.
echo ### Website
echo Open website/index.html with VS Code Live Server
echo OR run: python -m http.server 8080 inside website/ folder
) > docs\README.md

echo       Done.

REM ── Done ─────────────────────────────────────────────────────
echo [5/5] Setup complete.
echo.
echo  ================================================
echo   Folder structure created.
echo   Starter JSON files created in website/data/
echo   README created in docs/
echo  ================================================
echo.
echo  Next steps:
echo  1. Copy your HTML CSS JS into website/ folder
echo  2. Open CMD in src/subject1/
echo  3. We will compile Subject 1 there
echo.
pause