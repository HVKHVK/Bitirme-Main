#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <util/twi.h>
#include <util/delay.h>

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#define BLINK_DELAY_MS 500

volatile int first_algorithm_count;

//UART Initialize - Test Code
void init_uart(uint16_t baudrate){
    uint16_t UBRR_val = (F_CPU/16)/(baudrate-1);
    
    UBRR0H = UBRR_val >> 8;
    UBRR0L = UBRR_val;

    UCSR0B |= (1<<TXEN0) | (1<<RXEN0) | (1<<RXCIE0); 
    UCSR0C |= (1<<USBS0) | (3<<UCSZ00); 
}

//UART - Test Code
void uart_putc(unsigned char c){
    while(!(UCSR0A & (1<<UDRE0))); 
    UDR0 = c; 
}

//UART - Test Code
void uart_puts(char *s){
    while(*s){
        uart_putc(*s);
        s++;
    }
}

//ADC Enable For ADC Readings
void ADC_enable(){
    ADCSRA =           
    (1 << ADPS2) |     //set prescaler to 64, bit 2
    (1 << ADPS1) |     //set prescaler to 64, bit 1
    (1 << ADPS0);      //set prescaler to 64, bit 0
}

//ADC Reading For Second Algorithm
int get_voltage(){
    int i;
    
    ADMUX =           
    (1 << ADLAR) |     //Left shifts the result, for one byte
    (0 << REFS0) |     //Referance to input voltage values
    (0 << REFS1) |
    (0 << MUX3)  |     //use ADC0 MUX bit 3
    (0 << MUX2)  |     //use ADC0 MUX bit 2
    (0 << MUX1)  |     //use ADC0 MUX bit 1
    (0 << MUX0);       //use ADC0 MUX bit 0
    
    ADCSRA |= (1 << ADEN);          // Enable ADC
    
    ADCSRA |= (1 << ADSC);          // start ADC measurement
    while (ADCSRA & (1 << ADSC) );
    
    i = ADCH;
   
    ADCSRA &= ~(1 << ADEN);       //Disable ADC
    return i;
}

//ADC Reading For Door Opening 
int get_angle(){
	int i;

    ADMUX =            
    (1 << ADLAR) |     //Left shifts the result, for one byte
    (0 << REFS0) |     //Referance to input voltage values
    (0 << REFS1) |
    (0 << MUX3)  |     //use ADC1 MUX bit 3
    (0 << MUX2)  |     //use ADC1 MUX bit 2
    (0 << MUX1)  |     //use ADC1 MUX bit 1
    (1 << MUX0);       //use ADC1 MUX bit 0
    
    ADCSRA |= (1 << ADEN);          // Enable ADC
    
    ADCSRA |= (1 << ADSC);          // start ADC measurement
    while (ADCSRA & (1 << ADSC) );
    
    i = ADCH;
    
    ADCSRA &= ~(1 << ADEN);       //Disable ADC
    return i;
}

//Sleep Setups
void setup_sleep(){
  SMCR =
    (0 << SM2) |  
    (1 << SM1) |
    (0 << SM0) ;
}

//Enter Sleep Code
void go_sleep(){
  PCMSK2 |= (1 << PCINT22);
  PCICR |= (1 << PCIE2);
  SMCR |= (1 << SE);   // Sleep Enable
  sei();
  sleep_cpu(); // Same as sleep_mode()
  SMCR &= ~(1 << SE);  //Clears Sleep Enable//
  cli();
  PCICR &= ~(1 << PCINT2);
  PCMSK2 &= ~(1 << PCINT22);
}

//Primary Algorithm
int go_first(){
    PCMSK2 |= (1 << PCINT22);
    PCICR |= (1 << PCIE2);
    sei();
    _delay_ms(300);
    cli();

    if (first_algorithm_count <= 20 && first_algorithm_count >= 10)
        return 1;
    else
        return 0;
}

//Second Algorithm Compute Part
int go_second(){
    PCMSK2 |= (1 << PCINT22);
    PCICR |= (1 << PCIE2);
    sei();
    _delay_ms(40);
    cli();
    if (first_algorithm_count == 0)
        return 1;
    else
      return 0;
}

//Second Algorithm Decide Part
int edge(int dataArray[], int func, int out[], int count, int length,int volt){
  int i, j, k,t;
  int fir,sec,sum;
  count=0;

  for (i=0; i<length; i++){ 
    if(dataArray[i] == 1)
      count ++;
    else
      count = 0;

    if(count >= 10 && volt > 145)
      return 1;
  }
  return 0;
}

//Blink Check - Test Code
void blinkcheck(int no){
    int i = 0;
    while( i < no){
        PORTB |= _BV(PORTB5);
        _delay_ms(BLINK_DELAY_MS);

        PORTB &= ~_BV(PORTB5);
        _delay_ms(BLINK_DELAY_MS);
        i++;
    }
}

