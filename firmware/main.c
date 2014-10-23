/* Name: main.c
 * Author: Mitchel Humpherys
 * Copyright: 2014
 * License: Creative Commons
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#if 0
/* https://sites.google.com/site/qeewiki/books/avr-guide/analog-input */
int ADCsingleREAD(uint8_t adctouse)
{
    int ADCval;

    ADMUX = adctouse;         // use #1 ADC
    ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    while(ADCSRA & (1 << ADSC));      // Thanks T, this line waits for the ADC to finish

    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    return ADCval;
}
#endif

void sitfor(char sitfor)
{
    char i;
    for (i = 0; i < sitfor; ++i)
        _delay_ms(16);
}

void displaybit(char bit)
{
    PORTD = 1 << 4;
    if (bit)
        sitfor(20);
    else
        sitfor(2);
    PORTD = 0;
    if (bit)
        sitfor(2);
    else
        sitfor(20);
}

void displaychar(const char c)
{
    char i;
    for (i = 0; i < 8; ++i) {
        displaybit((c >> i) & 1);
        sitfor(20);
    }
}

int main(void)
{
    DDRD = 1 << 4;              /* make PD4 an output */

    for(;;) {
        char i;
        for (i = 0; i < 10; ++i) {
            displaychar(i);
            sitfor(50);
        }
    }

    return 0;   /* never reached */
}
