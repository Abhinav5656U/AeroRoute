/*
 *
 * FILE        : RouteTracker.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 2 — Dynamic Route Tracking
 * COURSE OUT  : CO2
 *
 */

#include "RouteTracker.h"
#include <iostream>
using namespace std;

// ════════════════════════════════════════════════════════════
// SINGLY LINKED LIST IMPLEMENTATION
// [PL CO2 - Task 2: SLL — Forward delivery path]
// ════════════════════════════════════════════════════════════

SinglyLinkedList::SinglyLinkedList()
    : head(nullptr), tail(nullptr), size(0), currentIndex(0) {}

SinglyLinkedList::~SinglyLinkedList() {
    // Free all nodes
    WaypointNode* cur = head;
    while (cur) {
        WaypointNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
}

// [PL CO2 - SLL: Add waypoint at tail]
void SinglyLinkedList::addWaypoint(int row, int col,
                                   const string& label) {
    WaypointNode* node = new WaypointNode(row, col, label);
    if (!head) {
        head = tail = node;
    } else {
        tail->next = node;
        tail       = node;
    }
    size++;
    cout << "[SLL] Added waypoint: " << label
              << " (" << row << "," << col << ") "
              << "| Size: " << size << "\n";
}

// [PL CO2 - SLL: Remove first node]
bool SinglyLinkedList::removeFirst() {
    if (!head) return false;
    WaypointNode* old = head;
    head = head->next;
    if (!head) tail = nullptr;
    cout << "[SLL] Removed: " << old->label << "\n";
    delete old;
    size--;
    if (currentIndex > 0) currentIndex--;
    return true;
}

// [PL CO2 - SLL: Remove last node]
bool SinglyLinkedList::removeLast() {
    if (!head) return false;
    if (head == tail) {
        delete head;
        head = tail = nullptr;
        size--;
        return true;
    }
    WaypointNode* cur = head;
    while (cur->next != tail) cur = cur->next;
    cout << "[SLL] Removed: " << tail->label << "\n";
    delete tail;
    tail = cur;
    tail->next = nullptr;
    size--;
    return true;
}

// [PL CO2 - SLL: Display forward traversal]
void SinglyLinkedList::displayForward() const {
    cout << "\n  [SLL] Forward Path:\n  HEAD -> ";
    WaypointNode* cur = head;
    int idx = 0;
    while (cur) {
        if (idx == currentIndex)
            cout << "[*" << cur->label << "*]";
        else
            cout << "[" << cur->label << "]";
        if (cur->next) cout << " -> ";
        cur = cur->next;
        idx++;
    }
    cout << " -> NULL\n\n";
}

// [PL CO2 - SLL: Advance drone to next waypoint]
void SinglyLinkedList::advanceCurrent() {
    if (currentIndex < size - 1) currentIndex++;
}

vector<WaypointData> SinglyLinkedList::toVector() const {
    vector<WaypointData> v;
    WaypointNode* cur = head;
    while (cur) {
        v.push_back({ cur->row, cur->col, cur->label });
        cur = cur->next;
    }
    return v;
}

int SinglyLinkedList::getSize()    const { return size; }
int SinglyLinkedList::getCurrent() const { return currentIndex; }
WaypointNode* SinglyLinkedList::getHead() const { return head; }

// ════════════════════════════════════════════════════════════
// DOUBLY LINKED LIST IMPLEMENTATION
// [PL CO2 - Task 2: DLL — Return path navigation]
// ════════════════════════════════════════════════════════════

DoublyLinkedList::DoublyLinkedList()
    : head(nullptr), tail(nullptr), size(0),
      currentIndex(0), goingForward(true) {}

DoublyLinkedList::~DoublyLinkedList() {
    WaypointNode* cur = head;
    while (cur) {
        WaypointNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
}

// [PL CO2 - DLL: Add waypoint (maintains prev pointers)]
void DoublyLinkedList::addWaypoint(int row, int col,
                                   const string& label) {
    WaypointNode* node = new WaypointNode(row, col, label);
    if (!head) {
        head = tail = node;
    } else {
        node->prev = tail;
        tail->next = node;
        tail       = node;
    }
    size++;
    cout << "[DLL] Added waypoint: " << label
              << " (" << row << "," << col << ") "
              << "| Size: " << size << "\n";
}

// [PL CO2 - DLL: Display forward head to tail]
void DoublyLinkedList::displayForward() const {
    cout << "\n  [DLL] Forward (head -> tail):\n  NULL <- ";
    WaypointNode* cur = head;
    int idx = 0;
    while (cur) {
        if (idx == currentIndex)
            cout << "[*" << cur->label << "*]";
        else
            cout << "[" << cur->label << "]";
        if (cur->next) cout << " <-> ";
        cur = cur->next;
        idx++;
    }
    cout << " -> NULL\n";
}

// [PL CO2 - DLL: Display backward tail to head]
void DoublyLinkedList::displayBackward() const {
    cout << "\n  [DLL] Return path (tail -> head):\n  NULL <- ";
    WaypointNode* cur = tail;
    while (cur) {
        cout << "[" << cur->label << "]";
        if (cur->prev) cout << " <-> ";
        cur = cur->prev;
    }
    cout << " -> NULL\n\n";
}

// [PL CO2 - DLL: Advance drone, direction aware]
void DoublyLinkedList::advanceCurrent() {
    if (goingForward) {
        if (currentIndex < size - 1) currentIndex++;
        else { goingForward = false; currentIndex--; }
    } else {
        if (currentIndex > 0) currentIndex--;
        else { goingForward = true; currentIndex++; }
    }
}

void DoublyLinkedList::reversDirection() {
    goingForward = !goingForward;
}

vector<WaypointData> DoublyLinkedList::toVector() const {
    vector<WaypointData> v;
    WaypointNode* cur = head;
    while (cur) {
        v.push_back({ cur->row, cur->col, cur->label });
        cur = cur->next;
    }
    return v;
}

int DoublyLinkedList::getSize()    const { return size; }
int DoublyLinkedList::getCurrent() const { return currentIndex; }

// ════════════════════════════════════════════════════════════
// CIRCULAR LINKED LIST IMPLEMENTATION
// [PL CO2 - Task 2: CLL — Patrol zone cycling]
// ════════════════════════════════════════════════════════════

CircularLinkedList::CircularLinkedList()
    : head(nullptr), current(nullptr),
      size(0), currentIndex(0) {}

CircularLinkedList::~CircularLinkedList() {
    if (!head) return;
    // Break the circle before freeing
    WaypointNode* tail = head;
    while (tail->next != head) tail = tail->next;
    tail->next = nullptr;

    WaypointNode* cur = head;
    while (cur) {
        WaypointNode* nxt = cur->next;
        delete cur;
        cur = nxt;
    }
}

// [PL CO2 - CLL: Add waypoint, maintain circular link]
void CircularLinkedList::addWaypoint(int row, int col,
                                     const string& label) {
    WaypointNode* node = new WaypointNode(row, col, label);
    if (!head) {
        head    = node;
        current = node;
        node->next = node; // points to itself
    } else {
        // Find tail (last node pointing to head)
        WaypointNode* tail = head;
        while (tail->next != head) tail = tail->next;
        tail->next = node;
        node->next = head; // maintain circle
    }
    size++;
    cout << "[CLL] Added patrol waypoint: " << label
              << " (" << row << "," << col << ") "
              << "| Size: " << size << "\n";
}

// [PL CO2 - CLL: Display circular traversal]
void CircularLinkedList::displayCircular() const {
    if (!head) return;
    cout << "\n  [CLL] Patrol Route (circular):\n  ";
    WaypointNode* cur = head;
    int idx = 0;
    do {
        if (idx == currentIndex)
            cout << "[*" << cur->label << "*]";
        else
            cout << "[" << cur->label << "]";
        cout << " -> ";
        cur = cur->next;
        idx++;
    } while (cur != head);
    cout << "(back to head)\n\n";
}

// [PL CO2 - CLL: Advance to next patrol point, wraps to head]
void CircularLinkedList::advanceCurrent() {
    if (!current) return;
    current = current->next;
    currentIndex = (currentIndex + 1) % size;
}

vector<WaypointData> CircularLinkedList::toVector() const {
    vector<WaypointData> v;
    if (!head) return v;
    WaypointNode* cur = head;
    do {
        v.push_back({ cur->row, cur->col, cur->label });
        cur = cur->next;
    } while (cur != head);
    return v;
}

int CircularLinkedList::getSize()    const { return size; }
int CircularLinkedList::getCurrent() const { return currentIndex; }

// ════════════════════════════════════════════════════════════
// ROUTE TRACKER IMPLEMENTATION
// ════════════════════════════════════════════════════════════

RouteTracker::RouteTracker() : activeRouteType(ROUTE_SLL) {}

void RouteTracker::setRouteType(const string& type) {
    activeRouteType = type;
    cout << "[RouteTracker] Route type set to: "
              << type << "\n";
}

void RouteTracker::addWaypoint(int row, int col,
                               const string& label) {
    if (activeRouteType == ROUTE_SLL) sll.addWaypoint(row,col,label);
    else if (activeRouteType == ROUTE_DLL) dll.addWaypoint(row,col,label);
    else if (activeRouteType == ROUTE_CLL) cll.addWaypoint(row,col,label);
}

void RouteTracker::advanceDrone() {
    if (activeRouteType == ROUTE_SLL) sll.advanceCurrent();
    else if (activeRouteType == ROUTE_DLL) dll.advanceCurrent();
    else if (activeRouteType == ROUTE_CLL) cll.advanceCurrent();
}

void RouteTracker::displayRoute() const {
    if (activeRouteType == ROUTE_SLL) sll.displayForward();
    else if (activeRouteType == ROUTE_DLL) {
        dll.displayForward();
        dll.displayBackward();
    }
    else if (activeRouteType == ROUTE_CLL) cll.displayCircular();
}

// [PL CO2 - Export route to JSON for website]
void RouteTracker::exportToJson() const {
    vector<WaypointData> wps;
    int currentIdx = 0;

    if (activeRouteType == ROUTE_SLL) {
        wps        = sll.toVector();
        currentIdx = sll.getCurrent();
    } else if (activeRouteType == ROUTE_DLL) {
        wps        = dll.toVector();
        currentIdx = dll.getCurrent();
    } else if (activeRouteType == ROUTE_CLL) {
        wps        = cll.toVector();
        currentIdx = cll.getCurrent();
    }

    writeRouteJson(activeRouteType, wps, currentIdx);
    cout << "[RouteTracker] Exported to route.json\n";
}

string RouteTracker::getActiveType() const {
    return activeRouteType;
}