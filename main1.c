#include <lpc21xx.h>
#include "header.h"

#define TEMP_LIMIT 400
#define LDR_LIMIT 400

#define SOIL_DRY_RAW 900
#define SOIL_WET_RAW 300

#define WATER_DETECTED 1

unsigned int temp_adc,ldr_adc,soil_adc,water;
unsigned int temp,soil_percent;
unsigned int status;

void gpio_init()
{
	  PINSEL0 &=~(3<<10);
    IODIR0 &= ~(1<<5);
    IODIR0 |= (7<<17);

    IOSET0 = 7<<17;
}

unsigned int water_read()
{
    return (IOPIN0>>5)&1;
}

unsigned int temperature_conversion(unsigned int adc)
{
    return (adc*3300)/1023;
}

unsigned int soil_conversion(unsigned int adc)
{
    int value;

    value=((SOIL_DRY_RAW-(int)adc)*100)/
          (SOIL_DRY_RAW-SOIL_WET_RAW);

    if(value<0)
        value=0;

    if(value>100)
        value=100;

    return value;
}

void led_control(unsigned int status)
{
    IOSET0=7<<17;

    if(status==0)
        IOCLR0=1<<17;

    else if(status==4)
        IOCLR0=1<<19;

    else
        IOCLR0=1<<18;
}

int main()
{
    adc_init();
    lcd_init();
    uart0_init(9600);
    gpio_init();

    while(1)
    {
        // Read sensors

        temp_adc=adc_read(1);
        ldr_adc=adc_read(2);
        soil_adc=adc_read(3);
        water=water_read();

        // Convert readings

        temp=temperature_conversion(temp_adc);
        soil_percent=soil_conversion(soil_adc);

        // Determine status

        status=0;

        if(water==WATER_DETECTED)
            status=4;

        else if(temp>TEMP_LIMIT)
            status=1;

        else if(ldr_adc<LDR_LIMIT)
            status=2;

        else if(soil_percent<30)
            status=3;

        // LED indication

        led_control(status);

        // LCD DISPLAY

        lcd_cmd(0x01);
        lcd_cmd(0x80);

        lcd_string("TEMP: ");
        lcd_integer(temp/10);
        lcd_data('.');
        lcd_integer(temp%10);
        lcd_string(" C");

        lcd_cmd(0xC0);

        if(ldr_adc<LDR_LIMIT)
            lcd_string("LIGHT: LOW");
        else
            lcd_string("LIGHT: NORMAL");

        delay_ms(1500);

        lcd_cmd(0x01);
        lcd_cmd(0x80);

        lcd_string("SOIL: ");
        lcd_integer(soil_percent);
        lcd_string("%");

        lcd_cmd(0xC0);

        if(water==WATER_DETECTED)
            lcd_string("WATER: DETECTED");
        else
            lcd_string("WATER: NOT FOUND");

        delay_ms(1500);

        // UART DISPLAY

        uart0_tx_string("\r\n------------------\r\n");

        uart0_tx_string("Temperature : ");
        uart0_integer(temp/10);
        uart0_tx('.');
        uart0_integer(temp%10);
        uart0_tx_string(" C\r\n");

        uart0_tx_string("Light Level : ");

        if(ldr_adc<LDR_LIMIT)
            uart0_tx_string("LOW\r\n");
        else
            uart0_tx_string("NORMAL\r\n");

        uart0_tx_string("Soil Moisture : ");
        uart0_integer(soil_percent);
        uart0_tx_string(" %\r\n");

        uart0_tx_string("Water Sensor : ");

        if(water==WATER_DETECTED)
            uart0_tx_string("DETECTED\r\n");
        else
            uart0_tx_string("NOT DETECTED\r\n");

        uart0_tx_string("System Status : ");

        if(status==0)
            uart0_tx_string("SAFE\r\n");

        else if(status==1)
            uart0_tx_string("HOT\r\n");

        else if(status==2)
            uart0_tx_string("DARK\r\n");

        else if(status==3)
            uart0_tx_string("WATER\r\n");

        else
            uart0_tx_string("ALERT\r\n");

        delay_ms(1000);
    }
}

