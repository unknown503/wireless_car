#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char *ssid = "null";
const char *password = "fffe026ffe9a";

byte ENA_pin = 14;
byte IN1 = 27;
byte IN2 = 26;
byte ENB_pin = 32;
byte IN3 = 25;
byte IN4 = 33;

const byte pwm_channel = 0;
const byte frequency = 30000;
const byte resolution = 8;

String carSpeed = "80";

enum STATES {
  STOP,
  FORWARD,
  BACKWARD,
  RIGHT,
  LEFT
};

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Robot Controls</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.1/css/all.min.css" />
    <link rel="stylesheet" href="https://unknown503.github.io/wireless_car/assets/styles.css" />
  </head>
  <body>
    <div class="container">
      <h1>Robot Controls</h1>
      <button id="forwardButton"><i class="fas fa-arrow-up"></i></button>
      <br />
      <button id="leftButton"><i class="fas fa-arrow-left"></i></button>
      <button id="stopButton"><i class="fas fa-stop"></i></button>
      <button id="rightButton"><i class="fas fa-arrow-right"></i></button>
      <br />
      <button id="backwardButton"><i class="fas fa-arrow-down"></i></button>
      <div class="slider">
        <h3>PWM Speed: <span id="speed">%SLIDERVALUE%</span></h3>
        <input
          type="range"
          min="0"
          max="255"
          value="%SLIDERVALUE%"
          id="slider"
          step="5"
          oninput="updateSliderPWM(this)"
        />
      </div>
    </div>
    <script src="https://unknown503.github.io/wireless_car/assets/js.js" defer></script>
  </body>
</html>
)rawliteral";

String processor(const String &var) {
  if (var == "SLIDERVALUE") return carSpeed;
  return String();
}

void move(int direction) {
  ledcWrite(pwm_channel, carSpeed.toInt());

  if (direction == STOP) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN4, LOW);
  } else if (direction == FORWARD) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN4, LOW);
  } else if (direction == BACKWARD) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN4, HIGH);
  } else if (direction == RIGHT) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN4, HIGH);
  } else if (direction == LEFT) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN4, LOW);
  } else {
    Serial.println("Nope: " + direction);
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(ENA_pin, OUTPUT);
  pinMode(ENB_pin, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(pwm_channel, frequency, resolution);
  ledcAttachPin(ENA_pin, pwm_channel);
  ledcAttachPin(ENB_pin, pwm_channel);

  ledcWrite(pwm_channel, carSpeed.toInt());
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/speed", HTTP_GET, [](AsyncWebServerRequest *request) {
    const char *param = "value";
    if (request->hasParam(param)) {
      String value = request->getParam(param)->value();
      Serial.println("Speed: "+value);
      carSpeed = value;
      ledcWrite(pwm_channel, carSpeed.toInt());
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
    const char *param = "direction";
    if (request->hasParam(param)) {
      String direction = request->getParam(param)->value();
      Serial.println("Moving: " + direction);
      move(direction.toInt());
    }
    request->send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
}
