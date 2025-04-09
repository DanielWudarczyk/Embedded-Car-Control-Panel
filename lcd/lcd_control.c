// Main author of this file: Dr. Eng. Krzysztof Murawski

#define Test 0  // 1 - for testing
                // 0 - normal operation
// Definitions for hardware resources
sfr  P2    = 0xA0;
sbit P2_6  = P2^6;
sbit P2_5  = P2^5;
sbit P2_4  = P2^4;
sbit P2_3  = P2^3;
sbit P2_2  = P2^2;
sbit P2_1  = P2^1;
sbit P2_0  = P2^0;

// General definitions
#define False    			0  
#define True     			1

// LCD display connection definitions
#define LCD_RS    P2_4         /* p1.4 LCD Register Select line           */
#define LCD_RW    P2_5         /* p1.5 LCD Read / Write line              */
#define LCD_E     P2_6         /* p1.6 LCD Enable line                    */
#define LCD_DB4   P2_0         /* high nibble of port 1 is used for data  */
#define LCD_DB5   P2_1         /* high nibble of port 1 is used for data  */
#define LCD_DB6   P2_2         /* high nibble of port 1 is used for data  */
#define LCD_DB7   P2_3         /* high nibble of port 1 is used for data  */

// LCD display command definitions
#define LCD_CONFIG		      0x28
#define LCD_CLEAR		        0x01
#define LCD_HOME		        0x02
#define LCD_ENTRY_MODE		  0x06
#define LCD_DISPLAY_OFF		  0x08
#define LCD_CURSOR_ON 		  0x0A
#define LCD_DISPLAY_ON		  0x0C
#define LCD_CURSOR_BLINK	  0x0D
#define LCD_CURSOR_LINE		  0x0E
#define LCD_CURSOR_COM		  0x0F
#define LCD_CURSOR_LEFT		  0x10
#define LCD_CURSOR_RIGHT	  0x14
#define LCD_SHIFT_LEFT		  0x18
#define LCD_SHIFT_RIGHT		  0x1C
#define LCD_SET_CGRAM_ADDR	0x40
#define LCD_SET_DDRAM_ADDR	0x80

// Screen definitions
static unsigned char code KlimaOFF[] = "AIR CONDITIONING"
                                       "      OFF       "
																			 "                "
																			 "                ";
																			
static unsigned char code KlimaON[] =  "AIR CONDITIONING"
                                       "       ON       "
																			 "        mode    "
																			 "blower power    ";
																			
static unsigned char code TRC[] =  "TRACTION CONTROL"
                                   "                "
																	 "                "
																	 "                ";
																			
static unsigned char code ClearScrean[] =  	"                "
																						"                "
																						"                "
																						"                ";
																						

// Imported procedures and functions
extern w1ms();
extern w5mS();
extern w50mS();

// Sending a command to the LCD display. 4-bit data bus
void Lcd_WriteControl (unsigned char LcdCommand)
 {
   unsigned char Lcd_Comm = 0;
   static bit LCDReady;
    
   LCD_RS = False;
   LCD_RW = False;

   Lcd_Comm = LcdCommand >> 4;
   P2 &= 0xF0;
   P2 |= Lcd_Comm;

   LCD_E = True;
   LCD_E = False;

   Lcd_Comm = LcdCommand & 0x0F;  
   P2 &=0xF0;      // P2 = P2 & 0xF0
   P2 |= Lcd_Comm;

   LCD_E = True;
   LCD_E = False;

   P2 |= 0x0F;     // P2 = P2 | 0x0F

   LCD_RW = True;
   LCD_RS = False;
   
   if (Test == 0)
    {
      LCDReady = 1;
      while (LCDReady == 1)
       {
         LCD_E = True;
         LCDReady = LCD_DB7;
         LCD_E	= False;
         LCD_E = True;
         LCD_E	= False;
       }
    }          
 }

