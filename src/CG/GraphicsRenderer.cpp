/*
 * FILE        : GraphicsRenderer.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : CG — Computer Graphics Lab
 * COURSE CODE : 2310218L
 * TASK        : Task 1 (CO1 CO2) + Task 2 (CO3 CO5)
 */

#include "GraphicsRenderer.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <sstream>
#include <stdexcept>
using namespace std;

// ── Singleton instance for GLUT callbacks ─────────────────────
static GraphicsRenderer* g_instance = nullptr;

GraphicsRenderer* GraphicsRenderer::getInstance() {
    return g_instance;
}
void GraphicsRenderer::setInstance(GraphicsRenderer* inst) {
    g_instance = inst;
}

// ════════════════════════════════════════════════════════════
// Constructor
// ════════════════════════════════════════════════════════════
GraphicsRenderer::GraphicsRenderer()
    : windowWidth(900), windowHeight(700),
      cellSize(50.0f),
      gridOriginX(-230.0f), gridOriginY(230.0f),
      activeRows(GRID_ROWS), activeCols(GRID_COLS),
      task3EffectsEnabled(false), cityTexture(0),
      routeType("SLL"),
      droneAngle(0.0f), droneScale(1.0f),
      translateX(0.0f), translateY(0.0f),
      rotationAngle(0.0f), scaleFactor(1.0f),
      vpXmin(-200.0f), vpXmax(200.0f),
      vpYmin(-200.0f), vpYmax(200.0f),
    clippedSegments(0), totalSegments(0),
    deliveryDistance(0.0f), trafficDistance(0.0f),
    lastDeliveryNode(0), lastTrafficNode(0), trafficDirection(1),
    simulationActive(false), collisionHold(false),
    simulationStatus("READY")
{
    // Initialise grid to empty
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++) {
            gridCells[r][c]  = CELL_EMPTY;
            gridLabels[r][c] = "";
        }

    dronePos   = { gridOriginX + 0.5f * cellSize,
                   gridOriginY - 0.5f * cellSize };
    gfxData    = {0, 0, 0, 1.0, 0};

    cout << "[CG] GraphicsRenderer initialised.\n";
}

// ════════════════════════════════════════════════════════════
// TransformMatrix operations
// [CG CO3 - Task 2: Matrix multiply and point transform]
// ════════════════════════════════════════════════════════════
TransformMatrix TransformMatrix::operator*(
    const TransformMatrix& o) const
{
    TransformMatrix result;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++) {
            result.m[i][j] = 0;
            for (int k = 0; k < 3; k++)
                result.m[i][j] += m[i][k] * o.m[k][j];
        }
    return result;
}

Point2D TransformMatrix::apply(const Point2D& p) const {
    // Homogeneous multiplication: [x' y' 1] = M * [x y 1]^T
    float x = m[0][0]*p.x + m[0][1]*p.y + m[0][2];
    float y = m[1][0]*p.x + m[1][1]*p.y + m[1][2];
    return Point2D(x, y);
}

void TransformMatrix::display(const string& label) const {
    cout << "\n  [CG CO3] Transform Matrix — " << label << ":\n";
    for (int i = 0; i < 3; i++) {
        cout << "  | ";
        for (int j = 0; j < 3; j++) {
            cout << fixed << setprecision(4)
                      << setw(10) << m[i][j] << " ";
        }
        cout << "|\n";
    }
    cout << "\n";
}

// ════════════════════════════════════════════════════════════
// Data Loading
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::loadGrid(
    int cells[GRID_ROWS][GRID_COLS],
    string labels[GRID_ROWS][GRID_COLS],
    int rows, int cols)
{
    if (rows < 1 || rows > GRID_ROWS ||
        cols < 1 || cols > GRID_COLS) {
        throw out_of_range("Active graphics grid dimensions are out of range.");
    }
    activeRows = rows;
    activeCols = cols;
    for (int r = 0; r < GRID_ROWS; r++)
        for (int c = 0; c < GRID_COLS; c++) {
            gridCells[r][c]  = cells[r][c];
            gridLabels[r][c] = labels[r][c];
        }
    cout << "[CG] Grid data loaded.\n";
}

void GraphicsRenderer::loadRoute(
    const vector<Point2D>& points,
    const string& type)
{
    routePoints = points;
    routeType   = type;
    cout << "[CG] Route loaded: " << type
              << " with " << points.size() << " waypoints.\n";
}

void GraphicsRenderer::setDronePosition(
    int row, int col, float angle)
{
    dronePos   = gridToGL(row, col);
    droneAngle = angle;
}

void GraphicsRenderer::setSimulationPaths(
    const vector<pair<int, int>>& deliveryPath,
    const vector<pair<int, int>>& trafficPath)
{
    deliveryGridPath = deliveryPath;
    trafficGridPath = trafficPath;
    routePoints.clear();
    for (const auto& cell : deliveryPath)
        routePoints.push_back(gridToGL(cell.first, cell.second));

    trafficPoints.clear();
    for (const auto& cell : trafficPath)
        trafficPoints.push_back(gridToGL(cell.first, cell.second));

    if (!routePoints.empty()) dronePos = routePoints.front();
    if (!trafficPoints.empty()) trafficPos = trafficPoints.front();
    deliveryDistance = 0.0f;
    trafficDistance = 0.0f;
    lastDeliveryNode = 0;
    lastTrafficNode = 0;
    trafficDirection = 1;
    collisionHold = false;
    simulationActive = routePoints.size() > 1;
    simulationStatus = simulationActive ? "FLYING" : "NO ROUTE";
}

