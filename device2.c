#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <avr/io.h> 
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <LiquidCrystal.h>

#define SET_BIT(reg, pin)           (reg) |= (1 << (pin))
#define CLEAR_BIT(reg, pin)         (reg) &= ~(1 << (pin))
#define WRITE_BIT(reg, pin, value)  (reg) = (((reg) & ~(1 << (pin))) | ((value) << (pin)))
#define BIT_VALUE(reg, pin)         (((reg) >> (pin)) & 1)
#define BIT_IS_SET(reg, pin)        (BIT_VALUE((reg),(pin))==1)

# define F_CPU 16000000UL
# define MYBAUD 19200
# define MYUBRR F_CPU / 16 / MYBAUD - 1

# define FSH const __FlashStringHelper*


// Debouncing variables
volatile uint8_t bit_counter1 = 0;
volatile uint8_t bit_counter2 = 0;
volatile uint8_t bit_counter3 = 0;
volatile uint8_t bit_counter4 = 0;

volatile uint8_t switch_closed1 = 0;
volatile uint8_t switch_closed2 = 0;
volatile uint8_t switch_closed3 = 0;
volatile uint8_t switch_closed4 = 0;

volatile uint8_t game_over = 0;


// Game variables
char x_ans_buff[15];
char y_ans_buff[15];
char z_ans_buff[15];

char x_buffer[55];
char y_buffer[55];
char z_buffer[55];

int counter = 0;

char coords[256];

const int PROGMEM game_coords[3][3] = { 
  {9,6,8}, 
  {-4,7,-2}, 
  {5,-3,8}
};


// UART definitions
#define  RX_BUFFER_SIZE  64
#define  TX_BUFFER_SIZE  64

// UART variables
unsigned char rx_buf;

static volatile uint8_t tx_buffer[TX_BUFFER_SIZE];
static volatile uint8_t tx_buffer_head;
static volatile uint8_t tx_buffer_tail;
static volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_buffer_head;
static volatile uint8_t rx_buffer_tail;

volatile int x = 0;
volatile int y = 0;
volatile int z = 0;

volatile int x_ans;
volatile int y_ans;
volatile int z_ans;

volatile int coords_1 = 1;
volatile int coords_2 = 0;

volatile int coord_state = 0;
volatile int coord_state_counter = 0;

LiquidCrystal lcd(9, 8, 13, 12, 11, 10);

// UART Functions
void uart_init(unsigned int ubrr){
 
    UBRR0H = (unsigned char)(ubrr >>8);
    UBRR0L = (unsigned char)(ubrr);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C =(3 << UCSZ00);

}
 
void uart_putchar(unsigned char data){
 
  while (!( UCSR0A & (1 << UDRE0))); /* Wait for empty transmit buffer*/
 
  UDR0 = data;            /* Put data into buffer, sends the data */
                                    
}

unsigned char uart_getchar(void){
   
  // If the flag isn't set to be received 
  if (!(UCSR0A & (1<< RXC0))) {
    return 0;
  }
   
  return UDR0;     
}

// ISR functions
// Time interrupt
ISR (PCINT2_vect) {

  if (y == y_ans && x == x_ans && z == z_ans) {
    // if the user has won    
    game_over = 1;
  } else {
    game_over = 0;
  }  

}

// Debouncing interrupt
ISR (TIMER2_OVF_vect) {

  uint8_t mask = 0b00011111;

  // up button
  unsigned char bit_state1 = BIT_IS_SET(PIND, 7); 
  // left button
  unsigned char bit_state2 = BIT_IS_SET(PIND, 2); 
  // right button
  unsigned char bit_state3 = BIT_IS_SET(PIND, 5);
  // down button
  unsigned char bit_state4 = BIT_IS_SET(PIND, 4); 

  bit_counter1 = ((bit_counter1 << 1) & mask ) | bit_state1 ;
  bit_counter2 = ((bit_counter2 << 1) & mask ) | bit_state2 ;
  bit_counter3 = ((bit_counter3 << 1) & mask ) | bit_state3 ;
  bit_counter4 = ((bit_counter4 << 1) & mask ) | bit_state4 ;

  if (bit_counter1 == mask) {
    switch_closed1 = 1;
    
  } else if(bit_counter1 == 0) {
    switch_closed1 = 0;
  }

  if (bit_counter2 == mask) {
    switch_closed2 = 1;
  } else if(bit_counter2 == 0) {
    switch_closed2 = 0;
  }

  if (bit_counter3 == mask) {
    switch_closed3 = 1;
  } else if(bit_counter3 == 0) {
    switch_closed3 = 0;
  }

  if (bit_counter4 == mask) {
    switch_closed4 = 1;
  } else if(bit_counter4 == 0) {
    switch_closed4 = 0;
  }

}

