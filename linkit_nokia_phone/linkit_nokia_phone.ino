#include <LGSM.h>

// PIN Number
#define PINNUMBER ""

char numtel[20];           // buffer for the incoming call


#include <fontALL.h>
#define PIN_SCE		13
#define PIN_RESET	12
#define PIN_DC		11
#define PIN_SDIN	10
#define PIN_SCLK	9

#define LCD_CMD		0 
#define LCD_C		LOW
#define LCD_D		HIGH

#define LCD_X		84
#define LCD_Y		48

unsigned char display_buffer[505];
unsigned char display_clear[505];
unsigned char display_fill[505];
#define FILLARRAY(a,n) a[0]=n, memcpy( ((char*)a)+sizeof(a[0]), a, sizeof(a)-sizeof(a[0]) );

int font_width;
int font_height;
unsigned long timer = 0;

#define ALIGN_LEFT		0
#define ALIGN_CENTER	1
#define ALIGN_RIGHT		2

#define VALIGN_TOP		0
#define VALIGN_MIDDLE	1
#define VALIGN_BOTTOM	2

const unsigned char* font;

#define PIN_BUTTON		8

struct _rect {
	int x;
	int y;
	int width;
	int height;
	int x0;
	int y0;
	int x1;
	int y1;
	int color;
	int fill;
	float radius;
	float speed;
	float angle;
};
_rect rect;

struct _typeset {
	char *characters;
	int spacing;
	int size_w;
	int size_h;
	int align_x;
	int align_y;
};
_typeset typeset;

struct _align {
	int left; //1 : 0;
	int center : 1;
	int right : 2;
	int top : 3;
	int middle : 4;
	int bottom : 5;
};
_align align;



void lcd_invert() {
	for (int i = 0; i < 504; i++) {
		display_buffer[i] = ~display_buffer[i];
	}
}

void lcd_clear(int color) {
	if (color == 0) {
		memset(display_buffer, 0, 504);
//		FILLARRAY(display_buffer, 0);
	} else {
		FILLARRAY(display_buffer, 255);
		//memset(display_buffer, 255, 504);
	}
}

void lcd_bitmap(const unsigned char* bmap, int x, int y) {
	int bitmap_width = pgm_read_byte(bmap + 0);
	int bitmap_height = pgm_read_byte(bmap + 1);
	int px;
	int py;

	for (int i = 0; i < 404; i++) {
		display_buffer[i] = pgm_read_byte(bmap + 3 + i);
	}
}

void lcd_buffer() {
	int x = 0;
	int y = 0;

	for (int i = 0; i < 504; i++) {
		lcd_write(1, display_buffer[i]);
		x++;
		if (x == LCD_X) {
			x = 0;
			y++;
		}
	}
}

void set_pixel(int x, int y, int color) {
	if (x < 0 || x > LCD_X - 1 || y < 0 || y > LCD_Y - 1) return;
	byte off_y = y % 8;
	int id = ((y / 8) * LCD_X) + x;

	//if (id > -1 && id < 505) {
		if (color == 0) {
			bitClear(display_buffer[id], off_y);
		} else {
			bitSet(display_buffer[id], off_y);
		}
	//}
}


void set_font(const unsigned char* font_temp) {
	font = font_temp;
	font_width = pgm_read_byte(font + 0) - 0;
	font_height = pgm_read_byte(font + 1);
}

void lcd_caption(String caption) {
	char char_temp[caption.length() + 1];
	caption.toCharArray(char_temp, caption.length() + 1);

	typeset.characters = char_temp;
	lcd_word();
}

void lcd_word() {
	int px;
	int py;
	int pos_x;
	String count_temp = "";
	count_temp += typeset.characters;
	int char_count = count_temp.length();
	int word_width = char_count * ((font_width + typeset.spacing) * typeset.size_w);

	for (int i = 0; i < *typeset.characters; i++) {
		pos_x = i * (font_width + typeset.spacing) * typeset.size_w;
		switch (typeset.align_x) {
			case 1: // center
				px = rect.x + (rect.width / 2) - (word_width / 2) + pos_x;
				break;
			case 2: // right
				px = rect.x + rect.width - word_width + pos_x;
				break;
//			case 0: // left
			default:
				px = rect.x + pos_x;
//				break;
		}
		switch (typeset.align_y) {
			case 4: // middle
				py = rect.y + (rect.height / 2) - ((font_height  / 2) * typeset.size_h);
				break;
			case 5: // bottom
				py = rect.height - rect.y - (font_height * typeset.size_h);
				break;
//			case 3: // top
			default:
				py = rect.y;
//				break;
		}
		lcd_character(*typeset.characters++, px, py, rect.color, typeset.size_w, typeset.size_h);
	}
}

