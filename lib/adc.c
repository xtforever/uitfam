/**
   - N_ADC_CHANNELS
     Angabe wieviele Kanäle des ATMega644 benutzt werden

   - CHANNELn
     Bitmuster zur Konfiguration des Logischen Channels n
     des ADC IRQ. Siehe Datenblatt ATMEGA 644, Register ADMUX
*/


#include "adc.h"
#include <util/delay.h>
#include <string.h>
#include <avr/interrupt.h>
#include "xitoa.h"


volatile ADC_IRQ_t ADC_IRQ;
volatile ADC_IRQ_t ADC_BUF;
static const uint8_t channel_mux[] = { CHANNEL0
#if (N_ADC_CHANNELS > 1)
, CHANNEL1
#endif
#if (N_ADC_CHANNELS > 2)
, CHANNEL2
#endif
#if (N_ADC_CHANNELS > 3)
, CHANNEL3
#endif
#if (N_ADC_CHANNELS > 4)
, CHANNEL4
#endif
#if (N_ADC_CHANNELS > 5)
, CHANNEL5
#endif
#if (N_ADC_CHANNELS > 6)
, CHANNEL6
#endif
#if (N_ADC_CHANNELS > 7)
, CHANNEL7
#endif
};

/**
*  @brief Zum Initialisieren wird die Funktion adc_init() verwendet.
*         Dadurch wird gleichzeitig die Messung gestartet.
*/
void adc_init(void)
{
  // ENABLE ADC, Prescaler MAX = 7
  // Prescaler 64 = ADPS2 | ADPS1
  memset( (void*)&ADC_IRQ, 0, sizeof ADC_IRQ );
  ADC_BUF.state = 0;
  ADMUX = channel_mux[0];
  ADCSRB = 0;
  ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADIE) | _BV(ADATE) | ADC_PRESCALE_64;
}

ISR(ADC_vect)
{
  register uint16_t adc_val = ADCW;
  register u8 channel, state;
  state = ADC_IRQ.state;

  if( (state & 1)  ) { /* nur jede 2. Messung verwenden */
    channel = state >> 1;
    if( channel_mux[channel] & _BV(ADC_VDIFF) ) adc_val ^= 512;
    ADC_IRQ.val[channel] = adc_val;
    if( channel >= (N_ADC_CHANNELS-1) ) {
      if( ADC_BUF.state == 0 ) {
        memcpy( (void*) &ADC_BUF, (void*) &ADC_IRQ, sizeof(ADC_BUF));
      }
      ADC_IRQ.state=0;
      ADMUX = channel_mux[0];
      return;
    }

    // skip to next channel for next conversion
    channel++;
    ADMUX = channel_mux[ channel ];
  }

  state++;
  ADC_IRQ.state = state;
}
