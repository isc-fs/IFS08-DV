#pragma once
#include "HardwareIO.hpp"

enum class EBSInitState {
	Start,
	WaitLow,
	CheckPressure,
	WaitTS,
	CheckActuator1,
	WaitInterActuatorCheck,
	CheckActuator2,
	Failed,
	Done
};

class EbsManager {
public:
    // Singleton access
    static EbsManager& getInstance();

    // Non-blocking version (RTOS-friendly)
    EBSInitState initSequenceStep();

    bool checkStoragePressures();
    bool checkBrakeLinePressure();
    bool ASBChecksOK();
    bool SafeManual();
    void activateEBS();
    void deactivateEBS();
    void reset();

    EBSInitState getInitState() const {
        return init_state_; // Read only from the outside
    }

    bool isActive() const {
        return is_active_; // Read only from the outside
    }

    // Delete copy constructor and assignment operator
    EbsManager(const EbsManager&) = delete;
    EbsManager& operator=(const EbsManager&) = delete;

private:
    EbsManager();

    // Constants
    // Constant for the timeout (in milliseconds)
    static constexpr uint32_t GENERAL_TIMEOUT_MS = 5000;
    static constexpr uint32_t INTER_ACTUATOR_WAIT_MS{5000};
    static constexpr float MAIN_STORAGE_THRESHOLD{10.0f};
    static constexpr float ACTUATOR_STORAGE_THRESHOLD{1.0f};
    static constexpr float BRAKE_PRESSURE_THRESHOLD{1.0f};

    // Thresholds to ensure safe manual driving
    static constexpr float EMPTY_MAIN_STORAGE_THRESHOLD{0.1f};
    static constexpr float EMPTY_ACTUATOR_STORAGE_THRESHOLD{0.1f};

    // Variables
    EBSInitState init_state_{EBSInitState::Start};
    uint32_t start_time_{0};
    bool is_active_{false};
};
