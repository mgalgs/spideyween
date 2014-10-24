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
static uint8_t ADCsingleREAD(uint8_t adctouse)
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

static void led_init(void)
{
    DDRD = 1 << 4;              /* make PD4 an output */
}

/*
 * http://www.electroons.com/electroons/servo_control.html
 * http://eliaselectronics.com/atmega-servo-tutorial/
 */
static void servo_init(void)
{
    TCCR1A |= (1 << COM1A1) | (1 << WGM11); // non-inverting mode for OC1A
	TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Mode 14, Prescaler 8

    /*
     * 16 MHz CPU, pulse width of 20 ms, means we need a pulse every
     * 320,000 cycles.  We can't count to 320,000 with a 16-bit register,
     * so we prescale it by 8 (we count by 8's).  We need to count by 8's
     * to 40,000 to get the 20 ms pulse.
     */
	ICR1 = 40000; // 320000 / 8 = 40000

	DDRB |= (1 << PB1); // OC1A set to output
}

/*
 * lookup table for OCR1A value needed to move the servo to the given
 * degree (indexed by degrees). Constructed with a linear interpolation,
 * found in lerp.el.
 */
static uint16_t servo_angle_to_ocr1a[180] = {
    2000, 2011, 2022, 2033, 2044, 2055, 2066, 2077, 2088, 2100, 2111, 2122, 2133, 2144, 2155, 2166, 2177, 2188, 2200, 2211, 2222, 2233, 2244, 2255, 2266, 2277, 2288, 2300, 2311, 2322, 2333, 2344, 2355, 2366, 2377, 2388, 2400, 2411, 2422, 2433, 2444, 2455, 2466, 2477, 2488, 2500, 2511, 2522, 2533, 2544, 2555, 2566, 2577, 2588, 2600, 2611, 2622, 2633, 2644, 2655, 2666, 2677, 2688, 2700, 2711, 2722, 2733, 2744, 2755, 2766, 2777, 2788, 2800, 2811, 2822, 2833, 2844, 2855, 2866, 2877, 2888, 2900, 2911, 2922, 2933, 2944, 2955, 2966, 2977, 2988, 3000, 3011, 3022, 3033, 3044, 3055, 3066, 3077, 3088, 3100, 3111, 3122, 3133, 3144, 3155, 3166, 3177, 3188, 3200, 3211, 3222, 3233, 3244, 3255, 3266, 3277, 3288, 3300, 3311, 3322, 3333, 3344, 3355, 3366, 3377, 3388, 3400, 3411, 3422, 3433, 3444, 3455, 3466, 3477, 3488, 3500, 3511, 3522, 3533, 3544, 3555, 3566, 3577, 3588, 3600, 3611, 3622, 3633, 3644, 3655, 3666, 3677, 3688, 3700, 3711, 3722, 3733, 3744, 3755, 3766, 3777, 3788, 3800, 3811, 3822, 3833, 3844, 3855, 3866, 3877, 3888, 3900, 3911, 3922, 3933, 3944, 3955, 3966, 3977, 3988
};

static void servo_set_degrees(uint8_t degrees)
{
    if (degrees > 179)
        degrees = 179;
    OCR1A = servo_angle_to_ocr1a[degrees];
}

static void sitfor(uint8_t sitfor)
{
    uint8_t i;
    for (i = 0; i < sitfor; ++i)
        _delay_ms(16);
}

static void displaybit(uint8_t bit)
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

static void displaychar(const uint8_t c)
{
    uint8_t i;
    for (i = 0; i < 8; ++i) {
        displaybit((c >> i) & 1);
        sitfor(20);
    }
}

static bool currently_spraying;

static void start_spraying(void)
{
    currently_spraying = true;
    PORTD = 1 << 4;
    servo_set_degrees(1);
}

static void stop_spraying(void)
{
    currently_spraying = false;
    PORTD = 0;
    servo_set_degrees(180);
}

static void attention(void)
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

    led_init();
    servo_init();
    /* (void)servo_init; */
    attention();
    stop_spraying();

    for(iters = 0; ; ++iters) {
        uint8_t adcval;

        adcval = ADCsingleREAD(0);
        (void)displaychar;

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
                }
            } else {
                stop_spraying();
                strikes = 0;
            }
        }
    }

    return 0;   /* never reached */
}
