/*
 * Project name:
      THERMO click - Thermocouple Test
 * Copyright:
      (c) mikroElektronika, 2012, Kirk Ewing 2017.
 * Revision History:
      20120709:
      - Initial release (DO);
	20170808
	- Converted to Creator Ci40
	20170809
	- Added in Internal Temperature reading
	20170809
	-Encapsulated the code properly
	20170810
	- Added in the code for Relay 4
 * Description:
      This is a simple project which demonstrates the use of THERMO click board.
      Temperature measured by the thermocouple is converter by MAX31855 and written to the console.
      Displayed temperature is in degree Celsius.
      Example also includes fault detection. (Open circuit, Short to GND and Short to VCC).
      https://download.mikroe.com/documents/datasheets/max31855.pdf
 * Test configuration:
     Dev. Board:      Ci40
     Ext. Modules:    THERMO click - ac:THERMO_click
                      http://www.mikroe.com/eng/products/view/930/thermo-click/
 * NOTES:
      - Place THERMO Click board in the mikroBUS socket 1.
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <letmecreate/core/common.h>
#include <letmecreate/core/gpio.h>
#include <letmecreate/core/gpio_monitor.h>
#include <letmecreate/core/spi.h>
#include <letmecreate/click/export.h>

/** Relay Click channel index */
enum RELAY4_CLICK_RELAY {
    RELAY4_CLICK_RELAY_1,
    RELAY4_CLICK_RELAY_2,
    RELAY4_CLICK_RELAY_3,
    RELAY4_CLICK_RELAY_4,
    RELAY4_CLICK_RELAY_COUNT
};

static int find_gpio(uint8_t mikrobus_index, uint8_t relay)
{
    uint8_t gpio_pin = 0;

    if (relay >= RELAY4_CLICK_RELAY_COUNT) {
        fprintf(stderr, "relay4: Invalid relay index.\n");
        return -1;
    }

    if (relay == RELAY4_CLICK_RELAY_1) {
        if (gpio_get_pin(mikrobus_index, TYPE_PWM, &gpio_pin) < 0) {
            fprintf(stderr, "relay4: Invalid pin type\n");
            return -1;
        }
    }
    else if (relay == RELAY4_CLICK_RELAY_2) {
        if (gpio_get_pin(mikrobus_index, TYPE_RST, &gpio_pin) < 0) {
           fprintf(stderr, "relay4: Invalid pin type\n");
           return -1;
        }
    }
    else if (relay == RELAY4_CLICK_RELAY_4) {
        	if (gpio_get_pin(mikrobus_index, TYPE_AN, &gpio_pin) < 0) {
            	fprintf(stderr, "relay4: Invalid pin type\n");
            	return -1;
        	}
    	}
    else if (relay == RELAY4_CLICK_RELAY_3) {
       	fprintf(stderr, "relay4: Can't set relay3 as it is a reserved pin\n");
        return -1;
        }
    
    return gpio_pin;
}

int relay4_click_set_relay_state(uint8_t mikrobus_index, uint8_t relay, uint8_t state)
{
    uint8_t gpio_pin = 0;
    int ret = 0;

    ret = find_gpio(mikrobus_index, relay);
    if (ret < 0)
        return -1;

    gpio_pin = (uint8_t)ret;

    if (gpio_init(gpio_pin) < 0
    ||  gpio_set_direction(gpio_pin, GPIO_OUTPUT) < 0
    ||  gpio_set_value(gpio_pin, !!state) < 0)
        return -1;

    return 0;
}

int relay4_click_enable_relay(uint8_t mikrobus_index, uint8_t relay)
{
    return relay4_click_set_relay_state(mikrobus_index, relay, 1);
}

int relay4_click_disable_relay(uint8_t mikrobus_index, uint8_t relay)
{
    return relay4_click_set_relay_state(mikrobus_index, relay, 0);
}


