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

int motorSpeeds[4] = {0, 0, 0, 0}; // Store the speeds for each motor

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
      <title>Individual Motor Control</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <script>
      function sendMotorCommand(motor, action) {
        fetch(`/motor?motor=${motor}&action=${action}`).then(response => response.text()).then(data => {
          document.getElementById('status').innerText = data;
        }).catch(error => {
          document.getElementById('status').innerText = 'Error: ' + error;
        });
      }
      </script>
    </head>
    <body>
      <h1>Individual Motor Control from ESP8266</h1>
      <div>
        <h2>Motor 1</h2>
        <button onclick="sendMotorCommand(1, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(1, 'down')">Speed Down</button>
      </div>
      <div>
        <h2>Motor 2</h2>
        <button onclick="sendMotorCommand(2, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(2, 'down')">Speed Down</button>
      </div>
      <div>
        <h2>Motor 3</h2>
        <button onclick="sendMotorCommand(3, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(3, 'down')">Speed Down</button>
      </div>
      <div>
        <h2>Motor 4</h2>
        <button onclick="sendMotorCommand(4, 'up')">Speed Up</button>
        <button onclick="sendMotorCommand(4, 'down')">Speed Down</button>
      </div>
      <p>Status: <span id="status">Awaiting command...</span></p>
    </body>
    </html>
    )rawliteral");
  });

  // Route for sending motor commands
  server.on("/motor", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("motor") && request->hasParam("action")) {
      int motor = request->getParam("motor")->value().toInt();
      String action = request->getParam("action")->value();

      if (motor >= 1 && motor <= 4) {
        if (action == "up") {
          motorSpeeds[motor - 1] = min(motorSpeeds[motor - 1] + 50, PWM_MAX_VALUE); // Increase speed by 50, limit to max value
        } else if (action == "down") {
          motorSpeeds[motor - 1] = max(motorSpeeds[motor - 1] - 50, PWM_MIN_VALUE); // Decrease speed by 50, limit to min value
        } else {
          request->send(400, "text/plain", "Invalid action. Use 'up' or 'down'.");
          return;
        }

        controlMotor(motor, motorSpeeds[motor - 1]);
        digitalWrite(LED_PIN, HIGH); // Turn on LED to indicate motor control command received
        request->send(200, "text/plain", "Motor " + String(motor) + " speed set to " + String(motorSpeeds[motor - 1]));
      } else {
        request->send(400, "text/plain", "Invalid motor number. Please provide a value between 1 and 4.");
      }
    } else {
      request->send(400, "text/plain", "Missing motor or action parameter");
    }
  });

  server.begin(); // Start the server
  Serial.println("Server Started");
}

void loop() {
  // No need for code here as AsyncWebServer handles requests asynchronously
}

// Function to control individual motor speed
void controlMotor(int motor, int speed) {
  switch (motor) {
    case 1:
      analogWrite(MOTOR1_PIN, speed);
      break;
    case 2:
      analogWrite(MOTOR2_PIN, speed);
      break;
    case 3:
      analogWrite(MOTOR3_PIN, speed);
      break;
    case 4:
      analogWrite(MOTOR4_PIN, speed);
      break;
  }
  Serial.println("Motor " + String(motor) + " speed set to: " + String(speed));
  delay(10); // Small delay for stability
  digitalWrite(LED_PIN, LOW); // Turn off LED after setting motor speed
}
