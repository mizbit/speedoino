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
#define 	DEBUG_TRANSFER 0
#define	 	DEBUG_TRANSFER_INTENSIV 0

speedo_filemanager_v2::speedo_filemanager_v2(){};

speedo_filemanager_v2::~speedo_filemanager_v2(){};


// hier kommt man rein, wenn in der hauptschleife festgestellt wurde
// das sich im seriellen Puffer daten befinden ... das koennte
// a) ein Command zum "links" sein
// b) quatsch
// nach "links" geht er mit isLeave=1 raus

void speedo_filemanager_v2::parse_command(){
	// Direct start in, cause start was already fetched //msgParseState	=	ST_START;
	unsigned char	msgParseState	=	ST_GET_SEQ_NUM;
	unsigned int	ii				=	0;
	unsigned char	checksum		=	MESSAGE_START;
	unsigned char	seqNum			=	1;
	unsigned int	msgLength		=	0;
	unsigned int	msgStartCounter	=	0;
	unsigned int	timeout			=	0;
	unsigned char	msgBuffer[285];
	unsigned char	c, *p;
	unsigned char   isLeave = 0;
	unsigned char 	last_file[22];
	unsigned long last_file_seek=-1; // max
	pSensors->m_reset->set_deactive(false,true);

	//*	main loop
	while (!isLeave){
		/*
		 * Collect received bytes to a complete message
		 */


		while ( (msgParseState != ST_PROCESS)  && (isLeave!=1) ){

			// solange keine Daten an der Schnittstelle anliegen
			// und wir auch noch keine WAIT_MS_FOR_DATA gewartet haben
			while(Serial.available()==0 && timeout<WAIT_MS_FOR_DATA){
				timeout++;
				if(timeout>=WAIT_MS_FOR_DATA){
					isLeave=1;					// exit the while loop on tops
					break;						// exit this while loop
				} else {
					_delay_ms(1);
				}
			};

			if(isLeave!=1){
				//////////////////
				if(DEBUG_TRANSFER){
					pOLED->string(0,"-",0,1);
					_delay_ms(500);
				}
				//////////////////
				c = Serial.read();
				timeout = 0;

				switch (msgParseState){
				case ST_START:
					//					pOLED->animation(3);
					//					pOLED->string(VISITOR_SMALL_1X_FONT,"LOADING...",5,0);

					if ( c == MESSAGE_START ){
						//////////////////
						if(DEBUG_TRANSFER){
							pOLED->string(0,"1",0,1);
							_delay_ms(500);
						}
						if(DEBUG_TRANSFER_INTENSIV){
							Serial.println("MSG gehalten");
						}
						//////////////////
						msgParseState	=	ST_GET_SEQ_NUM;
						checksum		=	MESSAGE_START;
						msgStartCounter	=	0;
					} else {
						msgStartCounter++;
						// wenn binnen 300 Bytes kein MESSAGE_START kommt Abbruch
						if(msgStartCounter>300){
							isLeave=1;
							break;
						}
					}
					break;

				case ST_GET_SEQ_NUM:
					if ((c == 1) || (c == seqNum)){
						//////////////////
						if(DEBUG_TRANSFER){
							pOLED->string(0,"2",0,1);
							_delay_ms(500);
						}
						if(DEBUG_TRANSFER_INTENSIV){
							Serial.print("seq nr: ");
							Serial.print(c,DEC);
							Serial.println(" erhalten");
						}
						//////////////////
						seqNum			=	c;
						msgParseState	=	ST_MSG_SIZE;
						checksum		^=	c;
					} else {
						msgParseState	=	ST_START;
					}
					break;

				case ST_MSG_SIZE:
					//////////////////
					if(DEBUG_TRANSFER){
						pOLED->string(0,"3",0,1);
						_delay_ms(500);
					}
					if(DEBUG_TRANSFER_INTENSIV){
						Serial.print("MSG size:");
						Serial.println(c,DEC);
					}
					//////////////////
					msgLength		=	c<<8;
					msgParseState	=	ST_MSG_SIZE_2;
					checksum		^=	c;
					break;

				case ST_MSG_SIZE_2:
					//////////////////
					if(DEBUG_TRANSFER){
						pOLED->string(0,"4",0,1);
						_delay_ms(500);
					}
					if(DEBUG_TRANSFER_INTENSIV){
						Serial.print("MSG size:");
						Serial.println(c,DEC);
					}
					//////////////////
					msgLength		|=	c;
					msgParseState	=	ST_GET_TOKEN;
					checksum		^=	c;
					break;

				case ST_GET_TOKEN:
					if ( c == TOKEN ){
						//////////////////
						if(DEBUG_TRANSFER){
							pOLED->string(0,"5",0,1);
							_delay_ms(500);
						}
						if(DEBUG_TRANSFER_INTENSIV){
							Serial.println("Token erhalten");
						}
						//////////////////
						msgParseState	=	ST_GET_DATA;
						checksum		^=	c;
						ii				=	0;
					} else {
						msgParseState	=	ST_START;
					}
					break;

				case ST_GET_DATA:
					//////////////////
					if(DEBUG_TRANSFER){
						pOLED->string(0,"6",0,1);
						_delay_ms(500);
					}
					if(DEBUG_TRANSFER_INTENSIV){
						Serial.println("Daten erhalten");
					}
					//////////////////
					msgBuffer[ii++]	=	c;
					checksum		^=	c;
					if (ii == msgLength ){
						msgParseState	=	ST_GET_CHECK;
					}
					break;

				case ST_GET_CHECK:
					//					char buffer[15];
					//					sprintf(buffer,"ist %i",c);
					//					pOLED->string(0,buffer,0,3);
					//					sprintf(buffer,"soll %i",checksum);
					//					pOLED->string(0,buffer,0,4);
					if ( c == checksum){
						//////////////////
						if(DEBUG_TRANSFER){
							pOLED->string(0,"7",0,1);
							_delay_ms(500);
						}
						if(DEBUG_TRANSFER_INTENSIV){
							Serial.println("Checksum correct");
						}
						//////////////////
						msgParseState	=	ST_PROCESS;
					} else {
						msgParseState	=	ST_START;
					}
					break;
				}	//	switch
			}; // if is leave != 1
		}	//	while(msgParseState!=ST_PROCESS && (isLeave!=1))
		// an dieser stelle endet die state machine des empfangens
		// hier kann sie entweder rauskommen wenn sie
		// timeout gegangen ist, dann ist ist isLeave=1
		// oder wenn sie was gueltiges empfangen hat, dann ist msgParseState ST_PROCESS


		if(isLeave!=1){
			/*
			 * Now process the STK500 commands, see Atmel Appnote AVR068
			 */
			bool change_disp=false; 

			if(DEBUG_TRANSFER){
				char buffer[10];
				if(floor(msgBuffer[0]/16)>10){
					buffer[0]='a'+(floor(msgBuffer[0]/16)-10);
				} else {
					buffer[0]='0'+floor(msgBuffer[0]/16);
				};

				if((msgBuffer[0]%16)>10){
					buffer[1]='a'+(msgBuffer[0]%16)-10;
				} else {
					buffer[1]='0'+(msgBuffer[0]%16);
				}
				buffer[2]='\0';
				pOLED->string(0,buffer,0,1);
			};

			/////////////////////////// SIGN ON ///////////////////////////////////////

			if(msgBuffer[0]==CMD_SIGN_ON){
				msgBuffer[0]	= 	CMD_SIGN_ON;
				msgBuffer[1] 	=	STATUS_CMD_OK;
				// hier irgendwie GIT_REV reinbringen, nur wie ?! darauf kann man nicht mit GIT_REV[0] zugreifen
				char buffer[21];
				sprintf(buffer,GIT_REV);
				int i=0;
				while(buffer[i]!='\0'){
					msgBuffer[2+i]=buffer[i];
					i++;
				};
				msgLength=i+2;

			} else if(msgBuffer[0]==CMD_LEAVE_FM){
				isLeave	=	1;

				/////////////////////////// SIGN ON ///////////////////////////////////////
				////////////////////////// UP DOWN LEFT RIGHT /////////////////////////////

			} else if(msgBuffer[0]==CMD_GO_LEFT){
				pMenu->go_left(true); // i wait on main loop, i won't update it myself
				isLeave	=	1;
				msgLength		=	2;
				msgBuffer[0]	= 	CMD_GO_LEFT;
				msgBuffer[1] 	=	STATUS_CMD_OK;

			} else if(msgBuffer[0]==CMD_GO_RIGHT){
				pMenu->go_right(true); // i wait on main loop, i won't update it myself
				isLeave	=	1;
				msgLength		=	2;
				msgBuffer[0]	= 	CMD_GO_RIGHT;
				msgBuffer[1] 	=	STATUS_CMD_OK;

			} else if(msgBuffer[0]==CMD_GO_UP){
				pMenu->go_up(true); // i wait on main loop, i won't update it myself
				isLeave	=	1;
				msgLength		=	2;
				msgBuffer[0]	= 	CMD_GO_UP;
				msgBuffer[1] 	=	STATUS_CMD_OK;

			} else if(msgBuffer[0]==CMD_GO_DOWN){
				pMenu->go_down(true); // i wait on main loop, i won't update it myself
				isLeave	=	1;
				msgLength		=	2;
				msgBuffer[0]	= 	CMD_GO_DOWN;
				msgBuffer[1] 	=	STATUS_CMD_OK;

				////////////////////////// UP DOWN LEFT RIGHT /////////////////////////////
				///////////////////////////// COMMAND DIR ////////////////////////////////

			} else if(msgBuffer[0]==CMD_DIR) {
				/* msgBuffer[0]==CMD_DIR
				 * msgBuffer[1]==NR_HIGH
				 * msgBuffer[2]==NR_LOW
				 * msgBuffer[3]==G
				 * msgBuffer[4]==P
				 * msgBuffer[5]==S
				 *
				 * messageLength=6
				 *
				 * rueckweg:
				 * msgBuffer[0]=CMD_DIR
				 * msgBuffer[1]=COMMAND_OK
				 * msgBuffer[2]=STATUS (File 1, DIR 2, Ende 0)
				 * msgBuffer[3]=filesize high uint_32
				 * msgBuffer[4]=filesize 2nd uint_32
				 * msgBuffer[5]=filesize 3rd uint_32
				 * msgBuffer[6]=filesize 4th uint_32
				 * msgBuffer[7..x]=DATA
				 */

				// buffer for dir name
				char dir[8];
				// copy reqested dir
				for(unsigned int i=3;i<msgLength;i++){
					dir[i-3]=msgBuffer[i];
				}
				// add end of string
				dir[msgLength-3]='\0';
				// get item id
				int item=(msgBuffer[1]<<8)|msgBuffer[2];

				// open root and maybe go on
				SdFile fm_handle,returner;
				fm_handle.openRoot(&pSD->volume);
				if(!(dir[0]=='/')){	// wenn es NICHT ".." ist
					returner.open(&fm_handle, dir, O_READ);
					fm_handle=returner;
				}
				// get filename and type of item
				char name[13];
				unsigned long size=0;
				int status=fm_handle.lsJKWNext(name,item,&size);
				returner.close();
				fm_handle.close();

				// write the returner
				msgBuffer[0]=CMD_DIR;
				msgBuffer[1]=STATUS_CMD_OK;

				if(status>0) {
					msgBuffer[2]=status;
					msgBuffer[3]=(size&0xFF000000)>>24; // filesize high nibble
					msgBuffer[4]=(size&0x00FF0000)>>16; // filesize 2nd nibble
					msgBuffer[5]=(size&0x0000FF00)>>8; // filesize 3rd nibble
					msgBuffer[6]=(size&0x000000FF); // filesize 4th nibble
					int i=0;
					while(name[i]!='\0'){
						msgBuffer[i+7]=name[i];
						i++;
					};
					msgLength		=	i+2+1+4; // i==anzahl an zeichen fuer name + 1 fuer type {1/2} + 2 fuer cmd/ok + 4 fuer filesize
				} else {
					msgLength		= 	3;
					msgBuffer[2]	= 	STATUS_EOF;
				};
				///////////////////////////// COMMAND DIR ////////////////////////////////
				////////////////////////////// GET FILE /////////////////////////////////

				/* hinweg:
				 * msgBuffer[0]=CMD_GET_FILE
				 * msgBuffer[1]=length of filename
				 * msgBuffer[2..]=filename  ... datei.txt oder folder/datei.txt
				 * msgBuffer[x]=high_nibble of cluster nr
				 * msgBuffer[x+1]=low_nibble of cluster nr

				 *
				 * rueckweg:
				 * msgBuffer[0]=CMD_GET_FILE
				 * msgBuffer[1]=COMMAND_OK
				 * msgBuffer[2..]=DATA
				 */

			} else if(msgBuffer[0]==CMD_GET_FILE){

				// 1. checken: haben wir im t2a_file noch was offen
				// 2. checken: ist der dateiname noch der gleiche wie der, den wir bekommen haben?
				// 3. wenn nicht: t2a_dir soll erst root oeffnen und dann eventuell unterverzeichnisse
				// 4. dann setseek()
				// 5. while() bla bla copy inhalt zur msg

				bool file_already_open=true;
				bool file_open_failed=false;
				bool file_seek_failed=false;
				int payload_start=0;

				if(fm_file.isFile()){
					for(unsigned int i=0;i<msgLength-2;i++){
						if(last_file[i]!=msgBuffer[i+3]){
							file_already_open=false;
						}
					}
				} else {
					file_already_open=false;
				};

				// wenn die datei noch nicht geoeffnet ist,
				// muessen wir
				// 1. checken ob sie in einem unterverzeichniss liegt
				// 1.1. wenn ja verzeichniss namen auslesen
				// 1.2. subverzeichniss oeffnen
				// 2. Dateiname auslesen
				// 3. Dateihandle oeffnen

				if(!file_already_open){
					payload_start=get_file_handle(&msgBuffer[0],&last_file[0],&fm_file,&fm_handle,O_CREAT| O_READ);
					if(payload_start<0)
						file_open_failed=true;
				} // filealready open

				// setze pointer
				if(!file_open_failed){
					unsigned long pos;
					pos=msgBuffer[payload_start]<<8;
					pos|=msgBuffer[payload_start+1];
					pos*=250;

					if(last_file_seek!=pos){
						if(!fm_file.seekSet(pos)){
							file_seek_failed=true;
							last_file_seek=pos;
						}
					};
				};

				// wenn immer noch alles gut, dann konnten wir die Datei oeffnen und auch den Filepointer dahin setzten wo er hin soll
				if(!file_open_failed && !file_seek_failed){
					//Serial.println("file_seek OK");
					int n=fm_file.read(msgBuffer, sizeof(byte)*250); // 250

					if(n > 0) { // 250 Byte happen
						// jetzt noch verschieben an die richtige stelle im buffer
						for(int i=n-1; i>=0; i--){
							msgBuffer[i+2]=msgBuffer[i];
						};
						msgLength=n+2; // n buchstaben + cmd + status ok = 252 im besten Fall
						msgBuffer[0]=CMD_GET_FILE;
						msgBuffer[1]=STATUS_CMD_OK;
						//Serial.println("gelesen: kommt noch");

					} else {
						msgLength=2; // n buchstaben + cmd + status eof
						msgBuffer[0]=CMD_GET_FILE;
						msgBuffer[1]=STATUS_EOF;
					}
				} else if(file_seek_failed){
					msgLength=2; // n buchstaben + cmd + status failed
					msgBuffer[0]=CMD_GET_FILE;
					msgBuffer[1]=STATUS_CMD_FAILED;
					last_file[0]='\0'; // damit er nicht denkt das haette geklappt
				} else {
					msgLength=2; // n buchstaben + cmd + status failed
					msgBuffer[0]=CMD_GET_FILE;
					msgBuffer[1]=STATUS_CMD_FAILED;
					last_file[0]='\0'; // damit er nicht denkt das haette geklappt
				}

				////////////////////////////// GET FILE /////////////////////////////////
				////////////////////////////// PUT FILE /////////////////////////////////

				// TRANSFER VOM HANDY ZUM TACHO
				/* hinweg:
				 * msgBuffer[0]=CMD_PUT_FILE
				 * msgBuffer[1]=length of filename
				 * msgBuffer[2..X]=filename  ... datei.txt oder folder/datei.txt
				 * msgBuffer[x+1]=high_nibble of cluster nr
				 * msgBuffer[x+2]=low_nibble of cluster nr
				 * msgBuffer[X+3..250]=Content
				 *
				 * rueckweg:
				 * msgBuffer[0]=CMD_PUT_FILE
				 * msgBuffer[1]=COMMAND_OK
				 */
			} else if(msgBuffer[0]==CMD_PUT_FILE && !((msgLength==2) && (msgBuffer[1]==STATUS_EOF))){
				bool file_already_open=true;
				bool file_open_failed=false;
				bool file_seek_failed=false;
				int start_of_payload=0;


				if(fm_file.isFile()){
					for(unsigned int i=0;i<msgLength-2;i++){
						if(last_file[i]!=msgBuffer[i+3]){
							file_already_open=false;
						}
					}
				} else {
					file_already_open=false;
				};


				// wenn die datei noch nicht geoeffnet ist,
				// muessen wir
				// 1. checken ob sie in einem unterverzeichniss liegt
				// 1.1. wenn ja verzeichniss namen auslesen
				// 1.2. subverzeichniss oeffnen
				// 2. Dateiname auslesen
				// 3. Dateihandle oeffnen

				if(!file_already_open){
					start_of_payload=get_file_handle(&msgBuffer[0],&last_file[0],&fm_file,&fm_handle, O_CREAT| O_RDWR | O_SYNC | O_APPEND);
					if(start_of_payload<0)
						file_open_failed=true;
				}

				// setze pointer
				if(!file_open_failed){
					int pos;
					pos=msgBuffer[start_of_payload]<<8;
					pos|=msgBuffer[start_of_payload+1];
					//					if(!fm_file.seekSet(pos*(msgLength-offset))){ // das ist noch totaler mist, das hier kein Seeken moeglich ist
					//						file_seek_failed=true;
					//					} else {
					//						//////////
					//						sprintf(buf,"fs:%i,pos:%i ",(int)fm_file.fileSize(),pos*(msgLength-offset));
					//						pOLED->string(0,buf,0,6);
					//						_delay_ms(1000);
					//					}
					//					////////
				};

				// wenn immer noch alles gut, dann konnten wir die Datei oeffnen und auch den Filepointer dahin setzten wo er hin soll
				if(!file_open_failed && !file_seek_failed){
					//Serial.println("file_seek OK");
					for(unsigned int i=0;i<msgLength-start_of_payload-2;i++){ // start_of_payload wird -2 weil noch 2 byte seek info
						msgBuffer[i]=msgBuffer[i+start_of_payload+2]; // msgBuffer[233]=msgBuffer[253]
					}

					int n=fm_file.write(msgBuffer, msgLength-start_of_payload-2); // 254 - 20

					if(n > 0) { // 250 Byte happen
						msgLength=2; // cmd + status ok
						msgBuffer[0]=CMD_PUT_FILE;
						msgBuffer[1]=STATUS_CMD_OK;
					} else {
						msgLength=2; // n buchstaben + cmd + status eof
						msgBuffer[0]=CMD_PUT_FILE;
						msgBuffer[1]=STATUS_EOF;
						last_file[0]='\0'; // damit er nicht denkt das haette geklappt
					}
				} else if(file_seek_failed){
					msgLength=3; // n buchstaben + cmd + status failed
					msgBuffer[0]=CMD_PUT_FILE;
					msgBuffer[1]=STATUS_CMD_FAILED;
					msgBuffer[2]='1';
					last_file[0]='\0'; // damit er nicht denkt das haette geklappt
				} else {
					msgLength=3; // n buchstaben + cmd + status failed
					msgBuffer[0]=CMD_PUT_FILE;
					msgBuffer[1]=STATUS_CMD_FAILED;
					msgBuffer[2]='2';
					last_file[0]='\0'; // damit er nicht denkt das haette geklappt
				}
			} else if(msgBuffer[0]==CMD_PUT_FILE && ((msgLength==2) && (msgBuffer[1]==STATUS_EOF))){
				fm_file.sync();
				fm_file.close();
				fm_handle.sync();
				fm_handle.close();
				msgLength=2; // n buchstaben + cmd + status eof
				msgBuffer[0]=CMD_PUT_FILE;
				msgBuffer[1]=STATUS_EOF;
				last_file[0]='\0'; // damit er nicht denkt das haette geklappt
				////////////////////////////// PUT FILE /////////////////////////////////
				////////////////////////////// DEL FILE /////////////////////////////////
			} else if(msgBuffer[0]==CMD_DEL_FILE){
				get_file_handle(&msgBuffer[0],&last_file[0],&fm_file,&fm_handle, O_CREAT| O_WRITE);
				if(fm_file.remove()){
					fm_file.close();
					fm_handle.close();
					msgLength=2; // cmd + status ok
					msgBuffer[0]=CMD_DEL_FILE;
					msgBuffer[1]=STATUS_CMD_OK;
				} else {
					msgLength=2; // cmd + status ok
					msgBuffer[0]=CMD_DEL_FILE;
					msgBuffer[1]=STATUS_CMD_FAILED;
				}
				////////////////////////////// DEL FILE /////////////////////////////////
				////////////////////////////// SHOW GFX /////////////////////////////////
				// TRANSFER VOM HANDY ZUM TACHO
				/* hinweg:
				 * msgBuffer[0]=CMD_SHOW_GFX
				 * msgBuffer[1]=length of filename
				 * msgBuffer[2..X]=filename  ... datei.txt oder folder/datei.txt
				 *
				 * rueckweg:
				 * msgBuffer[0]=CMD_SHOW_GFX
				 * msgBuffer[1]=COMMAND_OK
				 */
			} else if(msgBuffer[0]==CMD_SHOW_GFX){
				unsigned int length_of_filename=(unsigned int)msgBuffer[1];
				char filename[13];
				for(unsigned int i=0;i<length_of_filename;i++){
					filename[i]=msgBuffer[i+2];
				}
				filename[length_of_filename]='\0';
				int return_value=pOLED->sd2ssd(filename,0);
				if(return_value==0){
					pMenu->state=9999999;
					msgLength=2; // cmd + status ok
					msgBuffer[0]=CMD_SHOW_GFX;
					msgBuffer[1]=STATUS_CMD_OK;
				} else {
					msgLength=3; // cmd + status ok
					msgBuffer[0]=CMD_SHOW_GFX;
					msgBuffer[1]=STATUS_CMD_FAILED;
					msgBuffer[2]=return_value & 0xFF;
				}
				////////////////////////////// SHOW GFX /////////////////////////////////
				///////////////////////////// EMERGENCY /////////////////////////////////

			} else {
				msgLength		=	2;
				//msgBuffer[0]	=	msgBuffer[0]; // keep old command
				msgBuffer[1]	=	STATUS_CMD_UNKNOWN;
			}
			///////////////////////////// EMERGENCY /////////////////////////////////


			//////////////////////////// SEND BACK //////////////////////////////////
			Serial.print((char)MESSAGE_START);
			checksum	=	MESSAGE_START^0;

			Serial.print((char)seqNum);
			checksum	^=	seqNum;

			//c			=	msgLength&0x00FF;
			Serial.print((char)(msgLength>>8));
			checksum ^= (msgLength>>8);

			Serial.print((char)(msgLength&0xff));
			checksum ^= (msgLength&0xff);

			Serial.print((char)TOKEN);
			checksum ^= TOKEN;

			p	=	msgBuffer;
			while(msgLength){
				c	=	*p++;
				Serial.print((char)c);
				checksum ^=c;
				msgLength--;
			}
			Serial.print((char)checksum);

			seqNum++;
			msgParseState = ST_START;
			//////////////////////////// SEND BACK //////////////////////////////////

		}; // if isleave!=1
	}; // while isLeave!=1
	//	pMenu->display();
	// t2a_file.close();
	fm_file.close();
	fm_handle.close();
	pSensors->m_reset->set_active(false,true);
}; // fkt ende



