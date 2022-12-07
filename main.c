#include "gd32vf103.h"
#include "drivers.h"
#include "eclicw.h"
#include "lcd.h"
#include "delay.h"
#include "adc.h"
#include "fft.h"
#include "cordic-math.h"
#define EI 1
#define DI 0
#define SCREENHEIGHT 80
#define WAVEPOINTS 128
#define SAMPLERATE 7812

Complex wave[WAVEPOINTS]={0};

int mikrois(void){
	uint64_t mtime = get_timer_value();
	return ((mtime*4000*1000.0)/SystemCoreClock);
}

int main(void)
{ 
  for (int i = 0; i < WAVEPOINTS; i++)
  {
    wave[i].real = 0;
    wave[i].imag = 0;
  }
  
  int adcr;
  int value;
  int amount = 0;
  int length;
  ADC3powerUpInit(0); // Initialize ADC0, Ch3 & Ch16
  Lcd_SetType(LCD_INVERTED);
  Lcd_Init();
  LCD_Clear(BLACK);
  //mikrois();

  while (1)
  {
    //uint64_t time = get_timer_value();
    if (adc_flag_get(ADC0, ADC_FLAG_EOC) == SET)
    { // ...ADC done?
      value = adc_regular_data_read(ADC0);
      wave[amount].real = ((value-2047)>>4); //-2047 center runt x-axel >> 4 ger istället bas 2^8 fixedpoint
      wave[amount++].imag = 0;
      adc_flag_clear(ADC0, ADC_FLAG_EOC); // ......clear IF
      delay_1us(0);   //ändra för skala, 7812 = 1Hz
      adc_software_trigger_enable(ADC0, // Trigger another ADC conversion!
                                ADC_REGULAR_CHANNEL);
    }
    if(amount==WAVEPOINTS){
      delay_1ms(100);
      LCD_Clear(BLACK);
      LCD_Wait_On_Queue();
      fft(wave,WAVEPOINTS);
      draw_graph();
      for (int i = 0; i < 128; i++)
      {
        //LCD_DrawLine(i,40,i,(40 - (cordic_hypotenuse(wave[i].imag/(2/CORDIC_MATH_FRACTION_BITS), wave[i].real/(2/CORDIC_MATH_FRACTION_BITS))>>CORDIC_MATH_FRACTION_BITS)),RED);
        LCD_DrawLine(i,40,i,(40 - (cordic_hypotenuse(wave[i].imag/(64), wave[i].real/(64))/4)),RED);
         //LCD_DrawLine(i,40,i,(-cordic_hypotenuse(wave[i].imag, wave[i].real)>>CORDIC_MATH_FRACTION_BITS)+40,RED);
      }
      inverse_fft(wave,WAVEPOINTS);
      for (int i = 0; i < 128; i++)
      {
        LCD_DrawPoint(i,(wave[i].real/5)+40,BLUE);
      }
      
      LCD_Wait_On_Queue();
      amount=0;
    }
  }
}
