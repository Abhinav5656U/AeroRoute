#include "Package.h"
#include "../shared/Constants.h"
#include <cmath>
#include <stdexcept>
using namespace std;

int Package::idCounter = 1;

Package::Package(double weight,
                 double length, double width, double height,
                 int srcR, int srcC,
                 int dstR, int dstC,
                 const string& recipient,
                 const string& tracking)
    : packageId(idCounter++),
      trackingCode(tracking.empty()
                       ? "AR-PKG-" + to_string(packageId)
                       : tracking),
      weightKg(weight),
      lengthCm(length),
      widthCm(width),
      heightCm(height),
      status("PENDING"),
      srcRow(srcR),
      srcCol(srcC),
      dstRow(dstR),
      dstCol(dstC),
      recipientName(recipient)
{
    if (!isfinite(weightKg) || weightKg <= 0.0 ||
        !isfinite(lengthCm) || lengthCm <= 0.0 ||
        !isfinite(widthCm) || widthCm <= 0.0 ||
        !isfinite(heightCm) || heightCm <= 0.0) {
        throw invalid_argument(
            "Package weight and dimensions must be finite positive values.");
    }
    if (srcRow < 0 || srcRow >= GRID_ROWS ||
        srcCol < 0 || srcCol >= GRID_COLS ||
        dstRow < 0 || dstRow >= GRID_ROWS ||
        dstCol < 0 || dstCol >= GRID_COLS) {
        throw out_of_range("Package source and destination must be inside the city grid.");
    }
    if (recipientName.find_first_not_of(" \t\r\n") == string::npos) {
        throw invalid_argument("Package recipient cannot be empty.");
    }
}

Package::~Package() = default;

void Package::markLoaded() {
    if (status == "PENDING") status = "LOADED";
}

void Package::markInTransit() {
    if (status == "LOADED") status = "IN_TRANSIT";
}

void Package::markDelivered() {
    if (status == "LOADED" || status == "IN_TRANSIT") status = "DELIVERED";
}

void Package::display() const {
    cout << "Package " << trackingCode
              << " | Recipient: " << recipientName
              << " | Weight: " << weightKg << " kg"
              << " | Status: " << status << '\n';
}

bool Package::isWithinDroneLimit(double maxWeightKg) const {
    return weightKg <= maxWeightKg;
}

int Package::getId() const { return packageId; }
string Package::getTrackingCode() const { return trackingCode; }
double Package::getWeightKg() const { return weightKg; }
string Package::getStatus() const { return status; }
int Package::getDstRow() const { return dstRow; }
int Package::getDstCol() const { return dstCol; }
string Package::getRecipient() const { return recipientName; }