void GraphicsRenderer::setSimulationCallback(
    function<void(int, int, bool, bool)> callback)
{
    simulationCallback = callback;
}

void GraphicsRenderer::setTask3EffectsEnabled(bool enabled) {
    task3EffectsEnabled = enabled;
}

void GraphicsRenderer::toggleTask3Effects() {
    task3EffectsEnabled = !task3EffectsEnabled;
    cout << "[CG CO6 - Task 3] Texture and lighting effects "
         << (task3EffectsEnabled ? "enabled" : "disabled") << ".\n";
}

// ── Coordinate conversion ─────────────────────────────────────
// [CG CO1 - Task 1: grid coords to OpenGL world coords]
Point2D GraphicsRenderer::gridToGL(int row, int col) const {
    float x = gridOriginX + col * cellSize + cellSize * 0.5f;
    float y = gridOriginY - row * cellSize - cellSize * 0.5f;
    return Point2D(x, y);
}

// ════════════════════════════════════════════════════════════
// TASK 1 — PRIMITIVES
// [CG CO1 - Task 1: OpenGL point primitive]
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::drawPoint(
    float x, float y, float size,
    float r, float g, float b)
{
    glPointSize(size);
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
        glVertex2f(x, y);
    glEnd();
}

// [CG CO1 - Task 1: OpenGL line primitive]
void GraphicsRenderer::drawLine(
    float x1, float y1,
    float x2, float y2,
    float r,  float g, float b,
    bool dashed)
{
    if (dashed) {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0xAAAA); // dashed pattern
    }
    glColor3f(r, g, b);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    glEnd();
    if (dashed) glDisable(GL_LINE_STIPPLE);
}

// [CG CO1 - Task 1: OpenGL quad primitive]
void GraphicsRenderer::drawQuad(
    float x, float y,
    float w, float h,
    float r, float g, float b,
    bool filled)
{
    glColor3f(r, g, b);
    glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
        glVertex2f(x,     y    );
        glVertex2f(x + w, y    );
        glVertex2f(x + w, y - h);
        glVertex2f(x,     y - h);
    glEnd();
}

// [CG CO2 - Task 1: OpenGL polygon primitive]
void GraphicsRenderer::drawPolygon(
    const vector<Point2D>& vertices,
    float r, float g, float b,
    bool filled)
{
    glColor3f(r, g, b);
    glBegin(filled ? GL_POLYGON : GL_LINE_LOOP);
    for (const auto& v : vertices)
        glVertex2f(v.x, v.y);
    glEnd();
}

// [CG CO2 - Task 1: Circle using GL_TRIANGLE_FAN]
void GraphicsRenderer::drawCircle(
    float cx, float cy, float radius,
    float r, float g, float b,
    bool filled)
{
    int   segments = 32;
    float angle    = 0.0f;
    float step     = 2.0f * 3.14159265f / segments;

    glColor3f(r, g, b);
    glBegin(filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);
    if (filled) glVertex2f(cx, cy); // centre point for fan
    for (int i = 0; i <= segments; i++) {
        glVertex2f(
            cx + radius * cosf(angle),
            cy + radius * sinf(angle)
        );
        angle += step;
    }
    glEnd();
}

// ════════════════════════════════════════════════════════════
// CELL DRAWING — uses primitives above
// [CG CO1 CO2 - Task 1: Draw each node type]
// ════════════════════════════════════════════════════════════

// [CG CO1 - Warehouse: filled quad + W label point markers]
void GraphicsRenderer::drawWarehouseNode(float cx, float cy) {
    float hs = cellSize * 0.35f;
    // Filled quad background
    drawQuad(cx - hs, cy + hs, 2*hs, 2*hs,
             0.9f, 0.35f, 0.1f, true);
    // Outline
    drawQuad(cx - hs, cy + hs, 2*hs, 2*hs,
             1.0f, 0.6f, 0.2f, false);
    // Corner point markers
    drawPoint(cx - hs, cy + hs, 5.0f, 1.0f, 0.8f, 0.0f);
    drawPoint(cx + hs, cy + hs, 5.0f, 1.0f, 0.8f, 0.0f);
    drawPoint(cx - hs, cy - hs, 5.0f, 1.0f, 0.8f, 0.0f);
    drawPoint(cx + hs, cy - hs, 5.0f, 1.0f, 0.8f, 0.0f);
    // Roof lines (polygon)
    vector<Point2D> roof = {
        {cx - hs*1.1f, cy + hs*0.5f},
        {cx,           cy + hs*1.2f},
        {cx + hs*1.1f, cy + hs*0.5f}
    };
    drawPolygon(roof, 1.0f, 0.5f, 0.1f, true);
}

// [CG CO1 - Customer Site: circle + delivery box]
void GraphicsRenderer::drawCustomerNode(float cx, float cy) {
    float r = cellSize * 0.32f;
    drawCircle(cx, cy, r, 0.0f, 0.5f, 0.8f, true);
    drawCircle(cx, cy, r, 0.0f, 0.8f, 1.0f, false);
    // Inner cross
    drawLine(cx - r*0.5f, cy, cx + r*0.5f, cy,
             1.0f, 1.0f, 1.0f);
    drawLine(cx, cy - r*0.5f, cx, cy + r*0.5f,
             1.0f, 1.0f, 1.0f);
    drawPoint(cx, cy, 6.0f, 0.0f, 1.0f, 1.0f);
}

