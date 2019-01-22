//Copyright Affonso Amendola 2019
//Distributed under GPLv3, check the LICENSE file for some great licensing.
//
//So, I decided to write an EGA library, just for fun yknow, had nothing to do with my weekend,
//and the logical conclusion was "ALRIGHT, LETS CODE A FECKING EGA GRAPHICS LIBRARY, HELL YEAH", and so, 
//this is what I wrote,
//
//Hopefully, it works.
//
//Hopefully.
//

#include <time.h>

#define MISCELLANEOUS_OUTPUT_REGISTER			0x3C2
#define FEATURE_CONTROL_REGISTER 				0x3DA
#define FEATURE_CONTROL_REGISTER_MONO			0x3BA
#define INPUT_STATUS_REGISTER_0					0x3C2
#define	INPUT_STATUS_REGISTER_1					0x3D2
#define INPUT_STATUS_REGISTER_1_MONO			0x3B2

#define SEQUENCER_ADDRESS_REGISTER				0x3C4
#define SEQUENCER_DATA_REGISTER					0x3C5

#define SEQUENCER_RESET_INDEX					0x00
#define SEQUENCER_CLOCKING_MODE_INDEX			0x01
#define SEQUENCER_MAP_MASK_INDEX				0x02
#define SEQUENCER_CHARACTER_MAP_SELECT_INDEX	0x03
#define SEQUENCER_MEMORY_MODE_INDEX				0x04

#define CRTC_ADDRESS_REGISTER					0x3D4
#define CRTC_DATA_REGISTER						0x3D5
#define CRTC_ADDRESS_REGISTER_MONO				0x3B4
#define CRTC_DATA_REGISTER_MONO					0x3B5

#define CRTC_H_TOTAL_INDEX						0x00
#define CRTC_H_DISPLAY_END_INDEX				0x01
#define CRTC_START_H_BLANK_INDEX				0x02
#define CRTC_END_H_BLANK_INDEX					0x03
#define CRTC_START_H_RETRACE_INDEX				0x04
#define CRTC_END_H_RETRACE_INDEX				0x05
#define CRTC_V_TOTAL_INDEX						0x06
#define CRTC_OVERFLOW_INDEX						0x07
#define CRTC_PRESET_ROW_SCAN_INDEX				0x08
#define CRTC_MAX_SCAN_LINE_INDEX				0x09
#define CRTC_CURSOR_START_INDEX					0x0A
#define CRTC_CURSOR_END_INDEX					0x0B
#define CRTC_START_ADDRESS_HIGH_INDEX			0x0C
#define CRTC_START_ADDRESS_LOW_INDEX			0x0D
#define CRTC_CURSOR_LOCATION_HIGH_INDEX			0x0E
#define CRTC_CURSOR_LOCATION_LOW_INDEX			0x0F
#define CRTC_START_V_RETRACE_INDEX				0x10
#define CRTC_END_V_RETRACE_INDEX				0x11
#define CRTC_LIGHT_PEN_HIGH_INDEX				0x10
#define CRTC_LIGHT_PEN_LOW_INDEX				0x11
#define CRTC_V_DISPLAY_END_INDEX				0x12
#define CRTC_OFFSET_INDEX						0x13
#define CRTC_UNDERLINE_LOCATION_INDEX			0x14
#define CRTC_START_V_BLANK_INDEX				0x15
#define CRTC_END_V_BLANK_INDEX					0x16
#define CRTC_MODE_CONTROL_INDEX					0x17
#define CRTC_LINE_COMPARE_INDEX					0x18

#define GFX_1_POSITION_REGISTER					0x3CC
#define GFX_2_POSITION_REGISTER					0x3CA
#define GFX_ADDRESS_REGISTER					0x3CE
#define GFX_DATA_REGISTER						0x3CF

#define GFX_SET_RESET_INDEX						0x00
#define GFX_ENABLE_SET_RESET_INDEX				0x01
#define GFX_COLOR_COMPARE_INDEX					0x02
#define GFX_DATA_ROTATE_INDEX					0x03
#define GFX_READ_MAP_SELECT_INDEX				0x04
#define GFX_MODE_REGISTER_INDEX					0x05
#define GFX_MISC_INDEX							0x06
#define GFX_COLOR_DONT_CARE_INDEX				0x07
#define GFX_BIT_MASK_INDEX						0x08	

#define ATTR_CONTROLLER_ADDRESS_REGISTER 		0x3C0
#define ATTR_CONTROLLER_DATA_REGISTER 			0x3C0

#define	ATTR_CONTROLLER_PALLETE_INDEX			0x00 	//	Actually, the pallete registers goes from 
														//	index 0x00 to 0x0F, keep that in mind

