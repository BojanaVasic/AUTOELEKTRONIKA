/* Standard includes. */
#include <stdio.h>
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "extint.h"

/* Hardware simulator utility functions */
#include "HW_access.h"

/* SERIAL SIMULATOR CHANNEL TO USE */
#define COM_CH0 (0)
#define COM_CH1 (1)

	/* TASK PRIORITIES */
#define	prijem_kanal0 ( tskIDLE_PRIORITY + (UBaseType_t)6) //led_bar_task
#define ledovke (tskIDLE_PRIORITY + (UBaseType_t)5) //interrupt ledovke
#define	prijem_kanal1 (tskIDLE_PRIORITY + (UBaseType_t)4 ) //prosledjujemo string sa kanala 1 obradi
#define obrada_rezultata (tskIDLE_PRIORITY + (UBaseType_t)3 ) //obrada interrupta start, stpo, prag i ledovki
#define	slanje_kanal0 (tskIDLE_PRIORITY + (UBaseType_t)2 ) //obrada senzora
#define	display	( tskIDLE_PRIORITY + (UBaseType_t)1 )






static void led_bar_tsk(void* pvParameters);
static void SerialSend_Task(void* pvParameters);
static void SerialReceive0_Task(void* pvParameters);
static void SerialReceive1_Task(void* pvParameters);
static void ispis_both(void);
static void ispis_right(void);
static void ispis_left(void);
void main_demo(void);


static const char both[6] = "both ";
static const char left[6] = "left ";
static const char right[7] = "right ";



static uint8_t sistem_ON;
static uint8_t  r_point;



/* 7-SEG NUMBER DATABASE - ALL HEX DIGITS */
static const uint8_t hex[] = { 0x7c, 0x3f, 0x78, 0x76, 0x31, 0x06, 0x7D, 0x38, 0x79, 0x71 };



static SemaphoreHandle_t LED_INT_BinarySemaphore1;
static SemaphoreHandle_t Display_BinarySemaphore;
static SemaphoreHandle_t TXC_BinarySemaphore;
static SemaphoreHandle_t RXC_BinarySemaphore0;
static SemaphoreHandle_t RXC_BinarySemaphore1;
static QueueHandle_t serial_queue;
static QueueHandle_t serial_queue1;


typedef struct sBELT
{
	uint8_t driver;
	uint8_t co_driver;
} sBELT;

static sBELT seat_belt;

void ispis_both() {
	select_7seg_digit(0); //INT 
	set_7seg_digit(0x00); //nista
	select_7seg_digit(1);
	set_7seg_digit(hex[0]);//b
	select_7seg_digit(2);
	set_7seg_digit(hex[1]);//o
	select_7seg_digit(3);
	set_7seg_digit(hex[2]);//t
	select_7seg_digit(4);
	set_7seg_digit(hex[3]);//h

}
void ispis_left() {
	select_7seg_digit(0);
	set_7seg_digit(0x00);
	select_7seg_digit(1);
	set_7seg_digit(hex[7]);//l
	select_7seg_digit(2);
	set_7seg_digit(hex[8]);
	select_7seg_digit(3);
	set_7seg_digit(hex[9]);
	select_7seg_digit(4);
	set_7seg_digit(hex[2]);

}
void ispis_right() {
	select_7seg_digit(0);
	set_7seg_digit(hex[4]);
	select_7seg_digit(1);
	set_7seg_digit(hex[5]);
	select_7seg_digit(2);
	set_7seg_digit(hex[6]);
	select_7seg_digit(3);
	set_7seg_digit(hex[3]);
	select_7seg_digit(4);
	set_7seg_digit(hex[2]);
}
void ispis_prazan_display() {
	select_7seg_digit(0);
	set_7seg_digit(0x00);
	select_7seg_digit(1);
	set_7seg_digit(0x00);
	select_7seg_digit(2);
	set_7seg_digit(0x00);
	select_7seg_digit(3);
	set_7seg_digit(0x00);
	select_7seg_digit(4);
	set_7seg_digit(0x00);
}


static uint32_t prvProcessRXCInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	if (get_RXC_status(0) != 0) {
		xSemaphoreGiveFromISR(RXC_BinarySemaphore0, &xHigherPTW); 
	}
	if (get_RXC_status(1) != 0) {

		xSemaphoreGiveFromISR(RXC_BinarySemaphore1, &xHigherPTW); //
	}
	portYIELD_FROM_ISR(xHigherPTW); //
}



