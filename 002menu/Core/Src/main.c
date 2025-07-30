/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * @Author		 	: Santosh SV
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/*
 * The following handles are the handles for the tasks created in this application.
 */
TaskHandle_t menutaskhandle;
TaskHandle_t cmdtaskhandle;
TaskHandle_t printtaskhandle;
TaskHandle_t ledtaskhandle;
TaskHandle_t rtctaskhandle;

BaseType_t status;

QueueHandle_t q_data; // Queue to hold the user input data
QueueHandle_t q_print;

volatile uint8_t user_data;

volatile state_t curr_state = sMainMenu; // Current state of the application

TimerHandle_t led_handle_timer[3]; // Timer handles for the LED effects

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

void menutask(void *pvParameters);
void cmdtask(void *pvParameters);
void printtask(void *pvParameters);
void ledtask(void *pvParameters);
void rtctask(void *pvParameters);

/*
 * Sub functions declaration
 */

void process_command(command_t *cmd);
int extract_command(command_t *cmd);

/*
 * LED Effect function declaration
 */

void ledcallbackfunc(TimerHandle_t xTimer);

void led_effect1(void);
void led_effect2(void);
void led_effect(int n);
void led_effect_stop(void);

/*
 * Rtc functions declaration
 */

void show_time_date(void);
void rtc_configure_time(RTC_TimeTypeDef *time);
int validate_rtc_information(RTC_TimeTypeDef *time , RTC_DateTypeDef *date);
void show_time_date_itm(void);

uint8_t getnumber(uint8_t *p , int len);
int validate_rtc_information(RTC_TimeTypeDef *time , RTC_DateTypeDef *date);
void rtc_configure_date(RTC_DateTypeDef *date);
void rtc_configure_time(RTC_TimeTypeDef *time);
void show_time_date(void);
void show_time_date_itm(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  status = xTaskCreate(menutask, "menu task", 250, NULL, 2, &menutaskhandle);
  configASSERT(status == pdPASS);

  status = xTaskCreate(cmdtask, "cmd task", 250, NULL, 2, &cmdtaskhandle);
  configASSERT(status == pdPASS);

  status = xTaskCreate(printtask, "Print task", 250, NULL, 2, &printtaskhandle);
  configASSERT(status == pdPASS);

  status = xTaskCreate(ledtask, "led task", 250, NULL, 2, &ledtaskhandle);
  configASSERT(status == pdPASS);

  status = xTaskCreate(rtctask, "rtc task", 250, NULL, 2, &rtctaskhandle);
  configASSERT(status == pdPASS);

  q_data = xQueueCreate(10, sizeof(char*));
  configASSERT(q_data != NULL);

  q_print = xQueueCreate(10, sizeof(size_t*));
  configASSERT(q_print != NULL);

  for(int i = 0; i < 3; i++)
  {
	  led_handle_timer[i] = xTimerCreate("led timer", pdMS_TO_TICKS(1000), pdTRUE, (void*)(i+1), ledcallbackfunc);
  }

  HAL_UART_Receive_IT(&huart2, (uint8_t*)&user_data, 1);

  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_12;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t dummy ; // Dummy variable to receive data from the queue.The stored data will be discarded and we can add the newline character to the queue.
	/*
	 * Now the first thing we do is to check if the queues is full or not.
	 * And this is done by using the API function "xQueueIsQueueFullFromISR()".
	 */
	for(uint32_t i = 0 ; i < 4000 ; i++);

	if(!xQueueIsQueueFullFromISR(q_data)) //Here we are checking if the queue is full or not
	{
		/*
		 * If the queue is not full, we send the data to the queue.
		 * The first parameter is the queue handle, the second parameter is the data to be sent,
		 * and the third parameter is NULL because we are not using a task notification.
		 */
		xQueueSendToBackFromISR(q_data, (uint8_t*)&user_data, NULL);
	}
	else
	{
		if(user_data == '\n')
		{
			/*
			 * The main function after checking if the user data is a newline character is to add it to the queue.
			 * But before that, we need to remove the last element from the queue.
			 * This is done by using the API function "xQueueReceiveFromISR()".
			 * This function will remove the last element from the queue and store it in the dummy variable.
			 * And we add the newline character to the queue using the API function "xQueueSendFromISR()".
			 */
			xQueueReceiveFromISR(q_data,&dummy, NULL);
			xQueueSendFromISR(q_data, (uint8_t*)&user_data, NULL);
		}
	}
	if(user_data == '\n')
	{
		/*
		 * If the user data is a newline character, we send a notification to the cmd task.
		 * This is done by using the API function "xTaskNotifyFromISR()".
		 * The first parameter is the task handle, the second parameter is the notification value,
		 * and the third parameter is the notification action.
		 */
		xTaskNotifyFromISR(cmdtaskhandle, 0, eNoAction, NULL);
	}

	HAL_UART_Receive_IT(&huart2, (uint8_t*)&user_data, 1); // We need to call the HAL_UART_Receive_IT() function again to receive the next character.
}



