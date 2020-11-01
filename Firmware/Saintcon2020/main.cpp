/*
 * Saintcon2020.cpp
 *
 * Created: 9/2/2020 3:50:26 PM
 * Author : compukidmike
 */ 

#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <string.h>


#define BUTTONPIN 2
#define BUTTONPORT _SFR_IO8(0x18) //PORTB

#define TP1PIN 0
#define TP2PIN 1
#define TP1PINPORT _SFR_IO8(0x18) //PORTB
#define TP2PINPORT _SFR_IO8(0x18) //PORTB


#define DEBOUNCETIME 200
#define LONGPRESSTIME 2000

#define NUMLEDS 8

volatile uint8_t LEDS[NUMLEDS];


volatile uint16_t millis = 0;
volatile uint8_t milliscounter = 0;

volatile uint16_t lastIntTime = 0;
volatile uint8_t pwmvalue = 0;

volatile uint8_t ledcounter = 0;
volatile uint8_t pwmcounter = 0;


uint8_t displayState = 0;

uint16_t lastDisplayUpdate = 0;

uint16_t buttonDebounce = 0;

uint8_t displayCounter = 0;
uint8_t chaseDelay = 50;
uint8_t ringDelay = 50;
uint8_t waveDelay = 100;
uint8_t pulseDelay = 10;
uint16_t alternateDelay = 500;
uint16_t alternate2Delay = 500;
uint16_t circleDelay = 100;
bool countUp = true;

uint16_t lastTpUpdate = 0;
uint16_t tpDelay = 1000;
bool tpState = false;
uint8_t tpCounter = 0;

uint8_t numStates = 8;

bool lastButtonState = false;

uint8_t isButtonPressed();

void specialModes();

