#include <WiFi.h>
//ArduinoWebsockets library http://bit.ly/3LeAGVl
#include <ArduinoWebsockets.h>
// JSON (de)serialisation library http://bit.ly/3ZHI5Rs
#include <ArduinoJson.h>

// Set this to true if you are going to use static IP.
#define STATIC_IP true
#define DEBUG true

#define ONBOARD_BUTTON 0
// Wi-Fi credentials for the network the ESP will connect to
const char* ssid = "MIFI_D094";
const char* password = "1234567890";

// Websocket URL
const char* websockets_server_host = "192.168.0.103";  //Enter server adress
const uint16_t websockets_server_port = 1880;          // Enter server port

#ifdef STATIC_IP
// Configure your statis IP details.
IPAddress local_IP(192, 168, 0, 115);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(1, 1, 1, 1);    //optional
IPAddress secondaryDNS(8, 8, 8, 8);  //optional
#endif

using namespace websockets;

WebsocketsClient client;

// JSON Desrializer
DynamicJsonDocument doc(1024);

char response[200];
void setup() {
  pinMode(ONBOARD_BUTTON, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  // Initialize the serial port.
  Serial.begin(115200);
#ifdef STATIC_IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }
#endif

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("-");
  }
#ifdef DEBUG
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
#endif
  //   Cnnect to WS

  bool connected = client.connect(websockets_server_host, websockets_server_port, "/test/esp32");
  if (connected) {
    Serial.println("WS Connected!");
  } else {
    Serial.println("WS Connection failed    !");
  }
  // Callbacks for message and events
  client.onMessage(messageCallback);
  client.onEvent(eventCallback);

  // Ping to the remote WS server. Expect a pong
  client.ping();
}

void loop() {
  if (client.available()) {
    client.poll();
  }
  if (!digitalRead(ONBOARD_BUTTON)) {
    delay(300);
#ifdef DEBUG
    Serial.print("Pressed Button");
#endif
    sprintf(response, "{\"temperature\": %f, \"Humidity\": %f}", 2.3, 2.9);
    client.send(response);
  }
}

void messageCallback(WebsocketsMessage message) {
#ifdef DEBUG
  Serial.print("Message Received: ");
    Serial.println(message.data());
#endif
  deserializeJson(doc, message.data());
  if (doc["status"] == 1) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void eventCallback(WebsocketsEvent event, String data) {
  if (event == WebsocketsEvent::ConnectionOpened) {
    Serial.println("Connnection Opened");
  } else if (event == WebsocketsEvent::ConnectionClosed) {
    Serial.println("Connnection Closed");
  } else if (event == WebsocketsEvent::GotPing) {
    Serial.println("Pinged👋, responding with PONG!");
    client.pong();
  } else if (event == WebsocketsEvent::GotPong) {
    Serial.println("Got a Pong!");
  }
}