//Green LED Check - Test Code
void green(int no){
   int i = 0;
    while( i < no){
        PORTB |= _BV(PORTB4);
        _delay_ms(500);
        PORTB &= ~_BV(PORTB4);
        _delay_ms(500);
        i++;
    }
}

//Red LED Check - Test Code
void red(int no){
    int i = 0;
    while( i < no){
        PORTB |= _BV(PORTB3);
        _delay_ms(500);
        PORTB &= ~_BV(PORTB3);
        _delay_ms(500);
        i++;
    }
}

//Door Open Code - Door Opener
void pwm (int door_cont){
    if(door_cont == 0){
        int i = 0;
        while(i < 125){
            PORTD |= _BV(PORTD5);//Close
            _delay_ms(0.5);
            PORTD &= ~_BV(PORTD5);
             _delay_ms(1.5);
             i++;
        }
    }
    else if(door_cont == 1){
        int i = 0;
        while(i < 125){
            PORTD |= _BV(PORTD5);//Open
            _delay_ms(1.5);
            PORTD &= ~_BV(PORTD5);
            _delay_ms(0.5);
            i++;
        }
    }
}

//Door Open Code - Door Close Detection and Closing Code 
void wait_till_close(){
    int turn = 1;
    int angle=0;
    while (turn == 1){
        angle = get_angle();
        if(angle < 173){
            turn = 0;
        }
    }
    turn = 1;
    while (turn == 1){
        angle = get_angle();
        if(angle > 173){
            turn = 0;
        }
    }
    _delay_ms(500);
    pwm(0);
    angle=get_angle();
    if(angle < 173){
        pwm(1);
        PORTD &= ~_BV(PORTD2);//Buzzer
        _delay_ms(2000);
        PORTD |= _BV(PORTD2);//Buzzer
    }
}

//Door Open Code - Main and Buzzer Part
int open_the_door(int result){
        if(result == 1){
            PORTB |= _BV(PORTB2);
            PORTB &= ~_BV(PORTB3);
            
            PORTD &= ~_BV(PORTD2);
            _delay_ms(500);
            PORTD |= _BV(PORTD2);
            pwm(1);
            return 1;
        }
        else{
            PORTD &= ~_BV(PORTD2);
            _delay_ms(15);
            PORTD |= _BV(PORTD2);
            _delay_ms(45);
            PORTD &= ~_BV(PORTD2);
            _delay_ms(15);
            PORTD |= _BV(PORTD2);
            
            PORTB &= ~_BV(PORTB4);
            _delay_ms(500);
            return 0;
        }
}

//Interrupt
ISR(PCINT2_vect)
{
    first_algorithm_count++;
}

int main(void){
	//char buffer[2]; //Uart - Test Code
	int volt = 0;
    int angle=0;
    int open_signal_second_algorithm = 0;
    int result_of_first = 0;
    int count = 0;
    int infinite_loop = 1;
    int secondGo[20]={0};
    int edgeArray_second[19] = {0};
    int door_cont = 0;
  //init_uart(57600); //Uart init - Test Code

    ADC_enable();

    DDRB |= _BV(DDB5);
    DDRD |= _BV(DDD2);
    DDRD |= _BV(DDD5);    
    
    PORTB |= _BV(PORTB5);
    PORTD |= _BV(PORTD2);
    PORTD &= ~_BV(PORTD5);

    setup_sleep();
    
    while (infinite_loop == 1){
    	PORTB &= ~_BV(PORTB0);
    	PORTB &= ~_BV(PORTB1);
    	PORTB &= ~_BV(PORTB2);
      
    	go_sleep();
      /*  angle=get_angle();//Delete
        
        itoa(angle, buffer, 10); //Uart - Test Code
        uart_puts(buffer); //Uart - Test Code
*/
    	PORTB |= _BV(PORTB0);
      
    	first_algorithm_count = 0;
    	result_of_first = go_first();

    	if(result_of_first == 1){
        	PORTD &= ~_BV(PORTD2);
        	_delay_ms(15);
        	PORTD |= _BV(PORTD2);
        
        	volt = get_voltage();
        
        	PORTB |= _BV(PORTB3);
        	PORTB |= _BV(PORTB4);
        
        	PORTB |= _BV(PORTB1);
        
        	int i = 0;
        	for(i=0; i<20 ;i++){
            	first_algorithm_count=0;
            	secondGo[i] = go_second();
        	}
        
        	angle=get_angle();
        
      	 // itoa(angle, buffer, 10); //Uart - Test Code
      	 // uart_puts(buffer); //Uart - Test Code
        
        	count=0;

        	open_signal_second_algorithm = edge(secondGo, 0, edgeArray_second, count, 29, volt);
        
        	int door = 0;
        	if(angle > 22){
          	door = open_the_door(open_signal_second_algorithm);
        	}
        
        	PORTB &= ~_BV(PORTB3);
        	PORTB &= ~_BV(PORTB4);
        	_delay_ms(300);
        
        	if (door == 1){
         	wait_till_close();
        	}
    	}
  	}
  return 0;
}