int main(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	GIMSK |= (1<<PCIE1);
	PCMSK1 |= (1<<PCINT10);
	
	
	
	DDRA = 255;
	
	PORTB |= 1<<BUTTONPIN; //Turn on Pullup
	
	DDRB |= 1<<TP2PIN; //Turn on Output
	
	//DDRB |= 1<<TP1PIN; //Turn on Output
	PORTB |= 1<<TP1PIN; //Turn on Pullup
	
	//Setup Timer 1 for 1KHz
	// Clear registers
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;

	
	// 1000 Hz (1000000/((124+1)*8))
	OCR1A = 124;
	// CTC
	TCCR1B |= (1 << WGM12);
	// Prescaler 8
	TCCR1B |= (1 << CS11);
	// Output Compare Match A Interrupt Enable
	TIMSK1 |= (1 << OCIE1A);
	
	/*
	// 10000 Hz (1000000/((99+1)*1))
	OCR1A = 99;
	// CTC
	TCCR1B |= (1 << WGM12);
	// Prescaler 1
	TCCR1B |= (1 << CS10);
	// Output Compare Match A Interrupt Enable
	TIMSK1 |= (1 << OCIE1A);
	*/
	/*
	 // 5000 Hz (1000000/((24+1)*8))
	 OCR1A = 24;
	 // CTC
	 TCCR1B |= (1 << WGM12);
	 // Prescaler 8
	 TCCR1B |= (1 << CS11);
	 // Output Compare Match A Interrupt Enable
	 TIMSK1 |= (1 << OCIE1A);
	 */
	/*
	// 2000 Hz (1000000/((499+1)*1))
	OCR1A = 499;
	// CTC
	TCCR1B |= (1 << WGM12);
	// Prescaler 1
	TCCR1B |= (1 << CS10);
	// Output Compare Match A Interrupt Enable
	TIMSK1 |= (1 << OCIE1A);
	 */
	sei(); //enable interrupts
	
	/*while(1){
		for(int x = 0; x<20; x++){
			if(x<pwmvalue){
				LEDS[x] = true;
			} else {
				LEDS[x] = false;
			}
		}
	}*/
	
	displayState = eeprom_read_byte((uint8_t*)0);
	if(displayState > numStates) displayState = 0;
	
	if((PINB & 1<<TP1PIN) == 0) specialModes();
	PORTB &= ~(1<<TP1PIN); //Turn off Pullup
	

    while (1) 
    {
		/*if(!(PINB & 0b100)){
			if(lastButtonState == false){
				if(millis - buttonDebounce > DEBOUNCETIME){
					lastButtonState = true;
					buttonDebounce = millis;
					displayState += 1;
					displayCounter = 0;
					for(int x=0; x<8; x++){
						LEDS[x] = 0;
					}
					if(displayState > numStates-1){
						displayState = 0;
					}
				}
			}
		} else {
			//if(millis - buttonDebounce > DEBOUNCETIME){
				lastButtonState = false;
			//}
		}*/
		if(isButtonPressed()){
			displayState += 1;
			displayCounter = 0;
			countUp = true;
			tpCounter = 0;
			lastTpUpdate = millis;
			for(int x=0; x<8; x++){
				LEDS[x] = 0;
			}
			if(displayState > numStates - 1){
				displayState = 0;
			}
			eeprom_write_byte((uint8_t*)0,displayState);
		}
		switch(displayState){
			case 0: //Ring
				if(millis - lastDisplayUpdate > ringDelay){
					lastDisplayUpdate = millis;
					
					if(displayCounter < NUMLEDS){
						LEDS[displayCounter] = 100;
					} else {
						LEDS[displayCounter - NUMLEDS] = 0;
					}
					displayCounter ++;
					if(displayCounter > NUMLEDS*2){
						displayCounter = 0;
					}
				}
				break;
			case 1: //Chase
				if(millis - lastDisplayUpdate > chaseDelay){
					lastDisplayUpdate = millis;
					
					//for(int x = 0; x<NUMLEDS; x++){
						LEDS[displayCounter] = 100;
						if(displayCounter == 0){
							LEDS[NUMLEDS-1] = 0;
						} else {
							LEDS[displayCounter-1] = 0;
						}
					//}
					displayCounter ++;
					if(displayCounter > NUMLEDS-1){
						displayCounter = 0;
					}
				}
				break;
			case 2: //Blink
				if(millis - lastDisplayUpdate > waveDelay){
					lastDisplayUpdate = millis;
					
					if(displayCounter == 0){
						for(int x=0; x<NUMLEDS; x++){
							LEDS[x] = 100;
						}
					} else {
						for(int x=0; x<NUMLEDS; x++){
							LEDS[x] = 0;
						}
					}

					displayCounter ++;
					if(displayCounter > 1){
						displayCounter = 0;
					}
				}
				break;
			case 3: //Pulse
				if(millis - lastDisplayUpdate > pulseDelay){
					lastDisplayUpdate = millis;
					
					for(int x=0; x<NUMLEDS; x++){
						LEDS[x] = displayCounter;
					}

					if(countUp == true){
						displayCounter ++;
						if(displayCounter > 99){
							countUp = false;
						}
					} else {
						displayCounter --;
						if(displayCounter == 0){
							countUp = true;
						}
					}
				}
				break;
			case 4: //Alternate
				if(millis - lastDisplayUpdate > alternateDelay){
					lastDisplayUpdate = millis;
				
					if(displayCounter == 0){
						LEDS[0] = 100;
						LEDS[1] = 100;
						LEDS[2] = 100;
						LEDS[3] = 100;
						LEDS[4] = 0;
						LEDS[5] = 0;
						LEDS[6] = 0;
						LEDS[7] = 0;
					} else {
						LEDS[0] = 0;
						LEDS[1] = 0;
						LEDS[2] = 0;
						LEDS[3] = 0;
						LEDS[4] = 100;
						LEDS[5] = 100;
						LEDS[6] = 100;
						LEDS[7] = 100;
					}
					
					displayCounter ++;
					if(displayCounter > 1){
						displayCounter = 0;
					}
				}
				break;
			case 5: //Alternate 2
				if(millis - lastDisplayUpdate > alternate2Delay){
					lastDisplayUpdate = millis;
					
					if(displayCounter == 0){
						LEDS[0] = 100;
						LEDS[1] = 0;
						LEDS[2] = 100;
						LEDS[3] = 0;
						LEDS[4] = 100;
						LEDS[5] = 0;
						LEDS[6] = 100;
						LEDS[7] = 0;
						} else {
						LEDS[0] = 0;
						LEDS[1] = 100;
						LEDS[2] = 0;
						LEDS[3] = 100;
						LEDS[4] = 0;
						LEDS[5] = 100;
						LEDS[6] = 0;
						LEDS[7] = 100;
					}
					
					displayCounter ++;
					if(displayCounter > 1){
						displayCounter = 0;
					}
				}
				break;
			case 6: //Circle
				if(millis - lastDisplayUpdate > circleDelay){
					lastDisplayUpdate = millis;
					
					if(displayCounter == 0){
						LEDS[0] = 100;
						LEDS[1] = 0;
						LEDS[2] = 0;
						LEDS[3] = 0;
						LEDS[4] = 100;
						LEDS[5] = 0;
						LEDS[6] = 0;
						LEDS[7] = 0;
					} else if(displayCounter == 1){
						LEDS[0] = 0;
						LEDS[1] = 100;
						LEDS[2] = 0;
						LEDS[3] = 0;
						LEDS[4] = 0;
						LEDS[5] = 100;
						LEDS[6] = 0;
						LEDS[7] = 0;
					} else if(displayCounter == 2){
						LEDS[0] = 0;
						LEDS[1] = 0;
						LEDS[2] = 100;
						LEDS[3] = 0;
						LEDS[4] = 0;
						LEDS[5] = 0;
						LEDS[6] = 100;
						LEDS[7] = 0;
					} else {
						LEDS[0] = 0;
						LEDS[1] = 0;
						LEDS[2] = 0;
						LEDS[3] = 100;
						LEDS[4] = 0;
						LEDS[5] = 0;
						LEDS[6] = 0;
						LEDS[7] = 100;
					}

					displayCounter ++;
					if(displayCounter > 3){
						displayCounter = 0;
					}
				}
				break;
			case 7: //On
				for(int x=0; x<NUMLEDS; x++){
					LEDS[x] = 100;
				}
			break;
			//default:
				//displayState = 0;
		}
		
		if(millis - lastTpUpdate > tpDelay){
			lastTpUpdate = millis;
			if(tpCounter < displayState+1){
				if(tpState == false){			
					tpState = true;
					PORTB |= 1<<TP1PIN; //Turn on Pullup
				} else {
					tpState = false;
					PORTB &= ~(1<<TP1PIN); //Turn off Pullup
					tpCounter ++;
				}
			} else if(tpCounter > displayState + 2) {
				tpCounter = 0;
			} else {
				tpCounter ++;
			}
		}
		
	}
				
		
		/*
		for(int x = 0; x<20; x++){
			LEDS[x] = true;
			_delay_ms(50);
		}
		for(int x = 0; x<20; x++){
			LEDS[x] = false;
			_delay_ms(50);
		}
		for(int y = 0; y<5; y++){
			for(int x = 0; x<20; x+=4){
				LEDS[x] = true;
				if(x == 0){
					LEDS[19] = false;
				} else {
					LEDS[x-1] = false;
				}
			}
			_delay_ms(100);
			for(int x = 1; x<20; x+=4){
				LEDS[x] = true;
				LEDS[x-1] = false;
			}
			_delay_ms(100);
			for(int x = 2; x<20; x+=4){
				LEDS[x] = true;
				LEDS[x-1] = false;
			}
			_delay_ms(100);
			for(int x = 3; x<20; x+=4){
				LEDS[x] = true;
				LEDS[x-1] = false;
			}
			_delay_ms(100);
		}
		for(int x = 0; x<20; x++){
			LEDS[x] = false;
		}
		
    }*/
}