// [CG CO2 - Charging Pad: hexagon polygon + lightning]
void GraphicsRenderer::drawChargingNode(float cx, float cy) {
    // Hexagon using polygon primitive
    vector<Point2D> hex;
    float r = cellSize * 0.35f;
    for (int i = 0; i < 6; i++) {
        float a = i * 60.0f * 3.14159265f / 180.0f;
        hex.push_back({ cx + r * cosf(a), cy + r * sinf(a) });
    }
    drawPolygon(hex, 0.05f, 0.45f, 0.2f, true);
    drawPolygon(hex, 0.1f,  0.9f,  0.4f, false);
    // Lightning bolt lines
    drawLine(cx - r*0.15f, cy + r*0.55f,
             cx - r*0.25f, cy,
             1.0f, 1.0f, 0.0f);
    drawLine(cx - r*0.25f, cy,
             cx + r*0.15f, cy,
             1.0f, 1.0f, 0.0f);
    drawLine(cx + r*0.15f, cy,
             cx + r*0.05f, cy - r*0.55f,
             1.0f, 1.0f, 0.0f);
}

// [CG CO1 - Obstacle: X pattern using lines + red quad]
void GraphicsRenderer::drawObstacleNode(float cx, float cy) {
    float hs = cellSize * 0.38f;
    drawQuad(cx - hs, cy + hs, 2*hs, 2*hs,
             0.4f, 0.0f, 0.0f, true);
    // X cross lines
    glLineWidth(2.5f);
    drawLine(cx - hs*0.7f, cy + hs*0.7f,
             cx + hs*0.7f, cy - hs*0.7f,
             0.9f, 0.1f, 0.1f);
    drawLine(cx + hs*0.7f, cy + hs*0.7f,
             cx - hs*0.7f, cy - hs*0.7f,
             0.9f, 0.1f, 0.1f);
    glLineWidth(1.5f);
}

// ════════════════════════════════════════════════════════════
// DRONE BODY DRAWING
// [CG CO1 CO2 - Task 1: Drone using lines, quads, polygons]
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::drawDroneBody(
    float cx, float cy, float angle)
{
    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(angle, 0.0f, 0.0f, 1.0f); // rotation transform
    glScalef(droneScale, droneScale, 1.0f); // scale transform

    float armLen = cellSize * 0.38f;
    float armOff = armLen * 0.707f; // cos/sin 45 degrees

    // [CG CO1 - Arms as lines]
    glLineWidth(3.0f);
    glColor3f(0.2f, 0.5f, 0.7f);
    glBegin(GL_LINES);
        glVertex2f(0, 0); glVertex2f(-armOff,  armOff);
        glVertex2f(0, 0); glVertex2f( armOff,  armOff);
        glVertex2f(0, 0); glVertex2f(-armOff, -armOff);
        glVertex2f(0, 0); glVertex2f( armOff, -armOff);
    glEnd();
    glLineWidth(1.5f);

    // [CG CO2 - Rotors as circles]
    float rotorR = armLen * 0.28f;
    float positions[4][2] = {
        {-armOff,  armOff},
        { armOff,  armOff},
        {-armOff, -armOff},
        { armOff, -armOff}
    };
    for (int i = 0; i < 4; i++) {
        drawCircle(positions[i][0], positions[i][1],
                   rotorR, 0.3f, 0.8f, 1.0f, true);
        drawCircle(positions[i][0], positions[i][1],
                   rotorR, 0.0f, 1.0f, 1.0f, false);
        drawPoint(positions[i][0], positions[i][1],
                  4.0f, 1.0f, 1.0f, 1.0f);
    }

    // [CG CO1 - Body as quad]
    float bw = armLen * 0.3f;
    float bh = armLen * 0.2f;
    drawQuad(-bw, bh, 2*bw, 2*bh, 0.1f, 0.15f, 0.25f, true);
    drawQuad(-bw, bh, 2*bw, 2*bh, 0.4f, 0.7f,  1.0f,  false);

    // [CG CO2 - Direction arrow as polygon]
    float arrowH = armLen * 0.35f;
    vector<Point2D> arrow = {
        { 0.0f,         arrowH      },
        {-armLen*0.12f, arrowH*0.55f},
        { armLen*0.12f, arrowH*0.55f}
    };
    drawPolygon(arrow, 0.6f, 0.3f, 1.0f, true);

    // Camera (circle)
    drawCircle(0, -bh * 0.6f, bh * 0.4f,
               0.0f, 0.0f, 0.1f, true);
    drawCircle(0, -bh * 0.6f, bh * 0.4f,
               0.3f, 0.5f, 1.0f, false);

    // LEDs (points)
    drawPoint( bw * 0.6f, 0.0f, 7.0f, 1.0f, 0.0f, 0.0f);
    drawPoint(-bw * 0.6f, 0.0f, 7.0f, 0.0f, 1.0f, 0.3f);

    glPopMatrix();
}

void GraphicsRenderer::drawTrafficDrone() {
    const float arm = cellSize * 0.16f;
    drawLine(trafficPos.x - arm, trafficPos.y,
             trafficPos.x + arm, trafficPos.y,
             1.0f, 0.58f, 0.12f);
    drawLine(trafficPos.x, trafficPos.y - arm,
             trafficPos.x, trafficPos.y + arm,
             1.0f, 0.58f, 0.12f);
    drawCircle(trafficPos.x, trafficPos.y,
               cellSize * 0.11f, 0.9f, 0.34f, 0.12f, true);
    drawCircle(trafficPos.x, trafficPos.y,
               cellSize * 0.11f, 1.0f, 0.78f, 0.34f, false);
}

