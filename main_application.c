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
#define	prijem_kanal0 ( tskIDLE_PRIORITY + (UBaseType_t)6) //SerialReceive0
#define ledovke (tskIDLE_PRIORITY + (UBaseType_t)5) //start
#define	prijem_kanal1 (tskIDLE_PRIORITY + (UBaseType_t)4 ) //SerialReceive1
#define obrada_rezultata (tskIDLE_PRIORITY + (UBaseType_t)3 ) //obrada_senzora
#define	slanje_kanal0 (tskIDLE_PRIORITY + (UBaseType_t)2 ) //SerialSend
#define	display	( tskIDLE_PRIORITY + (UBaseType_t)1 ) //display





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
	uint8_t driver; //vezan_vozac ako je jedinica
	uint8_t co_driver; // vezan suvozac ili nije(mi odredjujemo na osnoivu praga i parametra
	uint8_t co_driver_sits; //tu je
} sBELT;

static sBELT seat_belt;

static void ispis_both(void) {
	if (select_7seg_digit(0) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(0x00) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(1) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[0]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(2) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[1]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(3) != 0) {
		printf("Greska_select \n");
	}

	if (set_7seg_digit(hex[2]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(4) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[3]) != 0) {
		printf("Greska_set \n");
	}

}
static void ispis_left(void) {
	if (select_7seg_digit(0) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(0x00) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(1) != 0) {
		printf("Greska_select \n");
	}

	if (set_7seg_digit(hex[7]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(2) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[8]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(3) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[9]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(4) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[2]) != 0) {
		printf("Greska_set \n");
	}

}
static void ispis_right(void) {
	if (select_7seg_digit(0) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[4]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(1) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[5]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(2) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[6]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(3) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[3]) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(4) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(hex[2]) != 0) {
		printf("Greska_set \n");
	}
}
static void ispis_prazan_display(void) {
	if (select_7seg_digit(0) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(0x00) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(1) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(0x00) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(2) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(0x00) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(3) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(0x00) != 0) {
		printf("Greska_set \n");
	}
	if (select_7seg_digit(4) != 0) {
		printf("Greska_select \n");
	}
	if (set_7seg_digit(0x00) != 0) {
		printf("Greska_set \n");
	}
}


static uint32_t prvProcessRXCInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	if (get_RXC_status(0) != 0) {
		if (xSemaphoreGiveFromISR(RXC_BinarySemaphore0, &xHigherPTW) != pdTRUE) {
			printf("Greska \n");
		}
	}
	if (get_RXC_status(1) != 0) {

		if (xSemaphoreGiveFromISR(RXC_BinarySemaphore1, &xHigherPTW) != pdTRUE) {
			printf("Greska \n");
		}
	}
	portYIELD_FROM_ISR((uint32_t)xHigherPTW);
}
static void SerialReceive0_Task(void* pvParameters)
{
	//configASSERT(pvParameters);
	uint8_t cc;
	char tmp_str[100], string_queue[100];
	static uint8_t i = 0;
	for (;;)
	{
		printf("task 2\n");
		if (xSemaphoreTake(RXC_BinarySemaphore0, portMAX_DELAY) != pdTRUE) {
			printf("Greska \n");
		}
		if (get_serial_character(COM_CH0, &cc) != 0) {
			printf("Greska \n");
		}
		//printf("karakter koji pristize %c\n", cc);
		if (cc != (uint8_t)43) { //1 200+
			tmp_str[i++] = (char)cc;

		}
		else {
			tmp_str[i] = '\0';
			i = 0;
			printf("String sa serijske %s \n", tmp_str);
			strcpy(string_queue, tmp_str);

			if (xQueueSend(serial_queue1, &string_queue, 0) != pdTRUE) {
				printf("Greska \n");
			}
			printf("Red za task 3 \n");
		}
	}
}

