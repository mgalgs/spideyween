/* Name: main.c
 * Author: Mitchel Humpherys
 * Copyright: 2014
 * License: Creative Commons
 */

#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    DDRD = 1 << 4;              /* make PD4 an output */
    for(;;) {
        char i;
        for (i = 0; i < 10; ++i)
            _delay_ms(16);
        PORTD ^= 1 << 4;        /* toggle the LED */
    }
    return 0;   /* never reached */
}
