/* Name: main.c
 * Author: Mitchel Humpherys
 * Copyright: 2014
 * License: Creative Commons
 */

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

/* increase to make the spider slower to start spraying */
#define LAZY_SPIDER_FACTOR 8

/* https://sites.google.com/site/qeewiki/books/avr-guide/analog-input */
uint8_t ADCsingleREAD(uint8_t adctouse)
{
    uint8_t ADCval;

    ADMUX = adctouse;         // use #1 ADC
    ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX |= (1 << ADLAR);    // Right adjust for 8 bit resolution

    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    while(ADCSRA & (1 << ADSC));      // Thanks T, this line waits for the ADC to finish

    ADCval = ADCH;

    return ADCval;
}

void sitfor(uint8_t sitfor)
{
    uint8_t i;
    for (i = 0; i < sitfor; ++i)
        _delay_ms(16);
}

void displaybit(uint8_t bit)
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

void displaychar(const uint8_t c)
{
    uint8_t i;
    for (i = 0; i < 8; ++i) {
        displaybit((c >> i) & 1);
        sitfor(20);
    }
}

void start_spraying(void)
{
    PORTD = 1 << 4;
}

void stop_spraying(void)
{
    PORTD = 0;
}

void attention(void)
{
    uint8_t i;
    for (i = 0; i < 10; ++i) {
        PORTD = 1 << 4;
        sitfor(1);
        PORTD = 0;
        sitfor(1);
    }
}

int main(void)
{
    uint8_t iters, strikes = 0;
    float astar = 0, astar_prev = 0;
    bool currently_spraying = false;

    DDRD = 1 << 4;              /* make PD4 an output */

    attention();

    for(iters = 0; ; ++iters) {
        uint8_t adcval;

        adcval = ADCsingleREAD(0);
        /* displaychar(adcval); */
        /* sitfor(50); */

        /*
         * adcval interpretation
         * =====================
         *
         * Some collected data:
         *
         *   10-15: infinity
         *   28-30: about 3 feet
         *
         * For our purposes, anytime the target gets closer than about 3
         * feet we want to spray them, so:
         *
         *    > 28: ATTACK!
         */

        /* http://electronics.stackexchange.com/a/30384/35725 */
        astar = astar_prev + ((adcval - astar_prev)  * 0.0005);
        astar_prev = astar;

        if (iters == 255) {
            if (astar > 28) {
                if (strikes++ > LAZY_SPIDER_FACTOR &&
                    currently_spraying == false) {
                    start_spraying();
                    currently_spraying = true;
                }
            } else {
                stop_spraying();
                currently_spraying = false;
                strikes = 0;
            }
        }
    }

    return 0;   /* never reached */
}
