#include <SharedData.h>
#include <Arduino.h>

SharedData::SharedData() {
    backingData = (BackingData*) malloc(sizeof(BackingData));
    backingData->calibrationDone = false;
    backingData->calibrationStart = false;
    backingData->locked = 0;
    backingData->offset = 0;
    backingData->currentPosition = MIN_POSITION;
    backingData->targetPosition = backingData->currentPosition;
    backingData->state = MachineState::CALIBRATION_NEEDED;
}

SharedData::~SharedData() {

}

long SharedData::getTargetPosition() {
    return backingData->targetPosition;
}

void SharedData::setTargetPosition(long targetPosition) {
    backingData->targetPosition = targetPosition;
    if(backingData->targetPosition < MIN_POSITION) {
		backingData->targetPosition = MIN_POSITION;
	} else if (backingData->targetPosition > MAX_POSITION) {
		backingData->targetPosition = MAX_POSITION;
	}
}

long SharedData::getCurrentPosition() {
    return backingData->currentPosition;
}

void SharedData::setCurrentPosition(long currentPosition) {
    backingData->currentPosition = currentPosition;
}

void SharedData::setPosition(long position) {
    setCurrentPosition(position);
    setTargetPosition(position);
}

void SharedData::setOffset(int offset) {
    backingData->offset = offset;
}

int SharedData::getOffset() {
    return backingData->offset;
}

bool SharedData::isLocked() {
    return backingData->locked;
}

bool SharedData::shouldCalibrationStart() {
    return backingData->calibrationStart;
}

void SharedData::setLocked(bool locked) {
    Serial.print("locked: ");
    Serial.println(locked);
    backingData->locked = locked;
}

void SharedData::markCalibrationDone() {
    backingData->calibrationDone = true;
}

MachineState SharedData::getState() {
    return backingData->state;
}

void SharedData::switchState(MachineState state) {
    if(state == IDLE && !backingData->calibrationDone) {
        Serial.print("Cannot go idle without calibration ");
        state = MachineState::CALIBRATION_NEEDED;
    }
    Serial.print("Switching state to ");
    Serial.print(state);
    Serial.print(" - ");
    Serial.println(machineStateDesc[state]);
    
    backingData->state = state;
}

BackingData* SharedData::getBackingData() {
    return backingData;
}

void SharedData::setBackingData(BackingData* newBackingData) {
    BackingData* currentBackingData = backingData;
    this->backingData = newBackingData;
    free(currentBackingData);
}