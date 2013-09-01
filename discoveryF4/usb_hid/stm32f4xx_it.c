/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides all exceptions handler and peripherals interrupt
  *          service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"
#include "usb_core.h"
#include "usbd_core.h"
#include "stm32f4_discovery.h"
#include "usbd_hid_core.h"
#include "font_table.h"

//Library config for this project!!!!!!!!!!!
#include "stm32f4xx_conf.h"


/* Private typedef -----------------------------------------------------------*/
typedef enum {
  FALSE,
  TRUE,
} BOOL;

typedef enum {
  Draw_Unfinish,
  Draw_Finish,
} Draw_Status;
/* Private define ------------------------------------------------------------*/
#define CURSOR_STEP     7
#define WIDTH	20
#define STEP_CNT 7

extern uint8_t Buffer[6];
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern __IO uint8_t DemoEnterCondition;
uint8_t Counter  = 0x00;
extern int8_t X_Offset;
extern int8_t Y_Offset;
extern __IO uint8_t UserButtonPressed;
__IO uint8_t TempAcceleration = 0;
uint8_t step[ 4 ][ 2 ] = { { 7, 0 }, { 0, 7 }, { -7, 0 }, { 0, -7 } };
uint8_t StepCnt = 0, CurDir = 0;

/* Private function prototypes -----------------------------------------------*/
extern USB_OTG_CORE_HANDLE           USB_OTG_dev;
static uint8_t *USBD_HID_GetPos (void);
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
void HID_Release();
Draw_Status Draw_Char(int8_t (*InfoArray)[3], uint8_t ArrayCnt, uint8_t FontSize);
void Draw_Words(uint8_t *String, uint8_t Length, uint8_t FontSize);
void Show_Direction_LED(int8_t HID_Bufferi[4]);

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  //uint8_t *buf;
  //uint8_t String[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  //uint8_t String[] = "CODED BY SHENGWEN";
  uint8_t String[] = "NCKU EMBEDDED CLASS - HACK THESE CHIPSETS";
  uint8_t Length = sizeof(String) - 1;
  uint8_t FontSize = 1;

  if ( !DemoEnterCondition )
	  return;
  
  //buf = USBD_HID_GetPos();
  //USBD_HID_SendReport (&USB_OTG_dev, buf, 4);
  
  Draw_Words(String, Length, FontSize);
}

/******************************************************************************/
/*                 STM32Fxxx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32fxxx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  This function handles EXTI0_IRQ Handler.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
  DemoEnterCondition = 0x01;
  
  /* Clear the EXTI line pending bit */
  EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);
}

/**
  * @brief  This function handles EXTI15_10_IRQ Handler.
  * @param  None
  * @retval None
  */
void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
	/* Reset SLEEPDEEP and SLEEPONEXIT bits */
	SCB->SCR &= (uint32_t)~((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));

	/* After wake-up from sleep mode, reconfigure the system clock */
	SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}

/**
  * @brief  This function handles OTG_HS Handler.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

/**
* @brief  USBD_HID_GetPos
* @param  None
* @retval Pointer to report
*/
static uint8_t *USBD_HID_GetPos (void)
{
  static uint8_t HID_Buffer[4] = {0};
  
  if ( StepCnt == 4 * WIDTH) {
	  HID_Release();
	  HID_Buffer[ 1 ] = 5;
	  HID_Buffer[ 2 ] = 5;
	  StepCnt %= 4 * WIDTH;
      return HID_Buffer;
  }

  HID_Buffer[ 0 ] = !(( StepCnt % WIDTH ) % 2);
  HID_Buffer[ 1 ] = step[ CurDir ][ 0 ];
  HID_Buffer[ 2 ] = step[ CurDir ][ 1 ];
  ++StepCnt;
  
  if ( StepCnt % WIDTH == 0 ) {
	  CurDir = ( ++CurDir ) % 4;
  }


  return HID_Buffer;
}

void HID_Release() {
	uint8_t HID_Buffer[4] = {0};
	USBD_HID_SendReport (&USB_OTG_dev, HID_Buffer, 4);
}

/**
* @brief  Automatic control the cursor to draw a char
* @param  Pointer of step array, count of step array
* @retval Draw_Status
*/
Draw_Status Draw_Char(int8_t (*InfoArray)[3], uint8_t ArrayCnt, uint8_t FontSize)
{
  
  static int8_t HID_Buffer[4] = {0};  
  static uint8_t StepRecord = 0;

  static uint8_t DrawCnt = 0;

  //Draw char
  if ( StepRecord < STEP_CNT) {
  	HID_Buffer[ 0 ] = InfoArray[ DrawCnt ][ 0 ];
	HID_Buffer[ 1 ] = InfoArray[ DrawCnt ][ 1 ] * FontSize;
	HID_Buffer[ 2 ] = InfoArray[ DrawCnt ][ 2 ] * FontSize;
	//Send new point
	USBD_HID_SendReport( &USB_OTG_dev, HID_Buffer, 4 );
	StepRecord++;
	
	Show_Direction_LED(HID_Buffer);
	return Draw_Unfinish;
  }
  else {
	DrawCnt++;
	StepRecord = 0;
	//Stop drawing this storke
	HID_Buffer[ 0 ] = 0;
	HID_Buffer[ 1 ] = 0;
	HID_Buffer[ 2 ] = 0;
	USBD_HID_SendReport( &USB_OTG_dev, HID_Buffer, 4 );	
	Show_Direction_LED(HID_Buffer);
  }	
  
  //Check the last char
  if ( DrawCnt == ArrayCnt ) {
	//reset
	StepRecord = 0;
	DrawCnt = 0;
	HID_Buffer[ 0 ] = 0;
	HID_Buffer[ 1 ] = 0;
	HID_Buffer[ 2 ] = 0; 
  	return Draw_Finish;  
  }
}

