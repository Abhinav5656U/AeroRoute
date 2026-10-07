/*
 *
 * FILE        : CustomerSite.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 *
 */

#ifndef CUSTOMER_SITE_H
#define CUSTOMER_SITE_H

#include "Location.h"
using namespace std;

class CustomerSite : public Location {
private:
    string customerName;
    string address;
    int pendingDeliveries;
    int completedDeliveries;
    bool requiresSignature;

public:
    CustomerSite(int row, int col,
                 const string& label,
                 const string& customerName,
                 const string& address = "",
                 bool requiresSignature = false);
    ~CustomerSite() override;

    void display() const override;
    string getTypeName() const override;
    int getTypeCode() const override;
    bool canDroneDock() const override;

    void receiveDelivery();
    void addPendingDelivery();

    string getCustomerName() const;
    int getPendingDeliveries() const;
    int getCompletedDeliveries() const;
    bool getRequiresSignature() const;
};

#endif