int speedo_filemanager_v2::get_file_handle(unsigned char *msgBuffer,unsigned char *last_file,SdFile *fm_file,SdFile *fm_handle,uint8_t flags){
	/* msgBuffer[0] =CMD
	 * msgBuffer[1] =FN_length
	 * msgBuffer[2..2+FN_length] =Filename
	 * msgBuffer[3+FN_length..]=Data
	 */

	unsigned int start_of_filename=2;
	unsigned int start_of_real_filename=0;
	unsigned int length_of_filename=(unsigned int)msgBuffer[1];
	char filename[13];
	char subdir[13];
	fm_handle->close();
	fm_file->close();
	fm_handle->openRoot(&pSD->volume);

	// checken ob ein "/" drin ist, und die datei somit in einem verzeichniss liegt
	bool is_subdir_file=false;
	for(unsigned int i=start_of_filename;i<start_of_filename+length_of_filename;i++){
		if(msgBuffer[i]=='/'){
			is_subdir_file=true;
			subdir[i-start_of_filename]='\0';
			start_of_real_filename=i+1;
			break;
		} else {
			subdir[i-start_of_filename]=msgBuffer[i];
		}
	};


	// verzeichnissnamen auslesen und oeffnen
	if(is_subdir_file){
		SdFile temp;
		if(!temp.open(fm_handle, subdir, O_READ)){
			return -2;
		}
		*fm_handle=temp; // das hier ist quatsch
	};

	// dateinamen auslesen
	for(unsigned int i=start_of_real_filename;i<start_of_filename+length_of_filename;i++){
		filename[i-start_of_real_filename]=msgBuffer[i];
		last_file[i-start_of_real_filename]=msgBuffer[i];
		if(i==(start_of_filename+length_of_filename-1)){
			filename[i-start_of_real_filename+1]='\0';
			last_file[i-start_of_real_filename+1]='\0';
		};
	};

	SdFile temp_f;
	// datei oeffnen
	if (!temp_f.open(fm_handle, filename,flags)){
		return -1;
	} else {
		*fm_file=temp_f;
	}

	return start_of_filename+length_of_filename;
}