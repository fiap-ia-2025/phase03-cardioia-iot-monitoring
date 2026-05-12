// ============================================================
//  CardioIA - Fase 3 | Parte 2: MQTT e Dashboard
//  Dispositivo : ESP32
//  Sensores    : DHT22 (temperatura + umidade) + Botao (BPM)
//  Cloud       : HiveMQ Cloud via MQTT
//  Resiliencia : Buffer circular em RAM (100 amostras)
//  Autor       : Equipe CardioIA - FIAP
// ============================================================

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "DHT.h"

// ------------------------------------------------------------
//  PINOS
// ------------------------------------------------------------
#define DHT_PIN     4        // Pino de dados do DHT22
#define DHT_TYPE    DHT22
#define BUTTON_PIN  15       // Botao que simula batimento cardiaco

// ------------------------------------------------------------
//  CONFIGURACAO WI-FI (WOKWI)
// ------------------------------------------------------------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

// ------------------------------------------------------------
//  CONFIGURACAO MQTT - HIVEMQ CLOUD
//  Substituir os placeholders pelos dados do cluster HiveMQ.
// ------------------------------------------------------------
const char* MQTT_SERVER   = "SEU_CLUSTER_HIVEMQ.s1.eu.hivemq.cloud";
const int   MQTT_PORT     = 8883;
const char* MQTT_USER     = "SEU_USUARIO_MQTT";
const char* MQTT_PASSWORD = "SUA_SENHA_MQTT";

const char* MQTT_CLIENT_ID = "cardioia-esp32-01";
const char* MQTT_TOPIC     = "cardioia/paciente01/vitals";

// ------------------------------------------------------------
//  CONSTANTES DE CONFIGURACAO
// ------------------------------------------------------------
const int MAX_AMOSTRAS = 100;              // Max. de amostras offline
const unsigned long LEITURA_MS = 2000;     // Intervalo de leitura (ms)
const unsigned long MQTT_RETRY_MS = 5000;  // Intervalo entre tentativas MQTT

// Simulacao de conectividade do dispositivo, reaproveitada da Parte 1
bool wifiConectado = false;
unsigned long ultimoToggleWifi = 0;
const unsigned long INTERVALO_TOGGLE = 15000; // alterna a cada 15 s (demo)

// Limites de alerta clinico
const float TEMP_ALERTA = 38.0;  // graus Celsius
const int   BPM_ALERTA  = 120;   // bpm
const float UMID_ALERTA = 90.0;  // %

// ------------------------------------------------------------
//  ESTRUTURA DE AMOSTRA
// ------------------------------------------------------------
struct Amostra {
  unsigned long timestamp; // ms desde boot
  float temperatura;       // graus Celsius
  float umidade;           // %
  int bpm;                 // batimentos por minuto
  bool alerta;             // true se algum valor excede limite
};

// ------------------------------------------------------------
//  VARIAVEIS GLOBAIS
// ------------------------------------------------------------
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure wifiSecureClient;
PubSubClient mqttClient(wifiSecureClient);

// Buffer circular de resiliencia offline
Amostra buffer[MAX_AMOSTRAS];
int bufferHead = 0;   // proxima posicao de escrita
int bufferCount = 0;  // amostras ainda nao enviadas

// Contagem de batimentos para calculo de BPM
volatile int contagemBatimentos = 0;             // incrementado no ISR
volatile unsigned long ultimoBatimentoISR = 0;   // debounce
const unsigned long DEBOUNCE_MS = 200;           // intervalo minimo entre batimentos
unsigned long ultimaJanelaBPM = 0;
int bpmAtual = 0;

// Controle de tempo
unsigned long ultimaLeitura = 0;
unsigned long ultimaTentativaMqtt = 0;

// ------------------------------------------------------------
//  ISR - INTERRUPCAO DO BOTAO COM DEBOUNCE
// ------------------------------------------------------------
void IRAM_ATTR isr_batimento() {
  unsigned long agora = millis();

  // Debounce: ignora pulsos com menos de 200ms do ultimo batimento
  if (agora - ultimoBatimentoISR > DEBOUNCE_MS) {
    contagemBatimentos++;
    ultimoBatimentoISR = agora;
  }
}

// ------------------------------------------------------------
//  CONECTA AO WI-FI DO WOKWI
// ------------------------------------------------------------
void conectarWifi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print(F("[WIFI] Conectando em "));
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }

  Serial.println();
  Serial.print(F("[WIFI] Conectado. IP: "));
  Serial.println(WiFi.localIP());
}

