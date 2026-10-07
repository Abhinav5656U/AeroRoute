/*
 *
 * FILE        : DeliveryScheduler.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 3 — Delivery Scheduling
 * COURSE OUT  : CO3 — Process queued deliveries and track
 *               completed drones using linked-list structures.
 *
 */

#include "DeliveryScheduler.h"

#include <iostream>
using namespace std;

LinkedListDeliveryQueue::LinkedListDeliveryQueue()
    : head(nullptr), tail(nullptr), size(0) {}

LinkedListDeliveryQueue::~LinkedListDeliveryQueue() {
    DeliveryRecord delivery;
    while (dequeue(delivery)) {}
}

bool LinkedListDeliveryQueue::enqueue(const DeliveryRecord& delivery) {
    Node* node = new Node(delivery);
    if (!tail) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
    ++size;
    return true;
}

bool LinkedListDeliveryQueue::dequeue(DeliveryRecord& delivery) {
    if (!head) return false;

    Node* removed = head;
    delivery = removed->delivery;
    head = head->next;
    if (!head) tail = nullptr;
    delete removed;
    --size;
    return true;
}

bool LinkedListDeliveryQueue::isEmpty() const {
    return head == nullptr;
}

int LinkedListDeliveryQueue::getSize() const {
    return size;
}

void LinkedListDeliveryQueue::display() const {
    cout << "  Pending deliveries (front -> rear): ";
    if (!head) {
        cout << "empty\n";
        return;
    }

    const Node* current = head;
    while (current) {
        cout << "[" << current->delivery.droneSerial << " -> "
                  << current->delivery.destination << "]";
        if (current->next) cout << " -> ";
        current = current->next;
    }
    cout << "\n";
}

LinkedListCompletedStack::LinkedListCompletedStack()
    : top(nullptr), size(0) {}

LinkedListCompletedStack::~LinkedListCompletedStack() {
    string droneSerial;
    while (pop(droneSerial)) {}
}

void LinkedListCompletedStack::push(const string& droneSerial) {
    Node* node = new Node(droneSerial);
    node->next = top;
    top = node;
    ++size;
}

bool LinkedListCompletedStack::pop(string& droneSerial) {
    if (!top) return false;

    Node* removed = top;
    droneSerial = removed->droneSerial;
    top = top->next;
    delete removed;
    --size;
    return true;
}

bool LinkedListCompletedStack::peek(string& droneSerial) const {
    if (!top) return false;
    droneSerial = top->droneSerial;
    return true;
}

bool LinkedListCompletedStack::isEmpty() const {
    return top == nullptr;
}

int LinkedListCompletedStack::getSize() const {
    return size;
}

void LinkedListCompletedStack::display() const {
    cout << "  Completed drones (top -> bottom): ";
    if (!top) {
        cout << "empty\n";
        return;
    }

    const Node* current = top;
    while (current) {
        cout << "[" << current->droneSerial << "]";
        if (current->next) cout << " -> ";
        current = current->next;
    }
    cout << "\n";
}

bool DeliveryScheduler::enqueueDelivery(const string& droneSerial,
                                        const string& destination) {
    if (droneSerial.empty() || destination.empty()) {
        cout << "  Cannot queue a delivery without a drone and destination.\n";
        return false;
    }

    pendingDeliveries.enqueue({droneSerial, destination});
    cout << "  Queued " << droneSerial << " for " << destination << ".\n";
    return true;
}

bool DeliveryScheduler::completeNextDelivery() {
    DeliveryRecord completed;
    if (!pendingDeliveries.dequeue(completed)) {
        cout << "  No pending delivery to complete.\n";
        return false;
    }

    completedDrones.push(completed.droneSerial);
    cout << "  Delivery to " << completed.destination << " completed; "
              << completed.droneSerial
              << " removed from the queue and added to the completed stack.\n";
    return true;
}

void DeliveryScheduler::displayPendingDeliveries() const {
    pendingDeliveries.display();
}

void DeliveryScheduler::displayCompletedDrones() const {
    completedDrones.display();
}

int DeliveryScheduler::getPendingCount() const {
    return pendingDeliveries.getSize();
}

int DeliveryScheduler::getCompletedCount() const {
    return completedDrones.getSize();
}