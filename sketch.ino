/*
 * Vinheria Agnello - Checkpoint 05 (Edge Computing & Computer Systems - FIAP)
 * [VERSÃO WOKWI] ESP32 + DHT (temperatura/umidade) + LDR (luminosidade) -> MQTT
 *
 * Bibliotecas: PubSubClient (Nick O'Leary), DHT sensor library (Adafruit) + Adafruit Unified Sensor
 */
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>

// ======== CONFIGURAÇÕES (ALTERE AQUI) ========
const char* WIFI_SSID   = "Wokwi-GUEST";
const char* WIFI_PASS   = "";
const char* MQTT_BROKER = "broker.hivemq.com";   // broker público (Wokwi não acessa IPs locais)
const int   MQTT_PORT   = 1883;
const char* DEVICE_ID   = "adega_g7_001"; // TROQUE por um ID único do seu grupo
// =============================================

// Pinos (conforme montagem do enunciado)
#define DHT_PIN  4      // DATA do DHT11
#define LDR_PIN  34     // LDR (entrada analógica)
#define LED_PIN  2      // LED1 (atuador de teste)
#define DHT_TYPE DHT22   // Wokwi simula DHT22. No hardware real use DHT11

// Tópicos (padrão FIWARE IoT Agent MQTT / Ultralight)
String TOPIC_ATTRS  = String("/TEF/") + DEVICE_ID + "/attrs";      // publica todos
String TOPIC_TEMP   = String("/TEF/") + DEVICE_ID + "/attrs/t";
String TOPIC_HUM    = String("/TEF/") + DEVICE_ID + "/attrs/h";
String TOPIC_LUM    = String("/TEF/") + DEVICE_ID + "/attrs/l";
String TOPIC_CMD    = String("/TEF/") + DEVICE_ID + "/cmd";        // recebe comandos
String TOPIC_CMDEXE = String("/TEF/") + DEVICE_ID + "/cmdexe";     // responde comandos

DHT dht(DHT_PIN, DHT_TYPE);
WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastPublish = 0;
const unsigned long INTERVAL_MS = 5000;

void connectWiFi() {
  Serial.printf("Conectando ao Wi-Fi %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.printf("\nWi-Fi OK. IP: %s\n", WiFi.localIP().toString().c_str());
}

void syncTime() {
  // Fuso de Brasília (UTC-3)
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  struct tm t;
  while (!getLocalTime(&t)) { delay(500); Serial.print("t"); }
  Serial.println("\nHora sincronizada via NTP.");
}

String timestampISO() {
  struct tm t;
  if (!getLocalTime(&t)) return "1970-01-01T00:00:00-03:00";
  char buf[30];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S-03:00", &t);
  return String(buf);
}

// Recebe comandos, ex.: "adega001@on|" ou "adega001@off|"
void onMessage(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  Serial.printf("Comando recebido [%s]: %s\n", topic, msg.c_str());

  String ack = String(DEVICE_ID);
  if (msg.indexOf("@on|") >= 0) {
    digitalWrite(LED_PIN, HIGH);
    ack += "@on|LED ligado";
  } else if (msg.indexOf("@off|") >= 0) {
    digitalWrite(LED_PIN, LOW);
    ack += "@off|LED desligado";
  } else {
    ack += "@unknown|comando invalido";
  }
  mqtt.publish(TOPIC_CMDEXE.c_str(), ack.c_str());
}

void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("Conectando ao broker MQTT... ");
    String clientId = String("esp32-") + DEVICE_ID + "-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("OK");
      mqtt.subscribe(TOPIC_CMD.c_str());
    } else {
      Serial.printf("falhou (rc=%d). Nova tentativa em 3s\n", mqtt.state());
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  analogReadResolution(12); // 0..4095
  connectWiFi();
  syncTime();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(onMessage);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  if (millis() - lastPublish >= INTERVAL_MS) {
    lastPublish = millis();

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    int ldrRaw = analogRead(LDR_PIN);
    int lum    = map(ldrRaw, 0, 4095, 0, 100); // % de luminosidade (ajuste/calibre se precisar)

    if (isnan(temp) || isnan(hum)) {
      Serial.println("Falha na leitura do DHT11!");
      return;
    }

    String ts = timestampISO();

    // Payload Ultralight (formato padrão do IoT Agent MQTT): chave|valor|chave|valor
    String payload = "t|" + String(temp, 1) + "|h|" + String(hum, 1) +
                     "|l|" + String(lum) + "|ts|" + ts;

    mqtt.publish(TOPIC_ATTRS.c_str(), payload.c_str());
    // Publicações individuais (facilita leitura no MyMQTT)
    mqtt.publish(TOPIC_TEMP.c_str(), String(temp, 1).c_str());
    mqtt.publish(TOPIC_HUM.c_str(),  String(hum, 1).c_str());
    mqtt.publish(TOPIC_LUM.c_str(),  String(lum).c_str());

    Serial.printf("[%s] T=%.1f C | U=%.1f %% | L=%d %%\n", ts.c_str(), temp, hum, lum);
  }
}