static void SerialSend_Task(void* pvParameters)
{
	static uint8_t t_point = 0;
	static uint32_t brojac = 0;
	static uint8_t trep = 1;
	static uint8_t led_vrednost = 0x00;
	static const uint8_t c1 = (uint8_t)'T';
	static uint8_t c2, c3, c4;
	for (;;) 
	{
		//printf("task 1\n");
		if (sistem_ON == (uint8_t)1) { 
			send_serial_character(COM_CH0, c1); 
			vTaskDelay(pdMS_TO_TICKS(100));
			if (seat_belt.driver == (uint8_t)0 || seat_belt.co_driver == (uint8_t)0) {
				if (seat_belt.driver == (uint8_t)0 && seat_belt.co_driver == (uint8_t)0) {
					if (t_point > (uint8_t)(sizeof(both) - (u_int)1)) {
						t_point = (uint8_t)0;
					}
					c2 = (uint8_t)both[t_point];
					t_point++;
					send_serial_character(1, c2);

				}
				else if (seat_belt.driver == (uint8_t)0 && seat_belt.co_driver == (uint8_t)1) {
					if (t_point > (uint8_t)(sizeof(left) - (u_int)1)) {
						t_point = (uint8_t)0;
					}
					c3 = (uint8_t)left[t_point];
					t_point++;
					send_serial_character(1, c3);
				}
				else if (seat_belt.co_driver == (uint8_t)0 && seat_belt.driver == (uint8_t)1) {
					if (t_point > (uint8_t)(sizeof(right) - (u_int)1)) {
						t_point = (uint8_t)0;
					}
					c4 = (uint8_t)right[t_point];
					t_point++;
					send_serial_character(1, c4);
				}
				else {
					ispis_prazan_display();
				}
				printf("Udje ovde\n");
				brojac++; //ovo je brojac koji se inkrementira svakih 100ms, iz tog razloga je 32-bitni broj
				if (brojac <= (uint32_t)200) {
					if (brojac % (uint32_t)10 == (uint8_t)0) {
						if (trep == (uint8_t)1) {
							led_vrednost = 0xff;
							trep = (uint8_t)0;
						}
						else {
							led_vrednost = 0x01;
							trep = (uint8_t)1;
						}
					}
				}
				else {
					if (trep == (uint8_t)1) {
						led_vrednost = 0xff;
						trep = (uint8_t)0;
					}
					else {
						led_vrednost = 0x01;
						trep = (uint8_t)1;

					}
				}
				if (sistem_ON == (uint8_t)1) {
					set_LED_BAR((uint8_t)1, led_vrednost); // MISRA: povratna vrednost funkcije set_LED_BAR je INT
				}
				else {
					set_LED_BAR((uint8_t)1, 0x00);
					brojac = (uint8_t)0;
				}

			}
			else {
				printf("oba su zavezana\n");
				set_LED_BAR((uint8_t)1, 0x01); //ONA VRACA INT
				brojac = (uint8_t)0;
			}

		}
	}
}


static void display_task(void* pvParameters) {

	static uint8_t ind = 0;

	for (;;)
	{
		if (sistem_ON == (uint8_t)1) {
			if (seat_belt.driver == (uint8_t)0 && seat_belt.co_driver == (uint8_t)0 && ind == (uint8_t)1) {
				ispis_both();
			}
			else if (seat_belt.driver == (uint8_t)0 && seat_belt.co_driver == (uint8_t)1) {
				ispis_left();
			}
			else if (seat_belt.co_driver == (uint8_t)0 && seat_belt.driver == (uint8_t)1) {
				ispis_right();
			}
			else {
				ispis_prazan_display();
			}
			ind = (uint8_t)1;
		}
		else {
			ispis_prazan_display();
		}
	}
}

static void SerialReceive0_Task(void* pvParameters)
{
	uint8_t cc;
	char tmp_str[100], string_queue[100];
	static uint8_t i = 0, tmp;
	for (;;)
	{
		printf("task 2\n");
		xSemaphoreTake(RXC_BinarySemaphore0, portMAX_DELAY);
		get_serial_character(COM_CH0, &cc);
		//printf("karakter koji pristize %c\n", cc);
		if (cc != (uint8_t)43) {
			if (cc >= (uint8_t)65 && cc <= (uint8_t)90) {
				tmp = cc + (uint8_t)32;
				tmp_str[i++] = (char)tmp;
			}
			else {
				tmp_str[i++] = (char)cc;
			}
		}
		else {
			tmp_str[i] = '\0';
			i = 0;
			printf("String sa serijske %s \n", tmp_str);
			strcpy(string_queue, tmp_str);

			xQueueSend(serial_queue1, &string_queue, 0);
			printf("Red za task 3 \n");
		}
	}
}

static uint32_t OnLED_ChangeInterrupt()
{
	BaseType_t higherPriorityTaskWoken = pdFALSE;
	printf("usao u onledchange\n");
	xSemaphoreGiveFromISR(LED_INT_BinarySemaphore1, &higherPriorityTaskWoken);
	portYIELD_FROM_ISR((uint32_t)higherPriorityTaskWoken);
}

