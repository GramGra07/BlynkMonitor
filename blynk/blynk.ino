#include <secrets.h>
#define BLYNK_TEMPLATE_ID TEMPLATE
#define BLYNK_TEMPLATE_NAME "testing"
// #define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define HALL_PIN 15
#define ECHO_PIN 32
#define TRIG_PIN 33
#define MAIN_LED_PIN 2

#define rPin 27
#define gPin 14
#define bPin 12

bool hallTriggered = 0;

bool notifyHall = 0;
bool notifyDist = 0;

int distanceLimit = 0;
int distance = 0;
bool moreThan = 0;
long duration;
bool hallConnected = 0;
bool pastHallConnected = 0;

bool stealthMode = 0;  //to add // how to turn off main light

bool rgbOn = 1;
int pastR = 255;
int pastG = 0;
int pastB = 0;

// Set your Blynk authentication token
char auth[] = AUTH;

// Set your WiFi credentials
char ssid1[] = SSID1;
char pass1[] = PASS1;

const char* ssid2 = SSID2;
const char* pass2 = PASS2;


int waitForConnectResult() {
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {  // 10 seconds timeout
    delay(100);
  }
  return WiFi.status();
}
void connectWiFi() {
  int attemptCount = 0;
  int maxAttempts = 5;

  while (attemptCount < maxAttempts) {
    WiFi.disconnect(true);  // Disconnect from any network
    delay(1000);            // Wait for the disconnection to complete

    // Serial.println("Trying to connect to Network1...");
    WiFi.begin(ssid1, pass1);
    if (waitForConnectResult() == WL_CONNECTED) {
      // Serial.println("Connected to Network1!");
      break;
    }

    WiFi.disconnect(true);  // Disconnect before trying the next network
    delay(1000);

    // Serial.println("Trying to connect to Network2...");
    WiFi.begin(ssid2, pass2);
    if (waitForConnectResult() == WL_CONNECTED) {
      // Serial.println("Connected to Network2!");
      break;
    }

    WiFi.disconnect(true);  // Disconnect before trying the next network
    delay(1000);

    // if (WiFi.status() != WL_CONNECTED) {
    //   // Serial.println("Could not connect to any network. Rebooting...");
    //   ESP.restart();
    // }
  }
}
bool didTrigger = 0;
void setup() {
  // Debug console
  Serial.begin(115200);
  connectWiFi();
  Blynk.begin(auth, WiFi.SSID().c_str(), WiFi.psk().c_str());

  pinMode(MAIN_LED_PIN, OUTPUT);

  pinMode(HALL_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(bPin, OUTPUT);
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(V6);
  Blynk.syncVirtual(V7);
  Blynk.syncVirtual(V5);
  Blynk.syncVirtual(V1);
  Blynk.syncVirtual(V9);
  Blynk.syncVirtual(V10);
}

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();
  if (pinValue == 0 || (!stealthMode && pinValue == 1)) {
    digitalWrite(MAIN_LED_PIN, pinValue);
  } else if (stealthMode) {
    digitalWrite(MAIN_LED_PIN, 0);
  }
}
BLYNK_WRITE(V9) {
  int val = param.asInt();
  notifyDist = val;
}
BLYNK_WRITE(V10) {
  int val = param.asInt();
  notifyHall = val;
}
BLYNK_WRITE(V5) {
  int val = param.asInt();
  distanceLimit = val;
}
BLYNK_WRITE(V6) {
  int val = param.asInt();
  moreThan = val;
}

BLYNK_WRITE(V7) {
  int val = param.asInt();
  rgbOn = val;
}

BLYNK_WRITE(V1) {
  int val = param.asInt();
  stealthMode = val;
  Blynk.syncVirtual(V0);
}
void loop() {
  Blynk.run();
  readHallEffect();
  readDistance();
  rgbOnOff();
  Serial.println(pastG);
}

void readHallEffect() {
  hallConnected = !digitalRead(HALL_PIN);
  if (hallConnected != pastHallConnected) {
    pastHallConnected = hallConnected;
    Blynk.virtualWrite(V2, hallConnected);
  }
  if (hallConnected) {
    setRGB(0, 255, 0);
    hallTriggered = 0;
  } else {
    if (hallTriggered == 0) {
      if (notifyHall == 1) {
        Blynk.logEvent("hall_not_connected");
      }
      hallTriggered = 1;
      setRGB(255, 0, 0);
    }
  }
}

void readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  Blynk.virtualWrite(V4, distance);
  if (moreThan == 1) {
    if (distance > distanceLimit) {
      // Serial.println("more than");
      if (didTrigger == 0) {
        if (notifyDist == 1) {
          Blynk.logEvent("distance_over_limit");
        }
        didTrigger = 1;
      }
    } else if (distance < distanceLimit) {
      didTrigger = 0;
    }
  } else if (moreThan == 0) {
    if (distance < distanceLimit) {
      // Serial.println("less than");
      if (didTrigger == 0) {
        if (notifyDist == 1) {
          Blynk.logEvent("distance_under_limit");
        }
        didTrigger = 1;
      }
    } else if (distance > distanceLimit) {
      didTrigger = 0;
    }
  }
}

void rgbOnOff() {
  if (rgbOn == 1 && stealthMode == 0) {
    analogWrite(rPin, pastR);
    analogWrite(gPin, pastG);
    analogWrite(bPin, pastB);
  } else {
    analogWrite(rPin, 0);
    analogWrite(gPin, 0);
    analogWrite(bPin, 0);
  }
}
void setRGB(int r, int g, int b) {
  pastR = r;
  pastG = g;
  pastB = b;
}