// Game functions
int hasWon(int x, int y, int z, int x_ans, int y_ans, int z_ans) {

  if (y == y_ans && x == x_ans && z == z_ans) {
    return 1;
  } else {
    return 0;
  }

}

// LED functions
int includeZDiff(int z_diff, int difference) {
    // difference is a number between 250 - 0 decrementing in 50 intervals
  	
  	switch(z_diff) {
        case 0:
            return difference;
            break;

        case 1:
            if (!(difference - 10 < 0)) {
                return difference - 10;
            } else {
                return 0;
            }
            break;

        case 2:
            if (!(difference - 20 < 0)) {
                return difference - 20;
            } else {
                return 0;
            }
            break;

        case 3:
            if (!(difference - 30 < 0)) {
                return difference - 30;
            } else {
                return 0;
            }
            break;

        case 4:
            if (!(difference - 40 < 0)) {
                return difference - 40;
            } else {
                return 0;
            }
            break;

        case 5:
            if (!(difference - 50 < 0)) {
                return difference - 50;
            } else {
                return 0;
            }
            break;

        case 6:
            if (!(difference - 60 < 0)) {
                return difference - 60;
            } else {
                return 0;
            }
            break;

        case 7:
            if (!(difference - 70 < 0)) {
                return difference - 70;
            } else {
                return 0;
            }
            break;

        case 8:
            if (!(difference - 80 < 0)) {
                return difference - 80;
            } else {
                return 0;
            }
            break;

        case 9:
            if (!(difference - 90 < 0)) {
                return difference - 90;
            } else {
                return 0;
            }
            break;

        
        case 10:
            if (!(difference - 100 < 0)) {
                return difference - 100;
            } else {
                return 0;
            }
            break;

        case 11:
            if (!(difference - 110 < 0)) {
                return difference - 110;
            } else {
                return 0;
            }
            break;

        case 12:
            if (!(difference - 120 < 0)) {
                return difference - 120;
            } else {
                return 0;
            }
            break;

        case 13:
            if (!(difference - 130 < 0)) {
                return difference - 130;
            } else {
                return 0;
            }
            break;

        case 14:
            if (!(difference - 140 < 0)) {
                return difference - 140;
            } else {
                return 0;
            }
            break;

        case 15:
            if (!(difference - 150 < 0)) {
                return difference - 150;
            } else {
                return 0;
            }
            break;

        case 16:
            if (!(difference - 160 < 0)) {
                return difference - 160;
            } else {
                return 0;
            }
            break;

        case 17:
            if (!(difference - 170 < 0)) {
                return difference - 170;
            } else {
                return 0;
            }
            break;

        case 18:
            if (!(difference - 180 < 0)) {
                return difference - 180;
            } else {
                return 0;
            }
            break;
            
    }
}

int getBrightness(int x1, int y1, int z1, int x2, int y2, int z2) {

    int a,b;

    if (x1 > x2) {
        a = x1 - x2;
    } else {
        a = x2 - x1;
    }

    if (y1 > y2) {
        b = y1 - y2;
    } else {
        b = y2 - y1;
    }

    double temp = (double)a + b;

    int difference = sqrt( temp );

    int z_diff;

    if (z1 < z2) {
        z_diff = z2 - z1;
    } else {
        z_diff = z1 - z2;
    }

    switch(difference) {
        // closest
        case 0:
            return includeZDiff(z_diff, 250);
            break;

        // closer
        case 1:
            return includeZDiff(z_diff, 200);
            break;

        // close
        case 2:
            return includeZDiff(z_diff, 150);
            break;
        // far
        case 3:
            return includeZDiff(z_diff, 100);
            break;

        // further
        case 4:
            return includeZDiff(z_diff, 50);
            break;

        // furthest away
        case 5:
            return includeZDiff(z_diff, 0);
            break;
    }

    return 0;

}

// LCD functions
void printScreen(const char msg1[], const char msg2[]) {
 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(msg1);
  lcd.setCursor(0,1);
  lcd.print(msg2);
  
}

