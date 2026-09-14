#include "runaction.hh"
#include <fstream>
#include <vector>
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4ScoringManager.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"

MyRunAction::MyRunAction()
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->SetDefaultFileType("csv");
    man->SetNtupleMerging(true);

    // Ntuple 0: Target
    man->CreateNtuple("Target", "Target"); 
    man->CreateNtupleIColumn("fEvent"); 
    man->CreateNtupleDColumn("PreStepEnergy_keV"); 
    man->CreateNtupleDColumn("PostStepEnergy_keV"); 
    man->CreateNtupleDColumn("fX_m");
    man->CreateNtupleDColumn("fY_m");
    man->CreateNtupleDColumn("fZ_m");
    man->FinishNtuple(0); 

    // Ntuple 1: Detector (BSA_Output_Neutrons) -> ID = 1
    man->CreateNtuple("Detector", "BSA_Output_Neutrons"); 
    man->CreateNtupleIColumn("fEvent");                 
    man->CreateNtupleDColumn("Energy_eV");            
    man->CreateNtupleDColumn("CosTheta");             
    man->CreateNtupleDColumn("fX_cm");                  
    man->CreateNtupleDColumn("fY_cm");                  
    man->CreateNtupleDColumn("R_cm");                   
    man->CreateNtupleDColumn("FluxWeight");           
    //man->CreateNtupleDColumn("CustomValue");          
    man->FinishNtuple(1); 

    // Ntuple 2: GammaOutput -> ID = 2
    man->CreateNtuple("GammaOutput", "BSA_Output_Gamma");
    man->CreateNtupleIColumn("fEvent");                 
    man->CreateNtupleDColumn("Energy_MeV");           
    man->CreateNtupleDColumn("fX_cm");                  
    man->CreateNtupleDColumn("fY_cm");                  
    man->CreateNtupleDColumn("FluxWeight");           
    man->FinishNtuple(2);

    // Ntuple 3: TargetInterface -> ID = 3
    man->CreateNtuple("TargetInterface", "Target_Moderator_Interface");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->FinishNtuple(3);

    // Ntuple 4 to 7 ...
    man->CreateNtuple("Target_Exit", "Neutrons_After_Target");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(4);

    man->CreateNtuple("Moderator_Exit", "Neutrons_After_Moderator");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(5);

    man->CreateNtuple("FastFilter_Exit", "Neutrons_After_FastFilter");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(6);

    man->CreateNtuple("GammaFilter_Exit", "Neutrons_After_GammaFilter");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(7);

    // تسجيل العدادات التراكمية (Accumulables)
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Register(nTarget);
    accumulableManager->Register(nModerator);
    accumulableManager->Register(nFastFilter);
    accumulableManager->Register(nGammaFilter);
    accumulableManager->Register(nGamma);
    accumulableManager->Register(nCollimator);
    accumulableManager->Register(nReflector);
    accumulableManager->Register(nDetector);
    
    accumulableManager->Register(dFastAccumulated);
    accumulableManager->Register(dGammaAccumulated);

    accumulableManager->Register(nThermalFluxCount);
    accumulableManager->Register(nEpithermal);
    accumulableManager->Register(nFast);
    accumulableManager->Register(nCurrentEpithermal);
}

MyRunAction::~MyRunAction()
{} 

void MyRunAction::BeginOfRunAction(const G4Run*)
{
    G4AccumulableManager* accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();

    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->OpenFile("output.csv");
}