static uint32_t OnLED_ChangeInterrupt()
{
	BaseType_t higherPriorityTaskWoken = pdFALSE;
	printf("usao u onledchange\n");
	if (xSemaphoreGiveFromISR(LED_INT_BinarySemaphore1, &higherPriorityTaskWoken) != pdTRUE) {
		printf("Greska \n");
	}
	portYIELD_FROM_ISR((uint32_t)higherPriorityTaskWoken);
}
static void start(void* pvParameters) {
	//configASSERT(pvParameters);
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
		if (xSemaphoreTake(LED_INT_BinarySemaphore1, portMAX_DELAY) != pdTRUE) {
			printf("Greska \n");
		}
		if (get_LED_BAR((uint8_t)0, &led_tmp) != 0) {
			printf("Greska_get\n");
		}
		printf("LED_TMP %d \n", led_tmp);
		if (led_tmp == (uint8_t)1) {
			if (xQueueSend(serial_queue1, &string_start, 0) != pdTRUE) {
				printf("Greska \n");
			}
		}
		else {
			if (xQueueSend(serial_queue1, &string_stop, 0) != pdTRUE) {
				printf("Greska \n");
			}
		}
	}
}

static void SerialReceive1_Task(void* pvParameters)
{
	//configASSERT(pvParameters);
	uint8_t cc = 0;
	char tmp_str[100], string_queue[100];
	static uint8_t i = 0, tmp;

	for (;;) //start, stop, prag
	{
		if (xSemaphoreTake(RXC_BinarySemaphore1, portMAX_DELAY) != pdTRUE) {
			printf("Greska \n");
		}
		printf("Ako smo kliknuli na send text - kanal 1\n");
		if (get_serial_character(COM_CH1, &cc) != 0) {
			printf("Greska_get\n");
		}
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

			if (xQueueSend(serial_queue1, &string_queue, 0) != pdTRUE) {
				printf("Greska_get\n");
			}
			printf("Red za task 3 \n");
		}
	}
}