// ------------------------------------------------------------
//  CONECTA AO HIVEMQ CLOUD
// ------------------------------------------------------------
bool conectarMqtt() {
  if (mqttClient.connected()) return true;
  if (millis() - ultimaTentativaMqtt < MQTT_RETRY_MS) return false;

  ultimaTentativaMqtt = millis();

  Serial.print(F("[MQTT] Conectando ao HiveMQ Cloud... "));

  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println(F("conectado."));
    return true;
  }

  Serial.print(F("falhou. Codigo rc="));
  Serial.println(mqttClient.state());
  return false;
}

// ------------------------------------------------------------
//  SIMULA ALTERNANCIA DE CONECTIVIDADE DO DISPOSITIVO
// ------------------------------------------------------------
void simularWifi() {
  unsigned long agora = millis();
  if (agora - ultimoToggleWifi >= INTERVALO_TOGGLE) {
    wifiConectado = !wifiConectado;
    ultimoToggleWifi = agora;

    Serial.print(F("\n[WIFI] Status simulado alterado: "));
    Serial.println(wifiConectado ? F("CONECTADO") : F("DESCONECTADO"));

    if (!wifiConectado && mqttClient.connected()) {
      mqttClient.disconnect();
      Serial.println(F("[MQTT] Desconectado para simular periodo offline."));
    }
  }
}

// ------------------------------------------------------------
//  SETUP
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("================================================"));
  Serial.println(F("   CardioIA - Monitor Cardiaco MQTT"));
  Serial.println(F("   FIAP - Fase 3 - IoT na Saude"));
  Serial.println(F("================================================"));

  dht.begin();

  // Configura botao com pull-up interno; dispara na borda de descida
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr_batimento, FALLING);

  // Em simulacao no Wokwi, simplificamos TLS sem certificado raiz.
  // Em producao, usar setCACert() com o certificado da autoridade confiavel.
  wifiSecureClient.setInsecure();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);

  ultimaJanelaBPM = millis();
  ultimaLeitura = millis();
  ultimoToggleWifi = millis();

  conectarWifi();

  Serial.println(F("[INFO] Sistema inicializado. Buffer offline: 100 amostras."));
  Serial.println(F("[INFO] Wi-Fi logico: DESCONECTADO (simulacao inicia offline)"));
  Serial.println(F("[INFO] Topico MQTT: cardioia/paciente01/vitals"));
  Serial.println(F("[INFO] Debounce configurado: 200ms entre batimentos"));
  Serial.println(F("------------------------------------------------"));
}

// ------------------------------------------------------------
//  CALCULA BPM (JANELA DE 5 SEGUNDOS)
// ------------------------------------------------------------
int calcularBPM() {
  unsigned long agora = millis();

  // Calcula BPM a cada 5 segundos (mais rapido para demonstracao)
  if (agora - ultimaJanelaBPM >= 5000) {
    noInterrupts();
    int bat = contagemBatimentos;
    contagemBatimentos = 0;
    interrupts();

    // Converte para BPM: batimentos em 5s * 12
    bpmAtual = bat * 12;
    ultimaJanelaBPM = agora;

    Serial.print(F("[DEBUG] Batimentos em 5s: "));
    Serial.print(bat);
    Serial.print(F(" | BPM: "));
    Serial.println(bpmAtual);
  }

  return bpmAtual;
}

// ------------------------------------------------------------
//  VERIFICA ALERTAS CLINICOS
// ------------------------------------------------------------
bool verificarAlerta(float temp, float umid, int bpm) {
  return (temp > TEMP_ALERTA || umid > UMID_ALERTA || bpm > BPM_ALERTA);
}

// ------------------------------------------------------------
//  MONTA PAYLOAD JSON COMPATIVEL COM NODE-RED
// ------------------------------------------------------------
void montarPayload(const Amostra& a, char* payload, size_t tamanho) {
  snprintf(
    payload,
    tamanho,
    "{\"deviceId\":\"%s\",\"ts\":%lu,\"temp\":%.1f,\"umid\":%.1f,\"bpm\":%d,\"alerta\":%s}",
    MQTT_CLIENT_ID,
    a.timestamp,
    a.temperatura,
    a.umidade,
    a.bpm,
    a.alerta ? "true" : "false"
  );
}

// ------------------------------------------------------------
//  PUBLICA UMA AMOSTRA NO MQTT
// ------------------------------------------------------------
bool publicarAmostra(const Amostra& a) {
  char payload[160];
  montarPayload(a, payload, sizeof(payload));

  bool enviado = mqttClient.publish(MQTT_TOPIC, payload);

  Serial.print(enviado ? F("[MQTT] Publicado: ") : F("[MQTT] Falha ao publicar: "));
  Serial.println(payload);

  return enviado;
}

