#include <REGX52.H>

unsigned char xdata Ones _at_ 0xFD00;   // Variable for controlling the ones digit
unsigned char xdata Tens _at_ 0xFE00;   // Variable for controlling the tens digit
unsigned char Digit_codes[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};   // Encoded digits to display on BCD
unsigned char Stations[][10] = {
    {0x40, 0x50, 0x15, 0x71, 0x00, 0x71, 0x15},
    {0x40, 0x79, 0x6D, 0x76, 0x77},
    {0x40, 0x2A, 0x77, 0x2A, 0x77}
}; // RMF FM, ESKA, WAWA

bit radio_on = 0, muted = 0, flashing = 1;
unsigned char station = 0, volume = 15, hold_counter = 0;
unsigned int time = 0;
unsigned char first_letter = 0, second_letter = 1, change = 0, idx = 0;

void Init(void)
{
    // UART initialization
    P3_4 = 0;
    SCON = 0x50;
    T2CON = 0x30;
    TH2 = RCAP2H = 0xFF;
    TL2 = RCAP2L = 0xDC;
    TI = 0;
    RI = 0;
    ES = 1;
    TR2 = 1;
    
    // Set up timers and interrupts
    TMOD = 0x00;
    EA = 1; 
    EX0 = 1; 
    IT0 = 1;
    ET0 = 1; 
    TR0 = 1; 
    ET1 = 1;
    TR1 = 1;
}

void Send(unsigned char Value)
{
    P3_4 = 1;
    TI = 0;
    SBUF = Value;
    while (TI == 0) { ; }
    TI = 0;
    P3_4 = 0;
}

// Function to display data briefly on BCD
void Display(unsigned char Tens_value, unsigned char Ones_value)
{
    unsigned int time = 0;
    while (time < 5000)
    {
        Tens = Tens_value;
        Ones = Ones_value;
        time++;
    }
}

// Handling incoming signals
void ISR_Serial(void) interrupt 4
{
    if (TI == 1) { TI = 0; }
    if (RI == 1)
    {
        RI = 0;
        
        if (radio_on)
        {
            if (SBUF == '1')   // Change station left
            {
                if (station == 0)
                {
                    station = 2;
                }
                else
                {
                    station = (station - 1) % 3;
                }
                change = 0;
                first_letter = 0;
                second_letter = 1;
            }
            else if (SBUF == '2')   // Change station right
            {
                station = (station + 1) % 3;
                change = 0;
                first_letter = 0;
                second_letter = 1;
            }
            else if (SBUF == '3')   // Mute/unmute the radio
            {
                if (!muted)   // Mute
                {
                    muted = 1;
                }
                else   // Unmute
                {
                    muted = 0;
                    Display(Digit_codes[volume / 10], Digit_codes[volume % 10]);
                }
            }
            else if (SBUF == '4')   // Decrease radio volume
            {
                if (volume != 0)
                {
                    volume--;
                }
                Display(Digit_codes[volume / 10], Digit_codes[volume % 10]);   // Display volume
            }
            else if (SBUF == '5')   // Increase radio volume
            {
                if (volume != 30)
                {
                    volume++;
                }
                Display(Digit_codes[volume / 10], Digit_codes[volume % 10]);
            }
        }
    }
}  

// Button handling
void ISR_INT0() interrupt 0
{
    if (radio_on == 0)   // Turn on the radio
    {
        radio_on = 1;
        Send('d');   // Send signal to keyboard (turn on LED)
    }
}

// Timer 0 interrupt handler
void ISR_T0() interrupt 1
{
    if (radio_on == 1)   // Turn off the radio
    {
        if (P3_2 == 0 && hold_counter < 14)
        {
            hold_counter++;
        }
        if (hold_counter > 13)   // Button pressed for enough time
        {
            Send('e');   // Send signal to keyboard (turn off LED)
            radio_on = 0;
            hold_counter = 0;
        }
    }
    if (P3_2 == 1)
    {
        hold_counter = 0;
    }
}

// Timer 1 interrupt handler
void ISR_T1() interrupt 3
{
    if (radio_on == 1)   // Handling station display
    {
        if (station == 0)   // Display first station
        {
            if (change == 10)   // Time to change letters on the screen
            {
                if (first_letter < 6)
                {
                    first_letter++;
                }
                else
                {
                    first_letter = 0;
                }
                if (second_letter < 6)
                {
                    second_letter++;
                }
                else
                {
                    second_letter = 0;
                }
                change = 0;
            }
            else
            {
                change++;
            }
        }
        else   // Station 1 or 2
        {
            if (change == 10)
            {
                if (first_letter < 4)
                {
                    first_letter++;
                }
                else
                {
                    first_letter = 0;
                }
                if (second_letter < 4)
                {
                    second_letter++;
                }
                else
                {
                    second_letter = 0;
                }
                change = 0;
            }
            else
            {
                change++;
            }
        }
    }
}

void main()
{
    Init();
    while (1)   // Continuously display the station
    {
        if (radio_on == 1)   // Radio is on
        {
            if (!muted)   // Radio playing
            {
                Tens = Stations[station][first_letter];
                Ones = Stations[station][second_letter];
            }
            else   // Radio muted - flashing symbol
            {
                if (flashing == 1)
                {
                    Display(0x46, 0x70);   // Display symbol
                    flashing = 0;
                }
                else
                {
                    Display(0x00, 0x00);   // Clear the screen
                    flashing = 1;
                }
            }
        }
    }
}
