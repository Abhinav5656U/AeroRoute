/*
 * ============================================================
 * FILE        : Constants.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * DESCRIPTION : Shared constants used across all subjects.
 *               Cell type codes here MUST match the values
 *               JavaScript reads from grid.json.
 * AUTHOR      : [Your Name] | MIT Academy of Engineering
 * ============================================================
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

// ── Grid dimensions ──────────────────────────────────────────
#define GRID_ROWS 10
#define GRID_COLS 10

// ── Cell types — must match dashboard.js CellType object ─────
#define CELL_EMPTY     0
#define CELL_WAREHOUSE 1
#define CELL_CUSTOMER  2
#define CELL_CHARGING  3
#define CELL_OBSTACLE  4

// ── Drone models ─────────────────────────────────────────────
#define DRONE_DELIVERY "AR-D1 Delivery"
#define DRONE_SCOUT    "AR-S1 Scout"
#define DRONE_HEAVY    "AR-H1 Heavy Lift"

// ── Route types ───────────────────────────────────────────────
#define ROUTE_SLL "SLL"   // Singly Linked List  — forward path
#define ROUTE_DLL "DLL"   // Doubly Linked List  — return path
#define ROUTE_CLL "CLL"   // Circular Linked List — patrol

// ── JSON output paths (relative to where exe runs) ───────────
#define JSON_GRID       "../../website/data/grid.json"
#define JSON_ROUTE      "../../website/data/route.json"
#define JSON_TELEMETRY  "../../website/data/telemetry.json"
#define JSON_REGISTERS  "../../website/data/registers.json"
#define JSON_MISSION    "../../website/data/mission.json"
#define JSON_INPUT      "../../website/input/user_input.json"

// ── Register base address ─────────────────────────────────────
#define PC_START 0x1000

#endif // CONSTANTS_H