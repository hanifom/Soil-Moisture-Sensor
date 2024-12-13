#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi credentials
const char* ssid = "Kucing Lucu";         // Replace with your Wi-Fi SSID
const char* password = "Kukucyakuc1!"; // Replace with your Wi-Fi password

// Pin definitions
#define RELAY_PIN 4       // ESP32 pin GPIO4 that connects to the relay
#define MOISTURE_PIN 36   // ESP32 pin GPIO36 (ADC0) that connects to the AOUT pin of the moisture sensor
#define THRESHOLD 1550    // Threshold for soil moisture (adjust this based on calibration)

void setup() {
  Serial.begin(9600);                 // Initialize serial communication for debugging

  // Wi-Fi setup
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.println(WiFi.localIP());     // Print the ESP32's IP address

  // Pin configuration
  pinMode(RELAY_PIN, OUTPUT);         // Set relay pin as output
  digitalWrite(RELAY_PIN, HIGH);      // Ensure relay is OFF initially
  pinMode(MOISTURE_PIN, INPUT);       // Set soil moisture pin as input
  analogSetAttenuation(ADC_11db);     // Configure ADC to handle up to 3.3V input
}

void loop() {
  // Read the analog value from the soil moisture sensor
  int value = analogRead(MOISTURE_PIN);

  // Map the sensor value to a percentage (calibrate these values: 1100 for dry, 2600 for wet)
  int moisturePercent = map(value, 1100, 2600, 100, 0);

  // Ensure the percentage is within bounds (0% to 100%)
  moisturePercent = constrain(moisturePercent, 0, 100);

  // Determine pump state based on threshold
  if (value > THRESHOLD) {
    Serial.print("The soil moisture is DRY => activating pump");
    digitalWrite(RELAY_PIN, LOW);  // Turn the pump ON
  } else {
    Serial.print("The soil moisture is WET => deactivating pump");
    digitalWrite(RELAY_PIN, HIGH); // Turn the pump OFF
  }

  // Display sensor data
  Serial.print(" (Moisture: ");
  Serial.print(moisturePercent);
  Serial.print("% - Raw ADC: ");
  Serial.print(value);
  Serial.println(")");

  // Send data to the server if Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Server URL
    String serverURL = "http://192.168.1.2:5000/data";

    // Create JSON payload
    String jsonData = "{";
    jsonData += "\"moisture_percent\": " + String(moisturePercent) + ", ";
    jsonData += "\"raw_value\": " + String(value);
    jsonData += "}";

    http.begin(serverURL);           // Specify the server URL
    http.addHeader("Content-Type", "application/json"); // Set content type to JSON

    int httpResponseCode = http.POST(jsonData); // Send the POST request

    if (httpResponseCode > 0) {
      String response = http.getString(); // Get the response to the request
      Serial.print("Server response: ");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // End the connection
  } else {
    Serial.println("Wi-Fi Disconnected");
  }

  delay(500); // Delay for 5 seconds before the next reading
}