// ------------------------------------------------------------
//  ADICIONA AMOSTRA AO BUFFER CIRCULAR
// ------------------------------------------------------------
void adicionarAoBuffer(Amostra a) {
  if (bufferCount < MAX_AMOSTRAS) {
    buffer[bufferHead] = a;
    bufferHead = (bufferHead + 1) % MAX_AMOSTRAS;
    bufferCount++;
  } else {
    // Buffer cheio: estrategia FIFO - descarta a amostra mais antiga
    buffer[bufferHead] = a;
    bufferHead = (bufferHead + 1) % MAX_AMOSTRAS;
    Serial.println(F("[AVISO] Buffer cheio - amostra mais antiga descartada (FIFO)"));
  }
}

// ------------------------------------------------------------
//  SINCRONIZA BUFFER COM A NUVEM VIA MQTT
// ------------------------------------------------------------
void sincronizarNuvem() {
  if (bufferCount == 0 || !mqttClient.connected()) return;

  Serial.println(F("\n[NUVEM] Sincronizando amostras pendentes..."));

  int inicio = (bufferHead - bufferCount + MAX_AMOSTRAS) % MAX_AMOSTRAS;
  bool sucesso = true;

  for (int i = 0; i < bufferCount; i++) {
    int idx = (inicio + i) % MAX_AMOSTRAS;

    if (!publicarAmostra(buffer[idx])) {
      sucesso = false;
      break;
    }

    delay(50); // pequeno intervalo para evitar rajada no broker
  }

  if (sucesso) {
    Serial.print(F("[NUVEM] "));
    Serial.print(bufferCount);
    Serial.println(F(" amostras sincronizadas. Buffer limpo.\n"));

    bufferHead = 0;
    bufferCount = 0;
  } else {
    Serial.println(F("[NUVEM] Sincronizacao interrompida. Buffer preservado.\n"));
  }
}

// ------------------------------------------------------------
//  IMPRIME LEITURA OFFLINE NO MONITOR SERIAL
// ------------------------------------------------------------
void imprimirLeituraOffline(const Amostra& a) {
  Serial.print(F("[OFFLINE] T="));
  Serial.print(a.timestamp);
  Serial.print(F("ms | Temp="));
  Serial.print(a.temperatura, 1);
  Serial.print(F("C | Umid="));
  Serial.print(a.umidade, 1);
  Serial.print(F("% | BPM="));
  Serial.print(a.bpm);
  Serial.print(F(" | Buffer="));
  Serial.print(bufferCount);
  Serial.print(F("/"));
  Serial.print(MAX_AMOSTRAS);

  if (a.alerta) {
    Serial.print(F(" | ALERTA CLINICO"));
  }

  Serial.println();
}

// ------------------------------------------------------------
//  LOOP PRINCIPAL
// ------------------------------------------------------------
void loop() {
  unsigned long agora = millis();

  // Mantem a infraestrutura de rede do Wokwi para permitir MQTT real
  conectarWifi();

  // Reaproveita a simulacao de conectividade da Parte 1
  simularWifi();

  if (wifiConectado) {
    conectarMqtt();
  }

  if (wifiConectado && mqttClient.connected()) {
    mqttClient.loop();
    sincronizarNuvem();
  }

  // Calcula BPM a cada janela de 5 segundos
  int bpm = calcularBPM();

  // Realiza leitura dos sensores no intervalo configurado
  if (agora - ultimaLeitura >= LEITURA_MS) {
    ultimaLeitura = agora;

    float temp = dht.readTemperature();
    float umid = dht.readHumidity();

    if (isnan(temp) || isnan(umid)) {
      Serial.println(F("[ERRO] Falha na leitura do DHT22 - ignorando amostra."));
      return;
    }

    Amostra amostra;
    amostra.timestamp = agora;
    amostra.temperatura = temp;
    amostra.umidade = umid;
    amostra.bpm = bpm;
    amostra.alerta = verificarAlerta(temp, umid, bpm);

    if (wifiConectado && mqttClient.connected()) {
      if (!publicarAmostra(amostra)) {
        adicionarAoBuffer(amostra);
        imprimirLeituraOffline(amostra);
      }
    } else {
      adicionarAoBuffer(amostra);
      imprimirLeituraOffline(amostra);
    }
  }
}
