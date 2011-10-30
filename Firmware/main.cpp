/* Speedoino - This file is part of the firmware.
 * Copyright (C) 2011 Kolja Windeler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"

//** create objects **//
debugging*			pDebug=new debugging();
speedo_sd*			pSD=new speedo_sd();
configuration*		pConfig=new configuration();
speedo_disp* 		pOLED=new speedo_disp();					// vor menu_disp();
speedo_menu* 		pMenu=new speedo_menu();			// pins aktivieren, sonst nix
speedo_speedo*		pSpeedo=new speedo_speedo();
speedo_sprint* 		pSprint=new speedo_sprint();
Speedo_sensors*		pSensors=new Speedo_sensors();
speedo_filemanager* pFilemanager=new speedo_filemanager(); // ob das gut geht weiß ich auch nicht ;)
speedo_timer*   	pTimer=new speedo_timer(); // brauch ich ja nur hier, den braucht sonst keiner
//** create objects **//


/******************* Basics *************************
 * INTERRUPTS:
 * GPS will be refresh on its own, calls UART Interrupt and sets a flag on successful receive of a sentence
 * Reed Sensor will call a interrupt as well and count peaks
 * Engine Rotation will call the third interrupt and increase some var, and calucate the rpm in the interrupt ( is that clever ? )
 *
 * SETUP
 * -> Load all settings from files and initialize the sensors etc
 *
 * LOOP (Execution order of MainLoop)
 * --> show activity on Watchdog port to prevent reset
 * --> check if UART based interrupt has set the "valid GPS Data"-flag
 * ---> if data available, call parse() to save data to an array.
 * ----> call navi_calc() if navi is active
 * ----> if the distance to navigation point is small enough
 * -----> generate a new gps order by calling generate_new_order(), this will open the SD card and search the right line
 * ----> every 30 sec, store the data to SD Card
 * --> Check if timer are ready to be executed
 * ---> collect and calculate temperature measurment values
 * ---> increase clock variables
 * ---> calculate speedo km ... so this is not 10000% exact as we to a kind of integration in 1 sec steps ..
 * ---> check if the flashers are (still) active
 * --> check in any button is pushed, if so
 * ---> recalculate state var and display stuff, corresponding to the state -- lets call it "the Menu"
 * --> based on that state: show speedo, a lot of code but basicly easy: check if the values are equal to the values of the last round
 * ---> if not, display them on the screen and store to internal storage ( disp_backup array )
 ******************* Basics *************************/

/******************* TODO List ***************************
 * -> die stelle finden an der mehr als 22 byte in das char array geschrieben werden
 * bzw rausfinden wann und warum manchmal die temperaturanzeige total am rad dreht
 * -> eine stelle finden an der man den sd error zurücksetzen kann, also im menü finden
 * -> bootloader mit verify
 ******************* TODO List ***************************/
