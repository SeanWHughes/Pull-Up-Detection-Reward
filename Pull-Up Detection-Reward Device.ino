

//defining variables for modes & tracking
int count = 0;        //keeps track of pull-ups done in a set
int toggle = 0;       //toggles between normal mode (5 reps/candy) & hard mode (10 reps/candy)
volatile int period;  //period of the waveform detected by the color sensor
volatile int timer1;  //time when wave goes from high to low in ticks

ISR(INT1_vect)  // code that runs when external interupt 1 is triggered
{
    if(PIND & 0b00001000) //if color sensor's output (pin 3) is high
    {
        TCNT1 = 0;        //reset the timer
    }
    else
    {
        timer1 = TCNT1;   //store current timer value
    }
}

void getColor()               //pings interrupt to sense color @ current moment
{
    //timer1 setup
    TCCR1A = 0b00000000;      //normal mode
    TCCR1B = 0b00000001;      //normal mode, prescaler = 1
    
    EIMSK |= 0b00000010;      //enable external interrupt for INT1 (pin 3)
    _delay_ms(5);             //delay to give interrupt time to trigger
    EIMSK &= 0b11111101;      //disable external interrupt for INT1 (pin 3)
    period = timer1*0.0625*2; //converting ticks to microseconds
}

void servo()  //actuates servo
{
    //timer 1 registers for PWM
    TCCR1A = 0b10000000;   //timer mode 8, clear OC1A on compare match (set output to low level)
    TCCR1B = 0b00010010;   //timer mode 8, prescaler = 8
    ICR1 = 20000;         //setting TOP value to give the timer a period of 20 milliseconds (50Hz)  
    OCR1A = 900;          //set compare register to start @ 900 in order to get pulse width = 0.9 ms   
    
    while(OCR1A<2100){
          OCR1A++;          //starts off increasing OCR1A up to 2100 micro secs
          _delay_ms(2);
          }
    while(OCR1A>900){
          OCR1A--;          //after reaching 2100 microsecs, decreases OCR1A down to 900 micro secs
          _delay_ms(2);
          }
}

int main(void)
{
    Serial.begin(9600);
    DDRD = 0b11110011;      //setting button & color sensor as inputs into arduino (bit 3,2 = low)
    DDRB = 0b00000010;      //setting micro servo as an output for the arduino (bit 1 = high)

    //color sensor registers
    EICRA |= 0b00000100;  //enable external interrupt for any logical change on pin 3
    sei();                //enable interupts globally  

    while(1)
    {
        getColor();         //gets color from color sensor
        if(200 < period && period < 280)    //checking for green color (thresholds experimentally determined)
        {
          _delay_ms(1000);  //wait 1.5 second under tension
          if(200 < period && period < 280)  //checking if user is still under tension
          {
            count = count+1;  //marking that a good pull up was done
            if(count%5 == 0 && toggle == 0)   //if sensor measures 5 good pull ups & normal mode activated
            {   
                servo();  //actuate servo
            }
            if(count%10 == 0 && toggle == 1)  //if sensor measures 10 good pull ups & hard mode activated
            {   
                servo();  //actuate servo
            }
          }
        }        
        else  //if color is not being detected then instead poll for button press
        {
            if(PIND & 0b00000100){ //check if button is not pushed
            }   //do nothing
            else                  //check if button is pushed
            {                  
                toggle ^= 1;      //toggles from normal mode (0) to hard mode (1) or vice versa
                count = 0;        //resets pull-up count to avoid miscounts
            }
        }
        _delay_ms(10);           //check color only every 0.25s
    }
}
