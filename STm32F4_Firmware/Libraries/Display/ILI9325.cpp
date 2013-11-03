/**
 ********************************************************************************
 *
 * Pin configuration
 *
 *        D7 : LCD Pin D15 -> PD10 [FSMC D15] (1.42)
 *        D6 : LCD Pin D14 -> PD9  [FSMC D14] (1.41)
 *        D5 : LCD Pin D13 -> PD8  [FSMC D13] (1.40)
 *        D4 : LCD Pin D12 -> PE15 [FSMC D12] (1.33)
 *        D3 : LCD Pin D11 -> PE14 [FSMC D11] (1.32)
 *        D2 : LCD Pin D10 -> PE13 [FSMC D10] (1.31)
 *        D1 : LCD Pin D9  -> PE12 [FSMC D09] (1.30)
 *        D0 : LCD Pin D8  -> PE11 [FSMC D08] (1.29)
 *
 *        RS :     		   -> PD11 [FSMS A16] (1.43)
 *       ~RW :             -> PD5  [FSMS NWE] (2.29) (USB OTG Over Current LED)
 *       ~RD :             -> PD4  [FSMS NOE] (2.32)
 *       ~CS :             -> PD7  [FSMS NE1] (2.27)
 *       ~RST:   		   -> PE10 (1.28)
 *       LED Backlight :   -> PE9  (1.27) (Timer 1 channel 1)
 */
#include "global.h"



#define LCD_REG      (*((volatile short *) 0x60000000))
#define LCD_RAM      (*((volatile short *) 0x60020000))

/* Global variables to set the text color */
volatile u8 textRed = 0xFF;
volatile u8 textGreen = 0xFF;
volatile u8 textBlue = 0xFF;

volatile u8 backRed = 0x00;
volatile u8 backGreen = 0x00;
volatile u8 backBlue = 0x00;

volatile uint16_t asciisize = 16;

ILI9325::ILI9325(){};
ILI9325::~ILI9325(){};

void ILI9325::init(void) {
	CtrlLinesConfig();
	FSMCConfig();

	//Pulse reset
	GPIO_ResetBits(GPIOE, GPIO_Pin_10);
	_delay_ms(300);
	GPIO_SetBits(GPIOE, GPIO_Pin_10);
	_delay_ms(100);

	//u16 readValue;
	//readValue = ReadReg(R0);
	ReadReg(R0);

	/* Start Initial Sequence ----------------------------------------------------*/
	//WriteReg(R227, 0x3008); // Internal clock
	//WriteReg(R231, 0x0012); // Internal clock
	//WriteReg(R239, 0x1231); // Internal clock
	WriteReg(R0, 0x0001); /* Start oscillator */
	WriteReg(R1, 0x0000); /* set SS and SM bit */
	WriteReg(R2, 0x0200); /* set 1 line inversion */
	WriteReg(R3, 0xC028 | BRG_Mode); /* 8bits/262K BGR=x. */
	WriteReg(R4, 0x0000); /* Resize */
	WriteReg(R8, 0x0207); /* set the back porch and front porch */
	WriteReg(R9, 0x0000); /* set non-display area refresh cycle ISC[3:0] */
	WriteReg(R10, 0x0000); /* FMARK function */
	WriteReg(R12, 0x0002); /* RGB interface setting */
	WriteReg(R13, 0x0000); /* Frame marker Position */
	WriteReg(R15, 0x0000); /* RGB interface polarity */

	/* Power On sequence ---------------------------------------------------------*/
	WriteReg(R16, 0x0000); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
	WriteReg(R17, 0x0000); /* DC1[2:0], DC0[2:0], VC[2:0] */
	WriteReg(R18, 0x0000); /* VREG1OUT voltage */
	WriteReg(R19, 0x0000); /* VDV[4:0] for VCOM amplitude */
	_delay_ms(200); /* Dis-charge capacitor power voltage (200ms) */
	WriteReg(R16, R16_VAL); /* SAP, BT[3:0], AP, DSTB, SLP, STB */
	WriteReg(R17, R17_VAL); /* DC1[2:0], DC0[2:0], VC[2:0] */
	_delay_ms(50); /* _delay_ms 50 ms */

	WriteReg(R18, R18_VAL); /* VREG1OUT voltage */
	_delay_ms(50); /* _delay_ms 50 ms */
	WriteReg(R19, R19_VAL); /* VDV[4:0] for VCOM amplitude */
	WriteReg(R41, R41_VAL); /* VCM[4:0] for VCOMH */
	WriteReg(R43, 0x000B); // Set Frame Rate
	_delay_ms(50); /* _delay_ms 50 ms */

	/* Adjust the Gamma Curve ----------------------------------------------------*/
	WriteReg(R48, 0x0001); // Fine Gamma KP1 - KP0 	[2:0] [2:0]
	WriteReg(R49, 0x0101); // Fine Gamma KP2 - KP3 	[2:0] [2:0]
	WriteReg(R50, 0x0101); // Fine Gamma KP4 - KP5 	[2:0] [2:0]
	WriteReg(R53, 0x0202); // Gradient RP1 - RP0 		[2:0] [2:0]
	WriteReg(R54, 0x0A00); // Amplitude VRP1 - VRP0	[4:0] [4:0]
	WriteReg(R55, 0x0507); // Fine Gamma KN1 - KN0 	[2:0] [2:0]
	WriteReg(R56, 0x0101); // Fine Gamma KN3 - KN2 	[2:0] [2:0]
	WriteReg(R57, 0x0101); // Fine Gamma KN5 - KN4 	[2:0] [2:0]
	WriteReg(R60, 0x0202); // Gradient RN1 - RN0 		[2:0] [2:0]
	WriteReg(R61, 0x0000); // Amplitude VRN1 - VRN0	[4:0] [4:0]

	/* Set GRAM area -------------------------------------------------------------*/
	WriteReg(R80, 0x0000); /* Horizontal GRAM Start Address */
	WriteReg(R81, 0x00EF); /* Horizontal GRAM End Address */
	WriteReg(R82, 0x0000); /* Vertical GRAM Start Address */
	WriteReg(R83, 0x013F); /* Vertical GRAM End Address */
	WriteReg(R32, 0x0000); /* GRAM horizontal Address */
	WriteReg(R33, 0x0000); /* GRAM Vertical Address */

	WriteReg(R96, 0x2700); /* Gate Scan Line */
	WriteReg(R97, 0x0001); /* NDL,VLE, REV */
	WriteReg(R106, 0x0000); /* set scrolling line */

	/* Partial Display Control ---------------------------------------------------*/
	WriteReg(R128, 0x0000);
	WriteReg(R129, 0x0000);
	WriteReg(R130, 0x0000);
	WriteReg(R131, 0x0000);
	WriteReg(R132, 0x0000);
	WriteReg(R133, 0x0000);

	/* Panel Control -------------------------------------------------------------*/
	WriteReg(R144, 0x0010);
	WriteReg(R146, 0x0600);

	_delay_ms(50);

	/* Set GRAM write direction and BGR = 1 */
	/* I/D=01 (Horizontal : increment, Vertical : decrement) */
	/* AM=1 (address is updated in vertical writing direction) */
	/* TRI=1, DFM=1 8 bit mode and 262K */
	WriteReg(R7, 0x0133); /* 262K color and display ON */
	/* Clear the LCD */
	_delay_ms(50);

	clear_screen(0, 0, 0);
	_delay_ms(50);
}

