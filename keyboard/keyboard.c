#include <REGX52.H>

unsigned char code Tab[] = {0xEF, 0xDF, 0xBF, 0x7F};  // Array of walking zeros across rows
unsigned char data i;
unsigned char bdata Key;  // Variable storing the current state of pins P2
bit Enable = 1;  // Flag ensuring that a button press is not registered multiple times
bit w1 = 0, w2 = 0, trc = 0;
unsigned int a;
unsigned int timeout = 1000;

void wait()
{
    for (a = 0; a < 5000; a++) { ; }
}

// Initialization
void Init(void)
{
    P3_4 = 0;   // Set to receive mode
    SCON = 0x50;   // Configure serial port - 8-bit UART with timer-controlled baud rate, REN enabled
    T2CON = 0x30;   // Set timer 2 - works as an auto-reload timer
    TH2 = RCAP2H = 0xFF;   // Set initial value and reload value
    TL2 = RCAP2L = 0xDC;   // Same as above
    TI = 0;   // Reset flag of successful byte send
    RI = 0;   // Clear receive buffer
    ES = 1;   // Enable UART interrupts
    TR2 = 1;   // Start timer 2
    EA = 1;   // Enable global interrupts
}

// Sending data
void Send(unsigned char Value)
{
    P3_4 = 1;   // Set to transmit mode
    TI = 0;
    SBUF = Value;
    while (TI == 0 && timeout--) { ; }
    timeout = 1000;
    TI = 0;
    P3_4 = 0;   // Set to receive mode
}

// Decision after receiving signal
void Decyzja(unsigned char Value)
{
    if (Value == 0xE7)  // 1
    {
        if (trc == 0)   // Traction control service mode off, radio service mode on
        {
            Send('1');   // Previous radio station
        }
        else   // Traction control mode
        {
            Send('a');   // Normal traction control mode
        }
        w1 = w2 = 0;   // Reset progress in entering the mode change code
    }
    else if (Value == 0xEB)  // 2
    {
        if (trc == 0)
        {
            Send('2');   // Next radio station
        }
        else
        {
            Send('b');   // Sport traction control mode
        }
        w1 = w2 = 0;
    }
    else if (Value == 0xED)  // 3
    {
        if (trc == 0)
        {
            Send('3');   // Mute/unmute radio
        }
        else
        {
            Send('c');   // Turn off traction control
        }
        w1 = w2 = 0;
    }
    else if (Value == 0xD7)  // 4
    {
        if (trc == 0)
        {
            Send('4');   // Decrease volume
        }
        w1 = w2 = 0;
    }
    else if (Value == 0xDB)  // 5
    {
        if (trc == 0)
        {
            Send('5');   // Increase volume
        }
        w1 = w2 = 0;
    }
    else if (Value == 0xDD)  // 6
    {
        Send('6');   // Turn on/off air conditioning
        w1 = w2 = 0;
    }
    else if (Value == 0xB7)  // 7
    {
        Send('7');   // Change air conditioning mode
        w1 = w2 = 0;
    }
    else if (Value == 0xBB)  // 8
    {
        Send('8');   // Decrease airflow level
        w1 = w2 = 0;
    }
    else if (Value == 0xBD)  // 9
    {
        Send('9');   // Increase airflow level
        w1 = w2 = 0;
    }
    else if (Value == 0x77)  // *
    {
        if (w1 == 0)
        {
            w1 = 1;   // First button in code to change service mode pressed
        }
        w2 = 0;
    }
    else if (Value == 0x7B)  // 0
    {
        if (w1 == 1 && w2 == 0)
        {
            w2 = 1;   // Second button in code to change service mode pressed
        }
        else
        {
            w1 = 0;
        }
    }
    else if (Value == 0x7D)  // #
    {
        // Third button in code to change service mode pressed
        if (w1 == 1 && w2 == 1)
        {
            if (trc == 0)
            {
                trc = 1;   // Turn on traction control service mode
            }
            else
            {
                trc = 0;   // Turn on radio service mode
            }
            // Double flash of the LED
            if (P0_0 == 1)
            {
                P0_0 = 0;
                wait();
                P0_0 = 1;
                wait();
                P0_0 = 0;
                wait();
                P0_0 = 1;
            }
            else
            {
                P0_0 = 1;
                wait();
                P0_0 = 0;
                wait();
                P0_0 = 1;
                wait();
                P0_0 = 0;
            }
        }
        w1 = w2 = 0;
    }
}

// Receiving data
void ISR_Serial(void) interrupt  4
{
    if (TI == 1) { TI = 0; }
    if (RI == 1)   // A byte of data has been received
    {
        RI = 0;

        if (SBUF == 'd')
        {
            P0_0 = 0;   // Turn on LED
        }
        else if (SBUF == 'e')
        {
            P0_0 = 1;   // Turn off LED
        }
    }
}

void main(void)
{
    Init();
    while (1)
    {
        P2 = Tab[i];   // Set zero on one of the rows
        Key = P2;   // Current state of the ports to which the keypad is connected
        if (Key != Tab[i])   // Key pressed
        {
            if (Enable == 1)
            {
                Decyzja(Key);   // Handle signal
                Enable = 0;
            }
        }
        else
        {
            i++;
            if (i > 3) { i = 0; }
            Enable = 1;
        }
    }
}
