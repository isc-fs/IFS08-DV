#include "StateManager.hpp"
#include "HardwareIO.hpp"
#include "CanGlobals.hpp"
#include "RosGlobals.hpp"

void StateManager::updateSignals()
{
	signals.asms_on = HardwareIO::read_asms_on();
	signals.ts_active = HardwareIO::read_TS_activated();
	signals.sdc_res_open = HardwareIO::read_sdc_res_open();

	signals.ebs_activated = ebs.isActive();
	signals.abs_checks_ok = ebs.ASBChecksOK();
	signals.brakes_engaged = ebs.checkBrakeLinePressure();

	// CAN Updates
	signals.r2d = g_can_r2d.load();
	signals.vehicle_standstill = g_can_vehicle_standstill.load();
	int mission_id = g_can_mission_id.load();
	signals.mission_selected = mission_id > 0;

	// ROS Updates
    signals.mission_finished = g_finished_cmd.load();
}

void StateManager::updateState() {
    if (signals.ebs_activated) {
        if (signals.mission_finished && signals.vehicle_standstill) {
            state = signals.sdc_res_open ? ASState::EMERGENCY : ASState::FINISHED;
        } else {
            state = ASState::EMERGENCY;
        }
    } else {
        if (signals.mission_selected && signals.asms_on && signals.abs_checks_ok && signals.ts_active) {
            if (signals.r2d) {
                state = ASState::DRIVING;
            } else if (signals.brakes_engaged) {
                state = ASState::READY;
            } else {
                state = ASState::OFF;
            }
        } else {
            state = ASState::OFF;
        }
    }
}

void StateManager::update() {
    updateSignals();
    updateState();
}

void StateManager::reset() {
    state = ASState::OFF;
    updateSignals();
}
