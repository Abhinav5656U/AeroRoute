/*
 * ============================================================
 * FILE        : CityGrid.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 1 — City Grid using 2D Arrays
 * COURSE OUT  : CO1
 * ============================================================
 */

#include "CityGrid.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cmath>

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
                          const std::string& label) {
    // Validate bounds
    if (!isValidCell(row, col)) {
        std::cout << "[CityGrid] INSERT FAILED: ("
                  << row << "," << col
                  << ") is out of bounds.\n";
        return false;
    }

    // Validate type
    if (type < CELL_WAREHOUSE || type > CELL_OBSTACLE) {
        std::cout << "[CityGrid] INSERT FAILED: "
                  << "type " << type << " is not valid.\n";
        return false;
    }

    // Check if cell is already occupied
    if (grid[row][col] != CELL_EMPTY) {
        std::cout << "[CityGrid] INSERT FAILED: ("
                  << row << "," << col
                  << ") already has a node (type "
                  << grid[row][col] << ").\n";
        return false;
    }

    // Insert
    grid[row][col]   = type;
    labels[row][col] = label.empty()
        ? "Node-" + std::to_string(nodeCount + 1)
        : label;

    if (type != CELL_OBSTACLE) nodeCount++;

    // Rebuild adjacency after structural change
    rebuildAdjacency();

    std::cout << "[CityGrid] INSERT OK: "
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
        std::cout << "[CityGrid] DELETE FAILED: out of bounds.\n";
        return false;
    }

    if (grid[row][col] == CELL_EMPTY) {
        std::cout << "[CityGrid] DELETE FAILED: ("
                  << row << "," << col << ") is already empty.\n";
        return false;
    }

    std::cout << "[CityGrid] DELETE OK: removed "
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
                          const std::string& newLabel) {
    if (!isValidCell(row, col)) {
        std::cout << "[CityGrid] UPDATE FAILED: out of bounds.\n";
        return false;
    }

    if (grid[row][col] == CELL_EMPTY) {
        std::cout << "[CityGrid] UPDATE FAILED: ("
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

    std::cout << "[CityGrid] UPDATE OK: ("
              << row << "," << col << ") type "
              << oldType << " -> " << newType << "\n";
    return true;
}

// ════════════════════════════════════════════════════════════
// SEARCH
// [PL CO1 - Task 1: Array Search Operation]
// ════════════════════════════════════════════════════════════
bool CityGrid::searchByLabel(const std::string& label,
                              int& foundRow,
                              int& foundCol) const {
    std::cout << "[CityGrid] SEARCH: looking for \""
              << label << "\"...\n";

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (labels[r][c] == label) {
                foundRow = r;
                foundCol = c;
                std::cout << "[CityGrid] SEARCH FOUND: \""
                          << label << "\" at ("
                          << r << "," << c << ")\n";
                return true;
            }
        }
    }

    std::cout << "[CityGrid] SEARCH: \""
              << label << "\" not found.\n";
    return false;
}

// ════════════════════════════════════════════════════════════
// DISPLAY — terminal formatted grid
// [PL CO1 - Task 1: Array Display Operation]
// ════════════════════════════════════════════════════════════
void CityGrid::displayGrid() const {
    std::cout << "\n";
    std::cout << "  AeroRoute City Grid (" << GRID_ROWS
              << "x" << GRID_COLS << ") |\n";
    std::cout << "  W=Warehouse C=Customer P=Charging "
              << "X=Obstacle .=Empty\n";
    std::cout << "\n";

    // Column index header
    std::cout << "     ";
    for (int c = 0; c < GRID_COLS; c++) {
        std::cout << std::setw(3) << c;
    }
    std::cout << "\n";

    // Top border
    std::cout << "    +";
    for (int c = 0; c < GRID_COLS; c++) std::cout << "---";
    std::cout << "+\n";

    // Grid rows
    for (int r = 0; r < GRID_ROWS; r++) {
        std::cout << " " << std::setw(2) << r << " |";
        for (int c = 0; c < GRID_COLS; c++) {
            char sym = '.';
            switch (grid[r][c]) {
                case CELL_WAREHOUSE: sym = 'W'; break;
                case CELL_CUSTOMER:  sym = 'C'; break;
                case CELL_CHARGING:  sym = 'P'; break;
                case CELL_OBSTACLE:  sym = 'X'; break;
                default:             sym = '.'; break;
            }
            std::cout << " " << sym << " ";
        }
        std::cout << "|\n";
    }

    // Bottom border
    std::cout << "    +";
    for (int c = 0; c < GRID_COLS; c++) std::cout << "---";
    std::cout << "+\n";
    std::cout << "\n  Total nodes: " << nodeCount << "\n\n";
}

// ════════════════════════════════════════════════════════════
// DISPLAY ADJACENCY MATRIX
// [PL CO1 - Task 1: Adjacency Matrix Display]
// Only shows the top-left 6x6 for readability
// ════════════════════════════════════════════════════════════
void CityGrid::displayAdjacency() const {
    std::cout << "\n  Adjacency Matrix (first 6 nodes shown):\n";
    std::cout << "  (1 = connected flight corridor exists)\n\n";
    int show = 6;
    std::cout << "      ";
    for (int j = 0; j < show; j++)
        std::cout << std::setw(3) << j;
    std::cout << "\n";
    for (int i = 0; i < show; i++) {
        std::cout << "  " << std::setw(3) << i << "  ";
        for (int j = 0; j < show; j++) {
            std::cout << std::setw(3) << adjMatrix[i][j];
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

// ── Accessors ─────────────────────────────────────────────────
int CityGrid::getCellType(int row, int col) const {
    if (!isValidCell(row, col)) return -1;
    return grid[row][col];
}

std::string CityGrid::getLabel(int row, int col) const {
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
void CityGrid::exportToJson() const {
    writeGridJson(
        const_cast<int(*)[GRID_COLS]>(grid),
        const_cast<std::string(*)[GRID_COLS]>(labels),
        nodeCount
    );
    std::cout << "[CityGrid] Exported to grid.json\n";
}