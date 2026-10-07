/*
 *
 * FILE        : DeliveryScheduler.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : Programming Lab | Course Code: 2310214L
 * TASK        : Task 3 — Delivery Scheduling
 * COURSE OUT  : CO3 — Implement a linked-list queue for pending
 *               deliveries and a linked-list stack for completed drones.
 *
 */

#ifndef DELIVERY_SCHEDULER_H
#define DELIVERY_SCHEDULER_H

#include <string>
using namespace std;

struct DeliveryRecord {
    string droneSerial;
    string destination;
};

class LinkedListDeliveryQueue {
private:
    struct Node {
        DeliveryRecord delivery;
        Node* next;

        explicit Node(const DeliveryRecord& record)
            : delivery(record), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int size;

public:
    LinkedListDeliveryQueue();
    ~LinkedListDeliveryQueue();

    LinkedListDeliveryQueue(const LinkedListDeliveryQueue&) = delete;
    LinkedListDeliveryQueue& operator=(const LinkedListDeliveryQueue&) = delete;

    bool enqueue(const DeliveryRecord& delivery);
    bool dequeue(DeliveryRecord& delivery);
    bool isEmpty() const;
    int getSize() const;
    void display() const;
};

class LinkedListCompletedStack {
private:
    struct Node {
        string droneSerial;
        Node* next;

        explicit Node(const string& serial)
            : droneSerial(serial), next(nullptr) {}
    };

    Node* top;
    int size;

public:
    LinkedListCompletedStack();
    ~LinkedListCompletedStack();

    LinkedListCompletedStack(const LinkedListCompletedStack&) = delete;
    LinkedListCompletedStack& operator=(const LinkedListCompletedStack&) = delete;

    void push(const string& droneSerial);
    bool pop(string& droneSerial);
    bool peek(string& droneSerial) const;
    bool isEmpty() const;
    int getSize() const;
    void display() const;
};

class DeliveryScheduler {
private:
    LinkedListDeliveryQueue pendingDeliveries;
    LinkedListCompletedStack completedDrones;

public:
    bool enqueueDelivery(const string& droneSerial,
                         const string& destination);
    bool completeNextDelivery();
    void displayPendingDeliveries() const;
    void displayCompletedDrones() const;
    int getPendingCount() const;
    int getCompletedCount() const;
};

#endif