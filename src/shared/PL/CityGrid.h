/*
 * ============================================================
 * FILE        : CityGrid.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 1 — City Grid using 2D Arrays
 * COURSE OUT  : CO1 — Model city grid/navigation space using
 *               1D/2D arrays and adjacency matrices.
 *               Implement insert delete update search display.
 * AUTHOR      : [Your Name] | MIT Academy of Engineering
 * DATE        : 2026-2027
 * ============================================================
 */

#ifndef CITY_GRID_H
#define CITY_GRID_H

#include <string>
#include "../Constants.h"
#include "../JsonWriter.h"

// ════════════════════════════════════════════════════════════
// [PL CO1 - Task 1: CityGrid Class]
// Represents the 10x10 urban delivery space as a 2D array.
// Each cell holds an integer type code matching CellType in JS.
// Adjacency matrix tracks valid flight corridors.
// ════════════════════════════════════════════════════════════
class CityGrid {
private:

    // [PL CO1 - Task 1: 2D Array — city grid]
    // Each element is a cell type: 0=empty 1=WH 2=CS 3=CP 4=OB
    int grid[GRID_ROWS][GRID_COLS];

    // [PL CO1 - Task 1: String Array — node labels]
    // Stores user-defined label for each non-empty cell
    std::string labels[GRID_ROWS][GRID_COLS];

    // [PL CO1 - Task 1: Adjacency Matrix]
    // Flattened: index = row*COLS + col
    // adj[i][j] = 1 means drone can fly directly from node i to j
    int adjMatrix[GRID_ROWS * GRID_COLS][GRID_ROWS * GRID_COLS];

    // Count of non-empty, non-obstacle nodes
    int nodeCount;

    // ── Private helpers ──────────────────────────────────────
    bool isValidCell(int row, int col) const;
    void rebuildAdjacency();
    int  flatIndex(int row, int col) const;

public:
    // Constructor — initialises grid to all zeros
    CityGrid();

    // ── CRUD Operations [PL CO1] ─────────────────────────────

    // INSERT: place a node type at (row, col) with optional label
    // Returns true on success, false if cell is out of bounds
    // or already occupied
    bool insertNode(int row, int col, int type,
                    const std::string& label = "");

    // DELETE: remove node at (row, col), reset to EMPTY
    // Returns true on success, false if cell was already empty
    bool deleteNode(int row, int col);

    // UPDATE: change the type of an existing node
    // Returns true on success, false if cell is empty
    bool updateNode(int row, int col, int newType,
                    const std::string& newLabel = "");

    // SEARCH: find a node by label, returns true and fills
    // foundRow/foundCol if found
    bool searchByLabel(const std::string& label,
                       int& foundRow, int& foundCol) const;

    // DISPLAY: print the grid to terminal in a readable format
    void displayGrid() const;

    // DISPLAY: print the adjacency matrix
    void displayAdjacency() const;

    // ── Accessors ────────────────────────────────────────────
    int getCellType(int row, int col)       const;
    std::string getLabel(int row, int col)  const;
    int getNodeCount()                      const;
    bool isAdjacent(int r1,int c1,int r2,int c2) const;

    // ── JSON export ──────────────────────────────────────────
    // Writes current grid state to website/data/grid.json
    void exportToJson() const;
};

#endif // CITY_GRID_H