static void start(void* pvParameters) { 
	uint8_t led_tmp;
	char string_start[6];
	string_start[0] = 's';
	string_start[1] = 't';
	string_start[2] = 'a';
	string_start[3] = 'r';
	string_start[4] = 't';
	string_start[5] = '\0';

	char string_stop[5];
	string_stop[0] = 's';
	string_stop[1] = 't';
	string_stop[2] = 'o';
	string_stop[3] = 'p';
	string_stop[4] = '\0';

	for (;;) {
		printf("START FUNKCIJA\n");
		xSemaphoreTake(LED_INT_BinarySemaphore1, portMAX_DELAY); //vraca pdtrue ili pdfalse
		get_LED_BAR((uint8_t)0, &led_tmp);
		printf("LED_TMP %d \n", led_tmp);
		if (led_tmp == (uint8_t)1) { 
		//	printf("iz funkcije %s \n", string_start);
			xQueueSend(serial_queue1, &string_start, 0);
			//	printf("Posle slanja reda start\n");
		}
		else { 
		//	printf("iz funkcije %s \n", string_stop);
			xQueueSend(serial_queue1, &string_stop, 0);
			//	printf("Posle slanja reda stop\n");

		}
	}
}

static void SerialReceive1_Task(void* pvParameters)
{
	uint8_t cc = 0;
	char tmp_str[100], string_queue[100];
	static uint8_t i = 0, tmp;

	for (;;)
	{
		xSemaphoreTake(RXC_BinarySemaphore1, portMAX_DELAY);
		printf("Ako smo kliknuli na send text - kanal 1\n");
		get_serial_character(COM_CH1, &cc);
		printf("karakter koji pristize %c\n", cc);
		if (cc != (uint8_t)43) {
			if (cc >= (uint8_t)65 && cc <= (uint8_t)90) { //velika slova prebacujemo u mala
				tmp = cc + (uint8_t)32;
				tmp_str[i++] = (char)tmp;
			}
			else {
				tmp_str[i++] = (char)cc;
			}
		}
		else {
			tmp_str[i] = '\0';
			i = 0;
			printf("String sa serijske %s \n", tmp_str);
			strcpy(string_queue, tmp_str);

			xQueueSend(serial_queue1, &string_queue, 0); //pdtrue i pdfalse
			printf("Red za task 3 \n");
		}
	}
}

static void obrada_senzora(void* pvParameters) {  // opet parametri

	char string_queue[100];
	static uint8_t i = 5, j = 2, digitalni = 0, pravilan_unos = 1, switch_pos = 4, cifra;
	static uint16_t suma = 0, suma1 = 0, analogni = 0;
	static uint16_t prag = 400;


	for (;;)
	{
		printf("pre primanja reda - obrada\n");
		xQueueReceive(serial_queue1, &string_queue, portMAX_DELAY); // pdtrue ili pdfalse
		printf("Posle primanja reda reda\n");
		printf("string ledovka: %s \n", string_queue);
		if (strcmp(string_queue, "start") == 0) {//start
			switch_pos = (uint8_t)0;
			printf("ovde usao na ledovku");
		}
		else if (strcmp(string_queue, "stop") == 0) {//stop+
			switch_pos = (uint8_t)1;
		}
		else if (string_queue[0] == 'p' && string_queue[1] == 'r' && string_queue[2] == 'a' && string_queue[3] == 'g' && string_queue[4] == 32) { //prag ....
			switch_pos = (uint8_t)2;
		}
		else if ((string_queue[0] == 48 || string_queue[0] == 49) && string_queue[1] == 32) {  //ignorisemo misra pravila
			switch_pos = (uint8_t)3;
			//MISRA: IGNORISALI ELSE iz razloga sto u slucaju da nista nije ispunjeno, nista i ne treba da radi
		}

		switch (switch_pos) {
		case 0:printf("START \n");
			sistem_ON = 1;
			seat_belt.driver = (uint8_t)0;
			seat_belt.co_driver = (uint8_t)0;
			digitalni = 0;
			analogni = 0;
			prag = 400;
			printf("Ukljucio timer\n");
			break;
		case 1:printf("STOP \n");
			sistem_ON = (uint8_t)0;
			seat_belt.driver = (uint8_t)0;
			seat_belt.co_driver = (uint8_t)0;
			digitalni = (uint8_t)0;
			analogni = (uint8_t)0;
			prag = (uint8_t)400;
			set_LED_BAR((uint8_t)1, 0x00); //MISRA: povratna vrednost je INT
			break;

		case 2: printf("PRAG \n");
			if (string_queue[i] >= '0' && string_queue[i] <= '9') {
				while (string_queue[i] != '\0') {
					cifra = string_queue[i++] - 48;//preskocili misra
					suma = suma * 10 + cifra;
				}
			}
			else {
				pravilan_unos = (uint8_t)0;
				break;
			}
			i = 5;
			if (pravilan_unos == (uint8_t)1) {
				prag = suma;

				if (digitalni == (uint8_t)1) {
					seat_belt.driver = (uint8_t)1;
				}
				else {
					seat_belt.driver = (uint8_t)0;
				}
				if (analogni > prag) {
					seat_belt.co_driver = (uint8_t)1;
				}
				else {
					seat_belt.co_driver = (uint8_t)0;
				}
			}
			suma = (uint16_t)0;
			pravilan_unos = (uint8_t)1;
			printf("Vrednost praga %d\n", prag);
			printf("Vrednost digitalnog senzora * %d\n", digitalni);
			printf("Vrednost analognog senzora *%d\n", analogni);
			break;

		case 3: printf("SENZORI\n");
			digitalni = string_queue[0] - 48;  //misra preskacemo
			j = 2;
			while (string_queue[j] != '\0') {
				if (string_queue[j] >= '0' && string_queue[j] <= '9') {
					cifra = string_queue[j++] - 48;
					suma1 = suma1 * 10 + cifra;
				}
				else {
					pravilan_unos = (uint8_t)0;
					break;
				}
			}
			j = 2;
			if (pravilan_unos == (uint8_t)1) {
				analogni = suma1;

				if (digitalni == (uint8_t)1) {
					seat_belt.driver = (uint8_t)1;
				}
				else {
					seat_belt.driver = (uint8_t)0;
				}

				if (analogni > prag) {
					seat_belt.co_driver = (uint8_t)1;
				}
				else {
					seat_belt.co_driver = (uint8_t)0;
				}
			}
			suma1 = 0;
			pravilan_unos = (uint8_t)1;
			printf("Vrednost praga %d\n", prag);
			printf("Vrednost digitalnog senzora %d i seat_belt.drivera  **%d\n", digitalni, seat_belt.driver);
			printf("Vrednost analognog senzora %d i seat_belt.co_drivera **%d\n", analogni, seat_belt.co_driver);
			break;
		default: printf("Usao u default\n");
			break;
		}


	}

}