/*******************************************************************************
 * Function Name  : Clear
 * Description    : Clears the hole LCD.
 * Input          : Color: the color of the background.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::clear_screen(){
	clear_screen(0,0,0);
}

void ILI9325::clear_screen(u8 r, u8 g, u8 b) {
	u32 index = 0;

	SetRotatedCursor(0x00, 0x00);

	WriteRAM_Prepare(); /* Prepare to write GRAM */

	for (index = 0; index < 76800; index++) {
		LCD_RAM = r << 8;
		LCD_RAM = g << 8;
		LCD_RAM = b << 8;
	}
}

/*******************************************************************************
 * Function Name  : DisplayOn
 * Description    : Enables the Display.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::DisplayOn(void) {
	/* Display On */
	WriteReg(R7, 0x0133); /* 262K color and display ON */
}

/*******************************************************************************
 * Function Name  : DisplayOff
 * Description    : Disables the Display.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::DisplayOff(void) {
	/* Display Off */
	WriteReg(R7, 0x0);
}

/*******************************************************************************
 * Function Name  : Disp_Image
 * Description    : Disables the Display.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::Disp_Image(unsigned short *gImage_ptr) {
	u32 i = 0;
	u16 raw;
	u16 red;
	u16 green;
	u16 blue;

	SetRotatedCursor(0x00, 0x00);

	WriteRAM_Prepare(); /* Prepare to write GRAM */

	for (i = 0; i < 76800; i++) {
		raw = gImage_ptr[i];
		blue = raw & 0XF800;
		green = (raw << 5) & 0xFC00;
		red = (raw & 0x1F) << 11;
		LCD_RAM = red;
		LCD_RAM = green;
		LCD_RAM = blue;
	}
}

void ILI9325::SetColors(u8 text_r, u8 text_g, u8 text_b, u8 back_r, u8 back_g, u8 back_b) {
	textRed = text_r;
	textGreen = text_g;
	textBlue = text_b;
	backRed = back_r;
	backGreen = back_g;
	backBlue = back_b;
}

void ILI9325::GetColors(u8 *text_r, u8 *text_g, u8 *text_b, u8 *back_r, u8 *back_g,	u8 *back_b) {
	*text_r = textRed;
	*text_g = textGreen;
	*text_b = textBlue;
	*back_r = backRed;
	*back_g = backGreen;
	*back_b = backBlue;
}

void ILI9325::SetTextColor(u8 red, u8 green, u8 blue) {
	textRed = red;
	textGreen = green;
	textBlue = blue;
}

void ILI9325::SetBackColor(u8 red, u8 green, u8 blue) {
	backRed = red;
	backGreen = green;
	backBlue = blue;
}


void ILI9325::PutPixel(int16_t x, int16_t y) {
	//if ((x > 239) || (y > 319)){
	if ((x > 319) || (y > 239)){
		return;
	}
	SetRotatedCursor(x, y);
	WriteRAM_Prepare();
	WriteRAM(textRed, textGreen, textBlue);
}

void ILI9325::setRotation(uint8_t x) {
	uint16_t t;
	switch(x) {
	default: t = 0x0030; break;
	case 1 : t = 0x0028; break;
	case 2 : t = 0x0000; break;
	case 3 : t = 0x0018; break;

	}
	WriteReg(R3, t ); // MADCTL
}


void ILI9325::Pixel(int16_t x, int16_t y, u8 r, u8 g, u8 b) {
	if ((x > 319) || (y > 239)){
		return;
	}
	SetRotatedCursor(x, y);
	WriteRAM_Prepare();
	WriteRAM(r, g, b);
}

void ILI9325::zeichen_small_1x(const uint8_t *font,uint8_t z, uint16_t spalte, uint16_t zeile, uint8_t offset){
	zeichen_small_scale(1,font,z,spalte,zeile,offset);
}
void ILI9325::zeichen_small_2x(const uint8_t *font,uint8_t z, uint16_t spalte, uint16_t zeile, uint8_t offset){
	zeichen_small_scale(2,font,z,spalte,zeile,offset);
}
void ILI9325::zeichen_small_3x(const uint8_t *font,uint8_t z, uint16_t spalte, uint16_t zeile, uint8_t offset){
	zeichen_small_scale(3,font,z,spalte,zeile,offset);
}
void ILI9325::zeichen_small_4x(const uint8_t *font,uint8_t z, uint16_t spalte, uint16_t zeile, uint8_t offset){
	zeichen_small_scale(4,font,z,spalte,zeile,offset);
}
void ILI9325::zeichen_small_5x(const uint8_t *font,uint8_t z, uint16_t spalte, uint16_t zeile, uint8_t offset){
	zeichen_small_scale(5,font,z,spalte,zeile,offset);
}
void ILI9325::zeichen_small_6x(const uint8_t *font,uint8_t z, uint16_t spalte, uint16_t zeile, uint8_t offset){
	zeichen_small_scale(6,font,z,spalte,zeile,offset);
}

