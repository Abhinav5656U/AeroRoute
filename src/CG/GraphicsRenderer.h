/*
 *
 * FILE        : GraphicsRenderer.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : CG — Computer Graphics Lab
 * COURSE CODE : 2310218L
 * TASK        : Task 1 — Graphics Primitives (CO1, CO2)
 *               Task 2 — Geometric Transformations &
 *                        Clipping (CO3, CO5)
 * COURSE OUT  : CO1 — Render 2D/3D delivery environment
 *               CO2 — Use OpenGL primitives
 *               CO3 — Apply affine transformations
 *               CO5 — Implement Cohen-Sutherland clipping
 * DATE        : 2026-2027
 *
 */

#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include <string>
#include <vector>
#include <cmath>
#include <functional>
#include <utility>

// ── OpenGL headers for Windows + freeGLUT ────────────────────
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include "../shared/Constants.h"
#include "../shared/JsonWriter.h"
using namespace std;

// ════════════════════════════════════════════════════════════
// [CG CO1 - Task 1: Point2D]
// Basic 2D point used across all graphics operations.
// ════════════════════════════════════════════════════════════
struct Point2D {
    float x;
    float y;
    Point2D(float px = 0.0f, float py = 0.0f)
        : x(px), y(py) {}
};

// ════════════════════════════════════════════════════════════
// [CG CO3 - Task 2: TransformMatrix]
// Stores a 3x3 homogeneous transformation matrix for 2D.
// Used to compose translation, rotation, and scaling.
//
//  | m00  m01  m02 |   | cos  -sin  tx |
//  | m10  m11  m12 | = | sin   cos  ty |
//  | m20  m21  m22 |   |  0    0    1  |
// ════════════════════════════════════════════════════════════
struct TransformMatrix {
    float m[3][3];

    // Constructor — identity matrix
    TransformMatrix() {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    // Multiply this matrix by another (matrix composition)
    TransformMatrix operator*(const TransformMatrix& other) const;

    // Apply matrix to a 2D point
    Point2D apply(const Point2D& p) const;

    // Display matrix values
    void display(const string& label) const;
};

// ════════════════════════════════════════════════════════════
// [CG CO5 - Task 2: Cohen-Sutherland region codes]
// Each bit represents which side of the viewport the
// endpoint is on.
//
//        |         |
//  1001  |  1000   |  1010
//        |         |
// -------+---------+-------
//        |         |
//  0001  |  0000   |  0010
//        |         |
// -------+---------+-------
//        |         |
//  0101  |  0100   |  0110
//        |         |
// ════════════════════════════════════════════════════════════
const int CS_INSIDE = 0; // 0000
const int CS_LEFT   = 1; // 0001
const int CS_RIGHT  = 2; // 0010
const int CS_BOTTOM = 4; // 0100
const int CS_TOP    = 8; // 1000

// ════════════════════════════════════════════════════════════
// [CG CO1 CO2 CO3 CO5 - GraphicsRenderer Class]
//
// Manages the full OpenGL rendering pipeline for AeroRoute:
//   Task 1: Draws city grid, drone, nodes using GL primitives
//   Task 2: Applies transformations, clips lines to viewport
//   Task 3: Maps a procedural city texture and landmark lighting
// ════════════════════════════════════════════════════════════
class GraphicsRenderer {
private:

    // ── Window dimensions ─────────────────────────────────────
    int windowWidth;
    int windowHeight;

    // ── Grid layout ───────────────────────────────────────────
    float cellSize;     // size of each grid cell in GL units
    float gridOriginX;  // top-left of grid in GL coords
    float gridOriginY;
    int activeRows;
    int activeCols;
    bool task3EffectsEnabled;
    GLuint cityTexture;

    // ── City grid data (matches CityGrid from PL) ─────────────
    int   gridCells[GRID_ROWS][GRID_COLS];
    string gridLabels[GRID_ROWS][GRID_COLS];

    // ── Route waypoints ───────────────────────────────────────
    vector<Point2D> routePoints;
    string          routeType;
    vector<pair<int, int>> deliveryGridPath;
    vector<pair<int, int>> trafficGridPath;
    vector<Point2D> trafficPoints;

    // ── Drone state ───────────────────────────────────────────
    Point2D dronePos;
    Point2D trafficPos;
    float   droneAngle;   // heading in degrees
    float   droneScale;   // zoom level

    // ── Transformation state [CG CO3] ─────────────────────────
    float   translateX;
    float   translateY;
    float   rotationAngle;
    float   scaleFactor;

    // ── Viewport for clipping [CG CO5] ────────────────────────
    float vpXmin, vpXmax;
    float vpYmin, vpYmax;

    // ── Clipping stats ────────────────────────────────────────
    int clippedSegments;
    int totalSegments;

    // ── Delivery simulation state ─────────────────────────────
    float deliveryDistance;
    float trafficDistance;
    int lastDeliveryNode;
    int lastTrafficNode;
    int trafficDirection;
    bool simulationActive;
    bool collisionHold;
    string simulationStatus;
    function<void(int, int, bool, bool)> simulationCallback;

