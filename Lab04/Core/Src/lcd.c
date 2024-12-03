

#include "lcdFont.h"
#include "lcd.h"
#include "fsmc.h"

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

unsigned char s[50];

_lcd_dev lcddev;

static void LCD_WR_DATA(uint16_t data);
static uint16_t LCD_RD_DATA(void);
static uint32_t mypow(uint8_t m, uint8_t n);

void LCD_WR_REG(uint16_t reg)
{
	LCD->LCD_REG = reg;
}

void LCD_WR_DATA(uint16_t data)
{
	LCD->LCD_RAM = data;
}

uint16_t LCD_RD_DATA(void)
{
	__IO uint16_t ram;
	ram = LCD->LCD_RAM;
	return ram;
}


void lcdSetAddress(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x1 >> 8);
	LCD_WR_DATA(x1 & 0xff);
	LCD_WR_DATA(x2 >> 8);
	LCD_WR_DATA(x2 & 0xff);
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y1 >> 8);
	LCD_WR_DATA(y1 & 0xff);
	LCD_WR_DATA(y2 >> 8);
	LCD_WR_DATA(y2 & 0xff);
	LCD_WR_REG(0x2c);
}

void lcdSetCursor(uint16_t x, uint16_t y)
{
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x >> 8);
	LCD_WR_DATA(x & 0xff);
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y >> 8);
	LCD_WR_DATA(y & 0xff);
}

void lcdSetDisplayOn(void)
{
	LCD_WR_REG(0X29);
}

void lcdSetDisplayOff(void)
{
	LCD_WR_REG(0X28);
}

uint16_t lcdReadPoint(uint16_t x, uint16_t y)
{
	uint16_t r = 0, g = 0, b = 0;
	lcdSetCursor(x, y);
	LCD_WR_REG(0X2E);
	r = LCD_RD_DATA();
	r = LCD_RD_DATA();
	b = LCD_RD_DATA();
	g = r & 0XFF;
	g <<= 8;
	return (((r >> 11) << 11) | ((g >> 10) << 5) | (b >> 11));
}

/**
 * @brief  Fill all pixels with a color
 * @param  color Color to fill the screen
 * @retval None
 */
void lcdClear(uint16_t color)
{
	uint16_t i, j;
	lcdSetAddress(0, 0, lcddev.width - 1, lcddev.height - 1);
	for (i = 0; i < lcddev.width; i++)
	{
		for (j = 0; j < lcddev.height; j++)
		{
			LCD_WR_DATA(color);
		}
	}
}

/**
 * @brief  Fill a group of pixels with a color
 * @param  xsta	Start column
 * @param  ysta	Start row
 * @param  xend	End column
 * @param  yend	End row
 * @param  color Color to fill
 * @retval None
 */
void lcdFill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend,
		uint16_t color)
{
	uint16_t i, j;
	lcdSetAddress(xsta, ysta, xend - 1, yend - 1);
	for (i = ysta; i < yend; i++)
	{
		for (j = xsta; j < xend; j++)
		{
			LCD_WR_DATA(color);
		}
	}
}

/**
 * @brief  Fill 1 pixel with a color
 * @param  x X coordinate
 * @param  y Y coordinate
 * @param  color Color to fill
 * @retval None
 */
void lcdDrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
	lcdSetAddress(x, y, x, y);
	LCD_WR_DATA(color);
}

/**
 * @brief  Draw a line with a color
 * @param  x1 X coordinate of start point
 * @param  y1 Y coordinate of start point
 * @param  x2 X coordinate of end point
 * @param  y2 Y coordinate of end point
 * @param  color Color to fill
 * @retval None
 */
void lcdDrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	uint16_t t;
	int xerr = 0, yerr = 0, delta_x, delta_y, distance;
	int incx, incy, uRow, uCol;
	delta_x = x2 - x1;
	delta_y = y2 - y1;
	uRow = x1;
	uCol = y1;
	if (delta_x > 0)
		incx = 1;
	else if (delta_x == 0)
		incx = 0;
	else
	{
		incx = -1;
		delta_x = -delta_x;
	}
	if (delta_y > 0)
		incy = 1;
	else if (delta_y == 0)
		incy = 0;
	else
	{
		incy = -1;
		delta_y = -delta_y;
	}
	if (delta_x > delta_y)
		distance = delta_x;
	else
		distance = delta_y;
	for (t = 0; t < distance + 1; t++)
	{
		lcdDrawPoint(uRow, uCol, color);
		xerr += delta_x;
		yerr += delta_y;
		if (xerr > distance)
		{
			xerr -= distance;
			uRow += incx;
		}
		if (yerr > distance)
		{
			yerr -= distance;
			uCol += incy;
		}
	}
}