void ILI9325::zeichen_small_scale(uint8_t scale,const uint8_t *font,uint8_t z, uint16_t spalte, uint16_t zeile, uint8_t offset){
	unsigned int stelle;
	if((z<0x20)||(z>0x7f))z=0x20;
	stelle = 8*(z-0x20);

	for(int y_i=0;y_i<8;y_i++){ // oder <=8?
		char a = font[stelle];/* 8 px */
		for(int repeat_y=0;repeat_y<scale;repeat_y++){
			uint8_t mask=0x80;
			for(int x_i=0;x_i<7;x_i++){ // oder <8?
				if(a&mask){
					for(int repeat_x=0;repeat_x<scale;repeat_x++){
						Pixel(spalte*6 + scale*x_i+repeat_x, zeile*7 + scale*y_i+repeat_y, textRed, textGreen, textBlue);
					}
				} else {
					for(int repeat_x=0;repeat_x<scale;repeat_x++){
						Pixel(spalte*6 + scale*x_i+repeat_x, zeile*7 + scale*y_i+repeat_y, backRed, backGreen, backBlue);
					}
				}
				mask=mask>>1;
			}
		}
		stelle++;
	}
}

void ILI9325::string(char *str,uint8_t spalte, uint8_t zeile){
	string(Speedo.default_font,str,spalte,zeile,0,0,0,255,255,255,0);
}

void ILI9325::string(uint8_t font,char *str,uint8_t spalte, uint8_t zeile){
	string(font,str,spalte,zeile,255,255,255,0,0,0,0);
}

void ILI9325::string(char *str,uint8_t spalte, uint8_t zeile, uint8_t back, uint8_t text){
	string(Speedo.default_font,str,spalte,zeile,text*15,text*15,text*15,back*15,0,0,0);
}

void ILI9325::string(uint8_t font,char *str,uint8_t spalte, uint8_t zeile, uint8_t back, uint8_t text, uint8_t offset){
	string(font,str,spalte,zeile,text*15,text*15,text*15,back*15,0,0,offset);
}

void ILI9325::string(uint8_t font,char *str,uint8_t spalte, uint8_t zeile, uint8_t text_r, uint8_t text_g, uint8_t text_b, uint8_t back_r, uint8_t back_g, uint8_t back_b, uint8_t offset){
	SetColors(text_r,text_g,text_b,back_r,back_g,back_b);

	switch(font){
	case SANS_SMALL_4X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_4x(&dejaVuSans5ptBitmaps[0],str[i],(spalte+i*4),zeile,offset);
		};
		break;
	case SANS_SMALL_3X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_3x(&dejaVuSans5ptBitmaps[0],str[i],(spalte+i*3),zeile,offset);
		};
		break;
	case SANS_SMALL_2X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_2x(&dejaVuSans5ptBitmaps[0],str[i],(spalte+i*2),zeile,offset);
		};
		break;
	case SANS_SMALL_1X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_1x(&dejaVuSans5ptBitmaps[0],str[i],(spalte+i),zeile,offset);
		};
		break;
	case VISITOR_SMALL_4X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_4x(&visitor_code[0],str[i],(spalte+i*4),zeile,offset);
		};
		break;
	case VISITOR_SMALL_3X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_3x(&visitor_code[0],str[i],(spalte+i*3),zeile,offset);
		};
		break;
	case VISITOR_SMALL_2X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_2x(&visitor_code[0],str[i],(spalte+i*2),zeile,offset);
		};
		break;
	case VISITOR_SMALL_1X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_1x(&visitor_code[0],str[i],(spalte+i),zeile,offset);
		};
		break;
		//	case SANS_BIG_1X_FONT:
		//		for(unsigned int i=0;i<strlen(str);i++){
		//			zeichen_big_1x(&sans_big[0],str[i],(spalte+i*4),zeile,offset);
		//		};
		//		break;
	case STD_SMALL_2X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_2x(&std_small[0],str[i],(spalte+i*2),zeile,offset);
		};
		break;
	case STD_SMALL_3X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_3x(&std_small[0],str[i],(spalte+i*3),zeile,offset);
		};
		break;
	case STD_SMALL_4X_FONT:
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_4x(&std_small[0],str[i],(spalte+i*4),zeile,offset);
		};
		break;
	default: // das hier ist eigentlich STD_SMALL_1X_FONT
		for(unsigned int i=0;i<strlen(str);i++){
			zeichen_small_1x(&std_small[0],str[i],(spalte+i),zeile,offset);
		};
		break;
	};
}

void ILI9325::DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction) {
	uint32_t i = 0;

	SetRotatedCursor(Xpos, Ypos);
	if (Direction == Horizontal) {
		WriteRAM_Prepare(); /* Prepare to write GRAM */
		for (i = 0; i < Length; i++) {
			WriteRAM(textRed, textGreen, textBlue);
		}
	} else {
		for (i = 0; i < Length; i++) {
			WriteRAM_Prepare(); /* Prepare to write GRAM */
			WriteRAM(textRed, textGreen, textBlue);
			Ypos++;
			SetRotatedCursor(Xpos, Ypos);
		}
	}

}

void ILI9325::DrawRect(uint16_t Xpos, uint16_t Ypos, uint8_t Height, uint16_t Width) {
	int x, y;
	x = 0;
	y = 0;
	while (x < Height + 1) {
		PutPixel(Xpos + x, Ypos);
		PutPixel(Xpos + x, Ypos + Width);
		x++;
	}
	while (y < Width + 1) {
		PutPixel(Xpos, Ypos + y);
		PutPixel(Xpos + Height, Ypos + y);
		y++;
	}
}

void ILI9325::DrawSquare(uint16_t Xpos, uint16_t Ypos, uint16_t a) {
	int x, y;
	x = 0;
	y = 0;
	while (x < a + 1) {
		PutPixel(Xpos + x, Ypos);
		PutPixel(Xpos + x, Ypos + a);
		x++;
	}
	while (y < a + 1) {
		PutPixel(Xpos, Ypos + y);
		PutPixel(Xpos + a, Ypos + y);
		y++;
	}
}

