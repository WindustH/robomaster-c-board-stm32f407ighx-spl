#include "def.h"
#include "mod/bsp.h"
#include "stm32f4xx.h"
#include "type.h"

#define MOTOR_CONTROL_ID_1_4 0x200U
#define MOTOR_CONTROL_ID_5_8 0x1FFU
#define MOTOR_FEEDBACK_BASE 0x200U

static volatile motStat motor_status[8] = {0};
static volatile i16 motor_current_targets[8] = {0};

// Setup function - configure CAN filter for motor feedback
static void setup_motor(void) {
  // Enter filter initialization mode
  CAN1->FMR |= CAN_FMR_FINIT;

  // Disable all filters
  CAN1->FA1R = 0;

  // Configure filter banks 0-3 for IDs 0x201-0x208 (32-bit identifier list
  // mode)
  CAN1->sFilterRegister[0].FR1 = (0x201U << 21) | (0x202U << 5);
  CAN1->sFilterRegister[1].FR1 = (0x203U << 21) | (0x204U << 5);
  CAN1->sFilterRegister[2].FR1 = (0x205U << 21) | (0x206U << 5);
  CAN1->sFilterRegister[3].FR1 = (0x207U << 21) | (0x208U << 5);

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
static u8 set_motor_current(i16 current) {
  motor_current_targets[MOTOR_ID] = current;
  return 0; // Success
}

// Transmit the current target currents to all motors
// Call this periodically (e.g., in control loop at 1kHz)
void send_motor_ctrl_msg() {
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
  for (u8 i = 0; i < 4; i++) {
    tx_data_1_4[i * 2] = (motor_current_targets[i] >> 8) & 0xFF;
    tx_data_1_4[i * 2 + 1] = motor_current_targets[i] & 0xFF;
  }

  // Fill data for motors 5-8 (ID 0x1FF)
  for (u8 i = 0; i < 4; i++) {
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
static const volatile motStat *read_feedback() {
  return &motor_status[MOTOR_ID];
}

// Function to update motor feedback from CAN messages
// Call this from CAN RX interrupt handler
static void update_motor_feedback(canRxH *rx_header, u8 *data) {
  u32 std_id = rx_header->StdId;
  static u16 pre_motor_th[8] = {0};
  if (std_id >= 0x201U && std_id <= 0x208U) {
    u8 motor_id = (u8)(std_id - 0x200U) - 1; // 1 to 8

    // Parse feedback data according to protocol:
    // DATA[0-1]: Rotor mechanical angle (0-8191 for 0-360°)
    // DATA[2-3]: Rotational speed (RPM)
    // DATA[4-5]: Actual torque current
    // DATA[6]: Motor temperature (°C)
    // DATA[7]: Null
    u16 raw_th = (u16)((data[0] << 8) | data[1]);
    i16 delta;
    if (pre_motor_th[motor_id] > raw_th &&
        pre_motor_th[motor_id] - raw_th > ENCODER_ROUND / 2)
      delta = ENCODER_ROUND + raw_th - pre_motor_th[motor_id];
    else if (raw_th - pre_motor_th[motor_id] > ENCODER_ROUND / 2)
      delta = raw_th - ENCODER_ROUND - pre_motor_th[motor_id];
    else
      delta = raw_th - pre_motor_th[motor_id];
    pre_motor_th[motor_id] = raw_th;

    motor_status[motor_id].th += (delta * 2.0f * PI) / (ENCODER_ROUND * 1.0f);
    motor_status[motor_id].v = (i16)((data[2] << 8) | data[3]);
    motor_status[motor_id].i = (i16)((data[4] << 8) | data[5]);
    motor_status[motor_id].T = data[6];
  }
  // bsp.led.show(0x00FFFFFF);
}

static void reset_motor_position() { motor_status[MOTOR_ID].th = 0.0f; }
// Export the motor module
const _MotorMod _motor = {.setup = setup_motor,
                          .set_current =
                              set_motor_current, // Sets target current
                          .status = read_feedback,
                          .send_ctrl_signal = send_motor_ctrl_msg,
                          .update_status = update_motor_feedback,
                          .reset_postition = reset_motor_position};