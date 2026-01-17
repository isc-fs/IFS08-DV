#include "CanInterface.hpp"
#include "main.h"
#include <cstring>
// include HAL CAN headers as needed
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

extern FDCAN_HandleTypeDef hfdcan1;

void CanInterface::init()
{
    // From looking at the main.c file, there is no MX_FDCAN1_Init(), 
    // so we assume it is not enabled in CubeMX.
    // The following is a template for what you would need to do.
    // You will need to enable FDCAN1 in CubeMX and configure it.

    FDCAN_FilterTypeDef sFilterConfig;

    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RX_FIFO0;
    sFilterConfig.FilterID1 = 0x000; // Set the ID you want to receive for the go signal
    sFilterConfig.FilterID2 = 0x7FF; // Mask to accept only the ID from FilterID1

    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    {
        Error_Handler();
    }
}

void CanInterface::sendControl(float accel, float steer)
{
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[8];

    TxHeader.Identifier = 0x000; // Set the ID for the control message
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_8;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    // Example for packing floats:
    // *(float*)(&TxData[0]) = accel;
    // *(float*)(&TxData[4]) = steer;

    // Pack your data into TxData using memcpy to avoid alignment and strict-aliasing issues.
    memcpy(&TxData[0], &accel, sizeof(float));
    memcpy(&TxData[4], &steer, sizeof(float));

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK)
    {
        Error_Handler();
    }
}

void CanInterface::sendR2D(bool r2d)
{
    FDCAN_TxHeaderTypeDef TxHeader;
    uint8_t TxData[1];

    TxHeader.Identifier = 0x000; // Set the ID for the R2D message
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.DataLength = FDCAN_DLC_BYTES_1;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    TxData[0] = r2d;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK)
    {
        Error_Handler();
    }
    else {
        g_can_r2d.store(true);
    }
}

void CanInterface::sendIMUData()
{
    // TODO: pack data into CAN frame and transmit
    // Read IMU from HardwareIO
    
    // Set atomic vehicle standstill based on the IMU velocity

    // Pack IMU data and send through CAN
    
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        FDCAN_RxHeaderTypeDef RxHeader;
        uint8_t RxData[8];

        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData) != HAL_OK)
        {
            // Don't call Error_Handler() from an ISR
            return;
        }

        // Check the ID of the incoming message and process it
        if (RxHeader.Identifier == 0x000) // Set the ID for the go signal
        {
            if (g_can_listen_go.load()) {
                // Directly send the R2D message from the ISR for lower latency
                CanInterface::sendR2D(true);
            }
        }
    }
}