void ILI9325::DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius) {
	int16_t D;/* Decision Variable */
	uint16_t CurX;/* Current X Value */
	uint16_t CurY;/* Current Y Value */

	D = 3 - (Radius << 1);
	CurX = 0;
	CurY = Radius;
	while (CurX <= CurY) {
		PutPixel(Xpos + CurX, Ypos + CurY);
		PutPixel(Xpos + CurX, Ypos - CurY);
		PutPixel(Xpos - CurX, Ypos + CurY);
		PutPixel(Xpos - CurX, Ypos - CurY);
		PutPixel(Xpos + CurY, Ypos + CurX);
		PutPixel(Xpos + CurY, Ypos - CurX);
		PutPixel(Xpos - CurY, Ypos + CurX);
		PutPixel(Xpos - CurY, Ypos - CurX);
		if (D < 0) {
			D += (CurX << 2) + 6;
		} else {
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}

void ILI9325::DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius) {
	int16_t D;
	uint16_t CurX;
	uint16_t CurY;
	D = 3 - (Radius << 1);
	CurX = 0;
	CurY = Radius;

	while (CurX <= CurY) {
		DrawUniLine(Xpos + CurX, Ypos + CurY, Xpos + CurX, Ypos - CurY);
		DrawUniLine(Xpos - CurX, Ypos + CurY, Xpos - CurX, Ypos - CurY);
		DrawUniLine(Xpos + CurY, Ypos + CurX, Xpos + CurY, Ypos - CurX);
		DrawUniLine(Xpos - CurY, Ypos + CurX, Xpos - CurY, Ypos - CurX);

		if (D < 0) {
			D += (CurX << 2) + 6;
		} else {
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
}
void ILI9325::DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height) {
	int x, y;
	x = 0;
	y = 0;
	while (x < Width) {
		while (y < Height) {
			PutPixel(Xpos + x, Ypos + y);
			y++;
		}
		y = 0;
		x++;
	}
}

void ILI9325::DrawFullSquare(uint16_t Xpos, uint16_t Ypos, uint16_t a) {
	int x, y;
	x = 0;
	y = 0;
	while (x < a) {
		while (y < a) {
			PutPixel(Xpos + x, Ypos + y);
			y++;
		}
		y = 0;
		x++;
	}
}

void ILI9325::DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 =	0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, curpixel = 0;

	deltax = abs(x2 - x1); /* The difference between the x's */
	deltay = abs(y2 - y1); /* The difference between the y's */
	x = x1; /* Start x off at the first pixel */
	y = y1; /* Start y off at the first pixel */

	if (x2 >= x1) /* The x-values are increasing */
	{
		xinc1 = 1;
		xinc2 = 1;
	} else /* The x-values are decreasing */
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	if (y2 >= y1) /* The y-values are increasing */
	{
		yinc1 = 1;
		yinc2 = 1;
	} else /* The y-values are decreasing */
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) /* There is at least one x-value for every y-value */
	{
		xinc1 = 0; /* Don't change the x when numerator >= denominator */
		yinc2 = 0; /* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax; /* There are more x-values than y-values */
	} else /* There is at least one y-value for every x-value */
	{
		xinc2 = 0; /* Don't change the x for every iteration */
		yinc1 = 0; /* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay; /* There are more y-values than x-values */
	}

	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		PutPixel(x, y); /* Draw the current pixel */
		num += numadd; /* Increase the numerator by the top of the fraction */
		if (num >= den) /* Check if numerator >= denominator */
		{
			num -= den; /* Calculate the new numerator value */
			x += xinc1; /* Change the x as appropriate */
			y += yinc1; /* Change the y as appropriate */
		}
		x += xinc2; /* Change the x as appropriate */
		y += yinc2; /* Change the y as appropriate */
	}
}

void ILI9325::Cross(u16 posX, u16 posY, u8 size) { // TODO zerstört durch drehen!
	u8 count = 0;
	if (posX < size || posY < size)
		return;

	SetRotatedCursor(posX - size, posY);
	WriteRAM_Prepare();
	for (count = 0; count < (size << 1) + 1; count++) {
		LCD_RAM = textRed << 8;
		LCD_RAM = textGreen << 8;
		LCD_RAM = textBlue << 8;
	}

	for (count = 0; count < (size << 1) + 1; count++) {
		SetRotatedCursor(posX, posY - size + count);
		WriteRAM_Prepare();
		LCD_RAM = textRed << 8;
		LCD_RAM = textGreen << 8;
		LCD_RAM = textBlue << 8;
	}
}

/*******************************************************************************
 * Function Name  : SetCursor
 * Description    : Sets the cursor position.
 * Input          : - Xpos: specifies the X position.
 *                  - Ypos: specifies the Y position.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::SetCursor(u8 Xpos, u16 Ypos) {
	WriteReg(R32, Xpos);
	WriteReg(R33, Ypos);
}

void ILI9325::SetRotatedCursor(u16 Xpos, u8 Ypos){
	WriteReg(R32, 240-Ypos);
	WriteReg(R33, Xpos);
}


/*******************************************************************************
 * Function Name  : WriteReg
 * Description    : Writes to the selected LCD register.
 * Input          : - LCD_REG: address of the selected register.
 *                  - LCD_RegValue: value to write to the selected register.
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::WriteReg(u8 LCD_Rega, u16 LCD_RegValue){
	LCD_REG = 0x00;
	LCD_REG = LCD_Rega << 8; //  dump?
	/* Write 16-bit LCD_REG */
	LCD_RAM = LCD_RegValue;
	LCD_RAM = LCD_RegValue << 8;
}

/*******************************************************************************
 * Function Name  : ReadReg
 * Description    : Reads the selected LCD Register.
 * Input          : None
 * Output         : None
 * Return         : LCD Register Value.
 *******************************************************************************/
u16 ILI9325::ReadReg(u8 LCD_Reg) {
	u16 val = 0;
	// Write 16-bit Index (then Read LCD_REG) *
	LCD_REG = 0x00;
	LCD_REG = LCD_REG << 8;
	/* Read 16-bit LCD_REG */
	val = LCD_RAM;
	val |= (LCD_RAM >> 8);
	return val;
}

/*******************************************************************************
 * Function Name  : WriteRAM_Prepare
 * Description    : Prepare to write to the LCD LCD_RAM.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::WriteRAM_Prepare(void) {
	LCD_REG = 0x00;
	LCD_REG = R34 << 8; /* Select GRAM LCD_REG */
}

/*******************************************************************************
 * Function Name  : WriteRAM
 * Description    : Writes to the LCD LCD_RAM.
 * Input          : - RGB_Code: the pixel color in RGB mode (5-6-5).
 * Output         : None
 * Return         : None
 *******************************************************************************/
void ILI9325::WriteRAM(u8 r, u8 g, u8 b) {
	LCD_RAM = r << 8;
	LCD_RAM = g << 8;
	LCD_RAM = b << 8;
}