#define ATTR_CONTROLLER_MODE_CONTROL_INDEX				0x10
#define ATTR_CONTROLLER_OVERSCAN_COLOR_INDEX			0x11
#define ATTR_CONTROLLER_COLOR_PLANE_ENABLE_INDEX		0x12
#define ATTR_CONTROLLER_H_PIXEL_PANNING_REGISTER_INDEX	0x13

#define COLOR_BLACK			0x0
#define COLOR_BLUE			0x1
#define COLOR_GREEN			0x2
#define COLOR_CYAN			0x3
#define COLOR_RED 			0x4
#define COLOR_MAGENTA		0x5
#define COLOR_BROWN			0x6
#define COLOR_LIGHT_GRAY	0x7
#define COLOR_GRAY 			0x8
#define COLOR_LIGHT_BLUE	0x9
#define COLOR_LIGHT_GREEN	0xA
#define COLOR_LIGHT_CYAN	0xB
#define COLOR_LIGHT_RED		0xC
#define COLOR_LIGHT_MAGENTA	0xD
#define COLOR_YELLOW		0xE
#define COLOR_WHITE			0xF

unsigned char far * FRAME_BUFFER = (unsigned char far *)0x00000000;

int SCREEN_RES_X = 0;
int SCREEN_RES_Y = 0;

int ECD_DISPLAY_CONNECTED = 0;

int SELECTED_PLANES = 0;
int ENABLED_SET_RESET = 0;
int SET_RESET = 0;

