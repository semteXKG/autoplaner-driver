#ifndef shareddata_h
#define shareddata_h

#include <Constants.h>
#include <Bounce2.h>

struct BackingData
{
    long targetPosition;
    long currentPosition;
    int offset;
    bool locked = false;
    bool calibrationDone = false;
    MachineState state;
};


class SharedData {
    private:
        BackingData* backingData;
    public:
        SharedData();
        ~SharedData();
        Bounce2::Button* bottomOut;
        void setTargetPosition(long targetPosition);
        long getTargetPosition();
        void setCurrentPosition(long currentPosition);
        long getCurrentPosition();
        void setPosition(long position);
        void setOffset(int offset);
        int getOffset();
        bool isLocked();
        void setLocked(bool locked);
        void markCalibrationDone();
        void switchState(MachineState state);
        MachineState getState();
        BackingData* getBackingData();
        void setBackingData(BackingData* backingData);
};

#endif