/*******************************************************************************
 * Function Name  : ReadRAM
 * Description    : Reads the LCD LCD_RAM.
 * Input          : None
 * Output         : None
 * Return         : LCD LCD_RAM Value.
 *******************************************************************************/
u32 ILI9325::ReadRAM(void) {
	uint32_t val = 0;
	uint16_t low = 0;
	uint16_t high = 0;

	u8 red;
	u8 green;
	u8 blue;

	// Write 16-bit Index (then Read LCD_REG)
	LCD_REG = 0x00;
	LCD_REG = R34 << 8; /* Select GRAM LCD_REG */
	/* Read 16-bit LCD_REG */
	val = LCD_RAM; //Dummy Read
	val = LCD_RAM; //Dummy Read
	high = LCD_RAM; //Read upper 8 bits
	low = LCD_RAM; //Read lower 8 bites
	val = (high & 0xFF00) | (low >> 8);
#ifdef BRG
	blue = (val >> 8) & 0xF8; //Keep five highest bits
	red = (val & 0x1F) << 3; //Keep five highest bits
#else
	red = (val >> 8) & 0xF8; //Keep five highest bits
	blue = (val & 0x1F) << 3;//Keep five highest bits
#endif
	green = (val >> 3) & 0xFC; //Keep six highest bits
	val = (red << 16) | (green << 8) | blue;
	return val;
}

/**
 *************************************************************************
 *
 * Set control lines in FSMC mode
 *
 *************************************************************************
 */
void ILI9325::CtrlLinesConfig() {
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO D and E clocks */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE,ENABLE);

	// Create GPIO D Init structure for used pins
	/* Pin configuration
	 *       ~RD :             -> PD4  [FSMS NOE] (2.32)
	 *       ~WR :             -> PD5  [FSMS NWE] (2.29) (USB OTG Over Current LED)
	 *
	 *       ~CS :             -> PD7  [FSMS NE1] (2.27)
	 *        D5 : LCD Pin D13 -> PD8  [FSMC D13] (1.40)
	 *        D6 : LCD Pin D14 -> PD9  [FSMC D14] (1.41)
	 *        D7 : LCD Pin D15 -> PD10 [FSMC D15] (1.42)
	 *        RS :     	 	   -> PD11 [FSMS A16] (1.43)
	 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* Initialize pins */
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* Configure GPIO D pins as FSMC alternate functions */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC); // NOE -> RD
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC); // NWE -> WR
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC); // NE1 -> CS
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC); // D13 -> D5
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC); // D14 -> D6
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC); // D15 -> D7
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC); // A16 -> RS

	/*
	 *       LED Backlight :   -> PE9  (1.27) (Timer 1 channel 1) // not used, hard wired to 3.3V
	 *
	 *        D0 : LCD Pin D8  -> PE11 [FSMC D08] (1.29)
	 *        D1 : LCD Pin D9  -> PE12 [FSMC D09] (1.30)
	 *        D2 : LCD Pin D10 -> PE13 [FSMC D10] (1.31)
	 *        D3 : LCD Pin D11 -> PE14 [FSMC D11] (1.32)
	 *        D4 : LCD Pin D12 -> PE15 [FSMC D12] (1.33)
	 */

	/* Create GPIO E Init structure for used pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* Initialize GPIOE pins */
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* Configure GPIO E pins as FSMC alternate functions */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC); // D8  -> D0
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC); // D9  -> D1
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC); // D10 -> D2
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC); // D11 -> D3
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC); // D12 -> D4

	/**       ~RST:   		   -> PE10 (1.28) **/

	/* Configure Reset Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	/* Initialize GPIOE pins */
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	return;
}

/**
 **********************************************************************************
 *
 * Configure FSMC controller
 * Read and write timing parameters are distinct
 *
 **********************************************************************************
 */
void ILI9325::FSMCConfig() {
	FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructureRead;
	FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructureWrite;

	/* Enable FSMC Clock */
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

	/* Define Read timing parameters */
	FSMC_NORSRAMTimingInitStructureRead.FSMC_AddressSetupTime = 3;
	FSMC_NORSRAMTimingInitStructureRead.FSMC_AddressHoldTime = 0;
	FSMC_NORSRAMTimingInitStructureRead.FSMC_DataSetupTime = 15;
	FSMC_NORSRAMTimingInitStructureRead.FSMC_BusTurnAroundDuration = 0;
	FSMC_NORSRAMTimingInitStructureRead.FSMC_CLKDivision = 1;
	FSMC_NORSRAMTimingInitStructureRead.FSMC_DataLatency = 0;
	FSMC_NORSRAMTimingInitStructureRead.FSMC_AccessMode = FSMC_AccessMode_A;

	/* Define Write Timing parameters */
	FSMC_NORSRAMTimingInitStructureWrite.FSMC_AddressSetupTime = 3;
	FSMC_NORSRAMTimingInitStructureWrite.FSMC_AddressHoldTime = 0;
	FSMC_NORSRAMTimingInitStructureWrite.FSMC_DataSetupTime = 6;
	FSMC_NORSRAMTimingInitStructureWrite.FSMC_BusTurnAroundDuration = 0;
	FSMC_NORSRAMTimingInitStructureWrite.FSMC_CLKDivision = 1;
	FSMC_NORSRAMTimingInitStructureWrite.FSMC_DataLatency = 0;
	FSMC_NORSRAMTimingInitStructureWrite.FSMC_AccessMode = FSMC_AccessMode_A;

	/* Define protocol type */
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1; //Bank1
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; //No mux
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM; //SRAM type
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b; //16 bits wide ? Why ever
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable; //No Burst
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable; // No wait
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low; //Don'tcare
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable; //No wrap mode
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState; //Don't care
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable; //Don't care
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; //Allow distinct Read/Write parameters
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; //Don't care

	// Set read timing structure
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructureRead;

	// Set write timing structure
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructureWrite;

	// Initialize FSMC for read and write
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	// Enable FSMC
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);

	return;
}

void ILI9325::GetType(char model[]) {
	u16 LCD_RegValue = 0x0000;
	const u8 zero = 0x30;
	LCD_RegValue = ReadReg(0x00);
	model[0] = ((LCD_RegValue >> 12) & 0x0F) + zero;
	model[1] = ((LCD_RegValue >> 8) & 0x0F) + zero;
	model[2] = ((LCD_RegValue >> 4) & 0x0F) + zero;
	model[3] = (LCD_RegValue & 0x0F) + zero;
	model[4] = 0;
}

