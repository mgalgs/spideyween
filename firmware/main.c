/* Name: main.c
 * Author: Mitchel Humpherys
 * Copyright: 2014
 * License: Creative Commons
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

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

void displaybit(char bit)
{
    char i;
    PORTD = 1 << 4;
    for (i = 0; i < 10; ++i) {
        if (bit)
            _delay_ms(16);
        else
            _delay_ms(1);
    }
    PORTD = 0;
}

void displaychar(const char c)
{
    char i;
    for (i = 0; i < 8; ++i)
        displaybit((c >> i) & 1);
}

int main(void)
{
    DDRD = 1 << 4;              /* make PD4 an output */

    for(;;) {
        char i;
        for (i = 0; i < 10; ++i)
            displaychar(i);
        for (i = 0; i < 255; ++i)
            _delay_ms(16);
    }

    return 0;   /* never reached */
}