static void obrada_senzora(void* pvParameters) {
	//configASSERT(pvParameters);
	char string_queue[100];
	static uint8_t i = 5, j = 4, digitalni = 0, pravilan_unos = 1, switch_pos = 4, cifra, digitalni_suvozac = 0;
	static uint16_t suma = 0, suma1 = 0, analogni = 0;
	static uint16_t prag = 400;
	char tmp;

	for (;;)
	{
		printf("pre primanja reda - obrada\n");
		if (xQueueReceive(serial_queue1, &string_queue, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}
		printf("Posle primanja reda reda\n");
		printf("string ledovka: %s \n", string_queue);
		if (strcmp(string_queue, "start\0") == 0) {//start
			switch_pos = (uint8_t)0;
			printf("ovde usao na ledovku");
		}
		else if (strcmp(string_queue, "stop\0") == 0) {//stop+
			switch_pos = (uint8_t)1;
		}
		else if (string_queue[0] == 'p' && string_queue[1] == 'r' && string_queue[2] == 'a' && string_queue[3] == 'g' && string_queue[4] == (char)32) { //prag ....
			switch_pos = (uint8_t)2;
		}
		else if ((string_queue[0] == '0' || string_queue[0] == '1') && string_queue[1] == (char)32 && (string_queue[2] == '0' || string_queue[2] == '1') && string_queue[3] == (char)32) {  //ignorisemo misra pravila
			switch_pos = (uint8_t)3;
			//MISRA: IGNORISALI ELSE iz razloga sto u slucaju da nista nije ispunjeno, nista i ne treba da radi
		}
		else {
			printf("Usao u else, misra\n");
		}

		switch (switch_pos) {
		case 0:printf("START \n");
			sistem_ON = 1;
			seat_belt.driver = (uint8_t)0;
			seat_belt.co_driver = (uint8_t)0;
			digitalni = 0;
			analogni = 0;
			prag = 400;
			break;
		case 1:printf("STOP \n");
			sistem_ON = (uint8_t)0;
			seat_belt.driver = (uint8_t)1;
			seat_belt.co_driver = (uint8_t)0;
			digitalni = (uint8_t)0;
			analogni = (uint8_t)0;
			prag = (uint8_t)400;
			if (set_LED_BAR((uint8_t)1, 0x00) != 0) {
				printf("Greska_set \n");
			}
			break;

		case 2: printf("PRAG \n"); //prag 543
			while (string_queue[i] != '\0') {
				if (string_queue[i] >= '0' && string_queue[i] <= '9') {
					cifra = (uint8_t)string_queue[i++] - (uint8_t)48;
					suma = suma * (uint16_t)10 + (uint16_t)cifra;
				}
				else {
					pravilan_unos = (uint8_t)0;
					break;
				}
			}
			i = 5;
			if (pravilan_unos == (uint8_t)1) {
				prag = suma;


				if (analogni < prag) {
					//1 1 300; 1 0 300
					seat_belt.co_driver_sits = 0;
					seat_belt.co_driver = 1; // ignorisi vezivanje
				}
				else {
					//1 1 600; 1 0 700
					seat_belt.co_driver = digitalni_suvozac;
				}
			}
			suma = (uint16_t)0;
			pravilan_unos = (uint8_t)1;
			printf("Vrednost pragau caseu PRAG %d\n", prag);
			printf("Vrednost digitalnog senzora * %d\n", digitalni);
			printf("Vrednost analognog senzora *%d\n", analogni);
			break;

		case 3: printf("SENZORI\n"); // 1 30gfg0
			seat_belt.driver = (uint8_t)string_queue[0] - (uint8_t)48; //vozac vezan ili nije
			printf("***************************%d", seat_belt.driver);
			digitalni_suvozac = (uint8_t)string_queue[2] - (uint8_t)48; // ovo je ono sto mi ocitamo
			j = 4;
			while (string_queue[j] != '\0') {
				if (string_queue[j] >= '0' && string_queue[j] <= '9') {
					cifra = (uint8_t)string_queue[j++] - (uint8_t)48;
					suma1 = suma1 * (uint16_t)10 + (uint16_t)cifra;
				}
				else {
					pravilan_unos = (uint8_t)0;
					break;
				}
			}
			j = 4;
			if (pravilan_unos == (uint8_t)1) {
				analogni = suma1;

				if (analogni < prag) {
					//1 1 300; 1 0 300
					seat_belt.co_driver_sits = (uint8_t)0;
					seat_belt.co_driver = (uint8_t)1; // ignorisi vezivanje
				}
				else {
					//1 1 600; 1 0 700
					seat_belt.co_driver = (uint8_t)digitalni_suvozac;
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

static void SerialSend_Task(void* pvParameters)
{
	//configASSERT(pvParameters);
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
			if (send_serial_character(COM_CH0, c1) != 0) {
				printf("Greska_send \n");
			}
			vTaskDelay(pdMS_TO_TICKS(100));
			if (seat_belt.driver == (uint8_t)0 || seat_belt.co_driver == (uint8_t)0) {
				if (seat_belt.driver == (uint8_t)0 && seat_belt.co_driver == (uint8_t)0) {
					if (t_point > (uint8_t)(sizeof(both) - (u_int)1)) {
						t_point = (uint8_t)0;
					}
					c2 = (uint8_t)both[t_point];
					t_point++;
					if (send_serial_character(1, c2) != 0) {
						printf("Greska \n");
					}
				}
				else if (seat_belt.driver == (uint8_t)0 && seat_belt.co_driver == (uint8_t)1) {
					if (t_point > (uint8_t)(sizeof(left) - (u_int)1)) {
						t_point = (uint8_t)0;
					}
					c3 = (uint8_t)left[t_point];
					t_point++;
					if (send_serial_character(1, c3) != 0) {
						printf("Greska \n");
					}
				}
				else if (seat_belt.co_driver == (uint8_t)0 && seat_belt.driver == (uint8_t)1) {
					if (t_point > (uint8_t)(sizeof(right) - (u_int)1)) {
						t_point = (uint8_t)0;
					}
					c4 = (uint8_t)right[t_point];
					t_point++;
					if (send_serial_character(1, c4) != 0) {
						printf("Greska \n");
					}
				}
				else {
					ispis_prazan_display();
				}
				printf("Udje ovde\n");
				brojac++; //ovo je brojac koji se inkrementira svakih 100ms, iz tog razloga je 32-bitni broj
				if (brojac <= (uint32_t)200) {
					if (brojac % (uint32_t)10 == (uint32_t)0) {
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
					if (set_LED_BAR((uint8_t)1, led_vrednost) != 0) {
						printf("Greska \n");
					}
				}
				else {
					if (set_LED_BAR((uint8_t)1, 0x00) != 0) {
						printf("Greska \n");
					}
					brojac = (uint8_t)0;
				}

			}
			else {
				printf("oba su zavezana\n");
				if (set_LED_BAR((uint8_t)1, 0x01) != 0) {
					printf("Greska \n");
				}
				brojac = (uint8_t)0;
			}

		}
	}
}


static void display_task(void* pvParameters) {
	//configASSERT(pvParameters);
	static uint8_t ind = 0;

	for (;;)
	{
		if (sistem_ON == (uint8_t)1) {
			if (seat_belt.driver == (uint8_t)0 && seat_belt.co_driver == (uint8_t)0 && ind == (uint8_t)1) {
				ispis_both();
			}
			else if (seat_belt.driver == (uint8_t)1 && seat_belt.co_driver == (uint8_t)0) {
				ispis_right();
			}
			else if (seat_belt.co_driver == (uint8_t)1 && seat_belt.driver == (uint8_t)0) {
				ispis_left();
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














/* MAIN - SYSTEM STARTUP POINT */
void main_demo(void)
{
	if (init_LED_comm() != 0) {
		printf("Neuspesna inicijalizacija \n");
	}
	if (init_7seg_comm() != 0) {
		printf("Neuspesna inicijalizacija \n");
	}
	//samo primamo podatke sa serijske
	if (init_serial_downlink(COM_CH0) != 0) {
		printf("Neuspesna inicijalizacija \n");
	}
	if (init_serial_uplink(COM_CH0) != 0) {
		printf("Neuspesna inicijalizacija \n");
	}
	if (init_serial_downlink(COM_CH1) != 0) {
		printf("Neuspesna inicijalizacija \n");
	}
	if (init_serial_uplink(COM_CH1) != 0) {
		printf("Neuspesna inicijalizacija \n");
	}

	/* Create LED interrapt semaphore */
	LED_INT_BinarySemaphore1 = xSemaphoreCreateBinary();
	if (LED_INT_BinarySemaphore1 == NULL) {
		printf("Greska \n");
	}
	Display_BinarySemaphore = xSemaphoreCreateBinary();
	if (Display_BinarySemaphore == NULL) {
		printf("Greska \n");
	}
	BaseType_t status;
	//task za obradu senzora
	status = xTaskCreate(obrada_senzora, "obrada senzora", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)obrada_rezultata, NULL);
	if (status != pdPASS) {
		for (;;) {}
	}

	status = xTaskCreate(start, "start", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)ledovke, NULL);
	if (status != pdPASS)
	{
		for (;;) {}
	}
	//TASK 4
	status = xTaskCreate(display_task, "display", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)display, NULL);

	if (status != pdPASS) {
		for (;;) {}
	}
	//kreiranje reda*

	serial_queue1 = xQueueCreate(1, 10u * sizeof(char));
	if (serial_queue1 == NULL) {
		printf("Greska\n");
	}

	/* SERIAL TRANSMITTER TASK */

	/* SERIAL RECEIVER TASK */
	//TASK 2
	status = xTaskCreate(SerialReceive0_Task, "SRx", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)prijem_kanal0, NULL);
	if (status != pdPASS) {
		for (;;) {}
	}
	r_point = (uint8_t)0;

	status = xTaskCreate(SerialReceive1_Task, "SRx", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)prijem_kanal1, NULL);
	if (status != pdPASS) {
		for (;;) {}
	}
	r_point = (uint8_t)0;

	//TASK 2b
	status = xTaskCreate(SerialSend_Task, "STx", configMINIMAL_STACK_SIZE, NULL, (UBaseType_t)slanje_kanal0, NULL);
	if (status != pdPASS) {
		for (;;) {}
	}
	/* Create TBE semaphore - serial transmit comm */
	RXC_BinarySemaphore0 = xSemaphoreCreateBinary();
	if (RXC_BinarySemaphore0 == NULL) {
		printf("Greska\n");
	}
	RXC_BinarySemaphore1 = xSemaphoreCreateBinary();
	if (RXC_BinarySemaphore1 == NULL) {
		printf("Greska\n");
	}
	TXC_BinarySemaphore = xSemaphoreCreateBinary();
	if (TXC_BinarySemaphore == NULL) {
		printf("Greska\n");
	}
	/* SERIAL RECEPTION INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_RXC, prvProcessRXCInterrupt);
	vPortSetInterruptHandler(portINTERRUPT_SRL_OIC, OnLED_ChangeInterrupt);

	ispis_prazan_display();
	if (set_LED_BAR(0, 0x00) != 0) {
		printf("Greska \n");
	}

	vTaskStartScheduler();

	for (;;) {}
}