void lcd_character(char character, int x, int y, int color, int size_w, int size_h) {	
	int px;
	int py;
	int tx;
	int ty;
	int char_id;
	unsigned int temp_buff;
	unsigned int font_buff;
	int bit_temp;
	int sx = 0;
	int sy = 0;

	if (font_width > 8) {
		bit_temp = 16;
	} else {
		bit_temp = 8;
	}
	char_id = character - 32;
	if (char_id > 0 && char_id < 96) {
		for (py = 0; py < bit_temp; py++) {
			font_buff = pgm_read_byte(font + 3 + py + (bit_temp * char_id));
			for (px = 0; px < bit_temp; px++) {
				temp_buff = bitRead(font_buff, bit_temp - px - 1);
				tx = (x - (size_w * size_w)) + ((px + size_w) * size_w);
				ty = (y - (size_h * size_h)) + ((py + size_h) * size_h);
				if (temp_buff == 1) {
					for (sy = 0; sy < size_h; sy++) {
						for (sx = 0; sx < size_w; sx++) {
							set_pixel(tx + sx, ty + sy, color);
						}
					}
				}
			}
		}
	}
}



void draw_circle() {
	int f = 1 - rect.radius;
	int ddF_x = 1;
	int ddF_y = -2 * rect.radius;
	int x = 0;
	int y = rect.radius;
	_rect rect_temp;

	rect_temp = rect;
	uint8_t pyy = y, pyx = x;
	if (rect.fill > 0) {
		rect.y0 = rect_temp.y;
		rect.x0 = rect_temp.x - rect_temp.radius;
		rect.x1 = rect_temp.x + rect_temp.radius;
		draw_row();
	}
	rect = rect_temp;
	set_pixel(rect.x, rect.y + rect.radius, rect.color);
	set_pixel(rect.x, rect.y - rect.radius, rect.color);
	set_pixel(rect.x + rect.radius, rect.y, rect.color);
	set_pixel(rect.x - rect.radius, rect.y, rect.color);
	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;	
		if (rect.fill > 0) {
			rect_temp = rect;
			if (pyy != y) {
				rect.x0 = rect.x - x;
				rect.x1 = rect.x + x;
				rect.y0 = rect.y + y;
				draw_row();
				rect.x0 = rect.x - x;
				rect.x1 = rect.x + x;
				rect.y0 = rect.y - y;
				draw_row();
			}
			if (pyx != x && x != y) {
				rect.x0 = rect.x - y;
				rect.x1 = rect.x + y;
				rect.y0 = rect.y + x;
				draw_row();
				rect.x0 = rect.x - y;
				rect.x1 = rect.x + y;
				rect.y0 = rect.y - x;
				draw_row();
			}
			pyy = y;
			pyx = x;
			rect = rect_temp;
		}
		set_pixel(rect.x + x, rect.y + y, rect.color);
		set_pixel(rect.x - x, rect.y + y, rect.color);
		set_pixel(rect.x + x, rect.y - y, rect.color);
		set_pixel(rect.x - x, rect.y - y, rect.color);
		set_pixel(rect.x + y, rect.y + x, rect.color);
		set_pixel(rect.x - y, rect.y + x, rect.color);
		set_pixel(rect.x + y, rect.y - x, rect.color);
		set_pixel(rect.x - y, rect.y - x, rect.color);
	}
}

void draw_line() {
	//if (x0 > display.hres*8 || y0 > display.vres || x1 > display.hres*8 || y1 > display.vres)
	//	return;
	//if (x0 == x1)
	//	draw_column(x0,y0,y1,c);
	//else if (y0 == y1)
	//	draw_row(y0,x0,x1,c);
	//else {
		int e;
		signed int dx,dy,j, temp;
		signed char s1,s2, xchange;
		signed int x,y;

		x = rect.x0;
		y = rect.y0;
		if (rect.x1 < rect.x0) {
			dx = rect.x0 - rect.x1;
			s1 = -1;
		}
		else if (rect.x1 == rect.x0) {
			dx = 0;
			s1 = 0;
		}
		else {
			dx = rect.x1 - rect.x0;
			s1 = 1;
		}
		if (rect.y1 < rect.y0) {
			dy = rect.y0 - rect.y1;
			s2 = -1;
		}
		else if (rect.y1 == rect.y0) {
			dy = 0;
			s2 = 0;
		}
		else {
			dy = rect.y1 - rect.y0;
			s2 = 1;
		}
		xchange = 0;   
		if (dy>dx) {
			temp = dx;
			dx = dy;
			dy = temp;
			xchange = 1;
		} 
		e = ((int)dy<<1) - dx;  
		for (j=0; j<=dx; j++) {
			set_pixel(x, y, rect.color);
			if (e>=0) {
				if (xchange==1) x = x + s1;
				else y = y + s2;
				e = e - ((int)dx<<1);
			}
			if (xchange==1)
				y = y + s2;
			else
				x = x + s1;
			e = e + ((int)dy<<1);
		}
//	}
}