void menutask(void *pvParameters)
{
  /* Menu task implementation */

	/*
	 * Menu task is responsible for displaying the main menu and processing the user input.
	 * This task contains the options like LED effect, RTC configuration, and exit.
	 * The task will wait for a notification from the cmd task and then process the command.
	 *
	 *
	 * The function of this task can be summarized as follows:
	 *
	 * 1. Display the main menu.
	 * 2. Wait for a notification from the cmd task.(Based on the user input you will send a notification to this task)
	 * 3.Check the length of the command received.(If it exceeds one its a invalid command)
	 * 4. If the command is valid, process the command based on the user input.(Through the switch case statement)
	 */
	uint32_t cmd_addr;
	command_t *cmd;
	int option;

	const char *menu_msg =  "\n========================\n"
									"Main Menu:\n"
			 	 	 	 	 "========================\n"
					"1. LED Effect\n"
					"2. RTC Configuration\n"
					"3. Exit\n"
					"\nPlease enter your choice with a number:";

	const char *menu_inv_msg =  "\n========================\n"
								"Invalid command\n"
			 	 	 	 	 	 "========================\n";
	while (1)
	{
		 xQueueSend(q_print, &menu_msg, portMAX_DELAY);
		 xTaskNotifyWait(0,0,&cmd_addr,portMAX_DELAY); // Wait for a notification from the cmd task
		 cmd = (command_t*)cmd_addr; // Cast the address to command_t pointer

		 if(cmd -> len ==1)
		 {
			 option = cmd -> payload[0] - 48; // Convert ASCII to integer
			 switch(option)
			 {
			 case 1:{
				 curr_state = sLedEffect;
				 xTaskNotify(ledtaskhandle, 0, eNoAction);
				 break;}
			 case 2:{
				 curr_state = sRtcMenu;
				 xTaskNotify(rtctaskhandle, 0, eNoAction);
				 break;}
			 case 3:
				 break;
			 default :{
				 xQueueSend(q_print, &menu_inv_msg, portMAX_DELAY);
				 continue;}
			 }
		 }
		 else
		 {
			 xQueueSend(q_print,&menu_inv_msg,portMAX_DELAY);
			 continue;
		 }

		 xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
	}

}



void cmdtask(void *pvParameters)
{
	BaseType_t ret;
	command_t cmd;
	/* Command task implementation */
	while (1)
	  {
		  ret = xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		  if(ret == pdTRUE)
		  {
			  process_command(&cmd);//For the code Ref @Sub functions
		  }

	  }
}


void printtask(void *pvParameters)
{
	uint32_t *msg;
	while(1)
	{
		xQueueReceive(q_print, &msg, portMAX_DELAY);
		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen((char*)msg), HAL_MAX_DELAY);
	 }
}