/**
 * Function Name  : SetBackLightLevel
 * Description    : Set back light intensity
 * Input          : Intensity level ranging from 0% to 100%
 * Output         : None
 * Return         : None
 *
 */
void ILI9325::SetBackLight(unsigned short level) {

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	uint16_t CCR1_Val = level;
	uint16_t PrescalerValue;

	// Start Timer 1 clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	//Enable port E clock
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	// Configure PE9 as output for PWM
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	// Connect PE9 to Timer 1 channel 1
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_TIM1);

	/* Compute the prescaler value
	 * Timer 1,8,9,10 and 11 are connected to APB2 which runs at SystemCoreClock
	 * Other timers are connected to APB1 and runs at SystemCoreClock/2
	 Prescaler = (TIM1CLK / TIM1 counter clock) - 1
	 Prescaler = ((SystemCoreClock) /Frequency) - 1
	 */
	PrescalerValue = (uint16_t)((48000000UL) / 100000) - 1; // TODO

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 100;
	TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR1_Val;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM1, ENABLE);

	/* TIM1 enable counter */
	TIM_Cmd(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}


void ILI9325::show_storry(const char* storry,const char* title){
	show_storry(storry,title,0x00);
}

void ILI9325::show_storry(const char* storry,const char* title, uint8_t type){
	char storry_char[strlen(storry)+1];
	char title_char[strlen(title)+1];

	strcpy(storry_char,storry);
	strcpy(title_char,title);

	show_storry(storry_char,strlen(storry),title_char,strlen(title),type);
}

void ILI9325::show_storry(const char* storry,unsigned int storry_length,char title[],unsigned int title_length){
	show_storry(storry,storry_length,title, title_length, 0x00);
}

void ILI9325::show_storry(const char* storry,unsigned int storry_length,char title[],unsigned int title_length, uint8_t type){
	// show title
	clear_screen();
	highlight_bar(0,0,320,24); // mit hintergrundfarbe nen kasten malen
	string(Speedo.default_font,title,2,0,DISP_BRIGHTNESS,0,0);

	// Generate borders
	unsigned int fill_line=0; // actual line
	unsigned int char_in_line=0; // count char in this line
	int von[4]={0,0,0,0}; // nutzen um zu bestimmten wo wir den "\0" setzen
	int bis[4]={0,0,0,0}; // nutzen um zu bestimmten wo wir den "\0" setzen

	for(unsigned int i=0; i<storry_length;i++){
		if(char_in_line>10){ // mindestesn mal 10 zeichen aufnehmen
			if((storry[i]==' ' || char_in_line==21) && (fill_line+1<(sizeof(bis)/sizeof(bis[0])))){
				bis[fill_line]=i; // damit haben wir das ende dieser Zeile gefunden
				fill_line++;
				von[fill_line]=i;// und den anfang der nächsten, wobei das noch nicht save ost
				if(storry[i]==' '){ // wir haben hier ein freizeichen, hätten wir sinnvoll weitergucken können?
					von[fill_line]++; // das freizeichen brauchen wie eh nicht mehr
					//haben ein volles wort, mal sehen ob ncoh was geht
					//aktuell sind char_in_line chars im puffer
					int onemoreword=0; // wieviele chars gehts denn weiter, falls sinnvoll
					for(unsigned int k=char_in_line; k<21; k++){
						if(storry[i+k-char_in_line]==' ') onemoreword=k-char_in_line;
					}
					if(onemoreword>0){
						// es scheint sinnig noch onemoreword buchstaben zu nutzen
						i+=onemoreword;
						von[fill_line]+=onemoreword;
						bis[fill_line-1]+=onemoreword;
					}
				}
				char_in_line=-1; // wird gleich inc -> dann sind wir fertig und der counter bei 0
			}
		}
		char_in_line++;
	};
	if(bis[fill_line]==0){ // letztes array ding
		bis[fill_line]=storry_length;
	};
	// we got the borders

	// draw to display
	for(unsigned int i=0; i<4; i++){ // nur 4 zeilen
		if(von[i]!=bis[i]){
			// reserve buffer
			char *buffer2;
			buffer2 = (char*) malloc (22);
			if (buffer2==NULL) Serial.puts_ln(USART1,("Malloc failed"));
			else memset(buffer2,'\0',sizeof(buffer2)/sizeof(buffer2[0]));

			int k=0;
			for(int j=von[i]; j<bis[i] && k<22; j++){
				if(!(i==0 && storry[j]=='#')){ // in der ersten zeile, das erste "#" an stelle 0 überlesen
					buffer2[k]=storry[j];
					k++;
				};
			};
			buffer2[k]='\0';
			string(Speedo.default_font,buffer2,0,i+2,0,DISP_BRIGHTNESS,0);

			//delete buffer
			free(buffer2);
		};
	}

	//
	unsigned int current_state=Menu.state;
	unsigned long current_timestamp=Millis.get();

	// set buttons if needed
	if(type>=DIALOG_NO_YES){
		Menu.set_buttons(true,false,false,true);
	}

	if(type==DIALOG_NO_YES){
		string(Speedo.default_font,("\x7E back        next \x7F"),0,7);
	}
	else if(type==DIALOG_GO_RIGHT_200MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<200){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_right(true);
		}
	} else if(type==DIALOG_GO_LEFT_200MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<200){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_left(true);
		}
	} else if(type==DIALOG_GO_RIGHT_500MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<500){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_right(true);
		}
	} else if(type==DIALOG_GO_LEFT_500MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<500){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_left(true);
		}
	} else if(type==DIALOG_GO_RIGHT_1000MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<1000){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_right(true);
		}
	} else if(type==DIALOG_GO_LEFT_1000MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<1000){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_left(true);
		}
	} else if(type==DIALOG_GO_RIGHT_2000MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<2000){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_right(true);
		}
	} else if(type==DIALOG_GO_LEFT_2000MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<2000){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_left(true);
		}
	} else if(type==DIALOG_GO_RIGHT_5000MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<5000){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_right(true);
		}
	} else if(type==DIALOG_GO_LEFT_5000MS){
		while(current_state==Menu.state && (Millis.get()-current_timestamp)<5000){
			_delay_ms(1);
		}
		if(current_state==Menu.state){
			Menu.go_left(true);
		}
	} else if(type==DIALOG_SHOW_500MS){
		_delay_ms(500);
	}
}

