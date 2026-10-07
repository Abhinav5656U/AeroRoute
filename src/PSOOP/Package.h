/*
 *
 * FILE        : Package.h
 * PROJECT     : AeroRoute — Autonomous Drone Delivery Simulator
 * SUBJECT     : PSOOP — Problem Solving Using OOP
 * COURSE CODE : 2310261L
 * TASK        : Task 1 — CO1
 * DESCRIPTION : Standalone entity — not a Location subclass.
 *               Represents a physical parcel the drone carries.
 *
 */

#ifndef PACKAGE_H
#define PACKAGE_H

#include <string>
#include <iostream>
using namespace std;

// ════════════════════════════════════════════════════════════
// [PSOOP CO1 - Task 1: Package entity class]
// Encapsulates all data about a delivery parcel.
// ════════════════════════════════════════════════════════════
class Package {
private:
    // [PSOOP CO1 - private: fully encapsulated package data]
    int         packageId;
    string trackingCode;
    double      weightKg;
    double      lengthCm;
    double      widthCm;
    double      heightCm;
    string status;       // PENDING LOADED IN_TRANSIT DELIVERED
    int         srcRow;
    int         srcCol;
    int         dstRow;
    int         dstCol;
    string recipientName;

    static int idCounter;

public:
    // Constructor
    Package(double weight,
            double length, double width, double height,
            int    srcR, int srcC,
            int    dstR, int dstC,
            const string& recipient = "Customer",
            const string& tracking  = "");

    ~Package();

    // Status transitions
    void markLoaded();
    void markInTransit();
    void markDelivered();

    // Display
    void display() const;

    // Validation
    bool isWithinDroneLimit(double maxWeightKg) const;

    // Getters
    int         getId()           const;
    string getTrackingCode() const;
    double      getWeightKg()     const;
    string getStatus()       const;
    int         getDstRow()       const;
    int         getDstCol()       const;
    string getRecipient()    const;
};

#endif // PACKAGE_H