#include <ESPAsyncWebServer.h> // Include ESPAsyncWebServer library for asynchronous communication
#include <ESP8266WiFi.h> // Include ESP8266 WiFi library

#define LED_PIN D5           // LED pin for indicating calibration status
#define MOTOR1_PIN D1        // GPIO pin controlling motor 1
#define MOTOR2_PIN D2        // GPIO pin controlling motor 2
#define MOTOR3_PIN D3        // GPIO pin controlling motor 3
#define MOTOR4_PIN D4        // GPIO pin controlling motor 4
#define PWM_MAX_VALUE 1023   // Define maximum PWM duty cycle value (10-bit resolution)
#define PWM_MIN_VALUE 0      // Define minimum PWM duty cycle value

// WiFi credentials
const char* ssid = "Motor_Control_AP";
const char* password = "12345678";

AsyncWebServer server(80); // Create an asynchronous server on port 80

void setup() {
  Serial.begin(115200); // Start serial communication for debugging with a baud rate of 115200
  pinMode(LED_PIN, OUTPUT); // Set LED pin as output for indicating status

  // Set motor pins as output
  pinMode(MOTOR1_PIN, OUTPUT);
  pinMode(MOTOR2_PIN, OUTPUT);
  pinMode(MOTOR3_PIN, OUTPUT);
  pinMode(MOTOR4_PIN, OUTPUT);

  // Set up WiFi in AP mode
  WiFi.softAP(ssid, password); // Start WiFi in AP mode
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP()); // Print the IP address

  // Set up server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", R"rawliteral(
    <!DOCTYPE HTML><html>
    <head>
      <title>Motor Control</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <script>
      function sendMotorCommand(speed) {
        fetch(`/motor?speed=${speed}`).then(response => response.text()).then(data => {
          document.getElementById('status').innerText = data;
        }).catch(error => {
          document.getElementById('status').innerText = 'Error: ' + error;
        });
      }
      </script>
    </head>
    <body>
      <h1>Motor Control from ESP8266</h1>
      <p>Speed: <input type="range" min="0" max="1023" value="0" oninput="sendMotorCommand(this.value)"></p>
      <p>Status: <span id="status">Awaiting command...</span></p>
    </body>
    </html>
    )rawliteral");
  });

  // Route for sending motor commands
  server.on("/motor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("speed")) {
      int speed = request->getParam("speed")->value().toInt();
      if (speed >= PWM_MIN_VALUE && speed <= PWM_MAX_VALUE) {
        controlMotors(speed);
        digitalWrite(LED_PIN, HIGH); // Turn on LED to indicate motor control command received
        request->send(200, "text/plain", "Speed set to " + String(speed));
      } else {
        request->send(400, "text/plain", "Invalid speed value. Please provide a value between " + String(PWM_MIN_VALUE) + " and " + String(PWM_MAX_VALUE));
      }
    } else {
      request->send(400, "text/plain", "Missing speed parameter");
    }
  });

  server.begin(); // Start the server
  Serial.println("Server Started");
}

void loop() {
  // No need for code here as AsyncWebServer handles requests asynchronously
}

// Function to control motor speeds
void controlMotors(int speed) {
  analogWrite(MOTOR1_PIN, speed);
  analogWrite(MOTOR2_PIN, speed);
  analogWrite(MOTOR3_PIN, speed);
  analogWrite(MOTOR4_PIN, speed);
  Serial.println("Motor speeds set to: " + String(speed));
  delay(10); // Small delay for stability
  digitalWrite(LED_PIN, LOW); // Turn off LED after setting motor speed
}