void ledtask(void *pvParameters)
{
  /* LED task implementation */

	/*
	 * This task is responsible for handling the LED effects, the first option in the main menu.
	 * The working of the task is as follows:
	 * 1. Wait for a notification from the cmd task.
	 * 2.IF the notification arrived from the cmd task then display the LED effect menu.
	 * 3.Wait for the notification from the cmd task again.
	 * 4.Now check the length of the command and process it that is send notification to the respective task.
	 * 5.If the command is invalid then display the invalid command message.
	 * 6.Lastly wait for the notification from the cmd task again.(Since the function are in a infinite loop they will execute again and again
	 *   to overcome this we use the xTaskNotifyWait() function to wait for the notification from the cmd task.)
	 */

	  while (1)
	  {
		  uint32_t cmd_addr;
		  command_t *cmd;

		  const char* led_msg = "\n========================\n"
				  	  	  	  "LED Effect Menu:\n"
				  	  	  	  "========================\n"
							  "1. Blink\n"
							  "2. Heart Beat\n"
				  	  	  	  "3. No Effect\n"
							  "4. Back to Main Menu\n"
							  "\nPlease enter your choice with a number:";

		  const char *led_inv_msg =  "\n========================\n"
									"Invalid command\n"
				  	  	  	  	  	  "========================\n";
		  while(1)
		  {
			 xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

			  xQueueSend(q_print, &led_msg, portMAX_DELAY);

			  xTaskNotifyWait(0,0,(uint32_t*)&cmd_addr,portMAX_DELAY); // Wait for a notification from the cmd task
			  cmd = (command_t*)cmd_addr; // Cast the address to command_t pointer

			  if(cmd->len == 1)
			  {
				  switch(cmd->payload[0])
				  {
				  case '1':
					  led_effect(1);
					  break;
				  case '2':
					  led_effect(2);
					  break;
				  case '3':
					  led_effect_stop();
					  break;
				  case '4':
					  curr_state = sMainMenu;
					  xTaskNotify(menutaskhandle, 0, eNoAction);
					  break;
				  default:{
					  xQueueSend(q_print, &led_inv_msg, portMAX_DELAY);
					  break;}
				  }
			  }
			  else
			  {
				  xQueueSend(q_print, &led_inv_msg, portMAX_DELAY);
			  }

			  curr_state = sMainMenu; // Reset the state to main menu after processing the command

			  xTaskNotify(menutaskhandle, 0, eNoAction); // Notify the menu task to display the main menu again
			  cmd = (command_t*)cmd_addr; // Reset the command pointer to NULL


		  }
	  }
}

