//WITHOUT VISIULAIZATION

#include "protonli.hh"
#include "construction.hh" 
#include "G4VisExecutive.hh" 
#include "G4ScoringManager.hh"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>

int main(int argc, char** argv)
{
    #ifdef G4MULTITHREADED // run GEANT4 in multithreaded mode if available
        G4MTRunManager* runManager = new G4MTRunManager();
      
        runManager->SetNumberOfThreads(NumThreads); 
    #else
        G4RunManager *runManager = new G4RunManager();
    #endif

    
    G4ScoringManager* scoringManager = G4ScoringManager::GetScoringManager();
    scoringManager->SetVerboseLevel(0);
    
    runManager->SetUserInitialization(new MyDetectorConstruction()); // initialize Geometry
    runManager->SetUserInitialization(new MyPhysicsList());            // initialize physics
    runManager->SetUserInitialization(new MyActionInitialization()); // initialize user actions

    runManager->Initialize(); // initialize the RunManager

    // ================================================================
    // Start a fresh CSV file for this 108-configuration sweep
    // ================================================================
    std::ofstream clearFile("bnct_108_sweep_results.csv", std::ios::trunc);
    clearFile << "ConfigID,ModThick_cm,RefThick_cm,FastFilt_cm,"
                 "GammaFilt_cm,CollLength_cm,RealEpithermalFlux,"
                 "RatioThermalEpi,DoseFastEpi,DoseGammaEpi,"
                 "Directionality\n";
    clearFile.close();
    // ================================================================

    G4UImanager *UImanager = G4UImanager::GetUIpointer();

    // ====================================================================================
    // 1. تفعيل أوامر الفيزياء الخاصة بالنيوترونات عالية الدقة لمنع تشويه الحسابات
    // ====================================================================================
    UImanager->ApplyCommand("/process/had/particle_hp/do_not_adjust_final_state true");

    // ====================================================================================
    // 2. تفعيل حلقات الـ 108 configurations برمجياً عبر الـ Nested For Loops
    // ====================================================================================

    // تعريف مصفوفات القيم الخمسة (حاصل الضرب = 3 * 3 * 3 * 2 * 2 = 108)
    G4double modValues[]   = {32.0*cm, 34.0*cm, 36.0*cm}; // 3 قيم
    G4double refValues[]   = {20.0*cm, 25.0*cm, 30.0*cm}; // 3 قيم
    G4double fastValues[]  = {0.4*cm,  0.5*cm,  0.6*cm};  // 3 قيم
    G4double gammaValues[] = {2.5*cm,  3.0*cm};            // قيمتان
    G4double collValues[]  = {3.0*cm,  4.0*cm};            // قيمتان

    // جلب مؤشر الـ DetectorConstruction لتطبيق القيم ودفع التحديثات الهندسية
    auto detectorConstruction = const_cast<MyDetectorConstruction*>(
        dynamic_cast<const MyDetectorConstruction*>(runManager->GetUserDetectorConstruction())
    );

    G4int configCounter = 0; // عداد لتتبع رقم المحاكاة الحالي

    // الحلقات المتداخلة الخمسة
    for (int i = 0; i < 3; i++) {            // للمودريتور
        for (int j = 0; j < 3; j++) {        // للريفلعيتر
            for (int k = 0; k < 3; k++) {   // للفلتر السريع
                for (int l = 0; l < 2; l++) {   // لفلتر الجاما
                    for (int m = 0; m < 2; m++) { // للكوليماتور
                        
                        configCounter++;

                        // تطبيق القيم الحالية على الهندسة
                        detectorConstruction->SetModeratorThickness(modValues[i]);
                        detectorConstruction->SetReflectorThickness(refValues[j]);
                        detectorConstruction->SetFastFilterThickness(fastValues[k]);
                        detectorConstruction->SetGammaFilterThickness(gammaValues[l]);
                        detectorConstruction->SetCollimatorLength(collValues[m]);

                        // طباعة معلومات الحالة الحالية على الشاشة لمتابعتها بدقة
                        G4cout << "\n========================================\n";
                        G4cout << ">>> Running Configuration [ " << configCounter << " / 108 ] <<<\n";
                        G4cout << " - Moderator Thickness   : " << modValues[i] / cm << " cm\n";
                        G4cout << " - Reflector Thickness   : " << refValues[j] / cm << " cm\n";
                        G4cout << " - Fast Filter Thickness : " << fastValues[k] / cm << " cm\n";
                        G4cout << " - Gamma Filter Thickness: " << gammaValues[l] / cm << " cm\n";
                        G4cout << " - Collimator Length     : " << collValues[m] / cm << " cm\n";
                        G4cout << "========================================\n" << G4endl;

                        // تشغيل المحاكاة للحالة الحالية (1 مليون بروتون)
                        runManager->BeamOn(1000000);
                    }
                }
            }
        }
    }

    G4cout << "\n====================================\n";
    G4cout << "All 108 configurations completed successfully!\n";
    G4cout << "====================================\n" << G4endl;
    // ====================================================================================

    // ====================================================================================
    // طباعة جدول المقارنة النهائي (Comparative Table) لجميع الـ 108 حالات
    // ====================================================================================
    G4cout << "\n\n=========================================================================================================" << G4endl;
    G4cout << "                                     COMPARATIVE TABLE FOR 108 CONFIGURATIONS                                    " << G4endl;
    G4cout << "=========================================================================================================" << G4endl;
    G4cout << std::left << std::setw(8)  << "Config" 
              << std::setw(8)  << "Mod(cm)" 
              << std::setw(8)  << "Ref(cm)" 
              << std::setw(8)  << "Fast(cm)" 
              << std::setw(8)  << "Gamma(cm)" 
              << std::setw(8)  << "Coll(cm)" 
              << std::setw(15) << "EpiFlux(n/cm2s)" 
              << std::setw(12) << "Th/Epi(<0.05)" 
              << std::setw(12) << "Dir(>0.7)" << G4endl;
    G4cout << "---------------------------------------------------------------------------------------------------------" << G4endl;

    std::ifstream inFile("bnct_108_sweep_results.csv");
    std::string line;
    bool headerSkipped = false;
    int tableCounter = 0;

    if (inFile.is_open()) {
        while (std::getline(inFile, line)) {
            if (!headerSkipped) { headerSkipped = true; continue; } // تخطي رأس الملف
            tableCounter++;
            
            std::stringstream ss(line);
            std::string val;
            std::vector<std::string> row;
            while (std::getline(ss, val, ',')) {
                row.push_back(val);
            }

            if (row.size() >= 11) {
                G4cout << std::left << std::setw(8)  << tableCounter
                          << std::setw(8)  << row[1]
                          << std::setw(8)  << row[2]
                          << std::setw(8)  << row[3]
                          << std::setw(8)  << row[4]
                          << std::setw(8)  << row[5]
                          << std::setw(15) << row[6]
                          << std::setw(12) << row[7]
                          << std::setw(12) << row[10] << G4endl;
            }
        }
        inFile.close();
    } else {
        G4cout << "Warning: Could not open bnct_108_sweep_results.csv for table generation!" << G4endl;
    }
    G4cout << "=========================================================================================================" << G4endl;
    // ====================================================================================

    delete runManager;
    return 0;
}
