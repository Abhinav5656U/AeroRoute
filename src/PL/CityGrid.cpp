/*
 *
 * FILE        : CityGrid.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 1 — City Grid using 2D Arrays
 * COURSE OUT  : CO1
 *
 */

#include "CityGrid.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>
using namespace std;

// ── Constructor ───────────────────────────────────────────────
// [PL CO1 - Task 1: Array Initialisation]
CityGrid::CityGrid() : nodeCount(0) {

    // Initialise every cell to EMPTY (0)
    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            grid[r][c]   = CELL_EMPTY;
            labels[r][c] = "";
        }
    }

    // Initialise adjacency matrix to 0 (no connections)
    memset(adjMatrix, 0, sizeof(adjMatrix));
}

// ── Private: bounds check ─────────────────────────────────────
bool CityGrid::isValidCell(int row, int col) const {
    return (row >= 0 && row < GRID_ROWS &&
            col >= 0 && col < GRID_COLS);
}

// ── Private: flat index for adjacency matrix ──────────────────
// [PL CO1 - Task 1: 1D Array index from 2D coords]
int CityGrid::flatIndex(int row, int col) const {
    return row * GRID_COLS + col;
}

// ── Private: rebuild adjacency matrix after any grid change ───
// [PL CO1 - Task 1: Adjacency Matrix construction]
// Two cells are adjacent (connected by a flight corridor) if:
// 1. Both are non-empty and non-obstacle
// 2. They are horizontally or vertically neighbours
void CityGrid::rebuildAdjacency() {

    // Reset all connections
    memset(adjMatrix, 0, sizeof(adjMatrix));

    for (int r1 = 0; r1 < GRID_ROWS; r1++) {
        for (int c1 = 0; c1 < GRID_COLS; c1++) {

            // Only non-empty, non-obstacle cells are nodes
            if (grid[r1][c1] == CELL_EMPTY ||
                grid[r1][c1] == CELL_OBSTACLE) continue;

            // Check 4 neighbours: up down left right
            int dr[] = {-1, 1,  0, 0};
            int dc[] = { 0, 0, -1, 1};

            for (int d = 0; d < 4; d++) {
                int r2 = r1 + dr[d];
                int c2 = c1 + dc[d];

                if (!isValidCell(r2, c2)) continue;
                if (grid[r2][c2] == CELL_EMPTY ||
                    grid[r2][c2] == CELL_OBSTACLE) continue;

                // Mark bidirectional connection
                int i = flatIndex(r1, c1);
                int j = flatIndex(r2, c2);
                adjMatrix[i][j] = 1;
                adjMatrix[j][i] = 1;
            }
        }
    }
}

// ════════════════════════════════════════════════════════════
// INSERT
// [PL CO1 - Task 1: Array Insert Operation]
// ════════════════════════════════════════════════════════════
bool CityGrid::insertNode(int row, int col, int type,
                          const string& label) {
    // Validate bounds
    if (!isValidCell(row, col)) {
        cout << "[CityGrid] INSERT FAILED: ("
                  << row << "," << col
                  << ") is out of bounds.\n";
        return false;
    }

    // Validate type
    if (type < CELL_WAREHOUSE || type > CELL_OBSTACLE) {
        cout << "[CityGrid] INSERT FAILED: "
                  << "type " << type << " is not valid.\n";
        return false;
    }

    // Check if cell is already occupied
    if (grid[row][col] != CELL_EMPTY) {
        cout << "[CityGrid] INSERT FAILED: ("
                  << row << "," << col
                  << ") already has a node (type "
                  << grid[row][col] << ").\n";
        return false;
    }

    // Insert
    grid[row][col]   = type;
    labels[row][col] = label.empty()
        ? "Node-" + to_string(nodeCount + 1)
        : label;

    if (type != CELL_OBSTACLE) nodeCount++;

    // Rebuild adjacency after structural change
    rebuildAdjacency();

    cout << "[CityGrid] INSERT OK: "
              << labels[row][col]
              << " (type " << type << ") at ("
              << row << "," << col << ")\n";
    return true;
}

// ════════════════════════════════════════════════════════════
// DELETE
// [PL CO1 - Task 1: Array Delete Operation]
// ════════════════════════════════════════════════════════════
bool CityGrid::deleteNode(int row, int col) {
    if (!isValidCell(row, col)) {
        cout << "[CityGrid] DELETE FAILED: out of bounds.\n";
        return false;
    }

    if (grid[row][col] == CELL_EMPTY) {
        cout << "[CityGrid] DELETE FAILED: ("
                  << row << "," << col << ") is already empty.\n";
        return false;
    }

    cout << "[CityGrid] DELETE OK: removed "
              << labels[row][col]
              << " from (" << row << "," << col << ")\n";

    if (grid[row][col] != CELL_OBSTACLE) nodeCount--;

    grid[row][col]   = CELL_EMPTY;
    labels[row][col] = "";

    rebuildAdjacency();
    return true;
}