void rtctask(void *pvParameters)
{
		const char* msg_rtc1 = "\n========================\n"
								"|         RTC          |\n"
								"========================\n"
								"Configure Time 		  ----> 0\n"
								"Configure Date            ----> 1\n"
								"Enable reporting          ----> 2\n"
								"Exit                      ----> 3\n"
								"\nEnter your choice here : ";

		const char *rtc_inv =  "\n========================\n"
								"Invalid command\n"
				 	 	 	  "========================\n";

		const char *msg_rtc_hh = "Enter hour(1-12):";
		const char *msg_rtc_mm = "Enter minutes(0-59):";
		const char *msg_rtc_ss = "Enter seconds(0-59):";
		const char *msg_rtc_tf = "Enter AM or PM (AM = 0 ; PM = 1)";

		const char *msg_rtc_dd  = "Enter date(1-31):";
		const char *msg_rtc_mo  ="Enter month(1-12):";
		const char *msg_rtc_dow  = "Enter day(1-7 sun:1):";
		const char *msg_rtc_yr  = "Enter year(0-99):";


		const char *msg_conf = "\nConfiguration successful\n";
		const char *msg_conf_inv = "\nConfiguration failed please verify the input\n";
		const char *msg_rtc_report = "Enable time&date reporting(y/n)?: ";


		uint32_t cmd_addr;
		command_t *cmd;

		int rtc_state = 0;

		RTC_TimeTypeDef time;
		RTC_DateTypeDef day;
		while(1)
		{
			xTaskNotifyWait(0,0,NULL,portMAX_DELAY);

			show_time_date();

			xQueueSend(q_print, &msg_rtc1, portMAX_DELAY);

			while(curr_state != sMainMenu)
			{
				xTaskNotifyWait(0, 0, &cmd_addr, portMAX_DELAY);
				cmd = (command_t*)cmd_addr;
				switch(curr_state)
				{
				    case sRtcMenu:
				    {
				        /* process RTC menu commands */
				        if(cmd->len == 1)
				        {
				            int menu_code = cmd->payload[0] - 48;
				            switch(menu_code)
				            {
				                case 0:
				                	curr_state = sRtcTimeConfig;
				                    xQueueSend(q_print, &msg_rtc_hh, portMAX_DELAY);
				                    break;
				                case 1:
				                    curr_state = sRtcDateConfig;
				                    xQueueSend(q_print, &msg_rtc_dd, portMAX_DELAY);
				                    break;
				                case 2:
				                    curr_state = sRtcReport;
				                    xQueueSend(q_print, &msg_rtc_report, portMAX_DELAY);
				                    break;
				                case 3:
				                    curr_state = sMainMenu;
				                    break;
				                default:
				                    curr_state = sMainMenu;
				                    xQueueSend(q_print, &rtc_inv, portMAX_DELAY);
				                    continue;
				            }
				        }
				        else {
				            curr_state = sMainMenu;
				            xQueueSend(q_print, &rtc_inv, portMAX_DELAY);
				            break;
				        }
				        break;
				    }
				    case sRtcTimeConfig:
				        {
				        	switch(rtc_state)
				        	{
				        	case HH_config:
				        	{
				        		uint8_t hour = getnumber(cmd -> payload, cmd -> len);
				        		time.Hours = hour;
				        		rtc_state = MM_config;
				        		xQueueSend(q_print, &msg_rtc_mm, portMAX_DELAY);
				        		break;
				        	}
				        	case MM_config:
				        	{
				        		uint8_t min = getnumber(cmd -> payload, cmd -> len);
				        		time.Minutes = min;
				        		rtc_state = SS_config;
				        		xQueueSend(q_print, &msg_rtc_ss, portMAX_DELAY);
				        		break;
				        	}
				        	case SS_config:
				        	{
				        		uint8_t sec = getnumber(cmd -> payload, cmd -> len);
				        		time.Seconds = sec;
				        		rtc_state = Timeformat_config;
				        		xQueueSend(q_print, &msg_rtc_tf, portMAX_DELAY);
				        		break;
				        	}
				        	case Timeformat_config:
				        	{
				        		uint8_t timefor = getnumber(cmd -> payload, cmd -> len);
				        		time.TimeFormat = timefor;
				        		if(!validate_rtc_information(&time, NULL))
				        		{
				        			rtc_configure_time(&time);
				        			xQueueSend(q_print, &msg_conf, portMAX_DELAY);
				        			show_time_date();
				        		}
				        		else
				        			xQueueSend(q_print, &msg_conf_inv, portMAX_DELAY);

				        		curr_state = sMainMenu;
								rtc_state = 0;
								break;
				        	}
				        	}//end of switch(rtc_state)
				        	break;
				        }//end of case sRtcTimeConfig
				    case sRtcDateConfig:
				    {
				    	switch(rtc_state)
				    	{
							case (Date_config):
							{
								uint8_t date = getnumber(cmd -> payload, cmd -> len);
								day.Date = date;
								rtc_state = Month_config;
								xQueueSend(q_print, &msg_rtc_mo,portMAX_DELAY);
								break;
							}
							case (Month_config):
							{
								uint8_t month = getnumber(cmd -> payload, cmd ->len);
								day.Month = month;
								rtc_state = Day_config;
								xQueueSend(q_print, &msg_rtc_dow, portMAX_DELAY);
								break;
							}
							case (Day_config):
							{
								uint8_t dow = getnumber(cmd ->payload, cmd -> len);
								day.WeekDay = dow;
								rtc_state = Year_config;
								xQueueSend(q_print, &msg_rtc_yr, portMAX_DELAY);
								break;
							}
							case (Year_config):
							{
								uint8_t year = getnumber(cmd -> payload, cmd ->len);
								day.Year = year;
								if(!validate_rtc_information(NULL, &day))
								{
									rtc_configure_date(&day);
									xQueueSend(q_print, &msg_conf, portMAX_DELAY);
									show_time_date();
								}
								else
									xQueueSend(q_print, &msg_conf_inv, portMAX_DELAY);

								curr_state = sMainMenu;
								rtc_state = 0;
								break;
							}//end of the case year_config

				    	}//end of the switch(rtc_state)
				    	break;
				    }//end of case sRtcDateConfig
				}
			}//end of while(curr_state != sMainMenu)

			xTaskNotify(menutaskhandle, 0, eNoAction); // Notify the menu task to display the main menu again

		}//end of while(1) loop
 }

/*
 * The following functions are used for the working of the rtc task.
 */

void show_time_date_itm(void)
{
	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;

	memset(&rtc_date,0,sizeof(rtc_date));
	memset(&rtc_time,0,sizeof(rtc_time));

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

	char *format;
	format = (rtc_time.TimeFormat == RTC_HOURFORMAT12_AM) ? "AM" : "PM";


	printf("%02d:%02d:%02d [%s]",rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds,format);
	printf("\t%02d-%02d-%2d\n",rtc_date.Month, rtc_date.Date, 2000 + rtc_date.Year);

}

void show_time_date(void)
{
	static char showtime[40];
	static char showdate[40];

	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;

	static char *time = showtime;
	static char *date = showdate;

	memset(&rtc_date,0,sizeof(rtc_date));
	memset(&rtc_time,0,sizeof(rtc_time));

	/* Get the RTC current Time */
	HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
	/* Get the RTC current Date */
	HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

	char *format;
	format = (rtc_time.TimeFormat == RTC_HOURFORMAT12_AM) ? "AM" : "PM";

	/* Display time Format : hh:mm:ss [AM/PM] */
	sprintf((char*)showtime,"%s:\t%02d:%02d:%02d [%s]","\nCurrent Time&Date",rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds,format);
	xQueueSend(q_print,&time,portMAX_DELAY);

	/* Display date Format : date-month-year */
	sprintf((char*)showdate,"\t%02d-%02d-%2d\n",rtc_date.Month, rtc_date.Date, 2000 + rtc_date.Year);
	xQueueSend(q_print,&date,portMAX_DELAY);
}

