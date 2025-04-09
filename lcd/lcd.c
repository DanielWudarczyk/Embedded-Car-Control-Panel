#include <REGX52.H>

// Declaration of external functions
extern void LcdInit();
extern void Lcd_DisplayString (char row, char column, char *string);
extern void Lcd_WriteControl (unsigned char LcdCommand);
extern void LcdKlimaOFF();
extern void LcdKlimaON();
extern void LcdTRC();

unsigned char data Var1, Var2, Var3;
unsigned char str[6];
bit view = 0, air_cond = 0, mode = 0, enable = 1;
unsigned char timer = 0;
unsigned char level = 3, trc = 0, hold_counter = 0;

void Init(void)
{
    // Initialize UART communication
    P3_4 = 0;
    SCON = 0x50;
    T2CON = 0x30;
    TH2 = RCAP2H = 0xFF;
    TL2 = RCAP2L = 0xDC;
    ES = 1;    
    EA = 1;    
    TR2 = 1;  
    
    // Timer 0 settings
    TMOD = 0x10;
    ET0 = 1;
    TR0 = 1;
}

// Helper function to convert char to string
void charToStr(unsigned char num, char *str) {
    char temp[6];
    char i = 0, j = 0;

    do {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

// Timer 0 interrupt handler
void ISR_T0() interrupt 1
{
    if (enable == 1)
    {
        if (P3_3 == 0 && hold_counter < 8)
        {
            hold_counter++;
        }
        if (hold_counter > 7)   // Button next to LCD held long enough
        {
            if (view == 0)   // If the air conditioning view was active
            {
                view = 1;  
                LcdTRC();   // Change to traction control view
                
                // Display current traction control mode
                if (trc == 0)
                {
                    Lcd_DisplayString(2, 1, "normal mode");   // Display mode on the screen
                    Lcd_WriteControl(0x0C);   // Remove the blinking cursor
                }
                else if (trc == 1)
                {
                    Lcd_DisplayString(2, 1, "sport mode ");
                    Lcd_WriteControl(0x0C);
                }
                else
                {
                    Lcd_DisplayString(2, 1, "off         ");
                    Lcd_WriteControl(0x0C);
                }
            }
            else   // Change to air conditioning view
            {
                view = 0;
                if (air_cond == 1)   // If air conditioning is on
                {
                    // Display air conditioning screen
                    LcdKlimaON();
                    charToStr(level, str);
                    Lcd_DisplayString(4, 14, str);
                    Lcd_WriteControl(0x0C);
                    if (mode == 0)
                    {
                        Lcd_DisplayString(3, 1, "heating");
                        Lcd_WriteControl(0x0C);
                    }
                    else
                    {
                        Lcd_DisplayString(3, 1, "cooling");
                        Lcd_WriteControl(0x0C);
                    }
                }
                else   // If air conditioning is off
                {
                    LcdKlimaOFF();
                }
            }
            enable = 0;   // With a long press, we don't constantly repeat the screen setting
        }
    }
    else if (P3_3 == 1)   // If button released
    {
        hold_counter = 0;
        enable = 1;
    }
}

// Handling incoming signals
void ISR_Serial(void) interrupt 4    
{
    if (TI == 1) { TI = 0; }
    if (RI == 1)
    {
        RI = 0;
        
        if (SBUF == '6')   // Turn on / off air conditioning
        {
            if (air_cond == 0)   // If it was off
            {
                air_cond = 1;   // Turn it on
                if (view == 0)   // If air conditioning view is active
                {
                    // Display parameters on the screen
                    LcdKlimaON();
                    charToStr(level, str);
                    Lcd_DisplayString(4, 14, str);
                    Lcd_WriteControl(0x0C);
                    if (mode == 0)
                    {
                        Lcd_DisplayString(3, 1, "heating");
                        Lcd_WriteControl(0x0C);
                    }
                    else
                    {
                        Lcd_DisplayString(3, 1, "cooling");
                        Lcd_WriteControl(0x0C);
                    }
                }
            }
            else   // If air conditioning was on
            {
                air_cond = 0;   // Turn it off
                if (view == 0)
                {
                    LcdKlimaOFF();   // Update the screen
                }
            }
        }
        else if (SBUF == '7')   // Change air conditioning mode
        {
            if (mode == 0)
            {
                mode = 1;   // Switch to cooling mode
                if (view == 0)
                {
                    // Update the screen
                    Lcd_DisplayString(3, 1, "cooling");
                    Lcd_WriteControl(0x0C);
                }
            }
            else
            {
                mode = 0;   // Switch to heating mode
                if (view == 0)
                {
                    // Update the screen
                    Lcd_DisplayString(3, 1, "heating");
                    Lcd_WriteControl(0x0C);
                }
            }
        }
        else if (SBUF == '8')   // Decrease blower speed
        {
            if (level > 1)
            {
                level--;
                if (view == 0 && air_cond == 1)
                {
                    // Update the screen
                    charToStr(level, str);
                    Lcd_DisplayString(4, 14, str);
                    Lcd_WriteControl(0x0C);
                }
            }
        }
        else if (SBUF == '9')   // Increase blower speed
        {
            if (level < 6)
            {
                level++;
                if (view == 0 && air_cond == 1)
                {
                    // Update the screen
                    charToStr(level, str);
                    Lcd_DisplayString(4, 14, str);
                    Lcd_WriteControl(0x0C);
                }
            }
        }
        // Traction control mode changes
        else if (SBUF == 'a')
        {
            trc = 0;
            if (view == 1)   // If traction control view is active
            {
                // Update the screen
                Lcd_DisplayString(2, 1, "normal mode");
                Lcd_WriteControl(0x0C);
            }
        }
        else if (SBUF == 'b')
        {
            trc = 1;
            if (view == 1)
            {
                // Update the screen
                Lcd_DisplayString(2, 1, "sport mode ");
                Lcd_WriteControl(0x0C);
            }
        }
        else if (SBUF == 'c')
        {
            trc = 2;
            if (view == 1)
            {
                // Update the screen
                Lcd_DisplayString(2, 1, "off         ");
                Lcd_WriteControl(0x0C);
            }
        }
    }
} 

void main(void)
{
    LcdInit();   // Initialize LCD display registers
    Init();
    LcdKlimaOFF();
    while (1) {;}
}
