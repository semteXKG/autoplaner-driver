#ifndef communicator_h
#define communicator_h

#include <esp_now.h>
#include <WiFi.h>
#include <SharedData.h>

class Communicator {
private:
    SharedData* sharedData;
    BackingData previousBackingData;
    boolean isChanged(BackingData currentData, BackingData previousData);
public:
    Communicator(SharedData* sharedData);
    ~Communicator();
    void tick();
    void updateBackingData(BackingData backingData);
};


#endif