    // ── Graphics data for JSON export ─────────────────────────
    GraphicsData gfxData;

    // ── Private drawing helpers ───────────────────────────────
    void drawGridLines();
    void drawTask3TexturedSurface();
    void drawTask3Lighting();
    void drawCell(int row, int col);
    void drawWarehouseNode(float cx, float cy);
    void drawCustomerNode(float cx, float cy);
    void drawChargingNode(float cx, float cy);
    void drawObstacleNode(float cx, float cy);
    void drawDroneBody(float cx, float cy, float angle);
    void drawTrafficDrone();
    void drawRotorArm(float cx, float cy,
                      float dx, float dy);
    void drawLegend();
    void drawHUD();

    // ── Coordinate conversion helpers ─────────────────────────
    Point2D gridToGL(int row, int col) const;
    Point2D glToGrid(float x, float y)  const;
    Point2D pointAlongPath(const vector<Point2D>& points,
                           float distance) const;
    float pathLength(const vector<Point2D>& points) const;
    void updateSimulation();

    // ── Cohen-Sutherland helpers [CG CO5] ─────────────────────
    int  computeRegionCode(float x, float y) const;
    bool clipLine(float& x1, float& y1,
                  float& x2, float& y2);

public:

    // Constructor
    GraphicsRenderer();

    // ── Data loading (from PL and COA modules) ────────────────
    void loadGrid(int cells[GRID_ROWS][GRID_COLS],
                  string labels[GRID_ROWS][GRID_COLS],
                  int rows = GRID_ROWS, int cols = GRID_COLS);
    void loadRoute(const vector<Point2D>& points,
                   const string& type);
    void setDronePosition(int row, int col, float angle);
    void setSimulationPaths(const vector<pair<int, int>>& deliveryPath,
                            const vector<pair<int, int>>& trafficPath);
    void setSimulationCallback(
        function<void(int, int, bool, bool)> callback);
    void setTask3EffectsEnabled(bool enabled);
    void toggleTask3Effects();

    // ════════════════════════════════════════════════════════
    // TASK 1 — Graphics Primitives [CG CO1, CO2]
    // ════════════════════════════════════════════════════════

    // Main scene render — called by GLUT display callback
    void renderScene();

    // Draw individual primitives
    void drawPoint(float x, float y, float size,
                   float r, float g, float b);

    void drawLine(float x1, float y1,
                  float x2, float y2,
                  float r, float g, float b,
                  bool dashed = false);

    void drawQuad(float x, float y,
                  float w, float h,
                  float r, float g, float b,
                  bool filled = true);

    void drawPolygon(const vector<Point2D>& vertices,
                     float r, float g, float b,
                     bool filled = true);

    void drawCircle(float cx, float cy, float radius,
                    float r, float g, float b,
                    bool filled = true);

    // ════════════════════════════════════════════════════════
    // TASK 2 — Transformations [CG CO3]
    // ════════════════════════════════════════════════════════

    // Build individual transformation matrices
    TransformMatrix makeTranslation(float tx, float ty);
    TransformMatrix makeRotation(float angleDeg);
    TransformMatrix makeScaling(float sx, float sy);

    // Apply composed transformation to drone position
    void applyTransform(float tx, float ty,
                        float angleDeg, float scale);

    // Move drone along route (translation)
    void translateDrone(float dx, float dy);

    // Rotate drone to face next waypoint
    void rotateDroneTo(float targetAngle);

    // Scale viewport (zoom in/out)
    void scaleView(float factor);

    // Display transformation matrix in terminal
    void displayTransformInfo() const;

    // ════════════════════════════════════════════════════════
    // TASK 2 — Cohen-Sutherland Clipping [CG CO5]
    // ════════════════════════════════════════════════════════

    // Set the clipping viewport boundaries
    void setViewport(float xmin, float xmax,
                     float ymin, float ymax);

    // Clip and draw a line against the viewport
    // Returns true if any part of line was visible
    bool clipAndDrawLine(float x1, float y1,
                         float x2, float y2,
                         float r,  float g, float b);

    // Clip and draw entire route path
    void clipAndDrawRoute();

    // Display clipping statistics
    void displayClippingInfo() const;

    // ── GLUT callbacks (set as static, call instance methods) ─
    static void displayCallback();
    static void reshapeCallback(int w, int h);
    static void keyboardCallback(unsigned char key, int x, int y);
    static void timerCallback(int value);

    // Singleton accessor for GLUT callbacks
    static GraphicsRenderer* getInstance();
    static void              setInstance(GraphicsRenderer* inst);

    // ── Accessors ────────────────────────────────────────────
    float getTranslateX()    const;
    float getTranslateY()    const;
    float getRotationAngle() const;
    float getScaleFactor()   const;
    int   getClippedCount()  const;

    // ── JSON export ──────────────────────────────────────────
    void exportToJson() const;

    // ── GLUT window launch ───────────────────────────────────
    void launchWindow(int argc, char** argv,
                      const string& title);
};

#endif // GRAPHICS_RENDERER_H