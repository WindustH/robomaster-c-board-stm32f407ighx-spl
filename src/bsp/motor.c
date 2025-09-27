#include "bsp.h"
#include "stm32f4xx.h"
#include "type.h"

static motorFb motor_status[8] = {0};
static i16 motor_current_targets[8] = {0};

// Setup function - configure CAN filter for motor feedback
static void setup_motor(void) {
  // Enter filter initialization mode
  CAN1->FMR |= CAN_FMR_FINIT;

  // Disable all filters
  CAN1->FA1R = 0;

  // Configure filter banks 0-3 for IDs 0x201-0x208 (32-bit identifier list
  // mode)
  CAN1->sFilterRegister[0].FR1 = (0x201U << 5) | ((0x202U << 5) << 16);
  CAN1->sFilterRegister[1].FR1 = (0x203U << 5) | ((0x204U << 5) << 16);
  CAN1->sFilterRegister[2].FR1 = (0x205U << 5) | ((0x206U << 5) << 16);
  CAN1->sFilterRegister[3].FR1 = (0x207U << 5) | ((0x208U << 5) << 16);

  // Set banks 0-3 to 32-bit scale
  CAN1->FS1R |= CAN_FS1R_FSC0 | CAN_FS1R_FSC1 | CAN_FS1R_FSC2 | CAN_FS1R_FSC3;

  // Set banks 0-3 to identifier list mode
  CAN1->FM1R |= CAN_FM1R_FBM0 | CAN_FM1R_FBM1 | CAN_FM1R_FBM2 | CAN_FM1R_FBM3;

  // Assign all to FIFO0
  CAN1->FFA1R &=
      ~(CAN_FFA1R_FFA0 | CAN_FFA1R_FFA1 | CAN_FFA1R_FFA2 | CAN_FFA1R_FFA3);

  // Activate filter banks 0-3
  CAN1->FA1R |=
      CAN_FA1R_FACT0 | CAN_FA1R_FACT1 | CAN_FA1R_FACT2 | CAN_FA1R_FACT3;

  // Exit filter initialization mode
  CAN1->FMR &= ~CAN_FMR_FINIT;
}

// Set target current for a specific motor (non-blocking, just update target)
static u8 set_current(u8 motor_id, i16 current) {
  if (motor_id < 1 || motor_id > 8) {
    return 1; // Invalid motor ID
  }
  motor_current_targets[motor_id - 1] = current;
  return 0; // Success
}

// Transmit the current target currents to all motors
// Call this periodically (e.g., in control loop at 1kHz)
void motor_transmit(void) {
  canTxH tx_header;
  u8 tx_data_1_4[8];
  u8 tx_data_5_8[8];
  u32 unused_mailbox;

  // Prepare header (same for both messages)
  tx_header.ExtId = 0;
  tx_header.IDE = 0;
  tx_header.RTR = 0;
  tx_header.DLC = 8;
  tx_header.TransmitGlobalTime = DISABLE;

  // Fill data for motors 1-4 (ID 0x200)
  for (int i = 0; i < 4; i++) {
    tx_data_1_4[i * 2] = (motor_current_targets[i] >> 8) & 0xFF;
    tx_data_1_4[i * 2 + 1] = motor_current_targets[i] & 0xFF;
  }

  // Fill data for motors 5-8 (ID 0x1FF)
  for (int i = 0; i < 4; i++) {
    tx_data_5_8[i * 2] = (motor_current_targets[i + 4] >> 8) & 0xFF;
    tx_data_5_8[i * 2 + 1] = motor_current_targets[i + 4] & 0xFF;
  }

  // Send frame for motors 1-4
  tx_header.StdId = MOTOR_CONTROL_ID_1_4;
  if (bsp.can.send(&tx_header, tx_data_1_4, &unused_mailbox) != 0) {
    return; // Failed to send
  }

  // Send frame for motors 5-8
  tx_header.StdId = MOTOR_CONTROL_ID_5_8;
  if (bsp.can.send(&tx_header, tx_data_5_8, &unused_mailbox) != 0) {
    return; // Failed to send
  }

  return; // Success
}

// Read function - read feedback from motor
static u8 read_feedback(u8 motor_id, motorFb *feedback) {
  if (motor_id < 1 || motor_id > 8 || feedback == NULL) {
    return 1; // Invalid parameters
  }
  *feedback = motor_status[motor_id - 1];
  return 0; // Success
}

// Function to update motor feedback from CAN messages
// Call this from CAN RX interrupt handler
void motor_update_feedback(canRxH *rx_header, u8 *data) {
  if (rx_header == NULL || data == NULL)
    return;

  u32 std_id = rx_header->StdId;
  if (std_id >= 0x201U && std_id <= 0x208U) {
    u8 motor_id = (u8)(std_id - 0x200U); // 1 to 8
    motor_status[motor_id - 1].th = (i16)((data[0] << 8) | data[1]);
    motor_status[motor_id - 1].v = (i16)((data[2] << 8) | data[3]);
    motor_status[motor_id - 1].i = (i16)((data[4] << 8) | data[5]);
    motor_status[motor_id - 1].T = data[6];
  }
}

// Export the motor module
const _MotorMod _motor = {.setup = setup_motor,
                          .set = set_current, // Sets target current
                          .status = read_feedback,
                          .ctrl_sig_daemon = motor_transmit,
                          .update_status = motor_update_feedback};

// Optional: Public function to get current target (for debugging)
i16 motor_get_target_current(u8 motor_id) {
  if (motor_id < 1 || motor_id > 8)
    return 0;
  return motor_current_targets[motor_id - 1];
}