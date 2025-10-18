// L293D
// Motor A
const int motorPin1 = 5;  // Pin 14 of L293
const int motorPin2 = 6;  // Pin 10 of L293
const int enableA = 3;    // Enable pin for Motor A

// Motor B
const int motorPin3 = 10; // Pin  7 of L293
const int motorPin4 = 9;  // Pin  2 of L293
const int enableB = 8;    // Enable pin for Motor B

// US sensor
const int trigPin = 11;
const int echoPin = 12;

// This will run only one time.
void setup() {
    Serial.begin(9600);

    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    // Set motor pins as outputs
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(motorPin3, OUTPUT);
    pinMode(motorPin4, OUTPUT);

    // Set enable pins as outputs
    pinMode(enableA, OUTPUT);
    pinMode(enableB, OUTPUT);
}

void forwards(int speed) {
    speed = constrain(speed, 0, 255);

    analogWrite(enableA, speed);
    analogWrite(enableB, speed);
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    // Motor B
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin4, LOW);
}

void stop() {
    // Stop PWM signal
    analogWrite(enableA, 0);
    analogWrite(enableB, 0);
    
    // Stop motor movement
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
}

void loop() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH);
    long distance = duration * 0.034 / 2;

    if (distance < 10) {
        forwards(10);
        Serial.println("Stopped");
    } else {
        forwards(255); // Full speed
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println(" cm");
    }

    delay(10);
}