void lcdDrawRect(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
	lcdDrawLine(x1, y1, x2, y1, color);
	lcdDrawLine(x1, y1, x1, y2, color);
	lcdDrawLine(x1, y2, x2, y2, color);
	lcdDrawLine(x2, y1, x2, y2, color);
}

/**
 * @param x horizontal starting position for drawing the character
 * @param y vertical starting position for drawing the character
 * @param character ASCII value of the character to be displayed
 * @param fc foreground color of the character
 * @param bc background color of the character
 * @param sizey height (16, 24, 32) of the character in pixels (sizex is typically half the height)
 * @param mode determine whether the background color is applied
 * (!= 0 only the foreground color pixels, skipping the background)
 */
void lcdShowChar(uint16_t x, uint16_t y, uint8_t character, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
	uint8_t temp, sizex, t, m = 0;
	uint16_t i, TypefaceNum;
	uint16_t x0 = x;
	sizex = sizey / 2;
	TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
	character = character - ' ';
	lcdSetAddress(x, y, x + sizex - 1, y + sizey - 1);
	for (i = 0; i < TypefaceNum; i++)
	{
		if (sizey == 12)
			;
		else if (sizey == 16)
			temp = ascii_1608[character][i];
		else if (sizey == 24)
			temp = ascii_2412[character][i];
		else if (sizey == 32)
			temp = ascii_3216[character][i];
		else
			return;
		for (t = 0; t < 8; t++)
		{
			if (!mode) {
				if (temp & (0x01 << t))
					LCD_WR_DATA(fc);
				else
					LCD_WR_DATA(bc);
				m++;
				if (m % sizex == 0)
				{
					m = 0;
					break;
				}
			}
			else
			{
				if (temp & (0x01 << t))
					lcdDrawPoint(x, y, fc);
				x++;
				if ((x - x0) == sizex)
				{
					x = x0;
					y++;
					break;
				}
			}
		}
	}
}

uint32_t mypow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	while (n--)
		result *= m;
	return result;
}

/**
 * @param x, y coordinate that the number start
 * @param num number to be displayed on the LCD
 * @param len The length of the number to be displayed
 * @param fc color of the digits
 * @param bc background color used behind the digits
 * @param sizey height (16, 24, 32) font size used for displaying the digits
 * @param mode determine whether the background color is applied
 * (!= 0 only the foreground color pixels, skipping the background)
 */
void lcdShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
		uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
	uint8_t t, temp;
	uint8_t enshow = 0;
	uint8_t sizex = sizey / 2;
	for (t = 0; t < len; t++) {
		temp = (num / mypow(10, len - t - 1)) % 10;
		if (enshow == 0 && t < (len - 1)) {
			if (temp == 0) {
				lcdShowChar(x + t * sizex, y, '0', fc, bc, sizey, mode); // modify which symbol to display in blank space
				continue;
			}
			else
				enshow = 1;
		}
		lcdShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, mode);
	}
}
void lcdShowIntNumCenter(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
		uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    uint8_t sizex = sizey / 2;
    uint16_t total_width = len * sizex;
    uint16_t start_x = x - (total_width / 2);
    uint16_t start_y = y - (sizey / 2);

    for (t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                lcdShowChar(start_x + t * sizex, start_y, '0', fc, bc, sizey, mode); // Display '0' instead of blank space
                continue;
            }
            else
                enshow = 1;
        }
        lcdShowChar(start_x + t * sizex, start_y, temp + 48, fc, bc, sizey, mode);
    }
}

void lcdShowFloatNum(uint16_t x, uint16_t y, float num, uint8_t len,
		uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
	uint8_t t, temp, sizex;
	uint16_t num1;
	sizex = sizey / 2;
	num1 = num * 100;
	for (t = 0; t < len; t++)
	{
		temp = (num1 / mypow(10, len - t - 1)) % 10;
		if (t == (len - 2))
		{
			lcdShowChar(x + (len - 2) * sizex, y, '.', fc, bc, sizey, mode);
			t++;
			len += 1;
		}
		lcdShowChar(x + t * sizex, y, temp + 48, fc, bc, sizey, mode);
	}
}

void lcdShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width,
		const uint8_t pic[])
{
	uint8_t picH, picL;
	uint16_t i, j;
	uint32_t k = 0;
	lcdSetAddress(x, y, x + length - 1, y + width - 1);
	for (i = 0; i < length; i++)
	{
		for (j = 0; j < width; j++)
		{
			picH = pic[k * 2];
			picL = pic[k * 2 + 1];
			LCD_WR_DATA(picH << 8 | picL);
			k++;
		}
	}
}

void lcdSetDirection(uint8_t dir)
{
	if ((dir >> 4) % 4)
	{
		lcddev.width = 320;
		lcddev.height = 240;
	}
	else
	{
		lcddev.width = 240;
		lcddev.height = 320;
	}
}

void initLCD(void)
{
	HAL_GPIO_WritePin(FSMC_RES_GPIO_Port, FSMC_RES_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(FSMC_RES_GPIO_Port, FSMC_RES_Pin, GPIO_PIN_SET);
	HAL_Delay(500);
	lcdSetDirection(DFT_SCAN_DIR);
	LCD_WR_REG(0XD3);
	lcddev.id = LCD_RD_DATA();
	lcddev.id = LCD_RD_DATA();
	lcddev.id = LCD_RD_DATA();
	lcddev.id <<= 8;
	lcddev.id |= LCD_RD_DATA();

	LCD_WR_REG(0xCF);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xC1);
	LCD_WR_DATA(0X30);
	LCD_WR_REG(0xED);
	LCD_WR_DATA(0x64);
	LCD_WR_DATA(0x03);
	LCD_WR_DATA(0X12);
	LCD_WR_DATA(0X81);
	LCD_WR_REG(0xE8);
	LCD_WR_DATA(0x85);
	LCD_WR_DATA(0x10);
	LCD_WR_DATA(0x7A);
	LCD_WR_REG(0xCB);
	LCD_WR_DATA(0x39);
	LCD_WR_DATA(0x2C);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x34);
	LCD_WR_DATA(0x02);
	LCD_WR_REG(0xF7);
	LCD_WR_DATA(0x20);
	LCD_WR_REG(0xEA);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0xC0);    //Power control
	LCD_WR_DATA(0x1B);   //VRH[5:0]
	LCD_WR_REG(0xC1);    //Power control
	LCD_WR_DATA(0x01);   //SAP[2:0];BT[3:0]
	LCD_WR_REG(0xC5);    //VCM control
	LCD_WR_DATA(0x30); 	 //3F
	LCD_WR_DATA(0x30); 	 //3C
	LCD_WR_REG(0xC7);    //VCM control2
	LCD_WR_DATA(0XB7);
	LCD_WR_REG(0x36);    // Memory Access Control

	LCD_WR_DATA(0x08 | DFT_SCAN_DIR);
	LCD_WR_REG(0x3A);
	LCD_WR_DATA(0x55);
	LCD_WR_REG(0xB1);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x1A);
	LCD_WR_REG(0xB6);    // Display Function Control
	LCD_WR_DATA(0x0A);
	LCD_WR_DATA(0xA2);
	LCD_WR_REG(0xF2);    // 3Gamma Function Disable
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0x26);    //Gamma curve selected
	LCD_WR_DATA(0x01);
	LCD_WR_REG(0xE0);    //Set Gamma
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x2A);
	LCD_WR_DATA(0x28);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x0E);
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x54);
	LCD_WR_DATA(0XA9);
	LCD_WR_DATA(0x43);
	LCD_WR_DATA(0x0A);
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0XE1);    //Set Gamma
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x15);
	LCD_WR_DATA(0x17);
	LCD_WR_DATA(0x07);
	LCD_WR_DATA(0x11);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x2B);
	LCD_WR_DATA(0x56);
	LCD_WR_DATA(0x3C);
	LCD_WR_DATA(0x05);
	LCD_WR_DATA(0x10);
	LCD_WR_DATA(0x0F);
	LCD_WR_DATA(0x3F);
	LCD_WR_DATA(0x3F);
	LCD_WR_DATA(0x0F);
	LCD_WR_REG(0x2B);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x01);
	LCD_WR_DATA(0x3f);
	LCD_WR_REG(0x2A);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xef);
	LCD_WR_REG(0x11); // Exit Sleep
	HAL_Delay(120);
	LCD_WR_REG(0x29); // Display on
	HAL_GPIO_WritePin(FSMC_BLK_GPIO_Port, FSMC_BLK_Pin, 1);
}