// ════════════════════════════════════════════════════════════
// DRAW GRID LINES
// [CG CO1 - Task 1: Grid using GL_LINES]
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::drawGridLines() {
    glColor3f(0.1f, 0.18f, 0.28f);
    glLineWidth(0.8f);

    float totalW = activeCols * cellSize;
    float totalH = activeRows * cellSize;

    // Vertical lines
    glBegin(GL_LINES);
    for (int c = 0; c <= activeCols; c++) {
        float x = gridOriginX + c * cellSize;
        glVertex2f(x, gridOriginY);
        glVertex2f(x, gridOriginY - totalH);
    }
    // Horizontal lines
    for (int r = 0; r <= activeRows; r++) {
        float y = gridOriginY - r * cellSize;
        glVertex2f(gridOriginX,          y);
        glVertex2f(gridOriginX + totalW, y);
    }
    glEnd();
    glLineWidth(1.5f);
}

// [CG CO6 - Task 3: procedural texture mapping for the city surface]
void GraphicsRenderer::drawTask3TexturedSurface() {
    if (cityTexture == 0) {
        unsigned char texels[32 * 32 * 3];
        for (int y = 0; y < 32; ++y) {
            for (int x = 0; x < 32; ++x) {
                const bool tile = ((x / 4 + y / 4) % 2) == 0;
                const unsigned char shade = tile ? 55 : 67;
                const int offset = (y * 32 + x) * 3;
                texels[offset] = static_cast<unsigned char>(shade - 10);
                texels[offset + 1] = shade;
                texels[offset + 2] = static_cast<unsigned char>(shade + 18);
            }
        }

        glGenTextures(1, &cityTexture);
        glBindTexture(GL_TEXTURE_2D, cityTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, texels);
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, cityTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    const float right = gridOriginX + activeCols * cellSize;
    const float bottom = gridOriginY - activeRows * cellSize;
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(gridOriginX, gridOriginY);
        glTexCoord2f(static_cast<float>(activeCols), 0.0f);
        glVertex2f(right, gridOriginY);
        glTexCoord2f(static_cast<float>(activeCols),
                     static_cast<float>(activeRows));
        glVertex2f(right, bottom);
        glTexCoord2f(0.0f, static_cast<float>(activeRows));
        glVertex2f(gridOriginX, bottom);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

// [CG CO6 - Task 3: smooth colored light falloff for city landmarks]
void GraphicsRenderer::drawTask3Lighting() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int row = 0; row < activeRows; ++row) {
        for (int col = 0; col < activeCols; ++col) {
            const int type = gridCells[row][col];
            if (type != CELL_WAREHOUSE && type != CELL_CHARGING) continue;

            const Point2D center = gridToGL(row, col);
            const float red = type == CELL_WAREHOUSE ? 0.20f : 0.10f;
            const float green = type == CELL_WAREHOUSE ? 0.58f : 0.62f;
            const float blue = type == CELL_WAREHOUSE ? 1.00f : 0.78f;
            const float radius = cellSize * 1.8f;
            glBegin(GL_TRIANGLE_FAN);
                glColor4f(red, green, blue, 0.30f);
                glVertex2f(center.x, center.y);
                for (int step = 0; step <= 40; ++step) {
                    const float angle =
                        6.28318530718f * static_cast<float>(step) / 40.0f;
                    glColor4f(red, green, blue, 0.0f);
                    glVertex2f(center.x + cosf(angle) * radius,
                               center.y + sinf(angle) * radius);
                }
            glEnd();
        }
    }
    glDisable(GL_BLEND);
}

// ════════════════════════════════════════════════════════════
// DRAW CELL — dispatches to correct node type
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::drawCell(int row, int col) {
    if (gridCells[row][col] == CELL_EMPTY) return;
    Point2D c = gridToGL(row, col);

    switch (gridCells[row][col]) {
        case CELL_WAREHOUSE: drawWarehouseNode(c.x, c.y); break;
        case CELL_CUSTOMER:  drawCustomerNode(c.x, c.y);  break;
        case CELL_CHARGING:  drawChargingNode(c.x, c.y);  break;
        case CELL_OBSTACLE:  drawObstacleNode(c.x, c.y);  break;
    }

    // Draw label below cell using points to simulate text
    // (glutBitmapCharacter for text rendering)
    if (!gridLabels[row][col].empty()) {
        glColor3f(0.6f, 0.8f, 1.0f);
        float tx = c.x - (gridLabels[row][col].size() * 3.5f);
        float ty = c.y - cellSize * 0.42f;
        glRasterPos2f(tx, ty);
        for (char ch : gridLabels[row][col])
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, ch);
    }
}

