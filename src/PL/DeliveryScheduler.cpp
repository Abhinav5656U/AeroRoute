#include "DeliveryScheduler.h"

#include <iostream>

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
    std::cout << "  Pending deliveries (front -> rear): ";
    if (!head) {
        std::cout << "empty\n";
        return;
    }

    const Node* current = head;
    while (current) {
        std::cout << "[" << current->delivery.droneSerial << " -> "
                  << current->delivery.destination << "]";
        if (current->next) std::cout << " -> ";
        current = current->next;
    }
    std::cout << "\n";
}

LinkedListCompletedStack::LinkedListCompletedStack()
    : top(nullptr), size(0) {}

LinkedListCompletedStack::~LinkedListCompletedStack() {
    std::string droneSerial;
    while (pop(droneSerial)) {}
}

void LinkedListCompletedStack::push(const std::string& droneSerial) {
    Node* node = new Node(droneSerial);
    node->next = top;
    top = node;
    ++size;
}

bool LinkedListCompletedStack::pop(std::string& droneSerial) {
    if (!top) return false;

    Node* removed = top;
    droneSerial = removed->droneSerial;
    top = top->next;
    delete removed;
    --size;
    return true;
}

bool LinkedListCompletedStack::peek(std::string& droneSerial) const {
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
    std::cout << "  Completed drones (top -> bottom): ";
    if (!top) {
        std::cout << "empty\n";
        return;
    }

    const Node* current = top;
    while (current) {
        std::cout << "[" << current->droneSerial << "]";
        if (current->next) std::cout << " -> ";
        current = current->next;
    }
    std::cout << "\n";
}

bool DeliveryScheduler::enqueueDelivery(const std::string& droneSerial,
                                        const std::string& destination) {
    if (droneSerial.empty() || destination.empty()) {
        std::cout << "  Cannot queue a delivery without a drone and destination.\n";
        return false;
    }

    pendingDeliveries.enqueue({droneSerial, destination});
    std::cout << "  Queued " << droneSerial << " for " << destination << ".\n";
    return true;
}

bool DeliveryScheduler::completeNextDelivery() {
    DeliveryRecord completed;
    if (!pendingDeliveries.dequeue(completed)) {
        std::cout << "  No pending delivery to complete.\n";
        return false;
    }

    completedDrones.push(completed.droneSerial);
    std::cout << "  Delivery to " << completed.destination << " completed; "
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