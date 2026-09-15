#ifndef RUNACTION_HH
#define RUNACTION_HH

#include <iostream>
#include <fstream>

#include "G4UserRunAction.hh"
#include "G4AnalysisManager.hh"
#include "G4RootAnalysisManager.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include "G4Accumulable.hh"

class MyDetectorConstruction; // أضيفي هذا السطر هنا

class MyRunAction : public G4UserRunAction
{
    public:
        MyRunAction();
        ~MyRunAction();

        virtual void BeginOfRunAction(const G4Run*);
        virtual void EndOfRunAction(const G4Run*);
        
        void SetDetectorConstruction(MyDetectorConstruction* det) { fDetector = det; }
        void SetConfigID(G4int id) { fConfigId = id; }
	
      
    // عدادات العد الهندسي واجتياز المكونات
    G4Accumulable<G4double> nTarget = 0;
    G4Accumulable<G4double> nModerator = 0;
    G4Accumulable<G4double> nGammaFilter = 0; 
    G4Accumulable<G4double> nFastFilter = 0;
    G4Accumulable<G4double> nCollimator = 0;
    G4Accumulable<G4double> nReflector = 0;
    G4Accumulable<G4double> nDetector = 0;
     
    // العدادات والجرعات التي تستقبل أوزاناً وقيم كسرية (G4double)
    G4Accumulable<G4double> dFastAccumulated = 0;
    G4Accumulable<G4double> dGammaAccumulated = 0;

    G4Accumulable<G4double> nThermalFluxCount = 0; 
    G4Accumulable<G4double> nEpithermal = 0;
    G4Accumulable<G4double> nCurrentEpithermal = 0; 
    G4Accumulable<G4double> nFast = 0;
    G4Accumulable<G4double> nGamma = 0;   
    
    private:
    MyDetectorConstruction* fDetector; 
    G4int fConfigId;
           
};

#endif