// ════════════════════════════════════════════════════════════
// DRAW LEGEND
// [CG CO1 - Task 1: Legend using primitives]
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::drawLegend() {
    float lx = gridOriginX + activeCols * cellSize + 20.0f;
    float ly = gridOriginY;

    glColor3f(0.7f, 0.9f, 1.0f);
    glRasterPos2f(lx, ly);
    const char* title = "LEGEND";
    for (const char* c = title; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    struct LegendEntry {
        float r, g, b;
        const char* label;
    };
    LegendEntry entries[] = {
        {0.9f, 0.35f, 0.1f, "Warehouse"},
        {0.0f, 0.5f,  0.8f, "Customer"},
        {0.05f,0.45f, 0.2f, "Charging"},
        {0.4f, 0.0f,  0.0f, "Obstacle"},
        {0.4f, 0.2f,  1.0f, "Drone"},
        {0.0f, 0.6f,  1.0f, "Route"},
    };

    for (int i = 0; i < 6; i++) {
        float ey = ly - 22.0f - i * 22.0f;
        drawQuad(lx, ey + 8.0f, 14.0f, 14.0f,
                 entries[i].r, entries[i].g, entries[i].b, true);
        glColor3f(0.75f, 0.85f, 0.95f);
        glRasterPos2f(lx + 20.0f, ey);
        for (const char* c = entries[i].label; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }
}

// ════════════════════════════════════════════════════════════
// DRAW HUD
// [CG CO2 - Task 1: HUD using lines and quads]
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::drawHUD() {
    float hx = -440.0f;
    float hy =  330.0f;

    // HUD background quad
    drawQuad(hx, hy, 180.0f, 116.0f,
             0.05f, 0.08f, 0.15f, true);
    drawQuad(hx, hy, 180.0f, 116.0f,
             0.0f, 0.4f, 0.8f, false);

    glColor3f(0.0f, 0.8f, 1.0f);

    auto hudText = [](float x, float y, const string& s) {
        glRasterPos2f(x, y);
        for (char c : s)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, c);
    };

    hudText(hx + 6, hy - 16, "AeroRoute v1.0");

    glColor3f(0.6f, 0.8f, 0.6f);
    hudText(hx + 6, hy - 32,
        "TX:" + to_string((int)translateX));
    hudText(hx + 6, hy - 44,
        "TY:" + to_string((int)translateY));
    hudText(hx + 6, hy - 56,
        "RZ:" + to_string((int)rotationAngle) + " deg");
    hudText(hx + 6, hy - 68,
        "SC:" + to_string(scaleFactor).substr(0,4));

    glColor3f(1.0f, 0.6f, 0.2f);
    hudText(hx + 6, hy - 84,
        "Clipped: " + to_string(clippedSegments) +
        "/" + to_string(totalSegments));
    glColor3f(collisionHold ? 1.0f : 0.4f,
              collisionHold ? 0.55f : 0.9f,
              0.35f);
    hudText(hx + 6, hy - 100, "Mission: " + simulationStatus);
}

// ════════════════════════════════════════════════════════════
// TASK 2 — TRANSFORMATIONS
// [CG CO3 - Task 2: Build transformation matrices]
// ════════════════════════════════════════════════════════════

// Translation matrix
//  | 1  0  tx |
//  | 0  1  ty |
//  | 0  0  1  |
TransformMatrix GraphicsRenderer::makeTranslation(
    float tx, float ty)
{
    TransformMatrix T;
    T.m[0][2] = tx;
    T.m[1][2] = ty;
    cout << "[CG CO3] Translation: tx=" << tx
              << " ty=" << ty << "\n";
    T.display("Translation");
    return T;
}

// Rotation matrix (counter-clockwise by angleDeg)
//  | cos(a)  -sin(a)  0 |
//  | sin(a)   cos(a)  0 |
//  |   0        0     1 |
TransformMatrix GraphicsRenderer::makeRotation(float angleDeg) {
    float rad = angleDeg * 3.14159265f / 180.0f;
    float c   = cosf(rad);
    float s   = sinf(rad);
    TransformMatrix R;
    R.m[0][0] =  c;  R.m[0][1] = -s;
    R.m[1][0] =  s;  R.m[1][1] =  c;
    cout << "[CG CO3] Rotation: angle="
              << angleDeg << " deg\n";
    R.display("Rotation");
    return R;
}

// Scaling matrix
//  | sx  0   0 |
//  | 0   sy  0 |
//  | 0   0   1 |
TransformMatrix GraphicsRenderer::makeScaling(
    float sx, float sy)
{
    TransformMatrix S;
    S.m[0][0] = sx;
    S.m[1][1] = sy;
    cout << "[CG CO3] Scaling: sx=" << sx
              << " sy=" << sy << "\n";
    S.display("Scaling");
    return S;
}

// [CG CO3 - Apply composed TRS transform to drone]
void GraphicsRenderer::applyTransform(
    float tx, float ty,
    float angleDeg, float scale)
{
    translateX    = tx;
    translateY    = ty;
    rotationAngle = angleDeg;
    scaleFactor   = scale;
    droneScale    = scale;

    // Build individual matrices
    TransformMatrix T = makeTranslation(tx, ty);
    TransformMatrix R = makeRotation(angleDeg);
    TransformMatrix S = makeScaling(scale, scale);

    // Compose: M = T * R * S
    // Order: Scale first, then rotate, then translate
    TransformMatrix M = T * R * S;
    M.display("Composed TRS (T*R*S)");

    // Transform the delivery route as well so the animated drone remains
    // aligned with its displayed path.
    for (Point2D& point : routePoints) {
        point = M.apply(point);
    }
    if (!routePoints.empty()) {
        dronePos = routePoints.front();
    } else {
        dronePos = M.apply(dronePos);
    }
    droneAngle     = angleDeg;

    cout << "[CG CO3] Drone position after transform: ("
              << dronePos.x << ", " << dronePos.y << ")\n";

    // Update JSON data
    gfxData.translate_x  = tx;
    gfxData.translate_y  = ty;
    gfxData.rotate_deg   = angleDeg;
    gfxData.scale        = scale;
    exportToJson();
}

void GraphicsRenderer::translateDrone(float dx, float dy) {
    dronePos.x += dx;
    dronePos.y += dy;
    translateX  = dx;
    translateY  = dy;
    cout << "[CG CO3] Translate drone: +"
              << dx << ", +" << dy << "\n";
}

void GraphicsRenderer::rotateDroneTo(float targetAngle) {
    droneAngle    = targetAngle;
    rotationAngle = targetAngle;
    cout << "[CG CO3] Rotate drone to: "
              << targetAngle << " deg\n";
}

void GraphicsRenderer::scaleView(float factor) {
    scaleFactor = factor;
    droneScale  = factor;
    cout << "[CG CO3] Scale factor: " << factor << "\n";
}

void GraphicsRenderer::displayTransformInfo() const {
    cout << "\n  [CG CO3] Current Transform State:\n";
    cout << "    Translation: ("
              << translateX << ", " << translateY << ")\n";
    cout << "    Rotation:    "
              << rotationAngle << " degrees\n";
    cout << "    Scale:       "
              << scaleFactor << "\n\n";
}

// ════════════════════════════════════════════════════════════
// TASK 2 — COHEN-SUTHERLAND LINE CLIPPING
// [CG CO5 - Task 2: Full implementation]
// ════════════════════════════════════════════════════════════

void GraphicsRenderer::setViewport(
    float xmin, float xmax,
    float ymin, float ymax)
{
    vpXmin = xmin; vpXmax = xmax;
    vpYmin = ymin; vpYmax = ymax;
    cout << "[CG CO5] Viewport set: "
              << "X[" << xmin << "," << xmax << "] "
              << "Y[" << ymin << "," << ymax << "]\n";
}

// [CG CO5 - Compute region code for a point]
int GraphicsRenderer::computeRegionCode(
    float x, float y) const
{
    int code = CS_INSIDE;
    if      (x < vpXmin) code |= CS_LEFT;
    else if (x > vpXmax) code |= CS_RIGHT;
    if      (y < vpYmin) code |= CS_BOTTOM;
    else if (y > vpYmax) code |= CS_TOP;
    return code;
}

// [CG CO5 - Cohen-Sutherland clipping algorithm]
// Modifies (x1,y1)-(x2,y2) to clipped endpoints.
// Returns false if line is completely outside viewport.
bool GraphicsRenderer::clipLine(
    float& x1, float& y1,
    float& x2, float& y2)
{
    int code1 = computeRegionCode(x1, y1);
    int code2 = computeRegionCode(x2, y2);

    while (true) {

        if (!(code1 | code2)) {
            // Both endpoints inside — accept line
            return true;
        }

        if (code1 & code2) {
            // Both endpoints in same outside region — reject
            return false;
        }

        // At least one endpoint is outside
        // Pick the outside endpoint
        int codeOut = code1 ? code1 : code2;
        float x, y;

        // Find intersection with viewport boundary
        if (codeOut & CS_TOP) {
            // Line crosses top boundary y = vpYmax
            x = x1 + (x2 - x1) * (vpYmax - y1) / (y2 - y1);
            y = vpYmax;
        }
        else if (codeOut & CS_BOTTOM) {
            // Line crosses bottom boundary y = vpYmin
            x = x1 + (x2 - x1) * (vpYmin - y1) / (y2 - y1);
            y = vpYmin;
        }
        else if (codeOut & CS_RIGHT) {
            // Line crosses right boundary x = vpXmax
            y = y1 + (y2 - y1) * (vpXmax - x1) / (x2 - x1);
            x = vpXmax;
        }
        else {
            // Line crosses left boundary x = vpXmin
            y = y1 + (y2 - y1) * (vpXmin - x1) / (x2 - x1);
            x = vpXmin;
        }

        // Update the outside endpoint to intersection point
        if (codeOut == code1) {
            x1    = x; y1    = y;
            code1 = computeRegionCode(x1, y1);
        } else {
            x2    = x; y2    = y;
            code2 = computeRegionCode(x2, y2);
        }
    }
}

// [CG CO5 - Clip a single line and draw if visible]
bool GraphicsRenderer::clipAndDrawLine(
    float x1, float y1,
    float x2, float y2,
    float r,  float g, float b)
{
    totalSegments++;
    float cx1 = x1, cy1 = y1;
    float cx2 = x2, cy2 = y2;

    bool visible = clipLine(cx1, cy1, cx2, cy2);

    if (visible) {
        // Line passes through viewport — draw clipped version
        drawLine(cx1, cy1, cx2, cy2, r, g, b, false);

        // Draw clipping markers if endpoints were moved
        if (cx1 != x1 || cy1 != y1) {
            drawPoint(cx1, cy1, 8.0f, 1.0f, 1.0f, 0.0f);
        }
        if (cx2 != x2 || cy2 != y2) {
            drawPoint(cx2, cy2, 8.0f, 1.0f, 1.0f, 0.0f);
        }
    } else {
        clippedSegments++;
    }

    return visible;
}

// [CG CO5 - Clip and draw entire route path]
void GraphicsRenderer::clipAndDrawRoute() {
    if (routePoints.size() < 2) return;

    clippedSegments = 0;
    totalSegments   = 0;

    for (int i = 0; i < (int)routePoints.size() - 1; i++) {
        float x1 = routePoints[i].x;
        float y1 = routePoints[i].y;
        float x2 = routePoints[i + 1].x;
        float y2 = routePoints[i + 1].y;

        clipAndDrawLine(x1, y1, x2, y2,
                0.0f, 0.8f, 1.0f);
    }

    // For CLL — connect last to first
    if (routeType == "CLL" && routePoints.size() > 1) {
        clipAndDrawLine(
            routePoints.back().x,  routePoints.back().y,
            routePoints.front().x, routePoints.front().y,
            0.0f, 0.8f, 1.0f
        );
    }

    gfxData.clipped_segments = clippedSegments;
}

void GraphicsRenderer::displayClippingInfo() const {
    cout << "\n  [CG CO5] Clipping Results:\n";
    cout << "    Viewport: X["
              << vpXmin << "," << vpXmax << "] Y["
              << vpYmin << "," << vpYmax << "]\n";
    cout << "    Total segments:   " << totalSegments   << "\n";
    cout << "    Clipped out:      " << clippedSegments << "\n";
    cout << "    Visible segments: "
              << (totalSegments - clippedSegments) << "\n\n";
}

// ════════════════════════════════════════════════════════════
// MAIN RENDER — called every frame by GLUT
// [CG CO1 CO2 CO3 CO5 - Full scene render]
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::renderScene() {

    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Apply global view scale (zoom)
    glScalef(scaleFactor, scaleFactor, 1.0f);

    if (task3EffectsEnabled) {
        drawTask3TexturedSurface();
        drawTask3Lighting();
    }

    // Draw grid lines
    drawGridLines();

    // Draw all non-empty cells
    for (int r = 0; r < activeRows; r++)
        for (int c = 0; c < activeCols; c++)
            drawCell(r, c);

    // Draw clipped route path
    clipAndDrawRoute();

    // Draw waypoint markers
    for (int i = 0; i < (int)routePoints.size(); i++) {
        float r  = (i == 0) ? 1.0f : 0.0f;
        float g  = (i == (int)routePoints.size()-1) ? 1.0f : 0.5f;
        float b  = 0.2f;
        drawCircle(routePoints[i].x, routePoints[i].y,
                   5.0f, r, g, b, true);
    }

    // Draw drone
    drawDroneBody(dronePos.x, dronePos.y, droneAngle);
    if (!trafficPoints.empty()) drawTrafficDrone();

    // Draw viewport boundary (clipping window visible)
    glEnable(GL_LINE_STIPPLE);
    glLineStipple(3, 0xF0F0);
    drawQuad(vpXmin, vpYmax,
             vpXmax - vpXmin, vpYmax - vpYmin,
             0.3f, 0.3f, 0.5f, false);
    glDisable(GL_LINE_STIPPLE);

    // Draw HUD and legend
    drawHUD();
    drawLegend();

    glutSwapBuffers();
}

// ════════════════════════════════════════════════════════════
// GLUT CALLBACKS
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::displayCallback() {
    if (g_instance) g_instance->renderScene();
}

void GraphicsRenderer::reshapeCallback(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)w / (float)h;
    gluOrtho2D(-450.0 * aspect, 450.0 * aspect, -350.0, 350.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (g_instance) {
        g_instance->windowWidth  = w;
        g_instance->windowHeight = h;
    }
}

// [CG CO3 - Keyboard: interactive transforms]
void GraphicsRenderer::keyboardCallback(
    unsigned char key, int, int)
{
    GraphicsRenderer* r = g_instance;
    if (!r) return;

    switch (key) {
        // Translation
        case 'w': r->translateDrone(  0,  5); break;
        case 's': r->translateDrone(  0, -5); break;
        case 'a': r->translateDrone( -5,  0); break;
        case 'd': r->translateDrone(  5,  0); break;
        // Rotation
        case 'q': r->rotateDroneTo(r->droneAngle + 15.0f); break;
        case 'e': r->rotateDroneTo(r->droneAngle - 15.0f); break;
        // Scale
        case '+': r->scaleView(r->scaleFactor + 0.1f); break;
        case '-': r->scaleView(r->scaleFactor - 0.1f); break;
        // Clip viewport shrink/grow
        case 'c':
            r->setViewport(r->vpXmin + 10, r->vpXmax - 10,
                           r->vpYmin + 10, r->vpYmax - 10);
            break;
        case 'v':
            r->setViewport(r->vpXmin - 10, r->vpXmax + 10,
                           r->vpYmin - 10, r->vpYmax + 10);
            break;
        // Export JSON
        case 'j':
            r->exportToJson();
            break;
        // [CG CO6 - Task 3: toggle surface texture and colored lighting]
        case 't':
        case 'T':
            r->toggleTask3Effects();
            break;
        // Quit
        case 27: // ESC
            cout << "[CG] Window closed.\n";
            glutLeaveMainLoop();
    }

    r->exportToJson();
    glutPostRedisplay();
}

void GraphicsRenderer::timerCallback(int) {
    if (!g_instance) return;
    g_instance->updateSimulation();
    glutPostRedisplay();
    glutTimerFunc(33, timerCallback, 0); // ~30fps
}

float GraphicsRenderer::pathLength(const vector<Point2D>& points) const {
    float length = 0.0f;
    for (size_t index = 1; index < points.size(); ++index) {
        const float dx = points[index].x - points[index - 1].x;
        const float dy = points[index].y - points[index - 1].y;
        length += sqrtf(dx * dx + dy * dy);
    }
    return length;
}

Point2D GraphicsRenderer::pointAlongPath(
    const vector<Point2D>& points, float distance) const
{
    if (points.empty()) return Point2D();
    if (distance <= 0.0f) return points.front();

    for (size_t index = 1; index < points.size(); ++index) {
        const Point2D& start = points[index - 1];
        const Point2D& end = points[index];
        const float dx = end.x - start.x;
        const float dy = end.y - start.y;
        const float segmentLength = sqrtf(dx * dx + dy * dy);
        if (distance <= segmentLength) {
            const float ratio = segmentLength == 0.0f
                ? 1.0f : distance / segmentLength;
            return Point2D(start.x + dx * ratio,
                           start.y + dy * ratio);
        }
        distance -= segmentLength;
    }
    return points.back();
}

void GraphicsRenderer::updateSimulation() {
    if (!simulationActive) return;

    const float deltaSeconds = 0.033f;
    const bool hasTrafficRoute = trafficPoints.size() > 1;
    float nextTrafficDistance = trafficDistance;
    int nextTrafficDirection = trafficDirection;
    Point2D nextTrafficPosition = trafficPos;
    bool trafficCanMove = true;
    if (hasTrafficRoute) {
        const float trafficLength = pathLength(trafficPoints);
        nextTrafficDistance =
            trafficDistance + trafficDirection * cellSize * deltaSeconds;
        if (nextTrafficDistance >= trafficLength) {
            nextTrafficDistance = trafficLength;
            nextTrafficDirection = -1;
        } else if (nextTrafficDistance <= 0.0f) {
            nextTrafficDistance = 0.0f;
            nextTrafficDirection = 1;
        }
        nextTrafficPosition =
            pointAlongPath(trafficPoints, nextTrafficDistance);
    }

    const float deliveryLength = pathLength(routePoints);
    const float nextDistance = min(
        deliveryLength, deliveryDistance + cellSize * 0.8f * deltaSeconds);
    const Point2D nextPosition = pointAlongPath(routePoints, nextDistance);
    const auto separated = [this](const Point2D& first,
                                  const Point2D& second) {
        const float dx = first.x - second.x;
        const float dy = first.y - second.y;
        return sqrtf(dx * dx + dy * dy) >= cellSize * 0.62f;
    };

    if (hasTrafficRoute) {
        trafficCanMove = separated(dronePos, nextTrafficPosition);
        if (!trafficCanMove) {
            nextTrafficDistance = trafficDistance;
            nextTrafficDirection = trafficDirection;
            nextTrafficPosition = trafficPos;
        }
    }
    const bool deliveryCanMove =
        !hasTrafficRoute || separated(nextPosition, nextTrafficPosition);
    collisionHold = hasTrafficRoute &&
                    (!trafficCanMove || !deliveryCanMove);

    if (hasTrafficRoute && trafficCanMove) {
        trafficDistance = nextTrafficDistance;
        trafficDirection = nextTrafficDirection;
        trafficPos = nextTrafficPosition;
        const int trafficNode = min(
            static_cast<int>(trafficGridPath.size()) - 1,
            static_cast<int>(trafficDistance / cellSize));
        if (trafficNode != lastTrafficNode) {
            lastTrafficNode = trafficNode;
            if (simulationCallback) {
                simulationCallback(
                    trafficGridPath[trafficNode].first,
                    trafficGridPath[trafficNode].second,
                    true, false);
            }
        }
    }

    if (!deliveryCanMove) {
        simulationStatus = "YIELDING";
        return;
    }

    const Point2D previousPosition = dronePos;
    deliveryDistance = nextDistance;
    dronePos = nextPosition;
    droneAngle = atan2f(dronePos.y - previousPosition.y,
                        dronePos.x - previousPosition.x) *
                 180.0f / 3.14159265f;
    simulationStatus = "FLYING";

    const float routeProgress = deliveryLength > 0.0f
        ? deliveryDistance / deliveryLength : 1.0f;
    const int deliveryNode = min(
        static_cast<int>(deliveryGridPath.size()) - 1,
        static_cast<int>(routeProgress *
                         (deliveryGridPath.size() - 1) + 0.0001f));
    while (lastDeliveryNode < deliveryNode) {
        ++lastDeliveryNode;
        const bool delivered = lastDeliveryNode ==
            static_cast<int>(deliveryGridPath.size()) - 1;
        if (simulationCallback) {
            simulationCallback(
                deliveryGridPath[lastDeliveryNode].first,
                deliveryGridPath[lastDeliveryNode].second,
                false, delivered);
        }
    }

    if (deliveryDistance >= deliveryLength) {
        simulationActive = false;
        simulationStatus = "DELIVERED";
    }
}

// ── Accessors ─────────────────────────────────────────────────
float GraphicsRenderer::getTranslateX()    const { return translateX; }
float GraphicsRenderer::getTranslateY()    const { return translateY; }
float GraphicsRenderer::getRotationAngle() const { return rotationAngle; }
float GraphicsRenderer::getScaleFactor()   const { return scaleFactor; }
int   GraphicsRenderer::getClippedCount()  const { return clippedSegments; }

// ════════════════════════════════════════════════════════════
// JSON EXPORT
// Writes graphics state to data/mission.json graphics field
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::exportToJson() const {
    // We write graphics data to mission.json
    // Entities vector is empty here — PSOOP fills that
    vector<EntityData> emptyEntities;
    GraphicsData gd = {
        translateX, translateY,
        rotationAngle, scaleFactor,
        clippedSegments
    };
    writeMissionJson(emptyEntities, gd);
    cout << "[CG] Exported graphics data to mission.json\n";
}

// ════════════════════════════════════════════════════════════
// LAUNCH GLUT WINDOW
// ════════════════════════════════════════════════════════════
void GraphicsRenderer::launchWindow(
    int argc, char** argv,
    const string& title)
{
    GraphicsRenderer::setInstance(this);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
                  GLUT_ACTION_GLUTMAINLOOP_RETURNS);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(title.c_str());

    // Background colour — dark navy
    glClearColor(0.03f, 0.05f, 0.10f, 1.0f);

    // Register callbacks
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutTimerFunc(33, timerCallback, 0);

    cout << "\n[CG] OpenGL window launched.\n";
    cout << "  Controls:\n";
    cout << "  W/A/S/D  — translate drone\n";
    cout << "  Q/E      — rotate drone\n";
    cout << "  +/-      — zoom in/out\n";
    cout << "  C/V      — shrink/grow clip viewport\n";
    cout << "  T        — toggle Task 3 surface texture and colored lights\n";
    cout << "  J        — export JSON now\n";
    cout << "  ESC      — close window\n\n";

    glutMainLoop();
    const int windowId = glutGetWindow();
    if (windowId != 0) glutDestroyWindow(windowId);
    GraphicsRenderer::setInstance(nullptr);
}