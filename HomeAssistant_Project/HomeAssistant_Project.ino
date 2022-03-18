#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "Panomporn 2g"
#define wifi_password "0812397357"

#define mqtt_server "192.168.1.63"
#define mqtt_user "jamPongsathorn"
#define mqtt_password "Jspb2211"

#define humidity_topic "room/sensor/humidity"
#define temperature_topic "room/sensor/temperature"
#define heatindex_topic "room/sensor/heatindex"

#define DHTPIN 5       // Gpio -D1
#define DHTTYPE DHT11  // DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) && (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

// variable
long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float hic = 0.0;
float diff = .0;


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // report every 1000 ms
  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    float newTemp = dht.readTemperature(false);
    float newHum = dht.readHumidity();
    float newHic = dht.computeHeatIndex(newTemp, newHum, false);

    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature: ");
      Serial.println(String(temp).c_str());
      client.publish(temperature_topic, String(temp).c_str(), true);
    }
    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("New humidity: ");
      Serial.println(String(hum).c_str());
      client.publish(humidity_topic, String(hum).c_str(), true);
    }
    if (checkBound(newHic, hic, diff)) {
      hic = newHic;
      Serial.print("New heat index: ");
      Serial.println(String(hic).c_str());
      client.publish(heatindex_topic, String(hic).c_str(), true);
    }
    Serial.println();
  }
}