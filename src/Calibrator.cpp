#include <Calibrator.h>
#include <ArduinoNvs.h>

static const char* KEY_OFFSET = "offset";

Calibrator::Calibrator(SharedData* sharedData, LockController* lockController) {
    this->sharedData = sharedData;
    this->lockController = lockController;
    int offset = NVS.getInt(KEY_OFFSET);
    sharedData->setOffset(offset);
}

Calibrator::~Calibrator() {
}

void Calibrator::tick() {
    if(this->sharedData->getState() == MachineState::CALIBRATION_NEEDED && this->sharedData->shouldCalibrationStart() && !this->sharedData->bottomOut->isPressed()) {
        Serial.println("Starting Calibration");
        lockController->unlock(false);
        sharedData->switchState(PREP_CALIBRATION);
    }
}