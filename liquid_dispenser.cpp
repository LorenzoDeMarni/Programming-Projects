#include <Servo.h>
#include <AFMotor.h>
    
// Pin definitions
#define IRpin A0
#define LIMIT_SWITCH A3

// Motor and Servo declarations
AF_DCMotor motor(1);
AF_DCMotor pump(4);
Servo servo;

// State variables
int IRval;
bool isDispensing = false;

void setup() {
    pinMode(IRpin, INPUT);
    pinMode(LIMIT_SWITCH, INPUT_PULLUP);
    motor.run(RELEASE);
    pump.run(RELEASE);
    servo.attach(9);
    servo.write(0); // Ensure the servo starts in its initial position
    Serial.begin(9600);
}

void loop() {
    IRval = digitalRead(IRpin);
    
    // Detect test tube and start dispensing process
    if (IRval == LOW && !isDispensing) {
        Serial.println("Test tube detected. Starting dispensing process...");
        isDispensing = true;

        // Rotate platform to align test tube
        rotatePlatform();

        // Dispense liquid
        dispenseLiquid();

        // Dispense powder
        dispensePowder();

        isDispensing = false;
        Serial.println("Dispensing process completed.");
    }
}

// Rotate the platform to align the test tube
void rotatePlatform() {
    motor.setSpeed(80); // Set motor speed
    motor.run(FORWARD); // Rotate forward
    delay(1000);        // Allow time for alignment
    motor.run(RELEASE); // Stop motor
    Serial.println("Platform rotated to next test tube.");
}

// Dispense liquid into the test tube
void dispenseLiquid() {
    pump.setSpeed(200); // Set pump speed
    pump.run(FORWARD);  // Activate pump
    delay(3000);        // Pump for 3 seconds
    pump.run(RELEASE);  // Stop pump
    Serial.println("Liquid dispensed.");
}

// Dispense powder into the test tube
void dispensePowder() {
    for (int i = 0; i < 2; i++) { // Dispense twice for accurate amount
        servo.write(30); // Open powder door
        delay(500);      // Wait for powder to drop
        servo.write(0);  // Close powder door
        delay(500);      // Ensure door is closed before next cycle
    }
    Serial.println("Powder dispensed.");
}