void ILI9325::string_centered(const char* text, uint8_t line){
	string_centered(text,line,false);
}

void ILI9325::string_centered(const char* text, uint8_t line, bool inverted){
	//    if(strlen(text)>20){
	//        return;
	//    };
	//
	//    uint16_t front_color=0x0f;
	//    uint16_t back_color=0x00;
	//    uint16_t start_pos=0;
	//    uint16_t length_of_char=22;
	//
	//    if(inverted){
	//        front_color=0x00;
	//        back_color=0x0f;
	//        if(strlen(text)<=17){ // 6*2pixel == 2 chars. 21 Chars - 2*2 = 17chars max
	//            length_of_char=17;
	//            start_pos=2;
	//            highlight_bar(0,line*8,128,8);
	//        } else {
	//            filled_rect(0,line*8,128,8,0x0f);
	//        }
	//    }
	//    char text_char[length_of_char]; // full display width +1
	//    strcpy(text_char,text);
	//    Menu.center_me(text_char,length_of_char); // full display width
	//    string(Speedo.default_font,text_char,start_pos,line,back_color,front_color,0);
}


void ILI9325::highlight_bar(uint16_t x,uint16_t y,uint16_t width,uint16_t height){
	for(int i=0;i<16;i++){
		SetTextColor(15*i,0,0);
		DrawLine(x+i,y,height,Vertical);
		DrawLine(x+width-i,y,height,Vertical);
	}

	SetTextColor(255,0,0);
	DrawFullRect(x+16,y,width-31,height);
	SetTextColor(0,0,0);
	//	x=floor(x/2)*2;
	//	width=floor(width/2)*2;
	//
	//	send_command(0x15);
	//	send_command(floor(x/2));
	//	send_command(floor((width-1+x)/2));
	//	send_command(0x75);
	//	send_command(y);
	//	send_command(height+y);
	//
	//	for(int j=0;j<height;j++){
	//		unsigned char b=0x34;
	//		for(int i=0;i<6;i++){
	//			send_char(b);
	//			b+=34;
	//		}
	//		b=0xff;
	//		for(int a=0;a<(width/2)-12;a++){
	//			send_char(b);
	//		};
	//		b=0xED;
	//		for(int i=0;i<6;i++){
	//			send_char(b);
	//			b-=34;
	//		}
	//	};
};

void ILI9325::filled_rect(uint16_t x,uint16_t y,uint16_t width,uint16_t height,unsigned char color){
	if(color==0x00){
		SetTextColor(0,0,0);
	} else {
		SetTextColor(255,0,0);
	}
	DrawFullRect(x,y,width,height);
}


void ILI9325::draw_gps(uint16_t x,uint16_t y, unsigned char sats){
	//    send_command(0x15);
	//    send_command(x);
	//    send_command(x+3);
	//    send_command(0x75);
	//    send_command(y);
	//    send_command(y+7);
	//    char xy[2][4];
	//    xy[0][0]=0x00;
	//    xy[1][0]=0x00;
	//    xy[0][1]=0x00;
	//    xy[1][1]=0x00;
	//    xy[0][2]=0x00;
	//    xy[1][2]=0x00;
	//    xy[0][3]=0x00;
	//    xy[1][3]=0x00;
	//
	//    // 4 symbols:
	//    // 0 GPS => cross
	//    // 3 GPS => empty (indicating minimal signal)
	//    // 4 GPS => single bow (indicating poor signal)
	//    // 5-X GPS => dual bow (indicating strong signal)
	//
	//    if(sats==0){
	//        xy[0][0]=0x0F;        //     X X
	//        xy[1][0]=0x0F;        //      X
	//        //xy[0][1]=0x00;    //     X X
	//        xy[1][1]=0xF0;        //
	//        xy[0][2]=0x0F;        //
	//        xy[1][2]=0x0F;        //
	//        //xy[0][3]=0x00;    //
	//        //xy[1][3]=0x00;    //
	//    } else if(sats==4){
	//        //xy[0][0]=0x00;    //
	//        //xy[1][0]=0x00;    //
	//        //xy[0][1]=0x00;    //    X
	//        //xy[1][1]=0x00;    //     X
	//        xy[0][2]=0xF0;        //
	//        //xy[1][2]=0x00;    //
	//        xy[0][3]=0x0F;        //
	//        //xy[1][3]=0x00;    //
	//    } else if(sats>4){
	//        xy[0][0]=0xFF;        //    XX
	//        //xy[1][0]=0x00;    //      X
	//        //xy[0][1]=0x00;    //    X  X
	//        xy[1][1]=0xF0;        //     X X
	//        xy[0][2]=0xF0;        //
	//        xy[1][2]=0x0F;        //
	//        xy[0][3]=0x0F;        //
	//        xy[1][3]=0x0F;        //
	//    };
	//
	//
	//    send_char(0x00);  send_char(0x00);  send_char(xy[0][0]);  send_char(xy[1][0]);    //
	//    send_char(0x00);  send_char(0x00);  send_char(xy[0][1]);  send_char(xy[1][1]);    //
	//    send_char(0x0F);  send_char(0x00);  send_char(xy[0][2]);  send_char(xy[1][2]);    // x
	//    send_char(0xF0);  send_char(0xF0);  send_char(xy[0][3]);  send_char(xy[1][3]);    //x x
	//    send_char(0xF0);  send_char(0x0F);  send_char(0x00);        send_char(0x00);        //x  x
	//    send_char(0xF0);  send_char(0x00);  send_char(0xF0);      send_char(0x00);        //x   x
	//    send_char(0x0F);  send_char(0x00);  send_char(0x0F);      send_char(0x00);        // x   x
	//    send_char(0x00);  send_char(0xFF);  send_char(0xF0);      send_char(0x00);        //  xxx
};

