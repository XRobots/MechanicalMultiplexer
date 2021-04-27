int pot[6]{};

struct {
  int prev{};
  int chg{};
} pots[3];

struct position {
  int flag{};
  const long value{};

  position(long v) : value{v} {}

} pos[3]{{16000}, {0}, {-16000}};

int diff[3]{};

int biggest{};

// wheel encoder interrupts

constexpr int encoder0PinA{2};
constexpr int encoder0PinB{3};

volatile long encoder0Pos = 0; // encoder 1
long encoder0Target = 0;
long encoderDiff = 0;

unsigned long currentMillis;
unsigned long previousMillis;

unsigned long currenPotMillis;
unsigned long previousPotMillis;

inline bool check_flag(int i) {
  return (pos[i].flag == 0) && (pots[i].chg == 0);
}

inline bool check_difference(int i) {
  const int target = abs(diff[i]);
  return ((i == 0) || ((target - abs(diff[0])) > 10)) &&
         ((i == 1) || ((target - abs(diff[1])) > 10)) &&
         ((i == 2) || ((target - abs(diff[2])) > 10));
}

inline int encoder_parity() {
  if (digitalRead(encoder0PinA) == digitalRead(encoder0PinB)) {
    return 1;
  }
  return -1;
}

void doEncoderA() { encoder0Pos += -encoder_parity(); }

void doEncoderB() { encoder0Pos += encoder_parity(); }

void setup() {

  pinMode(encoder0PinA, INPUT_PULLUP); // encoder pins
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

  if (currentMillis - previousMillis >= 10) { // start timed loop
    previousMillis = currentMillis;

    pot[0] = analogRead(A0);
    pot[0] = map(pot[0], 0, 1023, 1023, 0);
    pot[1] = analogRead(A1);
    pot[1] = map(pot[1], 0, 1023, 1023, 0);
    pot[2] = analogRead(A2);
    pot[2] = map(pot[2], 0, 1023, 1023, 0);
    pot[3] = analogRead(A3);
    pot[4] = analogRead(A4);
    pot[5] = analogRead(A5);

    diff[0] = pot[0] - pot[3];
    diff[1] = pot[1] - pot[4];
    diff[2] = pot[2] - pot[5];

    // work out which error is the biggest with some deadspot

    for (int i{0}; i < 3; i++) {
      if (check_difference(i)) {
        if (((i == 0) || check_flag(0)) && ((i == 1) || check_flag(1)) &&
            ((i == 2) || check_flag(2))) {
          pos[i].flag = 1;
          biggest = i;
          encoder0Target = pos[i].value; // move linear axis
        }
        break;
      }
    }

    // check if demand positions have changed since the last loop
    // I am running this slower so there are bigger changes in the pots on each
    // cycle of the loop when I am moving them

    if (currentMillis - previousPotMillis >= 100) { // start timed loop
      previousPotMillis = currentMillis;

      for (int i{0}; i < 3; i++) {
        pots[i].chg = (pot[i] == pots[i].prev) ? 0 : 1;
        pots[i].prev = pot[i];
      }
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
    if (encoderDiff < 300) { // if we got to the target then move the motor that
                             // drives the worm gears
      for (int i{0}; i < 3; i++) {
        if (biggest != i) {
          continue;
        }
        Serial.print("arrived at 0");
        if (diff[i] > 0 + 20) {
          analogWrite(6, 255);
          analogWrite(7, 0);
        } else if (diff[i] < 0 - 20) {
          analogWrite(6, 0);
          analogWrite(7, 255);
        } else {
          analogWrite(6, 0);
          analogWrite(7, 0);
          pos[i].flag = 0;
        }
        break;
      }
    }

    Serial.println();

    // drive linear axis

    if (encoder0Target <
        encoder0Pos - 250) { // allow for a deadzone of 250 encoder counts
      analogWrite(5, 255);
      analogWrite(4, 0);
    } else if (encoder0Target >
               encoder0Pos +
                   250) { // allow for a deadzone of 250 encoder counts
      analogWrite(5, 0);
      analogWrite(4, 255);
    }

    else {
      analogWrite(4, 0);
      analogWrite(5, 0);
    }
  }
}
