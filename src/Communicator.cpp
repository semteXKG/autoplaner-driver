#include <Communicator.h>

uint8_t broadcastAddress[] = {0x0C, 0xB8, 0x15, 0xF8, 0xF0, 0xEC};
esp_now_peer_info_t peerInfo;

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
  BackingData receivedReading;
  memcpy(&receivedReading, incomingData, sizeof(receivedReading));
  globalCommunicator->updateBackingData(receivedReading);
}

Communicator::Communicator(SharedData *sharedData)
{
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

void Communicator::updateBackingData(BackingData newBackingData) {
  std::lock_guard<std::mutex> lck(lockMutex);
  memcpy(&previousBackingData, &newBackingData, sizeof(BackingData));
  sharedData->setBackingData(newBackingData);
  Serial.print("new TargetPosition: ");
  Serial.print(sharedData->getTargetPosition());
  Serial.print("[");
  Serial.print(previousBackingData.targetPosition);
  Serial.println("]");
}

void Communicator::tick()
{
  BackingData currentBackingData = sharedData->getBackingData();
  if (isFreshlyStarted() || isChanged(currentBackingData, previousBackingData))
  {
    esp_now_send(broadcastAddress, (uint8_t *)&currentBackingData, sizeof(currentBackingData));
    memcpy(&previousBackingData, &currentBackingData, sizeof(BackingData));
    lastSent = millis();
  }
}

boolean Communicator::isFreshlyStarted() 
{
  long currentTimestamp = millis();
  return (currentTimestamp < 120000 && currentTimestamp - lastSent > 10000);
}


boolean Communicator::isChanged(BackingData currentData, BackingData previousData)
{
  std::lock_guard<std::mutex> lck(lockMutex);

  if (currentData.calibrationDone != previousData.calibrationDone)
  {
    Serial.println("calibrationDone");
    return true;
  }
  if (currentData.currentPosition != previousData.currentPosition)
  {
    Serial.println("currentPosition");
    return true;
  }

  if (currentData.locked != previousData.locked)
  {
    Serial.println("locked");
    return true;
  }

  if (currentData.offset != previousData.offset)
  {
    Serial.println("offset");
    return true;
  }
  if (currentData.state != previousData.state)
  {
    Serial.println("state");
    return true;
  }
  if (currentData.targetPosition != previousData.targetPosition)
  {
    Serial.print("targetPosition: ");
    Serial.print(previousData.targetPosition);
    Serial.print("->");
    Serial.println(currentData.targetPosition);
    return true;
  }
  return false;
}