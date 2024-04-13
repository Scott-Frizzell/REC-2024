#include <Servo.h>
#include "Checkpoint.h"

class TrackSection {
    """!
    A TrackSection object represents a track block. Each block has a trailing brake section, and allows only one ride vehicle to enter the block at a time.
    """
    private:
        Servo brakeRun;
        int id;
        int vehicleId;
        int stateDisengaged;
        int stateEngaged;
        Checkpoint* checkEnd;
        Checkpoint* checkBrake;
        TrackSection* nextSection;
    public:
        TrackSection() {
            id = millis();
            vehicleId = 0;
            waitingId = 0;
        }
    
        void configure(
                int servoPin,
                int _stateDisengaged,
                int _stateEngaged,
                Checkpoint* _checkEnd,
                Checkpoint* _checkBrake,
                TrackSection* _next
            )
        {
            brakeRun.attach(servoPin);
            stateDisengaged = _stateDisengaged;
            stateEngaged = _stateEngaged;
            checkEnd = _checkEnd;
            checkBrake = _checkBrake;
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
        }
        
        TrackSection* next() { return nextSection; }

        Checkpoint* getEnd() { return checkEnd; }

        Checkpoint* getBrake() { return checkBrake; }

        void setVehicle(int _id) { vehicleId = _id; }
};