// ════════════════════════════════════════════════════════════
// UPDATE
// [PL CO1 - Task 1: Array Update Operation]
// ════════════════════════════════════════════════════════════
bool CityGrid::updateNode(int row, int col, int newType,
                          const string& newLabel) {
    if (!isValidCell(row, col)) {
        cout << "[CityGrid] UPDATE FAILED: out of bounds.\n";
        return false;
    }

    if (grid[row][col] == CELL_EMPTY) {
        cout << "[CityGrid] UPDATE FAILED: ("
                  << row << "," << col << ") is empty. "
                  << "Use INSERT first.\n";
        return false;
    }

    int oldType = grid[row][col];
    grid[row][col] = newType;
    if (!newLabel.empty()) labels[row][col] = newLabel;

    // Adjust node count if obstacle status changed
    if (oldType == CELL_OBSTACLE && newType != CELL_OBSTACLE)
        nodeCount++;
    if (oldType != CELL_OBSTACLE && newType == CELL_OBSTACLE)
        nodeCount--;

    rebuildAdjacency();

    cout << "[CityGrid] UPDATE OK: ("
              << row << "," << col << ") type "
              << oldType << " -> " << newType << "\n";
    return true;
}

// ════════════════════════════════════════════════════════════
// SEARCH
// [PL CO1 - Task 1: Array Search Operation]
// ════════════════════════════════════════════════════════════
bool CityGrid::searchByLabel(const string& label,
                              int& foundRow,
                              int& foundCol) const {
    cout << "[CityGrid] SEARCH: looking for \""
              << label << "\"...\n";

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (labels[r][c] == label) {
                foundRow = r;
                foundCol = c;
                cout << "[CityGrid] SEARCH FOUND: \""
                          << label << "\" at ("
                          << r << "," << c << ")\n";
                return true;
            }
        }
    }

    cout << "[CityGrid] SEARCH: \""
              << label << "\" not found.\n";
    return false;
}

// ════════════════════════════════════════════════════════════
// DISPLAY — terminal formatted grid
// [PL CO1 - Task 1: Array Display Operation]
// ════════════════════════════════════════════════════════════
void CityGrid::displayGrid() const {
    cout << "\n";
    cout << "  AeroRoute City Grid (" << GRID_ROWS
              << "x" << GRID_COLS << ") |\n";
    cout << "  W=Warehouse C=Customer P=Charging "
              << "X=Obstacle .=Empty\n";
    cout << "\n";

    // Column index header
    cout << "     ";
    for (int c = 0; c < GRID_COLS; c++) {
        cout << setw(3) << c;
    }
    cout << "\n";

    // Top border
    cout << "    +";
    for (int c = 0; c < GRID_COLS; c++) cout << "---";
    cout << "+\n";

    // Grid rows
    for (int r = 0; r < GRID_ROWS; r++) {
        cout << " " << setw(2) << r << " |";
        for (int c = 0; c < GRID_COLS; c++) {
            char sym = '.';
            switch (grid[r][c]) {
                case CELL_WAREHOUSE: sym = 'W'; break;
                case CELL_CUSTOMER:  sym = 'C'; break;
                case CELL_CHARGING:  sym = 'P'; break;
                case CELL_OBSTACLE:  sym = 'X'; break;
                default:             sym = '.'; break;
            }
            cout << " " << sym << " ";
        }
        cout << "|\n";
    }

    // Bottom border
    cout << "    +";
    for (int c = 0; c < GRID_COLS; c++) cout << "---";
    cout << "+\n";
    cout << "\n  Total nodes: " << nodeCount << "\n\n";
}

// ════════════════════════════════════════════════════════════
// DISPLAY ADJACENCY MATRIX
// [PL CO1 - Task 1: Adjacency Matrix Display]
// Only shows the top-left 6x6 for readability
// ════════════════════════════════════════════════════════════
void CityGrid::displayAdjacency() const {
    cout << "\n  Adjacency Matrix (first 6 nodes shown):\n";
    cout << "  (1 = connected flight corridor exists)\n\n";
    int show = 6;
    cout << "      ";
    for (int j = 0; j < show; j++)
        cout << setw(3) << j;
    cout << "\n";
    for (int i = 0; i < show; i++) {
        cout << "  " << setw(3) << i << "  ";
        for (int j = 0; j < show; j++) {
            cout << setw(3) << adjMatrix[i][j];
        }
        cout << "\n";
    }
    cout << "\n";
}

// ── Accessors ─────────────────────────────────────────────────
int CityGrid::getCellType(int row, int col) const {
    if (!isValidCell(row, col)) return -1;
    return grid[row][col];
}

string CityGrid::getLabel(int row, int col) const {
    if (!isValidCell(row, col)) return "";
    return labels[row][col];
}

int CityGrid::getNodeCount() const { return nodeCount; }

bool CityGrid::isAdjacent(int r1,int c1,int r2,int c2) const {
    if (!isValidCell(r1,c1) || !isValidCell(r2,c2)) return false;
    return adjMatrix[flatIndex(r1,c1)][flatIndex(r2,c2)] == 1;
}

// ════════════════════════════════════════════════════════════
// JSON EXPORT
// Writes grid state to website/data/grid.json
// Website reads this file every 1 second
// ════════════════════════════════════════════════════════════
void CityGrid::exportToJson(int rows, int cols) const {
    writeGridJson(
        const_cast<int(*)[GRID_COLS]>(grid),
        const_cast<string(*)[GRID_COLS]>(labels),
        nodeCount, rows, cols
    );
    cout << "[CityGrid] Exported to grid.json\n";
}