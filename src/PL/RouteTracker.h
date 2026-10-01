/*
 * ============================================================
 * FILE        : RouteTracker.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 2 — Dynamic Route Tracking
 * COURSE OUT  : CO2 — Implement SLL DLL CLL for route tracking
 * ============================================================
 */

#ifndef ROUTE_TRACKER_H
#define ROUTE_TRACKER_H

#include <string>
#include <vector>
#include "../shared/Constants.h"
#include "../shared/JsonWriter.h"

// ════════════════════════════════════════════════════════════
// [PL CO2 - Task 2: Waypoint Node]
// Used as the node type for all three linked list variants.
// Each node stores a grid position and a descriptive label.
// ════════════════════════════════════════════════════════════
struct WaypointNode {
    int         row;
    int         col;
    std::string label;

    // Singly linked — forward pointer only
    WaypointNode* next;

    // Doubly linked — also has a back pointer
    WaypointNode* prev;

    // Constructor
    WaypointNode(int r, int c, const std::string& lbl)
        : row(r), col(c), label(lbl), next(nullptr), prev(nullptr) {}
};

// ════════════════════════════════════════════════════════════
// [PL CO2 - Task 2: Singly Linked List]
// Models the FORWARD delivery path.
// Drone traverses: head → node1 → node2 → ... → tail
// ════════════════════════════════════════════════════════════
class SinglyLinkedList {
private:
    WaypointNode* head;
    WaypointNode* tail;
    int           size;
    int           currentIndex;

public:
    SinglyLinkedList();
    ~SinglyLinkedList();

    void addWaypoint(int row, int col, const std::string& label);
    bool removeFirst();
    bool removeLast();
    void displayForward()  const;
    void advanceCurrent();

    // For JSON export
    std::vector<WaypointData> toVector()   const;
    int                       getSize()    const;
    int                       getCurrent() const;
    WaypointNode*             getHead()    const;
};

// ════════════════════════════════════════════════════════════
// [PL CO2 - Task 2: Doubly Linked List]
// Models the RETURN delivery path.
// Drone can traverse both directions:
// Forward:  head → ... → tail
// Backward: tail → ... → head
// ════════════════════════════════════════════════════════════
class DoublyLinkedList {
private:
    WaypointNode* head;
    WaypointNode* tail;
    int           size;
    int           currentIndex;
    bool          goingForward;

public:
    DoublyLinkedList();
    ~DoublyLinkedList();

    void addWaypoint(int row, int col, const std::string& label);
    void displayForward()  const;
    void displayBackward() const;
    void advanceCurrent();
    void reversDirection();

    std::vector<WaypointData> toVector()   const;
    int                       getSize()    const;
    int                       getCurrent() const;
};

// ════════════════════════════════════════════════════════════
// [PL CO2 - Task 2: Circular Linked List]
// Models PATROL zone cycling.
// After reaching tail, next points back to head — infinite loop.
// ════════════════════════════════════════════════════════════
class CircularLinkedList {
private:
    WaypointNode* head;
    WaypointNode* current; // tracks current patrol position
    int           size;
    int           currentIndex;

public:
    CircularLinkedList();
    ~CircularLinkedList();

    void addWaypoint(int row, int col, const std::string& label);
    void displayCircular() const;
    void advanceCurrent();  // moves to next, wraps around

    std::vector<WaypointData> toVector()   const;
    int                       getSize()    const;
    int                       getCurrent() const;
};

// ════════════════════════════════════════════════════════════
// [PL CO2 - Task 2: RouteTracker]
// Manages all three linked list types.
// Active list is selected by routeType string.
// Exports route data to JSON for the website.
// ════════════════════════════════════════════════════════════
class RouteTracker {
private:
    SinglyLinkedList  sll;
    DoublyLinkedList  dll;
    CircularLinkedList cll;
    std::string       activeRouteType;

public:
    RouteTracker();

    void setRouteType(const std::string& type); // "SLL" "DLL" "CLL"
    void addWaypoint(int row, int col, const std::string& label);
    void advanceDrone();
    void displayRoute() const;
    void exportToJson() const;

    std::string getActiveType() const;
};

#endif // ROUTE_TRACKER_H