// Setup
void setup(void){     

    CLEAR_BIT(DDRD, 7);  // setting D7 to read input
    CLEAR_BIT(DDRD, 2);  // setting D6 to read input
    CLEAR_BIT(DDRD, 5);  // setting D5 to read input
    CLEAR_BIT(DDRD, 4);  // setting D4 to read input

    CLEAR_BIT(DDRD, 3);  // setting D3 to read input

    SET_BIT(DDRD, 6);    // setting D2 to output

    TCCR2A = 0;
    TCCR2B = 4;

    TIMSK2 = 1;

  	// set up the LCD in 4-pin or 8-pin mode
  	lcd.begin(16,2);
 
    OCR0A = 0; // set PWM for 50% duty cycle
    
 
    TCCR0A |= (1 << COM0A1); // set none-inverting mode
    
 
    // TinkerCAD Errata: timer clocking must be enabled before WGM
    // set prescaler to 8 and starts PWM
    TCCR0B = (1 << CS01);
 
    TCCR0A |= (1 << WGM01) | (1 << WGM00);

    uart_init(MYUBRR);

    // initialise adc
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
 
    ADMUX = (1 << REFS0);

    // Setting up the pin change interrupt
    SET_BIT(PCMSK2, PCINT19);
    SET_BIT(PCICR, PCIE2);


    // Setting up the timer to change the coordinates
    TCCR1A = 0;
    TCCR1B = 4;

    sei();               // enabling timer overflow   
}