int main(void) {
	init();
	/******************** setup procedure ********************************************
	 * all initialisations must been made before the main loop
	 ******************** setup procedure ********************************************/
	Serial.begin(57600); // damit kann ich mit dem bluetooth reden, und das bluetooth kann so mit dem bootloader reden .. sind wir nich kommunikativ
	pDebug->sprintlnp(PSTR("=== Speedoino ==="));
	Wire.begin();				// BEFORE Clock_init(), Clock is in the sensor class and needs I²C
	pSensors->init(); 			// start every init sequence of each sensor
	pSD->init(); 				// try open SD Card

	pConfig->init(); 			// first load default value
	pConfig->read("BASE.TXT"); 	// load base config
	pConfig->read("SPEEDO.TXT");// speedovalues, avg,max,time
	pConfig->read("GANG.TXT");	// gang
	pConfig->read("TEMPER.TXT");// Temperatur
	pConfig->EEPROM_init(); 	// read vars from eeprom, reset Day Based storage etc
	pConfig->read_skin();		// skinning
	pConfig->check(); 			// check if Config read successfully

	pOLED->init_speedo(); 		// execute this AFTER Config->init(), init() will load  phase,config,startup. PopUp will be shown if sd access fails
	pMenu->init(); 				// adds the connection between pins and vars
	pMenu->display(); 			// execute this AFTER pOLED->init_speedo!! this will show the menu and, if state==11, draws speedosymbols

	pSpeedo->reset_bak(); 		// reset all storages, to force the redraw of the speedo

	pConfig->ram_info();
	pDebug->sprintlnp(PSTR("=== Setup finished ==="));
	Serial.flush(); // jaja, hallo liebes bluetooth modul, will keiner wissen das du alles echos solange wir nicht mit dem pc verbunden sind ...
	/******************** setup procedure ********************************************
	 * all initialisations must been made before the main loop, before THIS
	 ******************** setup procedure ********************************************/
	unsigned long   previousMillis = 0;
	/* main loop, this will be repeated on and on */
	/////////////////////////////////////////////////////////
	Serial.println("----------- Los gehts ------------");
	while(1){
		pSensors->m_reset->toggle(); 		// toggle pin, if we don't toggle it, the ATmega8 will reset us, kind of watchdog
		for(int s=100; s<=2000; s+=100){
			Serial3.print("$s");
			Serial3.print(s);
			Serial3.print("*");

			for(int a=10; a<=150; a+=10){
				Serial3.print("$a");
				Serial3.print(a);
				Serial3.print("*");

				delay(100);
				Serial3.print("$m500*");
				Serial.print("a=");
				Serial.print(a);
				Serial.print(" s=");
				Serial.print(s);
				Serial.println(" | Weiter ? (j/n)");
				while(!Serial.available()){
					if(Serial.read()=='n'){
						if(a!=10){
							a-=10;
						} else {
							a=140;
							s-=100;
						}; // if(a)
					} // if read()
				} // while
			} // for a
		} // for s
	} // while(1)
	Serial.println("----------- Das wars ------------");
	/////////////////////////////////////////////////////////
	for (;;) {
		pSensors->m_reset->toggle(); 		// toggle pin, if we don't toggle it, the ATmega8 will reset us, kind of watchdog
		pDebug->speedo_loop(21,1,0," "); 	// intensive debug= EVERY loop access reports the Menustate
		pSensors->m_gps->check_flag();    	// check if a GPS sentence is ready

		/************************* timer *********************/
		pTimer->every_sec(pConfig);		// 1000 ms
		pTimer->every_qsec();			// 250  ms
		pTimer->every_custom();  		// one custom timer, redrawing the speedo, time is defined by "refresh_cycle" in the base.txt
		/************************* push buttons *********************
		 * using true as argument, this will activate bluetooth input as well
		 ************************* push buttons*********************/
		if(pMenu->button_test(true)){     // important!! if we have a pushed button we will draw something, depending on the menustate
			pDebug->loop();
			pMenu->display();
		};
		/************************ every deamon activity is clear, now draw speedo ********************
		 * we are round about 0000[1]1 - 0000[1]9
		 ************************ every deamon activity is clear, now draw speedo ********************/
		if((pMenu->state/10)==1 || pMenu->state==8511111)  {
			pSpeedo->loop(previousMillis);
		}
		//////////////////// Sprint Speedo ///////////////////
		else if( pMenu->state==21 ) {
			pSprint->loop(previousMillis);
		}
		//////////////////// race mode ///////////////////
		else if(pMenu->state==311){

		}
		//////////////////// gps scan ///////////////////
		else if(pMenu->state==51){
			pSensors->m_gps->loop();
		}
		//////////////////// outline scan ///////////////////
		else if(pMenu->state==841){
			pSensors->m_speed->check_umfang();
		}
		////////////////// gear scan ///////////////
		else if(floor(pMenu->state/100)==82){
			pSensors->m_gear->calibrate();
		}
	} // end for
} // end main