static void _draw_circle_8(int xc, int yc, int x, int y, uint16_t c)
{
	lcdDrawPoint(xc + x, yc + y, c);

	lcdDrawPoint(xc - x, yc + y, c);

	lcdDrawPoint(xc + x, yc - y, c);

	lcdDrawPoint(xc - x, yc - y, c);

	lcdDrawPoint(xc + y, yc + x, c);

	lcdDrawPoint(xc - y, yc + x, c);

	lcdDrawPoint(xc + y, yc - x, c);

	lcdDrawPoint(xc - y, yc - x, c);
}

/**
 * @param xc, yc Center coordinates of the circle (xc is horizontal direction)
 * @param c Color to be used for drawing the circle.
 * @param r radius of the circle
 * @param Boolean indicating whether to fill the circle (non-zero value) or just draw the outline (zero value).
 */
void lcdDrawCircle(int xc, int yc, uint16_t c, int r, int fill)
{
	int x = 0, y = r, yi, d;

	d = 3 - 2 * r;

	if (fill) {
		while (x <= y)
		{
			for (yi = x; yi <= y; yi++)
				_draw_circle_8(xc, yc, x, yi, c);

			if (d < 0)
			{
				d = d + 4 * x + 6;
			}
			else
			{
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
	else
	{
		while (x <= y)
		{
			_draw_circle_8(xc, yc, x, y, c);
			if (d < 0)
			{
				d = d + 4 * x + 6;
			}
			else
			{
				d = d + 4 * (x - y) + 10;
				y--;
			}
			x++;
		}
	}
}

/**
 * @param x x-coordinate where the string display.
 * @param y y-coordinate where the string display.
 * @param *str Pointer to the string that needs to be displayed
 * @param fc Foreground color of the text (font color)
 * @param bc Background color behind the text
 * @param sizey Height of the characters (can be 16, 24, 32)
 * @param mode determine whether the background color is applied
 * (!= 0 only the foreground color pixels, skipping the background)
 */
void lcdShowString(uint16_t x, uint16_t y, char *str, uint16_t fc, uint16_t bc,
		uint8_t sizey, uint8_t mode)
{
	uint16_t x0 = x;
	uint8_t bHz = 0; // used to handle characters that are more than one byte long (e.g., Chinese characters)
	while (*str != 0)
	{
		if (!bHz)
		{
			if (x > (lcddev.width - sizey / 2) || y > (lcddev.height - sizey))
				return;
			if (*str > 0x80)
				bHz = 1;
			else
			{
				if (*str == 0x0D)
				{
					y += sizey;
					x = x0;
					str++;
				}
				else
				{
					lcdShowChar(x, y, *str, fc, bc, sizey, mode);
					x += sizey / 2;
				}
				str++;
			}
		}
	}
}
/**
 * @param x x-coordinate where the string should be centered.
 * @param y y-coordinate where the string should be centered.
 * @param *str Pointer to the string that needs to be displayed.
 * @param fc Foreground color of the text (font color).
 * @param bc Background color behind the text.
 * @param sizey Height of the characters (can be 16, 24, 32).
 * @param mode Determine whether the background color is applied
 * (!= 0 only the foreground color pixels, skipping the background).
 */
void lcdShowStringCenter(uint16_t x, uint16_t y, char *str, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode) {
    uint8_t sizex = sizey / 2;
    uint16_t str_len = 0;
    char *ptr = str;

    // Calculate the length of the string
    while (*ptr != '\0')
    {
        if (*ptr > 0x80)
        {
            // If the character is a multibyte character (e.g., Chinese character)
            str_len += 2;
            ptr++;
        }
        else
        {
            str_len++;
        }
        ptr++;
    }

    uint16_t total_width = str_len * sizex;
    uint16_t start_x = x - (total_width / 2);
    uint16_t start_y = y - (sizey / 2);

    lcdShowString(start_x, start_y, str, fc, bc, sizey, mode);
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
