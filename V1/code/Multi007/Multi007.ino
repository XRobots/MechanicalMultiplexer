int pot0;     // actual 0
int pot1;     // actual 1
int pot2;     // actual 2
int pot3;     // demand 0
int pot4;     // demand 1
int pot5;     // demand 2

int pot0Prev;
int pot1Prev;
int pot2Prev;

int pot0chg = 0;
int pot1chg = 0;
int pot2chg = 0;

int diff0;    // difference 0
int diff1;    // difference 1
int diff2;    // difference 2

int pos0Flag = 0;
int pos1Flag = 0;
int pos2Flag = 0;

int biggest;

// wheel encoder interrupts

#define encoder0PinA 2      // encoder 1
#define encoder0PinB 3

volatile long encoder0Pos = 0;    // encoder 1
long encoder0Target = 0;
long encoderDiff = 0;

unsigned long currentMillis;
unsigned long previousMillis;

unsigned long currenPotMillis;
unsigned long previousPotMillis;

void setup() {

    pinMode(encoder0PinA, INPUT_PULLUP);    // encoder pins
    pinMode(encoder0PinB, INPUT_PULLUP);

    attachInterrupt(0, doEncoderA, CHANGE);
    attachInterrupt(1, doEncoderB, CHANGE);

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);

    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
  
    Serial.begin(115200);
}

void loop() {

  currentMillis = millis();

    if (currentMillis - previousMillis >= 10) {  // start timed loop
          previousMillis = currentMillis;

          pot0 = analogRead(A0);
          pot0 = map(pot0,0,1023,1023,0);
          pot1 = analogRead(A1);
          pot1 = map(pot1,0,1023,1023,0);
          pot2 = analogRead(A2);
          pot2 = map(pot2,0,1023,1023,0);
          pot3 = analogRead(A3);
          pot4 = analogRead(A4);
          pot5 = analogRead(A5);

          diff0 = pot0 - pot3;
          diff1 = pot1 - pot4;
          diff2 = pot2 - pot5;

          // work out which error is the biggest with some deadspot

          if (abs(diff0) > (abs(diff1)+10) && abs(diff0) > (abs(diff2)+10)) {
              if (pos1Flag == 0 && pos2Flag == 0 && pot1chg == 0 && pot2chg == 0) {
                pos0Flag = 1;
                biggest = 0;
                encoder0Target = 16000;   // move linear axis
              }
          }
          else if (abs(diff1) > (abs(diff0)+10) && abs(diff1) > (abs(diff2)+10)) {
              if (pos0Flag == 0 && pos2Flag == 0 && pot0chg == 0 && pot2chg == 0) {
                pos1Flag = 1;
                biggest = 1;
                encoder0Target = 0;   // move linear axis
              }
          }
          else if (abs(diff2) > (abs(diff0)+10) && abs(diff2) > (abs(diff1)+10)) {
              if (pos0Flag == 0 && pos1Flag == 0 && pot0chg == 0 && pot1chg == 0) {
                pos2Flag = 1;
                biggest = 2;
                encoder0Target = -16000;    // move linear axis
              }
          }

          // check if demand positions have changed since the last loop
          // I am running this slower so there are bigger changes in the pots on each cycle of the loop when I am moving them

          if (currentMillis - previousPotMillis >= 100) {  // start timed loop
            previousPotMillis = currentMillis;

                if (pot0 == pot0Prev) {
                    pot0chg = 0;
                }
                else { pot0chg = 1; }
      
                if (pot1 == pot1Prev) {
                    pot1chg = 0;
                }
                else { pot1chg = 1; }
      
                if (pot2 == pot2Prev) {
                    pot2chg = 0;
                }
                else { pot2chg = 1; }
      
                // boommark 'previous' values so we can compare them
      
                pot0Prev = pot0;
                pot1Prev = pot1;
                pot2Prev = pot2;

          }

          // serial debug

          Serial.print("  zero:  ");
          Serial.print(diff0);
          Serial.print("  one:  ");
          Serial.print(diff1);
          Serial.print("  two:  ");
          Serial.print(diff2);
          Serial.print("     biggest is: ");
          Serial.print(biggest);
          Serial.print("   ");    
          Serial.print(pot1chg);
          Serial.print("   ");      

          // check if the linear axis got to the target yet
          encoderDiff = abs(encoder0Target - encoder0Pos);
          if (encoderDiff < 300) {    // if we got to the target then move the motor that drives the worm gears
            
            if (biggest == 0) {
              Serial.print("arrived at 0");            
              if (diff0 > 0 + 20) {
                analogWrite(6, 255);
                analogWrite(7, 0);
              }
              else if (diff0 < 0 - 20){
                analogWrite(6, 0);
                analogWrite(7, 255);
              }
              else {
                analogWrite(6, 0);
                analogWrite(7, 0);
                pos0Flag = 0;
              }              
            }
            
            else if (biggest == 1) {
              Serial.print("arived at 1");
              if (diff1 > 0 + 20) {
                analogWrite(6, 255);
                analogWrite(7, 0);
              }
              else if (diff1 < 0 - 20){
                analogWrite(6, 0);
                analogWrite(7, 255);
              }
              else {
                analogWrite(6, 0);
                analogWrite(7, 0);
                pos1Flag = 0;
              }
            }
            
            else if (biggest == 2) {
              Serial.print("arrived at 2");
              if (diff2 > 0 + 20){
                analogWrite(6, 255);
                analogWrite(7, 0);
              }
              else if (diff2 < 0 - 20){
                analogWrite(6, 0);
                analogWrite(7, 255);
              }
              else {
                analogWrite(6, 0);
                analogWrite(7, 0);
                pos2Flag = 0;
              }
            }
          }
          
          Serial.println();      
          
          // drive linear axis

          if (encoder0Target < encoder0Pos - 250) {   // allow for a deadzone of 250 encoder counts
              analogWrite(5, 255);
              analogWrite(4, 0);
          }
          else if (encoder0Target > encoder0Pos + 250) {   // allow for a deadzone of 250 encoder counts
              analogWrite(5, 0);
              analogWrite(4, 255);
          }

          else {
            analogWrite(4, 0);
            analogWrite(5, 0);
          }       
         
    }

}

inline int encoder_parity()
{
    if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB)) {
        return 1;
    }
    return -1;
}

void doEncoderA() {
    encoder0Pos += - encoder_parity();
}

void doEncoderB() {
    encoder0Pos += encoder_parity();
}
