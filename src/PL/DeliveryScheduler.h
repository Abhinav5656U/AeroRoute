#ifndef DELIVERY_SCHEDULER_H
#define DELIVERY_SCHEDULER_H

#include <string>

struct DeliveryRecord {
    std::string droneSerial;
    std::string destination;
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
        std::string droneSerial;
        Node* next;

        explicit Node(const std::string& serial)
            : droneSerial(serial), next(nullptr) {}
    };

    Node* top;
    int size;

public:
    LinkedListCompletedStack();
    ~LinkedListCompletedStack();

    LinkedListCompletedStack(const LinkedListCompletedStack&) = delete;
    LinkedListCompletedStack& operator=(const LinkedListCompletedStack&) = delete;

    void push(const std::string& droneSerial);
    bool pop(std::string& droneSerial);
    bool peek(std::string& droneSerial) const;
    bool isEmpty() const;
    int getSize() const;
    void display() const;
};

class DeliveryScheduler {
private:
    LinkedListDeliveryQueue pendingDeliveries;
    LinkedListCompletedStack completedDrones;

public:
    bool enqueueDelivery(const std::string& droneSerial,
                         const std::string& destination);
    bool completeNextDelivery();
    void displayPendingDeliveries() const;
    void displayCompletedDrones() const;
    int getPendingCount() const;
    int getCompletedCount() const;
};

#endif