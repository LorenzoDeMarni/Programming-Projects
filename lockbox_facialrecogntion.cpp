#include WiFiNINA.h;   
#include SPI.h;
#include MFRC522.h;                         


// Define pins for the relay
#define RELAY_PIN 8   // Relay connected to digital pin 8

// Define pins for the ultrasonic sensor
#define TRIG_PIN 5    // Trigger pin of ultrasonic sensor
#define ECHO_PIN 6    // Echo pin of ultrasonic sensor

// Define pins for RFID
#define SS_PIN 10     // SDA Pin for RFID
#define RST_PIN 9     // RST Pin for RFID

// WiFi credentials
const char* ssid = "_____"; 
const char* pass = "_____"; 

// Blynk Auth Token
const char* auth = "_____";

// Blynk Server
const char* server = "blynk.cloud";
const int port = 80;

// Virtual Pin to control the LED
const char* virtualPin = "V0"; 
const char* virtualPin2 = "V1";

int status = WL_IDLE_STATUS;

// RFID instance
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// Define the UID (Unique Identifier) of your RFID tag or fob
byte allowedUID1[] = {0xB3, 0x7B, 0x55, 0x4F};  // Example UID, replace with your own
byte allowedUID2[] = {0x13, 0x18, 0x11, 0xFE};  // Example UID, replace with your own

void setup() {
    // Start Serial communication
    Serial.begin(9600);

    // Initialize the relay pin as OUTPUT
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);  // Ensure the relay is ON (locked)

    // Initialize the ultrasonic sensor pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    // Initialize SPI and MFRC522
    SPI.begin();
    mfrc522.PCD_Init();

    Serial.println("Place your RFID tag near the reader...");

    // Wait for the Serial Monitor to connect
    while (!Serial) {
    ; // Wait for serial port to open
    }

    // Check WiFi module
    if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
    }

    // Connect to WiFi
    Serial.print("Connecting to WiFi");
    while (status != WL_CONNECTED) {
    Serial.print(".");
    status = WiFi.begin(ssid, pass);
    delay(5000); // Retry every 5 seconds
    }
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
    // Check RFID access
    checkRFID();
    // Check distance with the ultrasonic sensor
    long distance = getDistance();

    //If an object is detected within range, send an update to Blynk
    if (distance > 0 && distance <= 50) { // Adjust the range threshold as needed
    Serial.println("Object detected within range!");
    sendToBlynk(1); // Turn on the virtual pin
    
    } 
    WiFiClient client;
    // Connect to Blynk server
    if (!client.connect(server, port)) {
    Serial.println("Connection to Blynk server failed");
    
    delay(500);
    return;
    }

    // Construct the GET request to read the virtual pin
    client.print(String("GET /external/api/get?token=") + auth +
                "&" + virtualPin2 + " HTTP/1.1\r\n" +
                "Host: " + server + "\r\n" +
                "Connection: close\r\n\r\n");

    // Wait for the response
    bool responseParsed = false;
    while (client.connected() || client.available()) {

    if (client.available()) {
        String line = client.readStringUntil('\n');
        //Serial.println("Response Line: " + line);

        // Look for the actual response body (value)
        if (line.length() == 1 && !responseParsed) { 
        // The next line after an empty line contains the actual value
        String valueLine = client.readStringUntil('\n');
        int value = valueLine.toInt(); // Convert to integer
        //Serial.println("Parsed Value: " + String(value));

        // Set pin 8 based on the value
        if (value == 0) {
            Serial.println("Pin 8 is OFF");
            unlockSolenoid(); // Turn off pin 8
            delay(5000);
            sendToBlynk2(1);
            lockSolenoid();

        } else {
            Serial.println("Pin 8 is ON");
            lockSolenoid();
        }

        // Print the state of pin 8
        int pinState = digitalRead(RELAY_PIN); // Read the current state of pin 8
        // Serial.print("Current State of Pin 8: ");
        // Serial.println(pinState == HIGH ? "ON" : "OFF");

        responseParsed = true; // Ensure we only parse once
        }
    }
    }

    // Close the connection
    client.stop();
    
    
    // else {
    //   sendToBlynk(0); // Turn off the virtual pin
    
    // }

    
    // Wait before the next API call
    delay(1000); // Poll every 5 seconds
    // Delay between measurements
}

void checkRFID() {
    // Check if an RFID tag is present
    if (!mfrc522.PICC_IsNewCardPresent()) {
    return;  // If no new card, do nothing and keep looping
    }

    // Read the card
    if (!mfrc522.PICC_ReadCardSerial()) {
    return;  // If reading fails, do nothing and keep looping
    }

    // Print the UID to Serial Monitor (for debugging)
    //Serial.print("Tag UID:");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" 0x");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println();

    // Compare the read UID with the allowed UID
    if (compareUID(mfrc522.uid.uidByte, mfrc522.uid.size, allowedUID1, sizeof(allowedUID1)) ||
        compareUID(mfrc522.uid.uidByte, mfrc522.uid.size, allowedUID2, sizeof(allowedUID2))) {
    Serial.println("Access Granted using RFID!");
    
    unlockSolenoid();  // Unlock the solenoid and then loop to wait for a new read
    
    } else {
    Serial.println("Access Denied using RFID!");
    }

    // Halt the RFID reading process
    mfrc522.PICC_HaltA();
}

// Function to get distance from the ultrasonic sensor
long getDistance() {
    long duration, distance;

    // Send a 10us pulse to the TRIG pin
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Measure the response time from the ECHO pin
    duration = pulseIn(ECHO_PIN, HIGH);

    // Calculate the distance in centimeters
    distance = (duration / 2) / 29.1; 

    Serial.print("Measured Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    return distance;
}

// Function to send data to Blynk
void sendToBlynk(int value) {
    WiFiClient client;

    if (!client.connect(server, port)) {
    Serial.println("Connection to Blynk server failed");
    return;
    }

    String path = String("/external/api/update?token=") + auth +
                "&" + virtualPin + "=" + String(value);

    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                "Host: " + server + "\r\n" +
                "Connection: close\r\n\r\n");

    while (client.connected() || client.available()) {
    if (client.available()) {
        String response = client.readStringUntil('\n');
        Serial.println(response);
    }
    }

    client.stop();
}
void sendToBlynk2(int value) {
    WiFiClient client;

    if (!client.connect(server, port)) {
    Serial.println("Connection to Blynk server failed");
    return;
    }

    String path = String("/external/api/update?token=") + auth +
                "&" + virtualPin2 + "=" + String(value);

    client.print(String("GET ") + path + " HTTP/1.1\r\n" +
                "Host: " + server + "\r\n" +
                "Connection: close\r\n\r\n");

    while (client.connected() || client.available()) {
    if (client.available()) {
        String response = client.readStringUntil('\n');
        Serial.println(response);
    }
    }

    client.stop();
}

// Function to unlock the solenoid
void unlockSolenoid() {
    digitalWrite(RELAY_PIN, LOW);   // Turn the relay OFF to unlock (retract)
    Serial.println("Solenoid unlocked!");
}

// Function to lock the solenoid
void lockSolenoid() {
    digitalWrite(RELAY_PIN, HIGH);  // Turn the relay ON to lock (extend)
    Serial.println("Solenoid locked.");
}

// Function to compare two UIDs
bool compareUID(byte *uid1, byte uid1Size, byte *uid2, byte uid2Size) {
    if (uid1Size != uid2Size) {
    return false;
    }
    for (byte i = 0; i < uid1Size; i++) {
    if (uid1[i] != uid2[i]) {
        return false;
    }
    }
    return true;
}