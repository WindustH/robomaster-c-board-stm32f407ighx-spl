#include "bsp.h"
#include "stm32f4xx.h"

static void setup_impl(void) {
  // 1. Enable GPIO and CAN1 clocks
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

  // 2. Configure GPIOD pins 0 (RX) and 1 (TX) for CAN1
  // Set mode to Alternate Function
  GPIOD->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1);
  GPIOD->MODER |= (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1);
  // Set output type to Push-Pull
  GPIOD->OTYPER &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1);
  // Set speed to Very High
  GPIOD->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1);
  // No pull-up, pull-down
  GPIOD->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1);
  // Set Alternate Function to AF9 (CAN1)
  GPIOD->AFR[0] &= ~(GPIO_AFRL_AFSEL0 | GPIO_AFRL_AFSEL1);
  GPIOD->AFR[0] |= (9 << GPIO_AFRL_AFSEL0_Pos) | (9 << GPIO_AFRL_AFSEL1_Pos);

  // 3. Configure CAN1
  // Enter initialization mode
  CAN1->MCR |= CAN_MCR_INRQ;
  while (!(CAN1->MSR & CAN_MSR_INAK))
    ;

  // Exit sleep mode
  CAN1->MCR &= ~CAN_MCR_SLEEP;

  // Configure bit timing for 1Mbps (assuming 42MHz APB1 clock)
  // Prescaler = 3, BS1 = 12, BS2 = 1, SJW = 1
  CAN1->BTR = (0 << CAN_BTR_SJW_Pos) | (11 << CAN_BTR_TS1_Pos) |
              (0 << CAN_BTR_TS2_Pos) | (2 << CAN_BTR_BRP_Pos);

  // Configure other options: normal mode, no auto retransmission
  CAN1->MCR &= ~CAN_MCR_NART;

  // 5. Leave initialization mode
  CAN1->MCR &= ~CAN_MCR_INRQ;
  while (CAN1->MSR & CAN_MSR_INAK)
    ;

  // 6. Enable FIFO 0 message pending interrupt
  CAN1->IER |= CAN_IER_FMPIE0;

  // 7. Enable CAN1 RX0 interrupt in NVIC
  NVIC_SetPriority(CAN1_RX0_IRQn, 1);
  NVIC_EnableIRQ(CAN1_RX0_IRQn);
}

static u8 transmit_impl(canTxH *pHeader, u8 aData[], u32 *pTxMailbox) {
  // Find an empty mailbox
  u32 txMailbox = (CAN1->TSR & CAN_TSR_CODE) >> CAN_TSR_CODE_Pos;
  if (txMailbox > 2) {
    return 1; // Error, no empty mailbox
  }
  *pTxMailbox = txMailbox;

  // Set up the identifier
  CAN1->sTxMailBox[txMailbox].TIR =
      (pHeader->StdId << CAN_TI0R_STID_Pos) | (0 << CAN_TI0R_IDE_Pos);
  // Set up the DLC
  CAN1->sTxMailBox[txMailbox].TDTR = pHeader->DLC;
  // Set up the data
  CAN1->sTxMailBox[txMailbox].TDLR = ((u32)aData[3] << 24) |
                                     ((u32)aData[2] << 16) |
                                     ((u32)aData[1] << 8) | ((u32)aData[0]);
  CAN1->sTxMailBox[txMailbox].TDHR = ((u32)aData[7] << 24) |
                                     ((u32)aData[6] << 16) |
                                     ((u32)aData[5] << 8) | ((u32)aData[4]);

  // Request transmission
  CAN1->sTxMailBox[txMailbox].TIR |= CAN_TI0R_TXRQ;

  return 0; // Success
}

static u8 receive_impl(canRxH *pHeader, u8 aData[]) {
  if ((CAN1->RF0R & CAN_RF0R_FMP0) == 0) {
    return 1; // Error, no message pending
  }

  // Get the identifier
  pHeader->StdId =
      (CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_STID) >> CAN_RI0R_STID_Pos;
  // Get the DLC
  pHeader->DLC =
      (CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC) >> CAN_RDT0R_DLC_Pos;
  // Get the data
  aData[0] = (CAN1->sFIFOMailBox[0].RDLR >> 0) & 0xFF;
  aData[1] = (CAN1->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
  aData[2] = (CAN1->sFIFOMailBox[0].RDLR >> 16) & 0xFF;
  aData[3] = (CAN1->sFIFOMailBox[0].RDLR >> 24) & 0xFF;
  aData[4] = (CAN1->sFIFOMailBox[0].RDHR >> 0) & 0xFF;
  aData[5] = (CAN1->sFIFOMailBox[0].RDHR >> 8) & 0xFF;
  aData[6] = (CAN1->sFIFOMailBox[0].RDHR >> 16) & 0xFF;
  aData[7] = (CAN1->sFIFOMailBox[0].RDHR >> 24) & 0xFF;

  // Release the FIFO
  CAN1->RF0R |= CAN_RF0R_RFOM0;

  return 0; // Success
}

// Forward declaration for motor update function
void motor_update_feedback(canRxH *rx_header, uint8_t *data);

void CAN1_RX0_IRQHandler(void) {
  u8 rxData[8];
  canRxH rxHeader;

  if ((CAN1->RF0R & CAN_RF0R_FMP0) != 0) {
    receive_impl(&rxHeader, rxData);
    // Update motor feedback if this is a motor message
    motor_update_feedback(&rxHeader, rxData);
    // Process data (e.g., put it in a queue)
  }
}

const _CanMod _can = {
    .setup = setup_impl, .send = transmit_impl, .read = receive_impl};