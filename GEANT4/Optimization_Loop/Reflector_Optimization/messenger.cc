#include "messenger.hh"
#include "construction.hh"
//
MyDetectorMessenger::MyDetectorMessenger(MyDetectorConstruction* myDet)
: G4UImessenger(), fDetectorConstruction(myDet)
{
    fReflectorThicknessCmd = new G4UIcmdWithADoubleAndUnit("/detector/setReflectorThickness", this);
    fReflectorThicknessCmd->SetGuidance("Set the thickness of the reflector.");
    fReflectorThicknessCmd->SetParameterName("thickness", false);
    fReflectorThicknessCmd->SetUnitCategory("Length");
}

MyDetectorMessenger::~MyDetectorMessenger()
{
    delete fReflectorThicknessCmd;
}

void MyDetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue)
{
    if (command == fReflectorThicknessCmd) {
        fDetectorConstruction->SetReflectorThickness(fReflectorThicknessCmd->GetNewDoubleValue(newValue));
    }
}
