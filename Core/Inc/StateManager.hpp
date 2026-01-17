#pragma once
#include "EbsManager.hpp"

enum class ASState {
    OFF,        // ASSI off
    READY,      // ASSI yellow continuous
    DRIVING,    // ASSI yellow flashing
    EMERGENCY,  // ASSI blue flashing
    FINISHED    // ASSI blue continuous
    // Flashing: 2-5 Hz and 50% duty cycle
};

struct StateManagerSignals {
    bool asms_on = false; 			// I - HardwareIO
    bool ts_active = false; 		// I - HardwareIO
    bool sdc_res_open = false; 		// I - HardwareIO
    bool ebs_activated = false; 	// R - EbsManager
    bool abs_checks_ok = false; 	// R - EbsManager
    bool brakes_engaged = false; 	// I - EbsManager
    bool mission_selected = false; 	// R - CanInterface
    bool r2d = false; 				// R - CanInterface (Go signal)
    bool vehicle_standstill = true; // I - CanInterface (imu)
    bool mission_finished = false; 	// R - RosInterface
};

class StateManager {
public:
    static StateManager& getInstance() {
        static StateManager instance;
        return instance;
    }

    void update();
    const StateManagerSignals& getSignals() const { return signals; }
    const ASState& getState() const { return state; }
    void reset();
private:
    StateManager() = default;
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;

    void updateSignals();
    void updateState();

    StateManagerSignals signals;
    ASState state;
    EbsManager& ebs = EbsManager::getInstance();
};
