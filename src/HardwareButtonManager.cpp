#include <HardwareButtonManager.h>

HardwareButtonManager::HardwareButtonManager(gpio_num_t bottomOutButtonPin, SharedData* sharedData) {
    this->sharedData = sharedData;

    Bounce2::Button* bottomOutButton = new Bounce2::Button();
    bottomOutButton->attach(bottomOutButtonPin, INPUT_PULLUP);

    sharedData->bottomOut = bottomOutButton;
}

HardwareButtonManager::~HardwareButtonManager() {
}

void HardwareButtonManager::tick() {
    sharedData->bottomOut->update();
}
