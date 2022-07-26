#include <PZEM004Tv30.h>
#define relay 4
#define led 2
#include <WiFi.h>
#include <PubSubClient.h>

#define WIFISSID "SSID" // Put your WifiSSID here
#define PASSWORD "PASS" // Put your wifi password here
#define TOKEN "xxxx" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "NAME" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
//it should be a random and unique ascii string and different from all other devices

#define var_l1 "voltage" // Assign the variable label
#define var_l2 "current"
#define var_l3 "power"
#define var_l4 "energy"
#define var_l5 "frequency"
#define var_l6 "pf"
#define var_l_SUBSCRIBE "relay" // Assing the variable label
#define DEVICE_LABEL "energy_meter" // Assig the device label

char mqttBroker[]  = "things.ubidots.com";
char payload[2000];
char topic[150];

char topicSubscribe[100];
// Space to store values to send
char str_voltage[20];
char str_current[20];
char str_power[20];
char str_energy[20];
char str_frequency[20];
char str_pf[20];

WiFiClient ubidots;
PubSubClient client(ubidots);

PZEM004Tv30 pzem(&Serial2, 16, 17);

void setup() {
  int w = 0;
  Serial.begin(115200);
  pinMode(relay, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  Serial.println("DEVICE ON");
  delay(2000);
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT
  Serial.println();
  Serial.print("Waiting for WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);
  sprintf(topicSubscribe, "/v1.6/devices/%s/%s/lv", DEVICE_LABEL, var_l_SUBSCRIBE);

  client.subscribe(topicSubscribe);
  if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
    Serial.println("Connected");
    digitalWrite(2, HIGH);
    delay(2000);
    digitalWrite(2, LOW);
  }
}

void loop() {
  if (!client.connected()) {
    client.subscribe(topicSubscribe);   
    reconnect();
  }
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();

  dtostrf(voltage, 10, 2, str_voltage);
  dtostrf(current, 10, 2, str_current);
  dtostrf(power, 10, 2, str_power);
  dtostrf(energy, 10, 2, str_energy);
  dtostrf(frequency, 10, 2, str_frequency);
  dtostrf(pf, 10, 2, str_pf);
  
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\": %s,", var_l1, str_voltage);
  sprintf(payload, "%s\"%s\": %s,", payload, var_l2, str_current);
  sprintf(payload, "%s\"%s\": %s,", payload, var_l3, str_power);
  sprintf(payload, "%s\"%s\": %s,", payload, var_l4, str_energy);
  sprintf(payload, "%s\"%s\": %s,", payload, var_l5, str_frequency);
  sprintf(payload, "%s\"%s\": %s}", payload, var_l6, str_pf);

  Serial.print("Value of Voltage is: "); Serial.print(voltage); Serial.println(" V");
  Serial.print("Value of Current is: "); Serial.print(current); Serial.println(" A");
  Serial.print("Value of Power is: "); Serial.print(power); Serial.println(" W");
  Serial.print("Value of Energy is: "); Serial.print(energy); Serial.println(" kWh");
  Serial.print("Value of Frequency is: "); Serial.print(frequency); Serial.println(" Hz");
  Serial.print("Value of Power Factor is: ");Serial.println(pf);

  client.publish(topic, payload);
  client.loop();
  Serial.println("Publishing data to Ubidots Cloud");
  Serial.println(payload);
  Serial.println("");
  delay(5000);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
      digitalWrite(led,LOW);
      client.subscribe(topicSubscribe);
    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      wifi();
      Serial.println(" Try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}


void wifi() {
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  delay(500);
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  delay(500);
}

void callback(char* topicSubscribe, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  if (message == "0") {
    digitalWrite(relay, LOW);
  } else {
    digitalWrite(relay, HIGH);
  }
  Serial.write(payload, length);
  Serial.println();
}
  
