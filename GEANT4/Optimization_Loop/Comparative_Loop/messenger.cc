#include "messenger.hh"
#include "construction.hh"

MyDetectorMessenger::MyDetectorMessenger(MyDetectorConstruction* myDet)
: G4UImessenger(), fDetectorConstruction(myDet)
{
    // 1. Moderator Thickness Command
    fModThicknessCmd = new G4UIcmdWithADoubleAndUnit("/detector/setModeratorThickness", this);
    fModThicknessCmd->SetGuidance("Set the thickness of the moderator.");
    fModThicknessCmd->SetParameterName("thickness", false);
    fModThicknessCmd->SetUnitCategory("Length");

    // 2. Reflector Thickness Command
    fReflectorThicknessCmd = new G4UIcmdWithADoubleAndUnit("/detector/setReflectorThickness", this);
    fReflectorThicknessCmd->SetGuidance("Set the thickness of the reflector.");
    fReflectorThicknessCmd->SetParameterName("thickness", false);
    fReflectorThicknessCmd->SetUnitCategory("Length");

    // 3. Fast Filter Thickness Command
    fFastFilterThicknessCmd = new G4UIcmdWithADoubleAndUnit("/detector/setFastFilterThickness", this);
    fFastFilterThicknessCmd->SetGuidance("Set the thickness of the fast filter.");
    fFastFilterThicknessCmd->SetParameterName("thickness", false);
    fFastFilterThicknessCmd->SetUnitCategory("Length");

    // 4. Gamma Filter Thickness Command
    fGammaFilterThicknessCmd = new G4UIcmdWithADoubleAndUnit("/detector/setGammaFilterThickness", this);
    fGammaFilterThicknessCmd->SetGuidance("Set the thickness of the gamma filter.");
    fGammaFilterThicknessCmd->SetParameterName("thickness", false);
    fGammaFilterThicknessCmd->SetUnitCategory("Length");

    // 5. Collimator Length Command
    fCollimatorLengthCmd = new G4UIcmdWithADoubleAndUnit("/detector/setCollimatorLength", this);
    fCollimatorLengthCmd->SetGuidance("Set the length of the collimator.");
    fCollimatorLengthCmd->SetParameterName("length", false);
    fCollimatorLengthCmd->SetUnitCategory("Length");
}

MyDetectorMessenger::~MyDetectorMessenger()
{
    delete fModThicknessCmd;
    delete fReflectorThicknessCmd;
    delete fFastFilterThicknessCmd;
    delete fGammaFilterThicknessCmd;
    delete fCollimatorLengthCmd;
}

void MyDetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fModThicknessCmd) {
        fDetectorConstruction->SetModeratorThickness(fModThicknessCmd->GetNewDoubleValue(newValue));
    }
    else if (command == fReflectorThicknessCmd) {
        fDetectorConstruction->SetReflectorThickness(fReflectorThicknessCmd->GetNewDoubleValue(newValue));
    }
    else if (command == fFastFilterThicknessCmd) {
        fDetectorConstruction->SetFastFilterThickness(fFastFilterThicknessCmd->GetNewDoubleValue(newValue));
    }
    else if (command == fGammaFilterThicknessCmd) {
        fDetectorConstruction->SetGammaFilterThickness(fGammaFilterThicknessCmd->GetNewDoubleValue(newValue));
    }
    else if (command == fCollimatorLengthCmd) {
        fDetectorConstruction->SetCollimatorLength(fCollimatorLengthCmd->GetNewDoubleValue(newValue));
    }
}