/* MAIN - SYSTEM STARTUP POINT */
void main_demo(void) 
{
	init_LED_comm();
	init_7seg_comm();
	//samo primamo podatke sa serijske
	init_serial_downlink(COM_CH0);// inicijalizacija serijske RX na kanalu 0 //POVRATNA VREDNOST
	init_serial_uplink(COM_CH0);//POVRATNA VREDNOST
	init_serial_downlink(COM_CH1);// inicijalizacija serijske RX na kanalu 0//POVRATNA VREDNOST
	init_serial_uplink(COM_CH1);//POVRATNA VREDNOST

	/* Create LED interrapt semaphore */
	LED_INT_BinarySemaphore1 = xSemaphoreCreateBinary();
	Display_BinarySemaphore = xSemaphoreCreateBinary();

	BaseType_t status;
	//task za obradu senzora
	status = xTaskCreate(obrada_senzora, "obrada senzora", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)obrada_rezultata, NULL);
	if (status != pdPASS) { for (;;); }

	status = xTaskCreate(start, "start", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)ledovke, NULL);
	if (status != pdPASS) { for (;;); }
	//TASK 4
	status = xTaskCreate(display_task, "display", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)display, NULL);

	if (status != pdPASS) { for (;;); }
	//kreiranje reda*

	serial_queue1 = xQueueCreate(1, 10 * sizeof(char));


	/* SERIAL TRANSMITTER TASK */

	/* SERIAL RECEIVER TASK */
	//TASK 2
	xTaskCreate(SerialReceive0_Task, "SRx", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)prijem_kanal0, NULL);
	r_point = 0;

	xTaskCreate(SerialReceive1_Task, "SRx", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)prijem_kanal1, NULL);
	r_point = 0;

	//TASK 2b
	xTaskCreate(SerialSend_Task, "STx", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)slanje_kanal0, NULL);
	/* Create TBE semaphore - serial transmit comm */
	RXC_BinarySemaphore0 = xSemaphoreCreateBinary();
	RXC_BinarySemaphore1 = xSemaphoreCreateBinary();
	TXC_BinarySemaphore = xSemaphoreCreateBinary();
	/* SERIAL RECEPTION INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_RXC, prvProcessRXCInterrupt);

	vPortSetInterruptHandler(portINTERRUPT_SRL_OIC, OnLED_ChangeInterrupt);


	ispis_prazan_display();
	set_LED_BAR(1, 0x00);

	vTaskStartScheduler();

	for (;;);
}