int MAX31855_Read(float *tempValue, float *internalTempValue) {                                                                             
	int tmp, intTemp, remTemp;
	int internalTmp, internalIntTmp, internalRemTmp;
	float temperature, internalTemperature;
	
	uint8_t buffer[4] = {0x00, 0x00, 0x00, 0x00};           // Variable to be used for the 4 bytes coming from the registers
								// Clear the memory being used by the buffer
								// I was getting random numbers when spi_transfer was used incorrectly

	if (spi_transfer(NULL, buffer, sizeof(buffer)) < 0) {			// Read four bytes from SPI port, pass NULL for the TX buffer, buffer for the RX buffer                                                          
                printf("thermo: Failed to read temperature from sensor\n");                     
                return -1;                                                                                  
        }                                                                                                
        
	// printf ("The buffer 0 is %x \n", buffer[0]);  //debug code to print the buffer read from the MAX31855 to the console
        // printf ("The buffer 1 is %x \n", buffer[1]);
        // printf ("The buffer 2 is %x \n", buffer[2]);
        // printf ("The buffer 3 is %x \n", buffer[3]);
                                                                           
        tmp = buffer[0];                              // Place the first byte in tmp variable                                                 
        tmp = tmp << 8;                               // Bit shift first byte up 8 bits                                                   
        tmp = tmp | buffer[1];                        // Place second byte in tmp variable                                                   
        remTemp = tmp >> 2;                           // Remove D16 and D17 from the 32 bit number                                                   
        remTemp = remTemp & 0x03;                     // And with 0b111 to get the 3 bits that define decimal part of temperature value         
        temperature = remTemp * 0.25;                 // Multiply bottom three bits by 0.25 to get decimal part of temerature and store                                     
        intTemp = tmp >> 4;                           // Remove D16 through D19 which do not contain integer part of temperature value               
        temperature += intTemp;                       // Add integer part to temperature value                
        // printf("Read the Temp %.3f \n", temperature); // Write temperature to the console                                           
        
	internalTmp = buffer[2];			// Place the first byte in internalTmp variable
	internalTmp = internalTmp << 8;			// Bit shift first byte up 8 bits
	internalTmp = internalTmp | buffer[3];		// Place second byte in internalTmp variable 
	internalRemTmp = internalTmp >> 4;		// Remove D0 through D3 from the 32 bit number
	internalRemTmp = internalRemTmp & 0x1f;		// And with 0b11111 to get the 5 bits that define decimal part of internal temperature value
	internalTemperature = internalRemTmp * 0.0625;	// Multiply bottom three bits by 0.0625 to get decimal part of temerature and store
	internalIntTmp = internalTmp >> 9;		// Remove D16 through D19 which do not contain integer part of temperature value
	internalTemperature += internalIntTmp;		// Add integer part to internalTemperature value
	// printf("Read the Internal Temp %.4f \n", internalTemperature);	// Write internal temperature to the console 

        if((buffer[1] & 0x01) == 0x01){          // Fault detection                           
                 printf("Error!");                                                                
                 if((buffer[3] & 0x01) == 0x01){        // Open circuit fault?  
                         printf("Open circuit\n");           // Write text in first row
                         return -1;                                             
                 }                                                                                
                                                                                                  
                 if((buffer[3] & 0x02) == 0x02){        // Short to GND fault?                 
                         printf("Short to GND\n");           // Write text in first row
                         return -1;                                             
                 }                                                                   
                                                                                     
                 if((buffer[3] & 0x04) == 0x04){        // Short to Vcc fault?    
                         printf("Short to Vcc\n");           // Write text in first row
                         return -1;                                             
                 }                                                                   
         }                                                                           
         else{                                                                       
                 *tempValue = temperature;
		 *internalTempValue = internalTemperature;
		 //printf("Display the temp without error %.3f \n \n", temperature);             
        }                                                                           
        return 0;                                                              

}                                                                                                        
                                                                                                         
                                                                                                         
int main()                                                                                               
{                                                                                                        
        float thermocoupleTemp, internalTemp;
	
	spi_init();			// initialize the SPI bus                                                
	spi_select_bus(MIKROBUS_1);     // Ensure you have Mikrobus_1 selected for the SPI port                                                  
        
        while(1) {                                                                        
                relay4_click_disable_relay (MIKROBUS_2, RELAY4_CLICK_RELAY_2);
		relay4_click_disable_relay (MIKROBUS_2, RELAY4_CLICK_RELAY_4);
		relay4_click_enable_relay (MIKROBUS_2, RELAY4_CLICK_RELAY_1);
		sleep(1);
		
		if(MAX31855_Read(&thermocoupleTemp, &internalTemp) < 0) {
			printf("Error reading the temp");
		} else {
			printf("Thermocouple Relay1 temp is %.2f Internal temp is %.4f \n", thermocoupleTemp, internalTemp);
		}

		relay4_click_disable_relay (MIKROBUS_2, RELAY4_CLICK_RELAY_1);
		relay4_click_enable_relay (MIKROBUS_2, RELAY4_CLICK_RELAY_2);
		sleep(1);
		
		if(MAX31855_Read(&thermocoupleTemp, &internalTemp) < 0) {
			printf("Error reading the temp");
		} else {
			printf("Thermocouple Relay2 temp is %.2f Internal temp is %.4f \n", thermocoupleTemp, internalTemp);
		}

		relay4_click_disable_relay (MIKROBUS_2, RELAY4_CLICK_RELAY_2);
		relay4_click_enable_relay (MIKROBUS_2, RELAY4_CLICK_RELAY_4);
		sleep(1);
		
		if(MAX31855_Read(&thermocoupleTemp, &internalTemp) < 0) {
			printf("Error reading the temp");
		} else {
			printf("Thermocouple Relay4 temp is %.2f Internal temp is %.4f \n", thermocoupleTemp, internalTemp);
		}

        }                                                                                   
                                                                                            
	spi_release();			// release the SPI bus

	return 0;
}