int main(void) {      
    setup();      
  	x_ans = game_coords[0][0];  
  	y_ans = game_coords[0][1];
  	z_ans = game_coords[0][2];
  
    // determining how bright the LED needs to be from origin
  	OCR0A = getBrightness(0,0,0,x_ans,y_ans,z_ans);

    // getting the x y z coordinates as strings
  	itoa(x_ans, x_ans_buff, 10);
  	itoa(y_ans,y_ans_buff,10);  
    itoa(z_ans, z_ans_buff, 10);

    uint8_t prevState1= 0;
  	uint8_t prevState2= 0;
  	uint8_t prevState3= 0;
  	uint8_t prevState4= 0;
  
  	uint8_t prevState5= 0;

    // converting the coordinates to strings to be printed to the LCD
    itoa(x, x_buffer, 10);
    itoa(y, y_buffer, 10);
    itoa(z, z_buffer, 10);
                  
    strcpy(coords, "{");
    strcat(coords, x_buffer);
    strcat(coords, ",");
    strcat(coords, y_buffer);
    strcat(coords, ",");
    strcat(coords, z_buffer);
    strcat(coords, "}");
          
    printScreen("Find the coords", coords);
    
    uint16_t prev_pot = -10;
     
  	while (1) {	

      double time = (double) TCNT1 * 256.0  / 16000000.0;

      if(time > 1) {
        TCNT1 = 0;
        counter++;
      }

      if (counter == 15) {

        x_ans = game_coords[1][0];
      
        y_ans = game_coords[1][1];

        z_ans = game_coords[1][2];

        OCR0A = getBrightness(x,y,z,x_ans,y_ans,z_ans);

        printScreen("Coordinates have", "changed!");
        _delay_ms(1000);

        printScreen("Find the coords", coords);
            
      } else if (counter == 30) {

        x_ans = game_coords[2][0];
      
        y_ans = game_coords[2][1];

        z_ans = game_coords[2][2];

        OCR0A = getBrightness(x,y,z,x_ans,y_ans,z_ans);

        printScreen("Coordinates have", "changed!");
        _delay_ms(1000);

        printScreen("Find the coords", coords);

      }  else if (counter == 45) {
          printScreen("Game over!", "You lost :(");
          break;
      } 

      // check to see if we have received a 'w' char
      //   from the other player (meaning they have lost)    
      unsigned char data_received = uart_getchar();
      if (data_received != 0) {
        rx_buf = data_received;
      }
      
      if (rx_buf == 'w'){
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Game Over!");
        lcd.setCursor(0,1);
        lcd.print("You Lost :(");
        return 0;
      }  

      // check to see if this player has won
      if (game_over) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Game Over!");
        lcd.setCursor(0,1);
        lcd.print("You Won!");

        uart_putchar('w');             

        return 0;
      } 
            
      // process the adc first
      ADCSRA |= (1 << ADSC);

      while ( ADCSRA & (1 << ADSC) ) {}

      uint16_t pot = ADC;     
      
      if (pot != prev_pot) {

        prev_pot = pot;

        if (pot <= 1000) {
          z = 9;
        }

        if (pot <= 950) {
          z = 8;
        }

        if (pot <= 900) {
          z = 7;
        }  

        if (pot <= 850) {
          z = 6;
        }  

        if (pot <= 800) {
          z = 5;
        }

        if (pot <= 750) {
          z = 4;
        }

        if (pot <= 700) {
          z = 3;
        }

        if (pot <= 650) {
          z = 2;
        }

        if (pot <= 600) {
          z = 1;
        }

        if (pot <= 550) {
          z = 0;
        }

        if (pot <= 500) {
          z = -1;
        }

        if (pot <= 450) {
          z = -2;
        }

        if (pot <= 400) {
          z = -3;
        }        

        if (pot <= 350) {
          z = -4;
        }

        if (pot <= 300) {
          z = -5;
        }

        if (pot <= 250) {
          z = -6;
        }

        if (pot <= 200) {
          z = -7;
        }

        if (pot <= 150) {
          z = -8;
        }    

        if (pot <= 100) {
          z = -9;
        }     

        

        itoa(x, x_buffer, 10);
        itoa(y, y_buffer, 10);
        itoa(z, z_buffer, 10);
                
        strcpy(coords, "{");
        strcat(coords, x_buffer);
        strcat(coords, ",");
        strcat(coords, y_buffer);
        strcat(coords, ",");
        strcat(coords, z_buffer);
        strcat(coords, "}");
        
        printScreen("Find the coords", coords);
        OCR0A = getBrightness(x,y,z,x_ans,y_ans,z_ans);
        
      }      

      	// Up button
        if ( !switch_closed1 && ( switch_closed1 != prevState1 ) ) {            
          
          if (y != 9) {
            
          	y++; 
            
            OCR0A = getBrightness(x,y,z,x_ans,y_ans,z_ans);
                        
          }          
          
          itoa(x, x_buffer, 10);
          itoa(y, y_buffer, 10);
          itoa(z, z_buffer, 10);
                  
          strcpy(coords, "{");
          strcat(coords, x_buffer);
          strcat(coords, ",");
          strcat(coords, y_buffer);
          strcat(coords, ",");
          strcat(coords, z_buffer);
          strcat(coords, "}");
          
          printScreen("Find the coords", coords);
          
          prevState1 = switch_closed1;
        } else {
          prevState1 = switch_closed1;
        }
      
		    // Left
        if ( !switch_closed2 && ( switch_closed2 != prevState2 ) ) {            
          
          if (x != -9) {
          	x--;
            OCR0A = getBrightness(x,y,z,x_ans,y_ans,z_ans);
          }      
          
          itoa(x, x_buffer, 10);
          itoa(y, y_buffer, 10);
          itoa(z, z_buffer, 10);
                  
          strcpy(coords, "{");
          strcat(coords, x_buffer);
          strcat(coords, ",");
          strcat(coords, y_buffer);
          strcat(coords, ",");
          strcat(coords, z_buffer);
          strcat(coords, "}");
          
          printScreen("Find the coords", coords);
          prevState2 = switch_closed2;
        }else {
          prevState2 = switch_closed2;
        }
		
      	// Right
        if ( !switch_closed3 && ( switch_closed3 != prevState3 ) ) {            
          
          if (x != 9) { 
            x++; 
            OCR0A = getBrightness(x,y,z,x_ans,y_ans,z_ans);
          } 
          
          itoa(x, x_buffer, 10);
          itoa(y, y_buffer, 10);
          itoa(z, z_buffer, 10);
                  
          strcpy(coords, "{");
          strcat(coords, x_buffer);
          strcat(coords, ",");
          strcat(coords, y_buffer);
          strcat(coords, ",");
          strcat(coords, z_buffer);
          strcat(coords, "}");
          
          printScreen("Find the coords", coords);
          prevState3 = switch_closed3;
        } else {
          prevState3 = switch_closed3;
        }
		
      	// Bottom
        if ( !switch_closed4 && ( switch_closed4 != prevState4 ) ) {            
          if (y != -9) { 
            y--; 
            OCR0A = getBrightness(x,y,z,x_ans,y_ans,z_ans);
          } 
          
          itoa(x, x_buffer, 10);
          itoa(y, y_buffer, 10);
          itoa(z, z_buffer, 10);
                  
          strcpy(coords, "{");
          strcat(coords, x_buffer);
          strcat(coords, ",");
          strcat(coords, y_buffer);
          strcat(coords, ",");
          strcat(coords, z_buffer);
          strcat(coords, "}");
          
          printScreen("Find the coords", coords);
          prevState4 = switch_closed4;
        } else {
          prevState4 = switch_closed4;
        }
      	      
    }
  
  	return 0;
          
}