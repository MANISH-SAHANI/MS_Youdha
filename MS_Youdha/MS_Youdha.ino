#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <HTTPClient.h>

#define MAX_VALUES 10
#define THRESHOLD 1.3

// Sensor Settings
MPU6050 mpu;
WiFiClient client;

// Sensor readings
float zValues[MAX_VALUES] = {0};
float trackedValues[MAX_VALUES] = {0};
int storageUsed = 0;

// Wi-Fi Settings
const char *ssid = "SAHANI SAREES";
const char *password = "Hitman_OP";

// Previous AND Next Value
float m_Previous[5] = {0};
float m_Next[5] = {0};

// ThingSpeak Settings
unsigned long channelId = 2335803;
const char *apiKey = "HRP5N3SGIRBWW7TB";

// Twilio Settings
const char* accountSid = "AC0711cd7a5f8511d80c2a8a63d849ea3b";
const char* authToken = "dbf5a85f6a6baca73e8f538aa3de16c6";
const char* twilioPhoneNumber = "+18782134047";
const char* recipientPhoneNumber = "+917820816430";


void updateThingSpeakPrevious(float value)
{
    int fieldNumber = 1;
    int httpCode = ThingSpeak.writeField(channelId, fieldNumber, value, apiKey);

    if (httpCode == 200)
    {
        Serial.println(" ThingSpeak update successful");
    }
    else
    {
        Serial.println("Error updating ThingSpeak");
        Serial.println(httpCode);
    }

    delay(15000);
}

void updateThingSpeakThresholdVal(float value)
{
    int fieldNumber = 2;
    int httpCode = ThingSpeak.writeField(channelId, fieldNumber, value, apiKey);

    if (httpCode == 200)
    {
        Serial.println("ThingSpeak update successful");
        Serial.print("\n");
    }
    else
    {
        Serial.println("Error updating ThingSpeak");
        Serial.println(httpCode);
    }

    delay(15000);
}

void updateThingSpeakNext(float value)
{
    int fieldNumber = 3;
    int httpCode = ThingSpeak.writeField(channelId, fieldNumber, value, apiKey);

    if (httpCode == 200)
    {
        Serial.println(" ThingSpeak update successful");
    }
    else
    {
        Serial.println("Error updating ThingSpeak");
        Serial.println(httpCode);
    }

    delay(15000);
}

void setup()
{
    Wire.begin();
    mpu.initialize();

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize ThingSpeak
    ThingSpeak.begin(client);

    Serial.begin(9600); // Initialize Serial for output
}