void rtc_configure_time(RTC_TimeTypeDef *time)
{
	uint8_t tf = time->TimeFormat;

	time->TimeFormat = tf == RTC_HOURFORMAT12_AM ? RTC_HOURFORMAT12_AM : RTC_HOURFORMAT12_PM;
	time->DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
	time->StoreOperation = RTC_STOREOPERATION_RESET;

	HAL_RTC_SetTime(&hrtc,time,RTC_FORMAT_BIN);
}


void rtc_configure_date(RTC_DateTypeDef *date)
{
	HAL_RTC_SetDate(&hrtc,date,RTC_FORMAT_BIN);
}


int validate_rtc_information(RTC_TimeTypeDef *time , RTC_DateTypeDef *date)
{
	if(time){
		if( (time->Hours > 12) || (time->Minutes > 59) || (time->Seconds > 59) )
			return 1;
	}

	if(date){
		if( (date->Date > 31) || (date->WeekDay > 7) || (date->Year > 99) || (date->Month > 12) )
			return 1;
	}

	return 0;
}

uint8_t getnumber(uint8_t *p , int len)
{

	int value ;

	if(len > 1)
	   value =  ( ((p[0]-48) * 10) + (p[1] - 48) );
	else
		value = p[0] - 48;

	return value;

}

/*********************************************************************************************************************************
 	 	 	 	  	  This is the timer call back function which will be called whenever the timer expires.
 *********************************************************************************************************************************/
void ledcallbackfunc(TimerHandle_t xTimer)
{
	uint32_t led_effect_id = (uint32_t)pvTimerGetTimerID(xTimer);//Get the id of the timer that expired

	switch(led_effect_id)
	{
	case 1:
		{
			led_effect1();
			break;
		}
	case 2:
		{
			led_effect2();
			break;
		}
	default:
		{
			/* Do nothing */
			break;
		}
	}
}

/*
 * The following functions are the LED effect functions which will be called when the timer expires.
 */
void led_effect_stop(void)
{
	/*
	 * This function is used to stop the LED effect timer.
	 * It will stop all the timers that are running for the LED effects.
	 */
	for(int i = 0; i < 3; i++)
	{
		if(xTimerIsTimerActive(led_handle_timer[i]))
		{
			xTimerStop(led_handle_timer[i], portMAX_DELAY);
		}
	}
}
void led_effect(int n)
{
	led_effect_stop();
	xTimerStart(led_handle_timer[n-1], portMAX_DELAY); // Start the timer for the LED effect
}

void led_effect1(void)
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // Toggle the LED pin
}

void led_effect2(void)
{
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	 vTaskDelay(pdMS_TO_TICKS(100));
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	 vTaskDelay(pdMS_TO_TICKS(100));

	        // Long blink
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
	 vTaskDelay(pdMS_TO_TICKS(300));
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
	 vTaskDelay(pdMS_TO_TICKS(500)); // Toggle another LED pin
}

/********************************************************************************************************************************
 											The following functions are sub functions
 ********************************************************************************************************************************/

void process_command(command_t *cmd)
{
	extract_command(cmd);

	switch(curr_state)
	{
	case sMainMenu:
		xTaskNotify(menutaskhandle, (uint32_t)cmd, eSetValueWithOverwrite);
		break;
	case sLedEffect:
		xTaskNotify(ledtaskhandle,(uint32_t)cmd, eSetValueWithOverwrite);
		break;
	case sRtcMenu:
	case sRtcDateConfig:
	case sRtcTimeConfig:
	case sRtcReport:
		xTaskNotify(rtctaskhandle,(uint32_t)cmd, eSetValueWithOverwrite);
		break;
	}
}

int extract_command(command_t *cmd)
{
	uint8_t item;
	BaseType_t  status;

	status = uxQueueMessagesWaiting(q_data );
	if(!status) return -1;
	uint8_t i =0;

	do
	{
		status = xQueueReceive(q_data,&item,0);
		if(status == pdTRUE) cmd->payload[i++] = item;
	}while(item != '\n');

	cmd->payload[i-1] = '\0';
	cmd->len = i-1; /*save  length of the command excluding null char */

	return 0;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