ISR(TIM1_COMPA_vect) {
	millis += 1; //Update millisecond timer
	
	//PORTA = 255; //Turn all LEDs ON
	
	uint8_t ledstate = 255;
	
	if(pwmcounter > LEDS[0]) ledstate &= ~(1<<0);
	if(pwmcounter > LEDS[1]) ledstate &= ~(1<<1);
	if(pwmcounter > LEDS[2]) ledstate &= ~(1<<2);
	if(pwmcounter > LEDS[3]) ledstate &= ~(1<<3);
	if(pwmcounter > LEDS[4]) ledstate &= ~(1<<4);
	if(pwmcounter > LEDS[5]) ledstate &= ~(1<<5);
	if(pwmcounter > LEDS[6]) ledstate &= ~(1<<6);
	if(pwmcounter > LEDS[7]) ledstate &= ~(1<<7);
	
	PORTA = ledstate;
	
	pwmcounter += 5;
	if(pwmcounter > 100){
		pwmcounter = 1;
	}

}

uint8_t isButtonPressed(){
	uint8_t retval = 0;
	static int previousButtonState = 1<<BUTTONPIN;
	static uint32_t lastDebounceTime = 0;
	static int sent = 0;
	int currentButtonState = (PINB & 1<<BUTTONPIN);
	if(currentButtonState != previousButtonState){
		lastDebounceTime = millis;
		sent = 0;
	}
	previousButtonState = currentButtonState;
	if((millis - lastDebounceTime) > 50){
		if((currentButtonState == 0) & (sent == 0)) retval = 1;
		sent = 1;
	}
	return retval;
}

