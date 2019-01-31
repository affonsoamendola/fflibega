//Copyright Affonso Amendola 2019
//Distributed under GPLv3, check the LICENSE file for some great licensing.
//
//libEGA
//---------------------------------------------

#ifndef LIBEGA_H
#define LIBEGA_H

#define MISCELLANEOUS_OUTPUT_REGISTER			0x3C2
#define FEATURE_CONTROL_REGISTER 				0x3DA
#define FEATURE_CONTROL_REGISTER_MONO			0x3BA
#define INPUT_STATUS_REGISTER_0					0x3C2
#define	INPUT_STATUS_REGISTER_1					0x3D2
#define INPUT_STATUS_REGISTER_1_MONO			0x3B2

#define VBLANK_INTERRUPT 						0x0A
#define PIC_ICR									0x20
#define PIC_IMR									0x21

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

void set_write_image_size(int value);

void set_ega_mode(int mode);

unsigned char far * get_framebuffer();
unsigned char far * get_drawbuffer();
void page_flip();

void set_pixel(int x, int y, unsigned char color);
void draw_line_h(int x1, int x2, int y, unsigned char color);
void draw_line_v(int x, int y1, int y2, unsigned char color);
void draw_rectangle(int x1, int y1, int x2, int y2, unsigned char color);
void fill_screen(unsigned char color);

int fgeti(FILE * file, char separator);

void load_font(char * file_location, unsigned char far * address);
void load_pgm(char * file_location, unsigned char far * address);

void transfer_mem_to_display(unsigned char far * origin, int x, int y);

void transfer_tile_to_display(	unsigned char far * origin,
							 	int x, int y, 
								int tile_x, int tile_y, 
								int tile_size_x, int tile_size_y);

void transfer_mem_to_dest(	unsigned far * origin, unsigned far * destination, 
							int skip_bytes, int bytes);


void draw_char(int xc, int yc, char color, char c);
void draw_string(int x, int y, char color, char * string);

#endif //LIBEGA_H