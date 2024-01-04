#include <Arduino.h>
#include <SharedData.h>
#include <EnvironmentSensors.h>
#include <HardwareButtonManager.h>
#include <Calibrator.h>
#include <StepperController.h>
#include <LockController.h>
#include <ArduinoNvs.h>
#include <FastAccelStepper.h>
#include <Communicator.h>

#define DEBUG ""

static const gpio_num_t BOTTOM_OUT_BUTTON = GPIO_NUM_27;

static const gpio_num_t HEIGHT_STEPPER_PULSE = GPIO_NUM_33;
static const gpio_num_t HEIGHT_STEPPER_DIR = GPIO_NUM_32;

static const gpio_num_t LOCK_STEPPER_PULSE = GPIO_NUM_15;
static const gpio_num_t LOCK_STEPPER_DIR = GPIO_NUM_2;


SharedData* sharedData;
EnvironmentSensors* environmentSensors;
HardwareButtonManager* buttonManager;
Calibrator* calibrator;
StepperController* stepperController;
LockController* lockController;
FastAccelStepperEngine* engine;
Communicator* communicator;

void setup() {
	Serial.begin(115200);
	
	if (!NVS.begin("planer")) {
		ESP.restart();
	}

	engine = new FastAccelStepperEngine();
	engine->init();
	sharedData = new SharedData();
	buttonManager = new HardwareButtonManager(BOTTOM_OUT_BUTTON, sharedData);
	environmentSensors = new EnvironmentSensors(sharedData);
	stepperController = new StepperController(sharedData, engine, HEIGHT_STEPPER_PULSE, HEIGHT_STEPPER_DIR);
	lockController = new LockController(sharedData, engine, LOCK_STEPPER_PULSE, LOCK_STEPPER_DIR);
	calibrator = new Calibrator(sharedData, lockController);
	communicator = new Communicator(sharedData);
//	sharedData->switchState(MachineState::CALIBRATION_NEEDED);
	sharedData->switchState(MachineState::IDLE);
}


void loop() {
	buttonManager->tick();
	environmentSensors->tick();
	calibrator->tick();
	stepperController->tick();
	lockController->tick();
	communicator->tick();
}