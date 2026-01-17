#include "EbsManager.hpp"
#include "HardwareIO.hpp"

EbsManager::EbsManager() {}

EbsManager& EbsManager::getInstance() {
	static EbsManager instance;
	return instance;
}

EBSInitState EbsManager::initSequenceStep() {
	switch (init_state_) {
		case EBSInitState::Start:
			if (HardwareIO::read_SDC_is_ready()) {
				init_state_ = EBSInitState::WaitLow;
				start_time_ = HardwareIO::now_ms();
			}
			break;

		case EBSInitState::WaitLow:
			if (!HardwareIO::read_SDC_is_ready()) {
				init_state_ = EBSInitState::CheckPressure;
			} else if (HardwareIO::now_ms() - start_time_ > GENERAL_TIMEOUT_MS) {
				init_state_ = EBSInitState::Failed;
			}
			break;

		case EBSInitState::CheckPressure:
			if (checkStoragePressures()) {
				init_state_ = EBSInitState::WaitTS;
				HardwareIO::set_AS_close_SDC(true);
			}
			else {
				init_state_ = EBSInitState::Failed;
			}
			start_time_ = HardwareIO::now_ms();
			break;

		case EBSInitState::WaitTS:
			if (HardwareIO::read_TS_activated()) {
				init_state_ = EBSInitState::CheckActuator1;
				HardwareIO::enable_ebs_actuator_1(false); // is_active is not set because it would interfere with the StateManager
			}
			break;

		case EBSInitState::CheckActuator1:
			if (checkBrakeLinePressure()) {
				init_state_ = EBSInitState::WaitInterActuatorCheck;
				HardwareIO::enable_ebs_actuator_1(true); // is_active is not set because it would interfere with the StateManager
				start_time_ = HardwareIO::now_ms();
			}
			break;

		// Give time for the EBS to disengage
		case EBSInitState::WaitInterActuatorCheck:
			if (HardwareIO::now_ms() - start_time_ > INTER_ACTUATOR_WAIT_MS) {
				init_state_ = EBSInitState::CheckActuator2;
				HardwareIO::enable_ebs_actuator_2(false); // is_active is not set because it would interfere with the StateManager
			}
			break;

		case EBSInitState::CheckActuator2:
			if (checkBrakeLinePressure()) {
				init_state_ = EBSInitState::Done;
				HardwareIO::enable_ebs_actuator_2(true); // is_active is not set because it would interfere with the StateManager
			}
			break;

		case EBSInitState::Failed:
			break;

		case EBSInitState::Done:
			break;
	}

    return init_state_;
}

bool EbsManager::checkStoragePressures() {
    float main_p = HardwareIO::read_main_storage_pressure();
    float a1_p = HardwareIO::read_actuator1_storage_pressure();
    float a2_p = HardwareIO::read_actuator2_storage_pressure();

    return (main_p > MAIN_STORAGE_THRESHOLD) && (a1_p > ACTUATOR_STORAGE_THRESHOLD) && (a2_p > ACTUATOR_STORAGE_THRESHOLD);
}

bool EbsManager::checkBrakeLinePressure() {
    float b_p = HardwareIO::read_brake_pressure();

    return (b_p > BRAKE_PRESSURE_THRESHOLD);
}

bool EbsManager::ASBChecksOK() {
    return (init_state_ == EBSInitState::Done) && (checkStoragePressures());
}

bool EbsManager::SafeManual() {
    float main_p = HardwareIO::read_main_storage_pressure();
    float a1_p = HardwareIO::read_actuator1_storage_pressure();
    float a2_p = HardwareIO::read_actuator2_storage_pressure();

    return (main_p < EMPTY_MAIN_STORAGE_THRESHOLD) && (a1_p < EMPTY_ACTUATOR_STORAGE_THRESHOLD) && (a2_p < EMPTY_ACTUATOR_STORAGE_THRESHOLD);
}

void EbsManager::activateEBS() {
    HardwareIO::enable_ebs_actuator_1(false);
    HardwareIO::enable_ebs_actuator_2(false);
    is_active_ = true;
}

void EbsManager::deactivateEBS() {
    HardwareIO::enable_ebs_actuator_1(true);
    HardwareIO::enable_ebs_actuator_2(true);
    is_active_ = false;
}

void EbsManager::reset() {
    init_state_ = EBSInitState::Start;
    is_active_ = false;
    deactivateEBS();
}
