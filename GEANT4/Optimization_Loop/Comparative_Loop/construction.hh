#ifndef CONSTRUCTION_HH
#define CONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "detector.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4RunManager.hh"

class MyDetectorMessenger; // تعريف مسبق فقط

class MyDetectorConstruction : public G4VUserDetectorConstruction
{
    public:
  
    MyDetectorConstruction();
    ~MyDetectorConstruction();

    virtual G4VPhysicalVolume *Construct() override;
    virtual void ConstructSDandField() override;
    
    G4double GetModeratorThickness() const { return fModeratorThickness; }
    G4double GetReflectorThickness() const { return fReflectorThickness; }
    G4double GetFastFilterThickness() const { return fFastFilterThickness; }
    G4double GetGammaFilterThickness() const { return fGammaFilterThickness; }
    G4double GetCollimatorLength() const { return fCollimatorLength; }
    
    void SetModeratorThickness(G4double val);
    void SetReflectorThickness(G4double val);
    void SetFastFilterThickness(G4double val);
    void SetGammaFilterThickness(G4double val);
    void SetCollimatorLength(G4double val);
    
    private:
    
    MyDetectorMessenger* fMessenger;
    G4MultiFunctionalDetector* bsaScorer;
    
    G4LogicalVolume *logicTarget;
    G4LogicalVolume *logicFastFilter;
    G4LogicalVolume *logicModerator;
    G4LogicalVolume *logicGammaFilter;
    G4LogicalVolume *logicCollimator;
    G4LogicalVolume *logicReflector;
    G4LogicalVolume *logicDetector;
    
    G4double fModeratorThickness;
    G4double fReflectorThickness;
    G4double fFastFilterThickness;
    G4double fGammaFilterThickness;
    G4double fCollimatorLength;  
    
};

#endif

   