void loop()
{
    float acceleration = readZAcceleration();
    zValues[storageUsed] = acceleration;
    trackedValues[storageUsed] = acceleration;

    if ((storageUsed + 1) % MAX_VALUES == 0)
    { // Print values every 10 readings
        Serial.println("Recorded values:");
        for (int i = 0; i < MAX_VALUES; ++i)
        {
            int idx = (storageUsed - i + MAX_VALUES) % MAX_VALUES;
            Serial.print("x");
            Serial.print(idx + 1);
            Serial.print(": ");
            Serial.print(zValues[idx]);
            Serial.print(" ");
            if ((i + 1) % 10 == 0)
                Serial.println(); // Line break after every 10 values
        }
        Serial.println();
    }

    if (acceleration > THRESHOLD || acceleration < -THRESHOLD)
    {
        Serial.println("Triggered!");
        Serial.println("Sending Alert SMS........");
        SendingSms();
        Serial.println("Recorded values:");
        for (int i = 9; i >= 5; --i)
        {
            int idx = (storageUsed - i + MAX_VALUES) % MAX_VALUES;
            Serial.print("x");
            Serial.print(idx + 1);
            Serial.print(": ");
            Serial.print(zValues[idx]);
            Serial.print(" ");
        }
        Serial.print("Triggered value: ");
        Serial.println(acceleration);
        Serial.println("\nThreshold Value Uploading to Things-Speak");
        updateThingSpeakThresholdVal(acceleration);
        Serial.println("Previous values:");
        for (int i = 4; i >= 0; --i)
        {
            int idx = (storageUsed - i + MAX_VALUES) % MAX_VALUES;
            Serial.print("x");
            Serial.print(idx + 1);
            Serial.print(": ");
            Serial.print(zValues[idx]);
            m_Previous[i] = zValues[idx]; // MS
            Serial.print(" ");
        }

        Serial.print("\n");
        PreviousVal();

        Serial.println("\nNext values:");
        for (int i = 1; i <= 5; ++i)
        {
            int idx = (storageUsed + i) % MAX_VALUES;
            Serial.print("x");
            Serial.print(idx + 1);
            Serial.print(": ");
            Serial.print(zValues[idx]);
            m_Next[i] = zValues[idx]; // MS
            Serial.print(" ");
        }
        Serial.println("\n");
        NextVal();

        // // Verify printed values with manually tracked values
        // Serial.println("\nVerifying previous and next values:");
        // for (int i = 1; i <= 5; ++i)
        // {
        //     int idx = (storageUsed - i + MAX_VALUES) % MAX_VALUES;
        //     Serial.print("Tracked Previous x");
        //     Serial.print(i);
        //     Serial.print(": ");
        //     Serial.print(trackedValues[idx]);
        //     Serial.print(" | Printed Previous x");
        //     Serial.print(i);
        //     Serial.print(": ");
        //     Serial.print(zValues[idx]);
        //     Serial.print(" ");
        // }

        // for (int i = 1; i <= 5; ++i)
        // {
        //     int idx = (storageUsed + i) % MAX_VALUES;
        //     Serial.print("Tracked Next x");
        //     Serial.print(i);
        //     Serial.print(": ");
        //     Serial.print(trackedValues[idx]);
        //     Serial.print(" | Printed Next x");
        //     Serial.print(i);
        //     Serial.print(": ");
        //     Serial.print(zValues[idx]);
        //     Serial.print(" ");
        // }
        // Serial.println("\n");
    }

    storageUsed = (storageUsed + 1) % MAX_VALUES;
    delay(500); // Delay for .5 seconds before the next reading
}

float readZAcceleration()
{
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);
    float acceleration = (float)az / 16384.0; // Adjust this based on your sensor's sensitivity
    return acceleration;
}

void PreviousVal()
{
    Serial.print("\nPrevious Value Uploading to Things-Speak\n");
    for (int i = 4; i >= 0; --i)
    {
        Serial.print(m_Previous[i]);
        updateThingSpeakPrevious(m_Previous[i]);
        //Serial.print("\n");
    }
}

void NextVal()
{
    Serial.print("\nNext Value Uploading to Things-Speak\n");
    for (int i = 4; i >= 0; --i)
    {
        Serial.print(m_Next[i]);
        updateThingSpeakNext(m_Next[i]);
        //Serial.print("\n");
    }
}

void SendingSms() {
  Serial.println("........");

  // Set up the HTTP client
  HTTPClient http;
  String url = "https://api.twilio.com/2010-04-01/Accounts/" + String(accountSid) + "/Messages.json";

  // Create the message body
  String messageBody = "Body=Hello+from+your+ESP32&From=" + String(twilioPhoneNumber) + "&To=" + String(recipientPhoneNumber);

  // Start the HTTP POST request
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setAuthorization(accountSid, authToken);

  // Send the request
  int httpCode = http.POST(messageBody);

  // Check for a successful request
  if (httpCode > 0) {
    Serial.printf("[HTTP] POST request status code: %d\n", httpCode);

    // Print the response payload
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.printf("[HTTP] POST request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  // End the request
  http.end();

  Serial.println("SMS sent!\n");

  delay(5000); // Wait for 5 seconds before sending the next SMS
}