void ILI9325::draw_oil(uint16_t x,uint16_t y){
	//    send_command(0x15);
	//    send_command(x);
	//    send_command(x+9);
	//    send_command(0x75);
	//    send_command(y);
	//    send_command(y+7);
	//    send_char(0xFF); send_char(0xF0); send_char(0x0F); send_char(0xFF); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00);
	//    send_char(0xF0); send_char(0x0F); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00); send_char(0x0F); send_char(0xFF); send_char(0xF0); send_char(0x00);
	//    send_char(0xF0); send_char(0x0F); send_char(0xFF); send_char(0xFF); send_char(0xFF); send_char(0x0F); send_char(0xF0); send_char(0xF0); send_char(0x00); send_char(0x00);
	//    send_char(0x0F); send_char(0x0F); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x0F); send_char(0x00); send_char(0xF0); send_char(0x00);
	//    send_char(0x00); send_char(0xFF); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x0F); send_char(0xFF); send_char(0x00);
	//    send_char(0x00); send_char(0x0F); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x0F); send_char(0x00); send_char(0xFF); send_char(0x70); send_char(0xF0);
	//    send_char(0x00); send_char(0x00); send_char(0xFF); send_char(0xFF); send_char(0xFF); send_char(0xF0); send_char(0x00); send_char(0xFF); send_char(0x77); send_char(0xF0);
	//    send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x00); send_char(0x0F); send_char(0xFF); send_char(0x00);
};

void ILI9325::draw_water(uint16_t x,uint16_t y){
	//    send_command(0x15);
	//    send_command(x);
	//    send_command(x+9);
	//    send_command(0x75);
	//    send_command(y);
	//    send_command(y+7);
	//    send_char(0x00);  send_char(0x77);  send_char(0x00);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xF0);  send_char(0x00);  send_char(0x00);  send_char(0x00);
	//    send_char(0x00);  send_char(0x77);  send_char(0x00);  send_char(0x00);  send_char(0x0F);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);
	//    send_char(0x00);  send_char(0x77);  send_char(0xF0);  send_char(0x0F);  send_char(0xFF);  send_char(0xFF);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);
	//    send_char(0x00);  send_char(0x77);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0x00);  send_char(0x00);
	//    send_char(0x00);  send_char(0x77);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0xF0);  send_char(0x00);
	//    send_char(0x00);  send_char(0x77);  send_char(0xF0);  send_char(0x0F);  send_char(0xFF);  send_char(0xFF);  send_char(0x00);  send_char(0x0F);  send_char(0xF0);  send_char(0x00);
	//    send_char(0x00);  send_char(0x77);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x0F);  send_char(0xF0);  send_char(0x00);
	//    send_char(0x00);  send_char(0x77);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);  send_char(0x00);
};

void ILI9325::draw_air(uint16_t x,uint16_t y){
	//    send_command(0x15);
	//    send_command(x);
	//    send_command(x+2);
	//    send_command(0x75);
	//    send_command(y);
	//    send_command(y+7);
	//    send_char(0x00);  send_char(0xF0);  send_char(0x00);
	//    send_char(0x00);  send_char(0xFF);  send_char(0xF0);
	//    send_char(0x00);  send_char(0xF0);  send_char(0x00);
	//    send_char(0x00);  send_char(0xFF);  send_char(0xF0);
	//    send_char(0x00);  send_char(0xF0);  send_char(0x00);
	//    send_char(0x0F);  send_char(0xFF);  send_char(0x00);
	//    send_char(0xFF);  send_char(0xFF);  send_char(0xF0);
	//    send_char(0x0F);  send_char(0xFF);  send_char(0x00);
};

void ILI9325::draw_fuel(uint16_t x,uint16_t y){
	//    send_command(0x15);
	//    send_command(x);
	//    send_command(x+3);
	//    send_command(0x75);
	//    send_command(y);
	//    send_command(y+6);
	//    send_char(0x0F);  send_char(0xFF);  send_char(0xF0);  send_char(0x0F);
	//    send_char(0x0F);  send_char(0x00);  send_char(0xF0);  send_char(0xF0);
	//    send_char(0x0F);  send_char(0x00);  send_char(0xF0);  send_char(0xF0);
	//    send_char(0x0F);  send_char(0xFF);  send_char(0xF0);  send_char(0xF0);
	//    send_char(0x0F);  send_char(0xFF);  send_char(0xFF);  send_char(0x00);
	//    send_char(0x0F);  send_char(0xFF);  send_char(0xF0);  send_char(0x00);
	//    send_char(0xFF);  send_char(0xFF);  send_char(0xFF);  send_char(0x00);
};

void ILI9325::draw_clock(uint16_t x,uint16_t y){
	//    send_command(0x15);
	//    send_command(x);
	//    send_command(x+3);
	//    send_command(0x75);
	//    send_command(y);
	//    send_command(y+7);
	//    send_char(0x00);  send_char(0xFF);  send_char(0xF0);  send_char(0x00);
	//    send_char(0x0F);  send_char(0x0F);  send_char(0x0F);  send_char(0x00);
	//    send_char(0xF0);  send_char(0x0F);  send_char(0x00);  send_char(0xF0);
	//    send_char(0xF0);  send_char(0x0F);  send_char(0xF0);  send_char(0xF0);
	//    send_char(0xF0);  send_char(0x00);  send_char(0x00);  send_char(0xF0);
	//    send_char(0x0F);  send_char(0x00);  send_char(0x0F);  send_char(0x00);
	//    send_char(0x00);  send_char(0xFF);  send_char(0xF0);  send_char(0x00);
};

void ILI9325::draw_blitzer(uint16_t x,uint16_t y){
	//    send_command(0x15);
	//    send_command(x);
	//    send_command(x+4);
	//    send_command(0x75);
	//    send_command(y);
	//    send_command(y+14);
	//    send_char(0xFF); send_char(0xFF); send_char(0xFF); send_char(0xFF); send_char(0xF0);
	//    send_char(0xFF); send_char(0xF0); send_char(0x00); send_char(0xFF); send_char(0xF0);
	//    send_char(0xFF); send_char(0x00); send_char(0x00); send_char(0x0F); send_char(0xF0);
	//    send_char(0xFF); send_char(0x00); send_char(0x00); send_char(0x0F); send_char(0xF0);
	//    send_char(0xFF); send_char(0x00); send_char(0x00); send_char(0x0F); send_char(0xF0);
	//    send_char(0xFF); send_char(0xF0); send_char(0x00); send_char(0xFF); send_char(0xF0);
	//    send_char(0xFF); send_char(0xFF); send_char(0xFF); send_char(0xFF); send_char(0xF0);
	//    send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00);
	//    send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00);
	//    send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00);
	//    send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00);
	//    send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00);
	//    send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00);
	//    send_char(0x00); send_char(0x00); send_char(0xF0); send_char(0x00); send_char(0x00);
}

int ILI9325::animation(int a){
	return 0;
};

void ILI9325::draw_arrow(int arrow, int spalte, int zeile){

}