void set_ega_mode(int mode)
{
	//Receives a hex value and calls interrupt 10, with said value, making it enter the specified mode.
	//
	//If ECD_Display is 1, will use the other mode table
	//
	//Modes currently available:
	//====================================================================================================
	// Mode|			Type| 	Resolution|  Colors|  Rows/Cols|  Display Type|	 Max Pages|	 Base Address|
	//  00h|  Alphanumerical|	   640x200|      16|      40x25|    **Color/BW|          8|         B8000| 
	//  01h|  Alphanumerical|      640x200|      16|	  40x25|         Color|          8|	        B8000| 
	//  02h|  Alphanumerical|      640x200|      16|      80x25|    **Color/BW|          8|         B8000| 
	//  03h|  Alphanumerical|      640x200|      16|      80x25|         Color|          8|         B8000|  <-- This is the commonly used Text mode you see in DOS and etcetera
	//  04h|        Graphics|      320x200|       4|      40x25|         Color|          1|         B8000| 
	//  05h|        Graphics|      320x200|       4|	  40x25|    **Color/BW|          1|         B8000| 
	//  06h|        Graphics|      640x200|       2|      80x25|    **Color/BW|          1|         B8000| 
	//  07h|  Alphanumerical|      720x350|       4|      80x25|    Monochrome|          8|         B0000| 
	//  0Dh|        Graphics|      320x200|      16|      40x25|         Color|          8|         A0000|  <-- This is the commonly used EGA graphics mode, this "feels" like the EGA we know and love(hate)
	//  0Eh|        Graphics|      640x200|      16|      80x25|         Color|          4|         A0000| 
	//  0Fh|        Graphics|      640x350|       4|      80x25|    Monochrome|          2|         A0000| 
	//==Enhanced Color Display============================================================================                                             
	//  00h|  Alphanumerical|      320x350|   16/64|      40x25| High Res(ECD)|          8|         B8000| 
	//  01h|  Alphanumerical|      320x350|   16/64|      40x25| High Res(ECD)|          8|         B8000| 
	//  02h|  Alphanumerical|      640x350|   16/64|      80x25| High Res(ECD)|          8|         B8000| 
	//  03h|  Alphanumerical|      640x350|   16/64|      80x25| High Res(ECD)|          8|         B8000| 
	//  10h|        Graphics|      640x350|    4/16|      80x25| High Res(ECD)|          1|         A0000| 
	// *10h|        Graphics|      640x350|   16/64|      80x25| High Res(ECD)|          2|         A0000|
	//====================================================================================================
	//
	// * Means with more than 64k of gfx memory installed, I think
	//	
	// ** Color/BW modes aparently are the exact same as the Color modes, but with the Color Burst disabled.
	//
	// Modes 8, 9 and A are marked as RESERVED
	// Modes B and C are marked as RESERVED - INTERNAL USE
	// Will play around with them in the future, if you know something about them, hit me up.
	//
	// On the normal displays (i.e. NOT the enhanced color displays), I believe that the only colors able
	// to be shown are the 16 colors of the CGA pallette, I dont think you can change those if you`re using
	// a normal display, like you can on VGA boards, but with the Enhanced Color Display, YOU CAN!, I think,
	// from what I understood reading the manual, with the ECD, you can change the actual color of each of
	// the 16 color "slots", from a larger pallette of 64 colors, giving you some possible cool images.
	//
	// I'm not sure how well emulated are the ECD in DosBox or MAME, so I'll be using the "normal" modes for
	// my experiments, but I'll get to messing around with those modes eventually, if you know anything, hit
	// me up,

	if(mode >= 0x00 && mode <= 0x10)
	{
		_asm {
				mov ax, mode
				xor ah, ah
				int 10h
			 }
	}

	switch(mode)
	{
			case 0x00:
				FRAME_BUFFER = (unsigned char far *)0xB8000000;
				if(ECD_DISPLAY_CONNECTED == 0)
				{
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 200;
				}
				else
				{
					SCREEN_RES_X = 320;
					SCREEN_RES_Y = 350;
				}
				
			break;

			case 0x01:
				FRAME_BUFFER = (unsigned char far *)0xB8000000;
				if(ECD_DISPLAY_CONNECTED == 0)
				{
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 200;
				}
				else
				{
					SCREEN_RES_X = 320;
					SCREEN_RES_Y = 350;
				}
			break;

			case 0x02:
				FRAME_BUFFER = (unsigned char far *)0xB8000000;
				if(ECD_DISPLAY_CONNECTED == 0)
				{
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 200;
				}
				else
				{
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 350;
				}
			break;

			case 0x03:
				FRAME_BUFFER = (unsigned char far *)0xB8000000;
				if(ECD_DISPLAY_CONNECTED == 0)
				{
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 200;
				}
				else
				{
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 350;
				}
			break;

			case 0x04:
				FRAME_BUFFER = (unsigned char far *)0xB8000000;
				SCREEN_RES_X = 320;
				SCREEN_RES_Y = 200;
			break;

			case 0x05:
				FRAME_BUFFER = (unsigned char far *)0xB8000000;
				SCREEN_RES_X = 320;
				SCREEN_RES_Y = 200;
			break;

			case 0x06:
				FRAME_BUFFER = (unsigned char far *)0xB8000000;
				SCREEN_RES_X = 640;
				SCREEN_RES_Y = 200;
			break;

			case 0x07:
				FRAME_BUFFER = (unsigned char far *)0xB0000000;
				SCREEN_RES_X = 720;
				SCREEN_RES_Y = 350;
			break;

			case 0x0D:
				FRAME_BUFFER = (unsigned char far *)0xA0000000;
				SCREEN_RES_X = 320;
				SCREEN_RES_Y = 200;
			break;

			case 0x0E:
				FRAME_BUFFER = (unsigned char far *)0xA0000000;
				SCREEN_RES_X = 720;
				SCREEN_RES_Y = 350;
			break;

			case 0x0F:
				FRAME_BUFFER = (unsigned char far *)0xA0000000;
				SCREEN_RES_X = 720;
				SCREEN_RES_Y = 350;
			break;

			case 0x10:
				if(ECD_DISPLAY_CONNECTED)
				{
					FRAME_BUFFER = (unsigned char far *)0xA0000000;
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 350;
				}
			break;
	}
}

void set_pixel(int x, int y, unsigned char color)
{
	//Receives an x, y pair, and a color and plots a pixel with the respective color at position (x,y)
 	//on the screen.

 	//Makes use of the bitmask to prevent alteration of data already present and the set/reset
 	//register to actually place the color.

 	//I think there is a faster way of doing this, using the Map Mask register, but after a few hours 
 	//I gave up trying to use that, maybe later I'll figure it out and ill come back to messing around
 	//with this and hopefully make it faster, 

 	//As of right now, on my computer, running dosbox at max cycles, a screen fill is very perceptible,
 	//takes like half a second to fill the screen.
 
 	//Which is unacceptable.

	unsigned char bit_mask = 0x00;

	bit_mask = 0x80>>(x & 7);

	_asm 	{
				mov dx, GFX_ADDRESS_REGISTER
				mov al, GFX_SET_RESET_INDEX
				mov ah, color
				out dx, ax
				mov al, GFX_ENABLE_SET_RESET_INDEX
				mov ah, 0x0F
				out dx, ax
				mov al, GFX_BIT_MASK_INDEX
				mov ah, bit_mask
				out dx, ax
			}

	*(FRAME_BUFFER + (x>>3) + y*(SCREEN_RES_X>>3)) &= 0;
}

int main()
{
	int i = 0;
	int j = 0;


	return 0;
}