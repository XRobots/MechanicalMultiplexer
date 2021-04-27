int pot[6] {};

struct {
    int prev {};
    int chg {};
} pots [3];

struct position{
    int flag {};
    const long value {};

    position(long v)
     : value { v }
     {}

} pos [3] {{16000}, {0}, {-16000}};


int diff[3] {};

int biggest {};

// wheel encoder interrupts

constexpr int encoder0PinA { 2 };
constexpr int encoder0PinB { 3 };

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

          pot[0] = analogRead(A0);
          pot[0] = map(pot[0],0,1023,1023,0);
          pot[1] = analogRead(A1);
          pot[1] = map(pot[1],0,1023,1023,0);
          pot[2] = analogRead(A2);
          pot[2] = map(pot[2],0,1023,1023,0);
          pot[3] = analogRead(A3);
          pot[4] = analogRead(A4);
          pot[5] = analogRead(A5);

          diff[0] = pot[0] - pot[3];
          diff[1] = pot[1] - pot[4];
          diff[2] = pot[2] - pot[5];

          // work out which error is the biggest with some deadspot

          if (abs(diff[0]) > (abs(diff[1])+10) && abs(diff[0]) > (abs(diff[2])+10)) {
              if (pos[1].flag == 0 && pos[2].flag == 0 && pots[1].chg == 0 && pots[2].chg == 0) {
                pos[0].flag = 1;
                biggest = 0;
                encoder0Target = pos[0].value;   // move linear axis
              }
          }
          else if (abs(diff[1]) > (abs(diff[0])+10) && abs(diff[1]) > (abs(diff[2])+10)) {
              if (pos[0].flag == 0 && pos[2].flag == 0 && pots[0].chg == 0 && pots[2].chg == 0) {
                pos[1].flag = 1;
                biggest = 1;
                encoder0Target = pos[1].value;   // move linear axis
              }
          }
          else if (abs(diff[2]) > (abs(diff[0])+10) && abs(diff[2]) > (abs(diff[1])+10)) {
              if (pos[0].flag == 0 && pos[1].flag == 0 && pots[0].chg == 0 && pots[1].chg == 0) {
                pos[2].flag = 1;
                biggest = 2;
                encoder0Target = pos[2].value;    // move linear axis
              }
          }

          // check if demand positions have changed since the last loop
          // I am running this slower so there are bigger changes in the pots on each cycle of the loop when I am moving them

          if (currentMillis - previousPotMillis >= 100) {  // start timed loop
            previousPotMillis = currentMillis;

                if (pot[0] == pots[0].prev) {
                    pots[0].chg = 0;
                }
                else { pots[0].chg = 1; }
      
                if (pot[1] == pots[1].prev) {
                    pots[1].chg = 0;
                }
                else { pots[1].chg = 1; }
      
                if (pot[2] == pots[2].prev) {
                    pots[2].chg = 0;
                }
                else { pots[2].chg = 1; }
      
                // boommark 'previous' values so we can compare them
      
                pots[0].prev = pot[0];
                pots[1].prev = pot[1];
                pots[2].prev = pot[2];

          }

          // serial debug

          Serial.print("  zero:  ");
          Serial.print(diff[0]);
          Serial.print("  one:  ");
          Serial.print(diff[1]);
          Serial.print("  two:  ");
          Serial.print(diff[2]);
          Serial.print("     biggest is: ");
          Serial.print(biggest);
          Serial.print("   ");    
          Serial.print(pots[1].chg);
          Serial.print("   ");      

          // check if the linear axis got to the target yet
          encoderDiff = abs(encoder0Target - encoder0Pos);
          if (encoderDiff < 300) {    // if we got to the target then move the motor that drives the worm gears
            
            if (biggest == 0) {
              Serial.print("arrived at 0");            
              if (diff[0] > 0 + 20) {
                analogWrite(6, 255);
                analogWrite(7, 0);
              }
              else if (diff[0] < 0 - 20){
                analogWrite(6, 0);
                analogWrite(7, 255);
              }
              else {
                analogWrite(6, 0);
                analogWrite(7, 0);
                pos[0].flag = 0;
              }              
            }
            
            else if (biggest == 1) {
              Serial.print("arived at 1");
              if (diff[1] > 0 + 20) {
                analogWrite(6, 255);
                analogWrite(7, 0);
              }
              else if (diff[1] < 0 - 20){
                analogWrite(6, 0);
                analogWrite(7, 255);
              }
              else {
                analogWrite(6, 0);
                analogWrite(7, 0);
                pos[1].flag = 0;
              }
            }
            
            else if (biggest == 2) {
              Serial.print("arrived at 2");
              if (diff[2] > 0 + 20){
                analogWrite(6, 255);
                analogWrite(7, 0);
              }
              else if (diff[2] < 0 - 20){
                analogWrite(6, 0);
                analogWrite(7, 255);
              }
              else {
                analogWrite(6, 0);
                analogWrite(7, 0);
                pos[2].flag = 0;
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
