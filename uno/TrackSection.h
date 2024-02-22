#include <Servo.h>
#include "Checkpoint.h"

class TrackSection {
    //"""!
    //A TrackSection object represents a track block. Each block has a leading brake section, and allows only one ride vehicle to enter the block at a time.
    //"""
    private:
        Servo brakeRun;
        int id;
        int vehicleId;
        int stateDisengaged;
        int stateEngaged;
        Checkpoint* checkStart;
        Checkpoint* checkEnd;
        Checkpoint* checkBrakeClear;
        int waitingId;
        TrackSection* nextSection;
    public:
        TrackSection() {
            id = millis(); // TODO: maybe fix
            vehicleId = 0;
            waitingId = 0;
        }
    
        void configure(
                int servoPin,
                int _stateDisengaged,
                int _stateEngaged,
                Checkpoint* _checkStart,
                Checkpoint* _checkEnd,
                Checkpoint* _checkBrakeClear,
                TrackSection* _next
            )
        {
            brakeRun.attach(servoPin);
            stateDisengaged = _stateDisengaged;
            stateEngaged = _stateEngaged;
            checkStart = _checkStart;
            checkEnd = _checkEnd;
            checkBrakeClear = _checkBrakeClear;
            nextSection = _next;
            engage();
        }

        int getId() { return id; }

        int vehicle() { return vehicleId; }

        void clear() { vehicleId = 0; }
    
        void engage() {
            brakeRun.write(stateEngaged);
        }
    
        void disengage() {
            brakeRun.write(stateDisengaged);
            vehicleId = waitingId;
            waitingId = 0;
        }
    
        bool waiting() { return waitingId; }
        
        TrackSection* next() { return nextSection; }

        Checkpoint* getStart() { return checkStart; }

        Checkpoint* getEnd() { return checkEnd; }

        Checkpoint* getBrakeClear() { return checkBrakeClear; }

        void wait(int _id) { waitingId = _id; }
};
