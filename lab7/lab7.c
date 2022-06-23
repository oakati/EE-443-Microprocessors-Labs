#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define seven_seg_0 ((volatile unsigned int *) 0xFF200020)
#define jp2 ((volatile unsigned int *) 0xFF200070)
#define timer_status ((volatile unsigned int *) 0xFF202000)

void seven_seg_decoder(uint32_t number){
	
	int i = 0;
	uint32_t digits[4];
	uint32_t sev[4];
	for(i = 0 ; i < 4 ; i++){ 
		digits[i] = number%10; //extracting the current digit
		if(digits[i] == 0){
			sev[i] = 0b111111;
		}
		else if(digits[i]==1){
			sev[i] = 0b110;
		}
		else if(digits[i]==2){
			sev[i] = 0b1011011;
		}
		else if(digits[i]==3){
			sev[i] = 0b1001111;
		}
		else if(digits[i]==4){
			sev[i] = 0b1100110;
		}
		else if(digits[i]==5){
			sev[i] = 0b1101101;
		}
		else if(digits[i]==6){
			sev[i] = 0b1111101;
		}
		else if(digits[i]==7){
			sev[i] = 0b111;
		}
		else if(digits[i]==8){
			sev[i] = 0b1111111;
		}
		else if(digits[i]==9){
			sev[i] = 0b1101111;
		}
			number = number/10; //next digit
	}
	
	*seven_seg_0 = (sev[3]<<24) | (sev[2]<<16) | (sev[1]<<8) | sev[0];
	
}

int main(void){
	
	*(jp2+1) = 0b01; //setting the first bit of the gpio as output mode
	
	*(timer_status+2) = 1500; //counter start value low
	*(timer_status+3) = 0; // counter start value high
	*(timer_status+1) = 0b110; //start the timer and do it continuously also set the ito (interrupt bit) zero
	*timer_status = 0b10;
	
	uint32_t distance_counter = 0; //this value will be used to calculate distance
	uint32_t time_counter = 0;
	uint32_t distance = 0;
	uint32_t jp2_old;
	while(1){ // do this bunch of events continuously
		
		
		if((*timer_status & 0b01) == 1){ //if the to (timeout) is one, meaning that the time is over
			
			*timer_status = *timer_status & 0b10; // just after, set it again to 0, run bit is not effected by this operation
			// only the lsb bit is of concern
			
			if(time_counter == 0){
				*jp2 = *jp2 | 0b01; //setting the lsb 1
			}
			else{
				*jp2 = *jp2 & 0b10; // setting the lsb 0
			}
			
			if(time_counter == 66667){
				time_counter = 0;
			}
			else 
			{
				time_counter++;
			}
				
			if((*jp2&0b10) == 0b10){ //checking whether jp2 is 0b1x
				distance_counter++;
			}
			else if(jp2_old&0b10 == 0b10){
				distance = distance_counter*2.55 + 0.5;
				seven_seg_decoder(distance);
				distance_counter = 0;
			}	
		
			jp2_old = *jp2;
		}
	}
}
