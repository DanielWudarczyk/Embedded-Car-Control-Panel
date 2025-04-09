#include <REGX52.H>

unsigned char pwm_helper_motor1 = 0, pwm_helper_motor2 = 0;   // Variables used for controlling the bulb color and motor speed
unsigned char speed_nominator2 = 3, speed_denominator2 = 6;
unsigned char rotations2 = 0;
bit mode = 0, direction = 0, on = 0;

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
    
    // Initial motor and bulb settings
    P2_3 = P2_4 = 0;
    P2_1 = 1;
    P2_2 = 0;
    P2_5 = 1;
    P2_6 = 0;
    
    TMOD = 0x22;   // Timers 1 and 2 in mode 2
    
    // Enable external interrupts and timers
    IT0 = IT1 = 1; 
    ET0 = ET1 = 1; 
    EX0 = EX1 = 1; 
    TR0 = TR1 = 1;
}

// Handling incoming signals
void ISR_Serial(void) interrupt 4
{
    if (TI == 1) { TI = 0; }
    if (RI == 1)
    {
        RI = 0;
        
        if (SBUF == '6')   // Turn on the air conditioning
        {
            if (on == 0)
            {
                on = 1;
                direction = 0;
                rotations2 = 0;
            }
            else   // Turn off the air conditioning
            {
                on = 0;
                P2_3 = 0;
                P2_4 = 0;
            }
        }
        else if (SBUF == '7')   // Change air conditioning mode
        {
            if (mode == 0)
            {
                mode = 1;
                pwm_helper_motor1 = 0;
                // Turn on the upper bulb
                P2_1 = 0;
                P2_2 = 1;
            }
            else
            {
                mode = 0;
                pwm_helper_motor1 = 0;
                // Turn on the lower bulb
                P2_1 = 1;
                P2_2 = 0;
            }
        }
        else if (SBUF == '8')
        {
            if (speed_nominator2 > 1)
            {
                speed_nominator2--;   // Decrease motor speed
                pwm_helper_motor2 = 0;
            }
        }
        else if (SBUF == '9')
        {
            if (speed_nominator2 < 6)
            {
                speed_nominator2++;   // Increase motor speed
                pwm_helper_motor2 = 0;
            }
        }
    }
} 

// Handling INT0 interrupt - counting motor rotations and changing direction
void INT0_ISR() interrupt 0
{
    rotations2 += 1;
    if (rotations2 == 20)
    {
        rotations2 = 0;
        if (direction == 0)
        {
            // Left
            P2_5 = 0;
            P2_6 = 1;
            direction = 1;
        }
        else
        {
            // Right
            P2_5 = 1;
            P2_6 = 0;
            direction = 0;
        }
    }
}

// Timer 1 interrupt handler - controlling bulb color
void Motor1_T1_ISR() interrupt 3 
{
    if (on == 1)
    {
        if (mode == 0)   // Heating mode
        {
            if (pwm_helper_motor1 < 3)
            {
                P2_3 = 1;
            }
            else
            {
                P2_3 = 0;
            }
            
            pwm_helper_motor1++;
            
            if (pwm_helper_motor1 == 7)
            {
                pwm_helper_motor1 = 0;
            }
        }
        else   // Cooling mode
        {
            if (pwm_helper_motor1 < 5)
            {
                P2_3 = 1;
            }
            else
            {
                P2_3 = 0;
            }
            
            pwm_helper_motor1++;
            
            if (pwm_helper_motor1 == 5)
            {
                pwm_helper_motor1 = 0;
            }
        }
    }
}

// Timer 0 interrupt handler - controlling motor speed
void Motor2_T0_ISR() interrupt 1
{
    if (on == 1)
    {
        if (pwm_helper_motor2 < speed_nominator2)
        {
            P2_4 = 1;
        }
        else
        {
            P2_4 = 0;
        }
        
        pwm_helper_motor2++;
        
        if (pwm_helper_motor2 == speed_denominator2)
        {
            pwm_helper_motor2 = 0;
        }
    }
}

void main(void)
{
    Init();
    while (1)
    {
        ;
    }
}