void MyRunAction::EndOfRunAction(const G4Run* aRun)
{
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    G4AccumulableManager::Instance()->Merge();

    if (G4Threading::IsMasterThread()) 
    {
        G4double countTarget     = nTarget.GetValue();
        G4double countModerator  = nModerator.GetValue();
        G4double countFastFilt   = nFastFilter.GetValue();
        G4double countGammaFilt  = nGammaFilter.GetValue();

        G4double masterThermalCounts    = nThermalFluxCount.GetValue();
        G4double masterEpithermalCounts = nEpithermal.GetValue();
        G4double masterFastCounts       = nFast.GetValue();
        G4double masterCurrentEpithermal = nCurrentEpithermal.GetValue();
        G4double numDetectorNeutrons    = nDetector.GetValue();  

        G4double radius = 7.0 * CLHEP::cm; 
        G4double A_cm2 = (CLHEP::pi * radius * radius) / (CLHEP::cm * CLHEP::cm);
        G4double N_p = aRun->GetNumberOfEventToBeProcessed(); 

        G4double fluxThermal    = masterThermalCounts / (A_cm2 * N_p); 
        G4double fluxEpithermal = masterEpithermalCounts / (A_cm2 * N_p);
        G4double fluxFast       = masterFastCounts / (A_cm2 * N_p);
        G4double fluxGamma      = nGamma.GetValue() / (A_cm2 * N_p); 

        G4double beamCurrent = 30.0e-3; 
        G4double protonCharge = 1.602176634e-19; 
        G4double protonsPerSecond = beamCurrent / protonCharge; 

        G4double realFluxThermal    = fluxThermal * protonsPerSecond;
        G4double realFluxEpithermal = fluxEpithermal * protonsPerSecond;
        G4double realFluxFast       = fluxFast * protonsPerSecond;
        G4double realFluxGamma      = fluxGamma * protonsPerSecond;

        G4double ratioEpithermalThermal = (fluxThermal > 0.0) ? (realFluxEpithermal / realFluxThermal) : 0.0;
        
        G4double doseFastPerEpithermal  = (masterEpithermalCounts > 0.0) ? (dFastAccumulated.GetValue() / masterEpithermalCounts) : 0.0; 
        G4double doseGammaPerEpithermal = (masterEpithermalCounts > 0.0) ? (dGammaAccumulated.GetValue() / masterEpithermalCounts) : 0.0;

        G4double directionality = (masterEpithermalCounts > 0.0) ? (masterCurrentEpithermal / masterEpithermalCounts) : 0.0;

        G4cout << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << "    BNCT BEAM QUALITY REFERENCE VALUES   " << G4endl;
        G4cout << "=====================================================" << G4endl;
        G4cout << std::defaultfloat; 
        
    	 G4cout << "1. Epithermal Flux "
           << "(IAEA Reference Value: >= 5e8 n/cm^2.s) -> Value: " << realFluxEpithermal << G4endl;

    	if (realFluxFast > 0.0)
    	{
       	 G4double ratioEpithermalFast = realFluxEpithermal / realFluxFast;

      	G4cout << "2. Phi_epithermal / Phi_fast "
           << "(Additional Metric) -> Value: " << ratioEpithermalFast << G4endl;
    	}
    	else
	    {
       	 G4cout << "2. Phi_epithermal / Phi_fast "
           << "(Additional Metric) -> Value: undefined " << "(Phi_fast = 0)" << G4endl;
    	}

    	G4double ratioThermalEpithermal =
        (masterEpithermalCounts > 0.0) ? (masterThermalCounts / masterEpithermalCounts) : 0.0;

    	G4cout << "3. Phi_thermal / Phi_epithermal "
       << "(IAEA Reference Value: <= 0.05) -> Value: " << ratioThermalEpithermal << G4endl;

    	G4cout << std::scientific;

    	G4cout << "4. D_fast / Phi_epithermal "
       << "(IAEA Reference Value: <= 7e-13) -> Value: " << doseFastPerEpithermal << " Gy.cm^2" << G4endl;

    	G4cout << "5. D_gamma / Phi_epithermal "
       << "(IAEA Reference Value: <= 2e-13) -> Value: " << doseGammaPerEpithermal << " Gy.cm^2" << G4endl;

	    G4cout << std::defaultfloat;

	    G4cout << "6. Beam Directionality (J / Phi_epi) "
       << "(IAEA Reference Value: >= 0.7) -> Value: " << directionality << G4endl;
        G4cout << "=====================================================" << G4endl;
  	  }
}




