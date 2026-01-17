// CanInterface.hpp
#pragma once
#include "CanGlobals.hpp"

class CanInterface {
private:
    CanInterface() = default;
    ~CanInterface() = default;

    friend void CanTask(void *argument); // allow CanTask to instantiate

public:
    void init();
    void sendControl(float accel, float steer);
    static void sendR2D(bool r2d);
    void sendIMUData();

private:
    int m_mission_id{0};
    bool m_r2d{false};
    // add HAL handles/state here
};