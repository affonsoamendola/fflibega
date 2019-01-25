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

#include <bios.h>
#include <dos.h>

#include <stdio.h>
#include <stdlib.h>

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

#define EGA_TEXT_MODE		0x03
#define EGA_GRAPHICS_MODE	0x0D

#define CHAR_SET_ROM		0xF000FA6E

int CURRENT_PAGE = 0;

int PAGE_0_OFFSET = 0x0000;
int PAGE_1_OFFSET = 0x0000;

unsigned char far * PAGE_0_ADDRESS = 0x00000000;
unsigned char far * PAGE_1_ADDRESS = 0x00000000;

unsigned char far * IMAGE_STORAGE = 0x00000000;

int SCREEN_RES_X = 0;
int SCREEN_RES_Y = 0;

int ECD_DISPLAY_CONNECTED = 0;

int DOUBLE_BUFFER_ENABLED = 1;

int MONOCHROME_MODE = 0;

int WRITE_IMAGE_SIZE = 1;
//If 1, load_pgm will write the size of the image it's loading in the memory in the first two words
//before the real image data, so that the other functions can know how big is the image.
//
//You can disable this manually using set_write_image_size, if you want to load something directly to
//the screen

int FILE_POSITION = 0;
//For some bizarre reason I couldn't get ftell() and fseek() to work properly, this is a hack to make
//something similar work.
//I didn't care enough to look for the right way of using those functions, so I just said fuck it and
//made it my own way
//Should be faster anyway :P

void set_write_image_size(int value)
{
	WRITE_IMAGE_SIZE = value;
}

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

	MONOCHROME_MODE = 0;
	DOUBLE_BUFFER_ENABLED = 0;

	switch(mode)
	{
			case 0x00:
				PAGE_0_ADDRESS = (unsigned char far *)0xB8000000;
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
				PAGE_0_ADDRESS = (unsigned char far *)0xB8000000;
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
				PAGE_0_ADDRESS = (unsigned char far *)0xB8000000;
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
				PAGE_0_ADDRESS = (unsigned char far *)0xB8000000;
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
				PAGE_0_ADDRESS = (unsigned char far *)0xB8000000;
				SCREEN_RES_X = 320;
				SCREEN_RES_Y = 200;
			break;

			case 0x05:
				PAGE_0_ADDRESS = (unsigned char far *)0xB8000000;
				SCREEN_RES_X = 320;
				SCREEN_RES_Y = 200;
			break;

			case 0x06:
				PAGE_0_ADDRESS = (unsigned char far *)0xB8000000;
				SCREEN_RES_X = 640;
				SCREEN_RES_Y = 200;
			break;

			case 0x07:
				PAGE_0_ADDRESS = (unsigned char far *)0xB0000000;
				SCREEN_RES_X = 720;
				SCREEN_RES_Y = 350;
				MONOCHROME_MODE = 1;
			break;

			case 0x0D:
				SCREEN_RES_X = 320;
				SCREEN_RES_Y = 200;
				DOUBLE_BUFFER_ENABLED = 1;
				PAGE_0_OFFSET = 0x0000;
				PAGE_0_ADDRESS = (unsigned char far *)0xA0000000;
				PAGE_1_OFFSET = (320/4)*200;
				PAGE_1_ADDRESS =  PAGE_0_ADDRESS + PAGE_1_OFFSET;
				IMAGE_STORAGE = PAGE_1_ADDRESS + PAGE_1_OFFSET;
			break;

			case 0x0E:
				PAGE_0_ADDRESS = (unsigned char far *)0xA0000000;
				SCREEN_RES_X = 720;
				SCREEN_RES_Y = 350;
			break;

			case 0x0F:
				PAGE_0_ADDRESS = (unsigned char far *)0xA0000000;
				SCREEN_RES_X = 720;
				SCREEN_RES_Y = 350;
				MONOCHROME_MODE = 1;
			break;

			case 0x10:
				if(ECD_DISPLAY_CONNECTED)
				{
					PAGE_0_ADDRESS = (unsigned char far *)0xA0000000;
					SCREEN_RES_X = 640;
					SCREEN_RES_Y = 350;
				}
			break;
	}
}