/**
* @brief  Draw some words
* @param  Pointer of string, uint8_t of length, uint8_t of font size
* @retval None
*/
void Draw_Words(uint8_t *String, uint8_t Length, uint8_t FontSize)
{
  static uint8_t NextChar = 0;
  static BOOL AddSpace = FALSE;  

  Draw_Status Status = Draw_Unfinish;  
  
  if ( AddSpace == TRUE ) {
	Status = Draw_Char(char_Space, sizeof(char_Space) / sizeof(int8_t) / 3, FontSize);
	
	if ( Status == Draw_Finish ) {
  		AddSpace = FALSE;
	}
	return;
  }

  switch( *(String + NextChar) ) {
 	case 'A':
	  Status = Draw_Char(char_A, sizeof(char_A) / sizeof(int8_t) / 3, FontSize);		
	  break;
	/*---------------------------------------------------------------------------*/
	case 'B':
	  Status = Draw_Char(char_B, sizeof(char_B) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'C':
	  Status = Draw_Char(char_C, sizeof(char_C) / sizeof(int8_t) / 3, FontSize);
	break;
	/*---------------------------------------------------------------------------*/
	case 'D':
	  Status = Draw_Char(char_D, sizeof(char_D) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'E':
	  Status = Draw_Char(char_E, sizeof(char_E) / sizeof(int8_t) / 3, FontSize);	
	  break;
	/*---------------------------------------------------------------------------*/
	case 'F':
	  Status = Draw_Char(char_F, sizeof(char_F) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'G':
	  Status = Draw_Char(char_G, sizeof(char_G) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'H':
	  Status = Draw_Char(char_H, sizeof(char_H) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'I':
	  Status = Draw_Char(char_I, sizeof(char_I) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'J':
	  Status = Draw_Char(char_J, sizeof(char_J) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'K':
	  Status = Draw_Char(char_K, sizeof(char_K) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'L':
	  Status = Draw_Char(char_L, sizeof(char_L) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'M':
	  Status = Draw_Char(char_M, sizeof(char_M) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'N':
	  Status = Draw_Char(char_N, sizeof(char_N) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'O':
	  Status = Draw_Char(char_O, sizeof(char_O) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'P':
	  Status = Draw_Char(char_P, sizeof(char_P) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'Q':
	  Status = Draw_Char(char_Q, sizeof(char_Q) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'R':
	  Status = Draw_Char(char_R, sizeof(char_R) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'S':
	  Status = Draw_Char(char_S, sizeof(char_S) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'T':
	  Status = Draw_Char(char_T, sizeof(char_T) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'U':
	  Status = Draw_Char(char_U, sizeof(char_U) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'V':
	  Status = Draw_Char(char_V, sizeof(char_V) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'W':
	  Status = Draw_Char(char_W, sizeof(char_W) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'X':
	  Status = Draw_Char(char_X, sizeof(char_X) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'Y':
	  Status = Draw_Char(char_Y, sizeof(char_Y) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case 'Z':
	  Status = Draw_Char(char_Z, sizeof(char_Z) / sizeof(int8_t) / 3, FontSize);
	  break;
	/*---------------------------------------------------------------------------*/
	case ' ':
	  Status = Draw_Char(char_Space, sizeof(char_Space) / sizeof(int8_t) / 3, FontSize * 2);
	  break;
	/*---------------------------------------------------------------------------*/
	case '-':
	  Status = Draw_Char(char_Dash, sizeof(char_Dash) / sizeof(int8_t) / 3, FontSize);
	  break;
	default:
	  return;
  }
  
  if ( Status == Draw_Finish ) {
	NextChar++;
	AddSpace = TRUE;
  }
}

/**
* @brief  Light up the LED correspond to correct direction
* @param  int8_t of HID_Buffer[4]
* @retval None
*/
void Show_Direction_LED(int8_t HID_Buffer[4])
{
  static uint32_t brightness = 3000;
  static uint32_t regulator = 50;

  if ( (brightness == 3000) || (brightness == 200) ) {
  	regulator = -regulator;
  }
  brightness += regulator;
  
  //check coordinate X
  if ( HID_Buffer[ 1 ] > 0 ) {
	TIM4->CCR4 = brightness;
  }
  else if ( HID_Buffer[ 1 ] < 0 ) {
	TIM4->CCR2 = brightness;
  }
  else {
	TIM4->CCR2 = 0;
	TIM4->CCR4 = 0;
  }

  //Check coordinate Y
  if ( HID_Buffer[ 2 ] > 0 ) {
	TIM4->CCR1 = brightness;
  }
  else if ( HID_Buffer[ 2 ] < 0 ) {
  	TIM4->CCR3 = brightness;
  }
  else {
	TIM4->CCR1 = 0;
	TIM4->CCR3 = 0;
  }


}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
