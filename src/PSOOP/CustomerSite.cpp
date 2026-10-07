/*
 *
 * FILE        : CustomerSite.cpp
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 CO1 | Task 2 CO2
 *
 */

#include "CustomerSite.h"
using namespace std;

// [PSOOP CO1 - Task 1: Base class init via initialiser list]
CustomerSite::CustomerSite(int row, int col,
                           const string& label,
                           const string& custName,
                           const string& addr,
                           bool signature)
    : Location(row, col, label, CELL_CUSTOMER),
      customerName(custName),
      address(addr),
      pendingDeliveries(0),
      completedDeliveries(0),
      requiresSignature(signature)
{
    cout << "[PSOOP] CustomerSite constructed: "
              << label << "\n";
}

CustomerSite::~CustomerSite() {
    cout << "[PSOOP] CustomerSite destructed: "
              << getLabel() << "\n";
}

// [PSOOP CO2 - Task 2: Polymorphic display override]
void CustomerSite::display() const {
    cout << "\n  ┌─────────────────────────────┐\n";
    cout << "  │  CUSTOMER SITE              │\n";
    cout << "  └─────────────────────────────┘\n";
    displayBase();
    cout << "  Customer : " << customerName       << "\n";
    cout << "  Address  : " << address            << "\n";
    cout << "  Pending  : " << pendingDeliveries  << "\n";
    cout << "  Delivered: " << completedDeliveries<< "\n";
    cout << "  Signature: "
              << (requiresSignature ? "Required" : "Not Required")
              << "\n";
}

string CustomerSite::getTypeName()  const { return "CustomerSite"; }
int         CustomerSite::getTypeCode()  const { return CELL_CUSTOMER; }

// [PSOOP CO2 - Task 2: Cannot dock if signature required
//  and no completed deliveries yet as example logic]
bool CustomerSite::canDroneDock() const {
    return getIsActive();
}

void CustomerSite::receiveDelivery() {
    if (pendingDeliveries > 0) {
        pendingDeliveries--;
        completedDeliveries++;
        cout << "[PSOOP] Delivery completed at "
                  << getLabel()
                  << " | Total completed: "
                  << completedDeliveries << "\n";
    }
}

void CustomerSite::addPendingDelivery() {
    pendingDeliveries++;
    cout << "[PSOOP] Pending delivery added to "
              << getLabel()
              << " | Pending: " << pendingDeliveries << "\n";
}

string CustomerSite::getCustomerName()
    const { return customerName; }
int CustomerSite::getPendingDeliveries()
    const { return pendingDeliveries; }
int CustomerSite::getCompletedDeliveries()
    const { return completedDeliveries; }
bool CustomerSite::getRequiresSignature()
    const { return requiresSignature; }