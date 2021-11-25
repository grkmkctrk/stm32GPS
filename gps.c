/*
 * gps.c
 *
 *  Created on: Nov 25, 2021
 *      Author: grkm
 */

#include "gps.h"

int nmea0183_checksum(char *msg){
	int checksum = 0;
	int j = 0;
	//     $(1).............*nn\r(4)
	for (j = 1; j < strlen(msg) - 4; j++) {
		checksum ^= (unsigned) msg[j];
	}
	return checksum;
}

void separationGPSData(
		UART_HandleTypeDef* uartSendGPS,
		uint8_t* buff
		){

		memset(buffStr, 0, 255);
		sprintf(buffStr, "%s", buff);
		char *token, *expr;
		expr = strdup(buffStr); // duplicated into string
		// actually splitting the string by "\n" delimiter
		while ((token = strsep(&expr, "\n")) != NULL) { // separate \n by \n
			memset(nmeaSnt, 0, 80);
			sprintf(nmeaSnt, "%s", token);
//			HAL_UART_Transmit(uartSendGPS, (uint8_t*) nmeaSnt, 80, 70);
			if(validCondition){
				rawSum = strstr(nmeaSnt, "*"); // valid check
				memcpy(sumNumber, rawSum+1, 2); //copy after * the check number
				uint8_t checkSum = nmea0183_checksum(nmeaSnt);
				char number[2];
				sprintf(number, "%X", checkSum);
				if(validCondition2){
//					HAL_UART_Transmit(uartSendGPS, (uint8_t*) nmeaSnt, 80, 220);
//					HAL_UART_Transmit(uartSendGPS, (uint8_t*) '\n', 1, 220);
					gpsSeparationRAW();
//					HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsRaw.latRAW, 10, 220);
//					HAL_UART_Transmit(uartSendGPS, (uint8_t*) '\n', 1, 220);
					gpsSeparationWELL(3);
//					HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.H, 2, 220);
//					HAL_UART_Transmit(uartSendGPS, (uint8_t*) '\n', 1, 220);
					sendLaLoUTC(uartSendGPS);
				}
			}
		}
}

void sendLaLoUTC(UART_HandleTypeDef* uartSendGPS){
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsRaw.NS,      1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) sep,            1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.latDeg, 2, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) sep,            1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.latMin, 7, 5); // N 40deg47.6932min |
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) sep,            1, 5);

	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsRaw.EW,      1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) sep,            1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.lonDeg, 3, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) sep,            1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.lonMin, 7, 5); // E 030deg23.4483min |
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) sep,            1, 5);

	HAL_UART_Transmit(uartSendGPS, (uint8_t*) "UTC:",         4, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.H,      2, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) ":",            1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.M,      2, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) ":",            1, 5);
	HAL_UART_Transmit(uartSendGPS, (uint8_t*) gpsWell.S,      2, 5); // E 030deg23.4483min |

	HAL_UART_Transmit(uartSendGPS, (uint8_t*) enter,         1, 5);
}

void gpsSeparationWELL(int8_t UTC_Diff){
	memcpy(gpsWell.latDeg, gpsRaw.latRAW, 2);
	memcpy(gpsWell.latMin, gpsRaw.latRAW+2, 7);

	memcpy(gpsWell.lonDeg, gpsRaw.lonRAW, 3);
	memcpy(gpsWell.lonMin, gpsRaw.lonRAW+3, 7);

	memcpy(gpsWell.H, gpsRaw.UTC,   2);
	memcpy(gpsWell.M, gpsRaw.UTC+2, 2);
	memcpy(gpsWell.S, gpsRaw.UTC+4, 2);

	IntToChar(CharToInt(UTC_Diff));
}

void IntToChar(uint8_t hour){
	*(gpsWell.H) = (char) (hour/10 + '0');
	*(gpsWell.H + 1) = (char) (hour%10 + '0');
}

uint8_t CharToInt(int8_t diff){
	uint8_t hour = 0;
	uint8_t num = *gpsWell.H - '0';
	hour += 10*num;
	num = *(gpsWell.H+1) - '0';
	hour += num+diff;
	return hour;
}


void gpsSeparationRAW(){
	uint8_t index = 0;
	char* holdData;
	for (
			holdData = strtok(nmeaSnt, ",");
			holdData != NULL ;
			holdData = strtok(NULL, ",")
		){
		switch(index){
			case 1:
				gpsRaw.latRAW = strdup(holdData); break;
			case 2:
				gpsRaw.NS     = strdup(holdData); break;
			case 3:
				gpsRaw.lonRAW = strdup(holdData); break;
			case 4:
				gpsRaw.EW     = strdup(holdData); break;
			case 5:
				gpsRaw.UTC    = strdup(holdData); break;
		}index++;
	}
}

void initGPS(
		UART_HandleTypeDef* uartGetGPS,
		UART_HandleTypeDef* uartSendGPS,
		uint8_t* buff, uint8_t len,
		uint8_t* isReady,
		char* sepp
		){
		sep = sepp;
		enter = "\n";
		HAL_UART_Receive_DMA(uartGetGPS, buff, len);
		if(*isReady == 1){
			//HAL_UART_Transmit(uartSendGPS, (uint8_t*) buff, 255, 100);
			separationGPSData(uartSendGPS, buff);
			*isReady = 0;
		}

}