ISR(PCINT1_vect){
	
}

void specialModes(){
	displayState = 0;
	displayCounter = 0;
	uint16_t binaryDelay = 1000;
	char messages[8][19] = {{200,225,238,160,243,232,239,244,160,230,233,242,243,244,0},
						{201,160,225,233,237,160,244,239,160,237,233,243,226,229,232,225,246,229,0},
						{201,244,160,247,225,243,160,196,206,211,0},
						{201,244,167,243,160,225,160,244,242,225,240,161,0},
						{205,245,236,244,233,240,225,243,243,161,0},
						{200,225,227,235,160,244,232,229,160,208,236,225,238,229,244,161,0},
						{211,197,212,197,195,160,193,211,212,210,207,206,207,205,217,0},
						{196,239,238,167,244,160,226,236,233,238,235,0}};
	
	while(1){
		if(isButtonPressed()){
			displayState += 1;
			displayCounter = 0;
			lastDisplayUpdate = millis;
			tpCounter = 0;
			lastTpUpdate = millis;
			countUp = true;
			for(int x=0; x<8; x++){
				LEDS[x] = 0;
			}
			if(displayState > numStates - 1){
				displayState = 0;
			}
		}
		if(millis > (lastDisplayUpdate + binaryDelay)){
			lastDisplayUpdate = millis;
			
			for(int x = 0; x<NUMLEDS-1; x++){
				if(messages[displayState][displayCounter] & 1<<x){
					LEDS[7-x] = 100;
					} else {
					LEDS[7-x] = 0;
				}
			}

			displayCounter ++;
			if(displayCounter > strlen(messages[displayState])){
				displayCounter = 0;
			}
		}
		
		if(millis - lastTpUpdate > tpDelay){
			lastTpUpdate = millis;
			if(tpCounter < displayState+1){
				if(tpState == false){
					tpState = true;
					PORTB |= 1<<TP1PIN; //Turn on Pullup
					} else {
					tpState = false;
					PORTB &= ~(1<<TP1PIN); //Turn off Pullup
					tpCounter ++;
				}
				} else if(tpCounter > displayState + 2) {
				tpCounter = 0;
				} else {
				tpCounter ++;
			}
		}
	}
}
