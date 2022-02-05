#include <Arduino.h>

// pins - voltage on outputs are ~1.5V so we use the ADC
#define DataPin A0          // yellow
#define ClkPin  A1          // white

#define PinSample 4         // input pin to say measure
#define PinAck 5            // output pin to ack / move to next reading
#define PinDone 6           // input pin to say done

// Dial Indicator resolution: 100 - 0.01mm, 1000 - 0.001mm
#define Resolution 100
// #define Resolution 1000

// UART speed
#define UARTBaudRate 19200


// ADC threshold, ADC values greater than this are interpreted as logical 1, see loop()
#define ADC_Threshold 140

// data format
#define DATA_BITS_LEN 24
#define INCH_BIT 23
#define SIGN_BIT 20
#define START_BIT -1 // -1 - no start bit


// global variables
long counter;

// data capture and decode functions
bool getRawBit() {
    bool data;
    while (analogRead(ClkPin) > ADC_Threshold)
        ;
    while (analogRead(ClkPin) < ADC_Threshold)
        ;
    data = analogRead(DataPin) > ADC_Threshold;
    return data;
}

long getRawData() {
    long out = 0;
    for (int i = 0; i < DATA_BITS_LEN; i++) {
        out |= getRawBit() ? 1L << DATA_BITS_LEN : 0L;
        out >>= 1;
    }
    return out;
}

long getValue(bool &inch) {
    long out = getRawData();
    inch = out & (1L << INCH_BIT);
    bool sign = out & (1L << SIGN_BIT);
    out &= (1L << SIGN_BIT) - 1L;
    out >>= (START_BIT+1);
    if (sign)
        out = -out;
    return out;
}

// printing functions
void printBits(long v) {
    char buf[DATA_BITS_LEN + 1];
    for (int i = DATA_BITS_LEN - 1; i >= 0; i--) {
        buf[i] = v & 1 ? '1' : '0';
        v >>= 1;
    }
    buf[DATA_BITS_LEN] = 0;
    Serial.print(buf);
}

void prettyPrintValue(long value, bool inch, long counter) {
    double v = value;
#if Resolution == 100
    if (inch) {
        Serial.print(v / 2000, 4);
        Serial.print(",1,");
        Serial.println(counter);
    } else {
        Serial.print(v / 100, 2);
        Serial.print(",0,");
        Serial.println(counter);
    }
#else
    if (inch) {
        Serial.print(v / 20000, 5);
        Serial.print(" in");
    } else {
        Serial.print(v / 1000, 3);
        Serial.print(" mm");
    }
#endif
}

// Arduino setup and main loop

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup() {
    // set ADC prescale to 16 (set ADC clock to 1MHz)
    // this gives as a sampling rate of ~77kSps
    sbi(ADCSRA, ADPS2);
    cbi(ADCSRA, ADPS1);
    cbi(ADCSRA, ADPS0);

    Serial.begin(UARTBaudRate);

    pinMode(PinSample, INPUT_PULLUP);               // make sure to set 'active low' in UCCNC for M7
    pinMode(PinDone, INPUT_PULLUP);
    pinMode(PinAck, OUTPUT);

    digitalWrite(PinAck,LOW);

    counter = 0;

    // Serial.println("ready...");
}


void loop() {
    bool inch;
    long value;

    // are we done
    if (digitalRead(PinDone) == LOW){
      Serial.println("done");
      counter = 0;
      delay(1000);
    }

    // take a sample
    // if (true)
    if (digitalRead(PinSample) == LOW)
    {
      counter++;
      digitalWrite(PinAck,HIGH);      // set this up, continues on falling edge

      value = getValue(inch);
      prettyPrintValue(value, inch, counter);
      delay(100);                     // make sure this is more than the wait delay in the gcode, 2x is safe

      digitalWrite(PinAck, LOW);
    }


    // uncomment if you are interested in optimal ADC_Threshold value
    //Serial.print(' '); Serial.print(analogRead(ClkPin)/2);

    // uncomment if you are interested in raw data bits
    //Serial.print(' '); printBits(getRawData());

    // uncomment if you are interested in raw data
    //Serial.print(' '); Serial.print(value);
}