void draw_row() {
	for (int x = rect.x0; x < rect.x1; x++) {
		set_pixel(x, rect.y0, rect.color);
	}
}

void draw_coumn() {
	for (int y = rect.y0; y < rect.y1; y++) {
		set_pixel(rect.x0, y, rect.color);
	}	
}

void draw_rect() {
	_rect rect_temp;

	if (rect.fill > 0) {
		rect_temp = rect;
		for (int y = rect.y; y < rect.y + rect.height; y++) {
			rect.y0 = y;
			rect.x0 = rect.x;
			rect.x1 = rect.x + rect.width;
			draw_row();
		}		
		rect = rect_temp;
	}
	rect.x0 = rect.x;
	rect.y0 = rect.y;
	rect.x1 = rect.x + rect.width;
	rect.y1 = rect.y;
	draw_line();

	rect.x0 = rect.x;
	rect.y0 = rect.y;
	rect.x1 = rect.x;
	rect.y1 = rect.y + rect.height;
	draw_line();

	rect.x0 = rect.x + rect.width;
	rect.y0 = rect.y;
	rect.x1 = rect.x + rect.width;
	rect.y1 = rect.y + rect.height;
	draw_line();

	rect.x0 = rect.x;
	rect.y0 = rect.y + rect.height;
	rect.x1 = rect.x + rect.width;
	rect.y1 = rect.y + rect.height;
	draw_line();
}


void lcd_write(byte dc, byte data) {
	digitalWrite(PIN_DC, dc);
	digitalWrite(PIN_SCE, LOW);
	shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
//	delay(3);
	digitalWrite(PIN_SCE, HIGH);
}

void init_typeset() {
	set_font(font_tiny);
//	set_font(font_serif);
	typeset.align_x = ALIGN_LEFT;
	typeset.align_y = VALIGN_TOP;
	typeset.size_w = 1;
	typeset.size_h = 2;
	typeset.spacing = 1;
}

void init_lcd() {
	pinMode(PIN_SCE,   OUTPUT);
	pinMode(PIN_RESET, OUTPUT);
	pinMode(PIN_DC,    OUTPUT);
	pinMode(PIN_SDIN,  OUTPUT);
	pinMode(PIN_SCLK,  OUTPUT); 
	digitalWrite(PIN_RESET, LOW);
	digitalWrite(PIN_RESET, HIGH);

	lcd_write(LCD_CMD, 0x21);  // LCD Extended Commands.
	lcd_write(LCD_CMD, 0xB1);  // 0xBf. Set LCD Vop (Contrast). //B1
	lcd_write(LCD_CMD, 0x04);  // Set Temp coefficent. //0x04
	lcd_write(LCD_CMD, 0x15);  // LCD bias mode 1:48. //0x13
	lcd_write(LCD_CMD, 0x0C);  // 0x0C LCD in normal mode. 0x0d for inverse
	lcd_write(LCD_C, 0x20);
	lcd_write(LCD_C, 0x0C);

	for (int i = 0; i < 504; i++) {
		display_buffer[i] = 0x00;
		display_clear[i] = 0x00;
		display_fill[i] = 0xff;
	}
	rect.color = 1;
	init_typeset();
}

void lcd_message(String msg) {
	char ctmp[255];
	msg.toCharArray(ctmp, msg.length() + 1);
	typeset.characters = ctmp;
	rect.x = 1;
	rect.y = 1;
	lcd_clear(0);
	lcd_word();
	lcd_buffer();
}

void init_gsm() {
	String msg = "";
	LVoiceCall.hangCall();
	msg += "WAITING CALL";
	lcd_message(msg);
}

void setup() {
	pinMode(PIN_BUTTON, INPUT_PULLUP);
	init_lcd();
	init_gsm();
}

void loop_gsm() {
	String msg = "";
	msg += LVoiceCall.getVoiceCallStatus();
	msg += ":";
	switch (LVoiceCall.getVoiceCallStatus()) {
		case IDLE_CALL: // Nothing is happening
			msg += "IDLE CALL";
			break;
		case CALLING: // This should never happen, as we are not placing a call
			msg += "CALLING";
			break;
		case RECEIVINGCALL: // Yes! Someone is calling us
			msg += "CALL:";
			LVoiceCall.retrieveCallingNumber(numtel, 20);
			msg += numtel;
			LVoiceCall.answerCall();   
			break;
		case TALKING:  // In this case the call would be established
			msg += "TALKING";
			lcd_message(msg);
			msg = "";
			while (digitalRead(PIN_BUTTON) == HIGH){
				delay(100);
			}
			LVoiceCall.hangCall();
			msg += "HANG. WAITING CALL.";
			break;
	}
	lcd_message(msg);
	delay(1000);
}

void loop() {
	loop_gsm();
}