unsigned char far * get_framebuffer()
{
	//Returns a pointer to the current framebuffer address
	//
	//If DOUBLE_BUFFER_ENABLED == 0, simply returns the address to PAGE_0, no matter what is the
	//value of CURRENT_PAGE

	if(CURRENT_PAGE == 1 && DOUBLE_BUFFER_ENABLED)
	{
		return (unsigned char far *)PAGE_1_ADDRESS;
	}
	else
	{
		return (unsigned char far *)PAGE_0_ADDRESS;
	}
}

unsigned char far * get_drawbuffer()
{
	//Returns a pointer to the current drawbuffer address
	//
	//If DOUBLE_BUFFER_ENABLED == 0, simply returns the address to PAGE_0, no matter what is the
	//value of CURRENT_PAGE

	if(CURRENT_PAGE == 0 && DOUBLE_BUFFER_ENABLED)
	{
		return (unsigned char far *)PAGE_1_ADDRESS;
	}
	else
	{
		return (unsigned char far *)PAGE_0_ADDRESS;
	}
}

void page_flip()
{
	//Toggles the start address of the CRTC between PAGE_0 and PAGE_1

	//If DOUBLE_BUFFER_ENABLED == 0, does ABSOLUTELY JACK DIDDLY SQUAT (nothing)

	int crtc_corrected_address_register;
	int crtc_corrected_data_register;

	char low_address; 
	char high_address;

	if(DOUBLE_BUFFER_ENABLED == 1)
	{
		if(CURRENT_PAGE == 0)
		{
			low_address = (char)(PAGE_1_OFFSET & 0x00FF);
			high_address = (char)((PAGE_1_OFFSET & 0xFF00)>>8);

			CURRENT_PAGE = 1;
		}
		else
		{
			low_address = (char)(PAGE_0_OFFSET & 0x00FF);
			high_address = (char)((PAGE_0_OFFSET & 0xFF00)>>8);

			CURRENT_PAGE = 0;
		}

		if(MONOCHROME_MODE == 0)
		{
			crtc_corrected_address_register = CRTC_ADDRESS_REGISTER;
			crtc_corrected_data_register = CRTC_DATA_REGISTER;
		}
		else
		{
			crtc_corrected_address_register = CRTC_ADDRESS_REGISTER_MONO;
			crtc_corrected_data_register = CRTC_DATA_REGISTER_MONO;
		}
		
		_asm	{	
					mov dx, crtc_corrected_address_register
					mov al, CRTC_START_ADDRESS_LOW_INDEX
					xor ah, ah 
					out dx, ax
					mov dx, crtc_corrected_data_register
					xor ah, ah
					mov al, low_address
					out dx, ax

					mov dx, crtc_corrected_address_register
					mov al, CRTC_START_ADDRESS_HIGH_INDEX
					xor ah, ah 
					out dx, ax
					mov dx, crtc_corrected_data_register
					xor ah, ah
					mov al, high_address
					out dx, ax
				}
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

	*(get_drawbuffer() + (x>>3) + y*(SCREEN_RES_X>>3)) &= 0;
}

void draw_line_h(int x1, int x2, int y, unsigned char color)
{
	//Receives the X and Y coordinates of the start and finish of the horizontal line and its color, 
	//and draws it on the screen.

	unsigned char l_bitmask;
	unsigned char r_bitmask;
	unsigned char m_bitmask;
	
	int middle_start;
	int middle_end;

	int i;
	
	//Isnt really nescessary, but just in case SOMEONE sends an inverted line to this.
	if(x2 < x1)
	{
		int temp;

		temp = x2;
		x2 = x1;
		x1 = temp;
	}

	//Figuring out the bitmask of the edges of the line, since they arent nescessarily aligned to a byte
	//I couldve done everything aligned to the nibble and it probably would be slightly faster, but feck it

	l_bitmask = 0xFF >> (x1&7);
	r_bitmask = (0xFF << (7-(x2&7))) & 0xFF;


	//Read the set_pixel comments
	_asm 	{
				mov dx, GFX_ADDRESS_REGISTER
				mov al, GFX_SET_RESET_INDEX
				mov ah, color
				out dx, ax
				mov al, GFX_ENABLE_SET_RESET_INDEX
				mov ah, 0x0F
				out dx, ax
			}

	//In case the X2 and X1 positions are inside the same byte. 
	if(x1>>3 == x2>>3)
	{
		//This is basically the bitmask of the entire line
		m_bitmask = (unsigned char)(l_bitmask & r_bitmask);

		_asm	{
					mov dx, GFX_ADDRESS_REGISTER
					mov al, GFX_BIT_MASK_INDEX
					mov ah, m_bitmask
					out dx, ax
				}

		*(get_drawbuffer() + (x1>>3) + y*(SCREEN_RES_X>>3)) &= 0;
	}	
	else
	{
		//Figuring out the positions of the start and finish of the middle block
		middle_start = x1 + 8 - (x1&7);
		middle_end = x2 - (x2&7); 

		//Drawing the left edge.
		_asm	{
					mov dx, GFX_ADDRESS_REGISTER
					mov al, GFX_BIT_MASK_INDEX
					mov ah, l_bitmask
					out dx, ax
				}

		*(get_drawbuffer() + (x1>>3) + y*(SCREEN_RES_X>>3)) &= 0;

		//Figuring out if there is a middle section.
		if(middle_end - middle_start > 0)
		{
			//Setting registers for fast draw of the middle section
			_asm	{
						mov dx, GFX_ADDRESS_REGISTER
						mov al, GFX_BIT_MASK_INDEX
						mov ah, 0xFF
						out dx, ax
					}	

			//I think I can use a memmory fill here, but I'm not sure, I think i need to do a read,
			//for the latches to work, dunno, will try later.

			for (i = 0; i < ((middle_end - middle_start)>>3); i++)
			{
				*(get_drawbuffer() + ((middle_start>>3)+i) + y*(SCREEN_RES_X>>3)) &= 0;
			}
		}

		//Drawing right edge
		_asm	{
					mov dx, GFX_ADDRESS_REGISTER
					mov al, GFX_BIT_MASK_INDEX
					mov ah, r_bitmask
					out dx, ax
				}	

		*(get_drawbuffer() + (x2>>3) + y*(SCREEN_RES_X>>3)) &= 0;
	}
}

void draw_line_v(int x, int y1, int y2, unsigned char color)
{
	//Draws a vertical column on the x position, going from y1 to y2.
	//Works basically the same as the other functions

	unsigned char bit_mask;
	int i;

	bit_mask = 0x80 >> (x&7);

	if(y2 < y1)
	{
		int temp;

		temp = y2;
		y2 = y1;
		y1 = temp;
	}


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

	for(i = 0; i < (y2-y1); i++)
	{
		*(get_drawbuffer() + (x>>3) + (y1+i)*(SCREEN_RES_X>>3)) &= 0;
	}
}

void draw_rectangle(int x1, int y1, int x2, int y2, unsigned char color)
{
	//Receives the (x1, y1) and (x2, y2) pair of point and fills a rectangle between them, colored with color

	unsigned char l_bitmask;
	unsigned char r_bitmask;
	unsigned char m_bitmask;
	
	int middle_start;
	int middle_end;

	int i, j;
	
	//Isnt really nescessary, but just in case SOMEONE sends an inverted line to this.
	if(x2 < x1)
	{
		int temp;

		temp = x2;
		x2 = x1;
		x1 = temp;
	}

	if(y2 < y1)
	{
		int temp;

		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	//Figuring out the bitmask of the edges of the line, since they arent nescessarily aligned to a byte
	//I couldve done everything aligned to the nibble and it probably would be slightly faster, but feck it

	l_bitmask = 0xFF >> (x1&7);
	r_bitmask = (0xFF << (7-(x2&7))) & 0xFF;


	//Read the set_pixel comments
	_asm 	{
				mov dx, GFX_ADDRESS_REGISTER
				mov al, GFX_SET_RESET_INDEX
				mov ah, color
				out dx, ax
				mov al, GFX_ENABLE_SET_RESET_INDEX
				mov ah, 0x0F
				out dx, ax
			}

	//In case the X2 and X1 positions are inside the same byte. 
	if(x1>>3 == x2>>3)
	{
		//This is basically the bitmask of the entire line
		m_bitmask = (unsigned char)(l_bitmask & r_bitmask);

		_asm	{
					mov dx, GFX_ADDRESS_REGISTER
					mov al, GFX_BIT_MASK_INDEX
					mov ah, m_bitmask
					out dx, ax
				}

			for(i = 0; i < (y2-y1); i++)
		{
			*(get_drawbuffer() + (x1>>3) + (y1+i)*(SCREEN_RES_X>>3)) &= 0;
		}
	}	
	else
	{
		//Figuring out the positions of the start and finish of the middle block
		middle_start = x1 + 8 - (x1&7);
		middle_end = x2 - (x2&7); 

		//Drawing the left edge.
		_asm	{
					mov dx, GFX_ADDRESS_REGISTER
					mov al, GFX_BIT_MASK_INDEX
					mov ah, l_bitmask
					out dx, ax
				}

		for(i = 0; i < (y2-y1); i++)
		{
			*(get_drawbuffer() + (x1>>3) + (y1+i)*(SCREEN_RES_X>>3)) &= 0;
		}

		//Figuring out if there is a middle section.
		if(middle_end - middle_start > 0)
		{
			//Setting registers for fast draw of the middle section
			_asm	{
						mov dx, GFX_ADDRESS_REGISTER
						mov al, GFX_BIT_MASK_INDEX
						mov ah, 0xFF
						out dx, ax
					}	

			//I think I can use a memmory fill here, but I'm not sure, I think i need to do a read,
			//for the latches to work, dunno, will try later.

			for (i = 0; i < ((middle_end - middle_start)>>3); i++)
			{
					for(j = 0; j < (y2-y1); j++)
					{
						*(get_drawbuffer() + ((middle_start>>3)+i) + (y1+j)*(SCREEN_RES_X>>3)) &= 0;
					}
			}
		}

		//Drawing right edge
		_asm	{
					mov dx, GFX_ADDRESS_REGISTER
					mov al, GFX_BIT_MASK_INDEX
					mov ah, r_bitmask
					out dx, ax
				}	

		for(i = 0; i < (y2-y1); i++)
		{
			*(get_drawbuffer() + (x2>>3) + (y1+i)*(SCREEN_RES_X>>3)) &= 0;
		}
	}
}

void fill_screen(unsigned char color)
{
	int screen_size;

	unsigned char far * draw_buffer;

	draw_buffer = get_drawbuffer();

	screen_size = (SCREEN_RES_X/16)*SCREEN_RES_Y;

	//Sets up the registers and makes write operations to the memmory 2 bytes at a time

	//I think theres of way of doing this a bit faster using write mode 2, but I'm not sure,
	//this way isn't doing any OUTs per cycle, and that one isn't either, so the performance
	//should be similar, will do a test later.
	_asm 	{
			mov dx, GFX_ADDRESS_REGISTER
			mov al, GFX_SET_RESET_INDEX
			mov ah, color
			out dx, ax
			mov al, GFX_ENABLE_SET_RESET_INDEX
			mov ah, 0x0F
			out dx, ax
			mov al, GFX_BIT_MASK_INDEX
			mov ah, 0xFF
			out dx, ax
			les di, draw_buffer
			mov al, 0x00
			mov ah, al
			mov cx, screen_size
			rep stosw
		}
}

int fgeti(FILE * file, char separator)
{
	int current_char = 0;
	int current_value = 0;

	int i;

	//Reads an integer value from a file until it hits a predetermined separator.
	//returns the int.

	for(i= 0; i<7; i++)
	{	
		current_char = fgetc(file);
		if(current_char >= 48 && current_char <= 58)
		{
			current_value = current_value*10 + current_char - 48;
			FILE_POSITION += 1;
		}
		else if(current_char == separator)
		{
			FILE_POSITION += 1;
			break;
		}
		else
		{
			set_ega_mode(EGA_TEXT_MODE);
			printf("Got a weird unexpect char at offset: %i", ftell(file));
			exit(EXIT_FAILURE);
		}
	}

	return current_value;
}

void load_font(char * file_location, unsigned char far * address)
{
		//Receives a DOS file location and loads it in the specified memory location

	FILE * file;

	int format_error = 0;

	int size_x = 0;
	int size_y = 0;

	int x, y = 0;
	char c = 0;

	char bitmask = 0;

	char current_pixel = 0;

	int data_offset = 0;

	file = fopen(file_location, "r");

	//Checking if the file was actually loaded and if it is a semi-valid PGM file.
	if(file == NULL)
	{
		set_ega_mode(EGA_TEXT_MODE);
		printf("Null File Pointer: %s", file_location);
		exit(EXIT_FAILURE);
	}

	FILE_POSITION = 0;

	current_pixel = fgetc(file);
	if(current_pixel != 'P')format_error = 1;
	current_pixel = fgetc(file);
	if(current_pixel != '2')format_error = 1;
	current_pixel = fgetc(file);
	if(current_pixel != '\n')format_error = 1;

	FILE_POSITION += 3;

	if(format_error)
	{
		set_ega_mode(EGA_TEXT_MODE);
		printf("PGM Identifier not found, exiting, check your files: %s", file_location);
		exit(EXIT_FAILURE);
	}

	format_error = 0;

	size_x = fgeti(file,  ' ');
	size_y = fgeti(file,  '\n');

	if(size_x != 192)
	{
		format_error = 1;
	}
	if(size_y != 32)
	{
		format_error = 1;
	}

	if(format_error)
	{
		set_ega_mode(EGA_TEXT_MODE);
		printf("Ẁrong file size for a font file, should be 192 x 32. File: %s", file_location);
		exit(EXIT_FAILURE);
	}

	format_error = 0;

	fgeti(file, '\n');

	for(p = 0; p < 4; p++)
	{
		for(y = 0; y < 8; y++)
		{
			for(x = 0; x < (192>>3); x++)
			{
				
			}
		}
	}
}

void load_pgm(char * file_location, unsigned char far * address)
{
	//Receives a DOS file location and loads it in the specified memory location

	FILE * file;

	int format_error = 0;

	int size_x = 0;
	int size_y = 0;

	int x, y = 0;
	char c = 0;

	char bitmask = 0;

	char current_pixel = 0;

	int data_offset = 0;

	file = fopen(file_location, "r");

	//Checking if the file was actually loaded and if it is a semi-valid PGM file.
	if(file == NULL)
	{
		set_ega_mode(EGA_TEXT_MODE);
		printf("Null File Pointer: %s", file_location);
		exit(EXIT_FAILURE);
	}

	FILE_POSITION = 0;

	current_pixel = fgetc(file);
	if(current_pixel != 'P')format_error = 1;
	current_pixel = fgetc(file);
	if(current_pixel != '2')format_error = 1;
	current_pixel = fgetc(file);
	if(current_pixel != '\n')format_error = 1;

	FILE_POSITION += 3;

	if(format_error)
	{
		set_ega_mode(EGA_TEXT_MODE);
		printf("PGM Identifier not found, exiting, check your files: %s", file_location);
		exit(EXIT_FAILURE);
	}

	format_error = 0;

	size_x = fgeti(file,  ' ');
	size_y = fgeti(file,  '\n');

	fgeti(file, '\n');

	//Sets the set/reset values to 0x0f, the actual choosing of the color to write is done later with
	//a set to the Map Mask register, so this is kept all 1s, as to not interfere with that.

	_asm 	{
				mov dx, GFX_ADDRESS_REGISTER
				mov al, GFX_SET_RESET_INDEX
				mov ah, 0x0F
				out dx, ax
				mov al, GFX_ENABLE_SET_RESET_INDEX
				mov ah, 0x0F
				out dx, ax
				}

	//Saves the location of the data in the file, so we can go back and set another color
	data_offset = FILE_POSITION;

	//Saves image size at the 4 leading bytes of the image being saved to memory, so that other
	//functions can quickly reference its size, without having to receive it as a parameter.
	if(WRITE_IMAGE_SIZE == 1)
	{
		*((int *)address) = size_x;
		*((int *)address + 2) = size_y;
	}

	//Doing each color bit at a time, so 0001, 0010 0100 and 1000
	for(c = 1; c <= 8; c *= 2)
	{	
		fseek(file, data_offset, SEEK_SET);
		
		//Sets the Map Mask to the current color bit being checked, as to not rewrite already
		//written bits.
		_asm 	{
				mov dx, SEQUENCER_ADDRESS_REGISTER
				mov al, SEQUENCER_MAP_MASK_INDEX
				mov ah, c
				out dx, ax
				}
		
		for(x = 0; x < size_x; x++)
		{

			//Bitmasking to not mess with stuff already written
			bitmask = 0x80 >> (x & 7);

				_asm 	{
						mov dx, GFX_ADDRESS_REGISTER
						mov al, GFX_BIT_MASK_INDEX
						mov ah, bitmask
						out dx, ax
						}

			//Doing column by column to save OUT operations, they are really slow, and doing one every
			//pixel would be horrible for loading performance.

			//This does mean, however, that the PGM file must be made transposed.
			for(y = 0; y < size_y; y++)
			{
				
				current_pixel =  fgeti(file, '\n');

				if(current_pixel & c)
				{
					*(address + (x>>3) + y*(size_x>>3) + (4*WRITE_IMAGE_SIZE)) &= 0;
				}
			}
		}
	}

	FILE_POSITION = 0;

	//Always close yo files yo.
	fclose(file);
}

void transfer_mem_to_display(unsigned char far * origin, int x, int y)
{	
	//Receives a pointer to an image loaded in memory with load_pgm and a position on screen, and
	//copies it (very quickly!), to that position on screen, taking into consideration the x and y
	//size of the image, so it appears correctly.

	char current_pixel;

	int image_x;
	int image_y;

	int i, j = 0;

	current_pixel = 0;

	image_x = *((int *)origin);
	image_y = *((int *)origin+2);
	
	_asm 	{
				//Sets the sequencer to 0x0F, so that every bit plane is being written to.
				mov dx, SEQUENCER_ADDRESS_REGISTER
				mov al, SEQUENCER_MAP_MASK_INDEX
				mov ah, 0x0F
				out dx, ax
				//Changes to mode 1, so that the transfer can happen 32 bits at a time, copying
				//a byte from every plane at the same time.
				mov dx, GFX_ADDRESS_REGISTER
				mov al, GFX_MODE_REGISTER_INDEX
				mov ah, 0x01
				out dx, ax

				//Sets the bit mask to 0xFF so that the entire image is copied.
				//Needs to change this so images 
				mov al, GFX_BIT_MASK_INDEX
				mov ah, 0xFF
				out dx, ax
			}

	for (j = 0; j < image_y; j++)
	{
		for(i=0; i < (image_x>>3); i++)
		{
			//This gives a warning, because current_pixel is assigned but never used, but it's needed.

			//Does a read operation, setting the internal EGA latches with the contents of every bitplane
			current_pixel = *(origin + 4 + i + j*(image_x>>3));

			//Does a write operation, writing the internal EGA latches to memory.
			*(get_drawbuffer()+((x>>3)+i) + (y+j)*(SCREEN_RES_X>>3)) = 0x0;
		}
	}

	//Reset the mode register to default mode.
	_asm 	{
			mov dx, GFX_ADDRESS_REGISTER
			mov al, GFX_MODE_REGISTER_INDEX
			mov ah, 0x00
			out dx, ax
			}
}

void draw_char(int xc, int yc, char c, char color, int transparency)
{
	unsigned char far *address;
	unsigned char far *work_char;

	char char_offset;
	char char_line;

	char bitmask;

	int x, y;

	address = get_drawbuffer() + (xc>>3) + yc*(SCREEN_RES_X>>3);
	work_char = CHAR_SET_ROM + c*8;

	char_offset = xc&7;

	bitmask = 0xFF >> char_offset;

	//Needs to or it with 0x10 so that it sets up the data rotate register with the OR function.
	if(transparency)
		char_offset |= 0x10;
	
	_asm 	{
				mov dx, GFX_ADDRESS_REGISTER
				mov al, GFX_SET_RESET_INDEX
				mov ah, color
				out dx, ax
				mov al, GFX_ENABLE_SET_RESET_INDEX
				mov ah, 0x0F
				out dx, ax
				mov al, GFX_DATA_ROTATE_INDEX
				mov ah, char_offset
				out dx, ax
			}

	for(x = 0; x < 2; x++)
	{
		if(x == 1)
		{
			bitmask ^= 0xFF;
		}

		_asm 	{
				mov dx, GFX_ADDRESS_REGISTER
				mov al, GFX_BIT_MASK_INDEX
				mov ah, bitmask
				out dx, ax
				}

		for (y = 0; y < 8; y++)
		{
			char_line = *(work_char + y);
			getch();
			*(address+x+y*(SCREEN_RES_X>>3)) &= 0;
			
		}
	}
}

int main()
{
	set_ega_mode(EGA_GRAPHICS_MODE);
//	load_pgm("handsome.pgm", IMAGE_STORAGE);
//	transfer_mem_to_display(IMAGE_STORAGE, 0, 0);
	
	draw_char(4,1, '1', 12, 0);

	page_flip();

	getch();
	return 0;
}