// Sending data to the LCD display. 4-bit data bus
static void Lcd_WriteData (unsigned char LcdData)
 {
   unsigned char Lcd_Data = 0;
   static bit LCDReady;
 
   LCD_RS = True;
   LCD_RW = False;

   Lcd_Data = LcdData >> 4;
   P2 &= 0xF0;
   P2 |= Lcd_Data;

   LCD_E = True;
   LCD_E = False;

   Lcd_Data = LcdData & 0x0F;  
   P2 &=0xF0;   
   P2 |= Lcd_Data;

   LCD_E = True;
   LCD_E = False;

   P2 |= 0x0F;

   LCD_RW = True;
   LCD_RS = False;
   
   if (Test == 0)
    {
      LCDReady = 1;
      while (LCDReady == 1)
       {
         LCD_E = True;
         LCDReady = LCD_DB7;
         LCD_E	= False;
   	     LCD_E	= True;
   	     LCD_E	= False;
       }
    }             
 }

// Displaying a character at the cursor position
void Lcd_DisplayCharacter (char a_char)
 {
   Lcd_WriteData(a_char);
 }

/* Set the cursor to the specified row and column.
*     1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16
*    ----------------------------------------------------------------
* 1 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
* 2 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
* 3 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
* 4 |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
*/
void Lcd_Cursor (char row, char column)
 {
   if (row == 1) Lcd_WriteControl (0x80 + column - 1);
   if (row == 2) Lcd_WriteControl (0xc0 + column - 1);
   if (row == 3) Lcd_WriteControl (0x90 + column - 1);
   if (row == 4) Lcd_WriteControl (0xd0 + column - 1);
 }

// Display a string in the specified row (without wrapping)
void Lcd_DisplayRow (char row, char *string)
 {
   char i;
   Lcd_Cursor (row, 1);
   for (i=0; i<16; i++) Lcd_DisplayCharacter (*string++);
 }

void Lcd_DisplayScreen (char *ptr)
 {
   Lcd_DisplayRow(1,ptr +  0);
   Lcd_DisplayRow(2,ptr + 16);
   Lcd_DisplayRow(3,ptr + 32);
   Lcd_DisplayRow(4,ptr + 48);
 }

// Clear the screen
/*void LcdClear(void)
 {
	 Lcd_DisplayScreen(ClearScrean);
 }*/

// Screen for air conditioning off
void LcdKlimaOFF(void)
{
  Lcd_DisplayScreen(KlimaOFF);        
}

// Screen for air conditioning on
void LcdKlimaON(void)
{
  Lcd_DisplayScreen(KlimaON);        
}

// Traction control screen
void LcdTRC(void)
{
  Lcd_DisplayScreen(TRC);        
}

// Display a string from the specified column and row
void Lcd_DisplayString (char row, char column, char *string)
 {
   Lcd_Cursor (row, column);
   while (*string) Lcd_DisplayCharacter (*string++);
 }

// LCD initialization; 4 rows, 4-bit data bus
void LcdInit(void)
 {
   w50mS();
   P2 = 0x83;
   LCD_E   = True;
     w1ms();
   LCD_E   = False;
   w5mS();
   LCD_E   = True;
     w1ms();
   LCD_E   = False;
   w1ms();
   LCD_E   = True;
     w1ms();
   LCD_E   = False;
   w1ms();
   LCD_DB4 = False;
   LCD_E   = True;
     w1ms();
   LCD_E   = False;
   w1ms();
   Lcd_WriteControl(LCD_CONFIG);     
   Lcd_WriteControl(LCD_CLEAR);      
   Lcd_WriteControl(LCD_DISPLAY_OFF);
   Lcd_WriteControl(LCD_DISPLAY_ON); 
   Lcd_WriteControl(LCD_ENTRY_MODE); 
   Lcd_WriteControl(LCD_CURSOR_COM); 
   Lcd_WriteControl(LCD_CLEAR);      
 }
