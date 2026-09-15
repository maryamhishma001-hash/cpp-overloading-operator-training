#include "runaction.hh"
#include "construction.hh"
#include <fstream>
#include <vector>
#include "G4AccumulableManager.hh"
#include "G4Run.hh"
#include "G4ScoringManager.hh"
#include "G4SDManager.hh"
#include "G4THitsMap.hh"
#include "G4Event.hh"
#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"

MyRunAction::MyRunAction()
: fConfigId(0) // تهيئة عداد التكوينات يبدأ من الصفر
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

    // Ntuple 4: Target_Exit -> ID = 4
    man->CreateNtuple("Target_Exit", "Neutrons_After_Target");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(4);

    // Ntuple 5: Moderator_Exit -> ID = 5
    man->CreateNtuple("Moderator_Exit", "Neutrons_After_Moderator");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(5);

    // Ntuple 6: FastFilter_Exit -> ID = 6
    man->CreateNtuple("FastFilter_Exit", "Neutrons_After_FastFilter");
    man->CreateNtupleIColumn("fEvent");
    man->CreateNtupleDColumn("Energy_eV");
    man->CreateNtupleDColumn("Weight");
    man->FinishNtuple(6);

    // Ntuple 7: GammaFilter_Exit -> ID = 7
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
    // 1. إغلاق ملف الـ Analysis الافتراضي إن وجد
    G4AnalysisManager* man = G4AnalysisManager::Instance();
    man->Write();
    man->CloseFile();

    // 2. دمج الـ Accumulables بين الـ Threads
    G4AccumulableManager* const accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Merge();

    // 3. التنفيذ فقط على الـ Master Thread لمنع التضارب
    if (G4Threading::IsMasterThread()) 
    {
        fConfigId++; // زيادة رقم التكوين تلقائياً مع كل وتشغيل وانتهاء للـ Run

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

        G4double ratioThermalEpithermal = (masterEpithermalCounts > 0.0) ? (realFluxThermal / realFluxEpithermal) : 0.0;
        G4double doseFastPerEpithermal  = (masterEpithermalCounts > 0.0) ? (dFastAccumulated.GetValue() / masterEpithermalCounts) : 0.0; 
        G4double doseGammaPerEpithermal = (masterEpithermalCounts > 0.0) ? (dGammaAccumulated.GetValue() / masterEpithermalCounts) : 0.0;
        G4double directionality = (masterEpithermalCounts > 0.0) ? (masterCurrentEpithermal / masterEpithermalCounts) : 0.0;

        // طباعة نتائج الـ Run الحالي فقط
        G4cout << "\n-----------------------------------------------------" << G4endl;
        G4cout << "                     RUN RESULTS                     " << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        G4cout << "Neutrons exiting Moderator   : " << countModerator << G4endl;
        G4cout << "Neutrons exiting Fast Filter : " << countFastFilt << G4endl;
        G4cout << "Neutrons exiting Gamma Filter: " << countGammaFilt << G4endl;
        G4cout << "Total Detector Crossings     : " << numDetectorNeutrons << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;

        G4cout << "\n-----------------------------------------------------" << G4endl;
        G4cout << "              BSA PERFORMANCE METRICS                " << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        G4cout << "Epithermal Flux        : " << realFluxEpithermal << " n/cm2/s" << G4endl;
        G4cout << "Reference Value        : > 5.00e+08 n/cm2/s" << G4endl;

        G4cout << "\nThermal / Epithermal   : " << ratioThermalEpithermal << G4endl;
        G4cout << "Reference Value        : < 0.05" << G4endl;

        G4cout << "\nFast Dose / Epithermal : " << doseFastPerEpithermal << " Gy cm2 / n" << G4endl;
        G4cout << "Reference Value        : < 7.00e-13 Gy cm2 / n" << G4endl;

        G4cout << "\nGamma Dose / Epithermal: " << doseGammaPerEpithermal << " Gy cm2 / n" << G4endl;
        G4cout << "Reference Value        : < 2.00e-13 Gy cm2 / n" << G4endl;

        G4cout << "\nDirectionality J/Phi   : " << directionality << G4endl;
        G4cout << "Reference Value        : > 0.70" << G4endl;
        G4cout << "-----------------------------------------------------" << G4endl;
        G4cout << "\n"; 

        // --- حفظ النتائج تلقائياً في ملف الـ CSV للـ 108 حالات ---
        std::ifstream checkFile("bnct_108_sweep_results.csv");
        bool isEmpty = !checkFile.is_open() || checkFile.peek() == std::ifstream::traits_type::eof();
        checkFile.close();

        std::ofstream summaryFile("bnct_108_sweep_results.csv", std::ios::app);
        
        if (isEmpty) {
            summaryFile << "ConfigID,ModThick_cm,RefThick_cm,FastFilt_cm,GammaFilt_cm,CollLength_cm,RealEpithermalFlux,RatioThermalEpi,DoseFastEpi,DoseGammaEpi,Directionality\n";
        }

        const MyDetectorConstruction* detectorConstruction = static_cast<const MyDetectorConstruction*>(
            G4RunManager::GetRunManager()->GetUserDetectorConstruction()
        );
        
        G4double modThick  = detectorConstruction ? (detectorConstruction->GetModeratorThickness() / CLHEP::cm) : 0.0;
        G4double refThick  = detectorConstruction ? (detectorConstruction->GetReflectorThickness() / CLHEP::cm) : 0.0;
        G4double fastThick = detectorConstruction ? (detectorConstruction->GetFastFilterThickness() / CLHEP::cm) : 0.0;
        G4double gammaThick= detectorConstruction ? (detectorConstruction->GetGammaFilterThickness() / CLHEP::cm) : 0.0;
        G4double collLength= detectorConstruction ? (detectorConstruction->GetCollimatorLength() / CLHEP::cm) : 0.0;

        summaryFile << fConfigId << "," // تسجيل رقم التكوين التسلسلي في أول عمود بالـ CSV
                    << modThick << ","
                    << refThick << ","
                    << fastThick << ","
                    << gammaThick << ","
                    << collLength << ","
                    << realFluxEpithermal << ","
                    << ratioThermalEpithermal << ","
                    << doseFastPerEpithermal << ","
                    << doseGammaPerEpithermal << ","
                    << directionality << "\n";
        
        summaryFile.close();
    }
}
