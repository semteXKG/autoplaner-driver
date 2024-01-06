#include <Communicator.h>

uint8_t broadcastAddress[] = {0x0C, 0xB8, 0x15, 0xF8, 0xF0, 0xEC};
esp_now_peer_info_t peerInfo;

static const char* KEY_LOCKSTATE = "lockState";
static const char* KEY_OFFSET = "offset";

Communicator *globalCommunicator;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  if(status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Packet Send Status:\tDelivery Fail");
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  BackingData* receivedReading = (BackingData*) malloc(sizeof(BackingData));
  memcpy(receivedReading, incomingData, sizeof(BackingData));
  globalCommunicator->updateBackingData(receivedReading);
}

Communicator::Communicator(SharedData *sharedData)
{
  previousBackingData = (BackingData*) malloc(sizeof(BackingData));
  globalCommunicator = this;
  this->sharedData = sharedData;
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

Communicator::~Communicator()
{
}

void Communicator::updateBackingData(BackingData* newBackingData) {
  std::lock_guard<std::mutex> lck(lockMutex);
  checkForNvRamUpdates(newBackingData, sharedData->getBackingData());
  printDiffChanges(true, newBackingData, sharedData->getBackingData());
  memcpy(previousBackingData, newBackingData, sizeof(BackingData));
  sharedData->setBackingData(newBackingData);
}

void Communicator::checkForNvRamUpdates(BackingData* newData, BackingData* existingData) {
  if(newData->locked != existingData->locked) {
    Serial.println("Updating locked in NVRAM");
    NVS.setInt(KEY_LOCKSTATE, newData->locked ? 1 : 0);
  }

if(newData->offset != existingData->offset) {
    Serial.println("Updating offset in NVRAM");
    NVS.setInt(KEY_OFFSET, newData->offset);
  }
}

void Communicator::printDiffChanges(boolean fromRemote, BackingData* newData, BackingData* existingData) {
  if(memcmp(newData, existingData, sizeof(BackingData)) != 0) {
    Serial.printf("Remote:\t%d,\tState:\t%s,\ttarget:\t%d,\tcurrent:\t%d,\toffset:\t%d,\tlocked:\t%d,\tcalibrationDone:\t%d,\tcalibrationStart:\t%d\n", 
      fromRemote, machineStateDesc[newData->state], newData->targetPosition, newData->currentPosition, newData->offset, newData->locked, newData->calibrationDone, newData->calibrationStart);
  }
}

void Communicator::tick()
{
  std::lock_guard<std::mutex> lck(lockMutex);

  BackingData* currentBackingData = sharedData->getBackingData();
  if (isFreshlyStarted() || isChanged(currentBackingData, previousBackingData))
  {
    printDiffChanges(false, currentBackingData, previousBackingData);
    esp_now_send(broadcastAddress, (uint8_t *)currentBackingData, sizeof(BackingData));
    memcpy(previousBackingData, currentBackingData, sizeof(BackingData));
    lastSent = millis();
  }
}

boolean Communicator::isFreshlyStarted() 
{
  long currentTimestamp = millis();
  return (currentTimestamp < 120000 && currentTimestamp - lastSent > 10000);
}


boolean Communicator::isChanged(BackingData* currentData, BackingData* previousData)
{
  if (currentData->calibrationDone != previousData->calibrationDone)
  {
    Serial.println("Locally modified: calibrationDone");
    return true;
  }
  if (currentData->calibrationStart != previousData->calibrationStart)
  {
    Serial.println("Locally modified: calibrationStart");
    return true;
  }
  if (currentData->currentPosition != previousData->currentPosition)
  {
    Serial.println("Locally modified: currentPosition");
    return true;
  }

  if (currentData->locked != previousData->locked)
  {
    Serial.println("Locally modified: locked");
    return true;
  }

  if (currentData->offset != previousData->offset)
  {
    Serial.println("Locally modified: offset");
    return true;
  }
  if (currentData->state != previousData->state)
  {
    Serial.println("Locally modified: state");
    return true;
  }
  if (currentData->targetPosition != previousData->targetPosition)
  {
    Serial.println("Locally modified: targetPosition");
    return true;
  }
  return false;
}