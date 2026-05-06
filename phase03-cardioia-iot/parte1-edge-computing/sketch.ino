// ============================================================
//  CardioIA – Fase 3 | Monitoramento Contínuo IoT na Saúde
//  Dispositivo : ESP32
//  Sensores    : DHT22 (temperatura + umidade) + Botão (BPM)
//  Resiliência : Array em RAM (100 amostras) + Monitor Serial
//  Autor       : Equipe CardioIA – FIAP
// ============================================================

#include <Arduino.h>
#include "DHT.h"

// ──────────────────────────────────────────────
//  PINOS
// ──────────────────────────────────────────────
#define DHT_PIN     4        // Pino de dados do DHT22
#define DHT_TYPE    DHT22
#define BUTTON_PIN  15       // Botão que simula batimento cardíaco

// ──────────────────────────────────────────────
//  CONSTANTES DE CONFIGURAÇÃO
// ──────────────────────────────────────────────
const int    MAX_AMOSTRAS      = 100;   // Máx. de amostras offline
const unsigned long LEITURA_MS = 2000;  // Intervalo de leitura (ms)

// Limites de alerta clínico
const float  TEMP_ALERTA  = 38.0;  // °C
const int    BPM_ALERTA   = 120;   // bpm
const float  UMID_ALERTA  = 90.0;  // %

// ──────────────────────────────────────────────
//  ESTRUTURA DE AMOSTRA
// ──────────────────────────────────────────────
struct Amostra {
  unsigned long timestamp; // ms desde boot
  float temperatura;       // °C
  float umidade;           // %
  int   bpm;               // batimentos por minuto
  bool  alerta;            // true se algum valor excede limite
};

// ──────────────────────────────────────────────
//  VARIÁVEIS GLOBAIS
// ──────────────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);

// Buffer circular de resiliência offline
Amostra buffer[MAX_AMOSTRAS];
int  bufferHead  = 0;   // próxima posição de escrita
int  bufferCount = 0;   // amostras ainda não enviadas

// Contagem de batimentos para cálculo de BPM
volatile int  contagemBatimentos = 0;    // incrementado no ISR
volatile unsigned long ultimoBatimentoISR = 0;  // debounce
const unsigned long DEBOUNCE_MS = 200;   // intervalo mínimo entre batimentos (ms)
unsigned long ultimaJanelaBPM    = 0;
int           bpmAtual           = 0;

// Simulação de conectividade Wi-Fi (variável booleana)
bool wifiConectado = false;
unsigned long ultimoToggleWifi = 0;
const unsigned long INTERVALO_TOGGLE = 15000; // alterna a cada 15 s (demo)

// Controle de tempo das leituras
unsigned long ultimaLeitura = 0;

// ──────────────────────────────────────────────
//  ISR – Interrupção do botão COM DEBOUNCE
// ──────────────────────────────────────────────
void IRAM_ATTR isr_batimento() {
  unsigned long agora = millis();
  
  // Debounce: ignora pulsos com menos de 200ms do último batimento
  if (agora - ultimoBatimentoISR > DEBOUNCE_MS) {
    contagemBatimentos++;
    ultimoBatimentoISR = agora;
  }
}

// ──────────────────────────────────────────────
//  SETUP
// ──────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println(F("╔══════════════════════════════════╗"));
  Serial.println(F("║   CardioIA – Monitor Cardíaco    ║"));
  Serial.println(F("║   FIAP – Fase 3 – IoT na Saúde   ║"));
  Serial.println(F("╚══════════════════════════════════╝"));

  dht.begin();

  // Configura botão com pull-up interno; dispara na borda de descida
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr_batimento, FALLING);

  ultimaJanelaBPM  = millis();
  ultimaLeitura    = millis();
  ultimoToggleWifi = millis();

  Serial.println(F("[INFO] Sistema inicializado. Buffer offline: 100 amostras."));
  Serial.println(F("[INFO] Wi-Fi: DESCONECTADO (simulação inicia offline)"));
  Serial.println(F("[INFO] Debounce configurado: 200ms entre batimentos"));
  Serial.println(F("---------------------------------------------------"));
}

// ──────────────────────────────────────────────
//  CALCULA BPM (janela de 5 segundos)
// ──────────────────────────────────────────────
int calcularBPM() {
  unsigned long agora = millis();
  
  // Calcula BPM a cada 5 segundos (mais rápido para demo)
  if (agora - ultimaJanelaBPM >= 5000) {
    noInterrupts();
    int bat = contagemBatimentos;
    contagemBatimentos = 0;
    interrupts();
    
    // Converte para BPM: batimentos em 5s * 12
    bpmAtual = bat * 12;
    ultimaJanelaBPM = agora;
    
    // Debug: mostra a contagem para validação
    Serial.print(F("[DEBUG] Batimentos em 5s: "));
    Serial.print(bat);
    Serial.print(F(" → BPM: "));
    Serial.println(bpmAtual);
  }
  return bpmAtual;
}

// ──────────────────────────────────────────────
//  VERIFICA ALERTAS CLÍNICOS
// ──────────────────────────────────────────────
bool verificarAlerta(float temp, float umid, int bpm) {
  return (temp > TEMP_ALERTA || umid > UMID_ALERTA || bpm > BPM_ALERTA);
}

// ──────────────────────────────────────────────
//  ADICIONA AMOSTRA AO BUFFER CIRCULAR
// ──────────────────────────────────────────────
void adicionarAoBuffer(Amostra a) {
  if (bufferCount < MAX_AMOSTRAS) {
    // Buffer ainda tem espaço
    buffer[bufferHead] = a;
    bufferHead = (bufferHead + 1) % MAX_AMOSTRAS;
    bufferCount++;
  } else {
    // Buffer cheio: estratégia FIFO – descarta a mais antiga
    // (sobrescreve a posição mais antiga)
    buffer[bufferHead] = a;
    bufferHead = (bufferHead + 1) % MAX_AMOSTRAS;
    // bufferCount permanece MAX_AMOSTRAS
    Serial.println(F("[AVISO] Buffer cheio – amostra mais antiga descartada (FIFO)"));
  }
}

// ──────────────────────────────────────────────
//  SINCRONIZA BUFFER COM A "NUVEM" (Serial)
//  Representa o envio via MQTT quando online
// ──────────────────────────────────────────────
void sincronizarNuvem() {
  if (bufferCount == 0) return;

  Serial.println(F("\n[NUVEM] Iniciando sincronização de amostras pendentes..."));

  // Calcula o índice da amostra mais antiga no buffer circular
  int inicio = (bufferHead - bufferCount + MAX_AMOSTRAS) % MAX_AMOSTRAS;

  for (int i = 0; i < bufferCount; i++) {
    int idx = (inicio + i) % MAX_AMOSTRAS;
    Amostra& a = buffer[idx];

    // Formato JSON simulando payload MQTT
    Serial.print(F("[MQTT→NUVEM] {"));
    Serial.print(F("\"ts\":"));    Serial.print(a.timestamp);
    Serial.print(F(",\"temp\":"));  Serial.print(a.temperatura, 1);
    Serial.print(F(",\"umid\":"));  Serial.print(a.umidade, 1);
    Serial.print(F(",\"bpm\":"));   Serial.print(a.bpm);
    Serial.print(F(",\"alerta\":")); Serial.print(a.alerta ? "true" : "false");
    Serial.println(F("}"));
  }

  Serial.print(F("[NUVEM] "));
  Serial.print(bufferCount);
  Serial.println(F(" amostras sincronizadas. Buffer limpo.\n"));

  // Limpa o buffer após sincronização bem-sucedida
  bufferHead  = 0;
  bufferCount = 0;
}

// ──────────────────────────────────────────────
//  IMPRIME LEITURA NO MONITOR SERIAL (offline)
// ──────────────────────────────────────────────
void imprimirLeituraSerial(Amostra& a) {
  Serial.print(F("[OFFLINE] T="));
  Serial.print(a.timestamp);
  Serial.print(F("ms | Temp="));
  Serial.print(a.temperatura, 1);
  Serial.print(F("°C | Umid="));
  Serial.print(a.umidade, 1);
  Serial.print(F("% | BPM="));
  Serial.print(a.bpm);
  Serial.print(F(" | Buffer="));
  Serial.print(bufferCount);
  Serial.print(F("/"));
  Serial.print(MAX_AMOSTRAS);
  if (a.alerta) {
    Serial.print(F(" | ⚠ ALERTA CLÍNICO!"));
  }
  Serial.println();
}

// ──────────────────────────────────────────────
//  SIMULA ALTERNÂNCIA DE CONECTIVIDADE Wi-Fi
// ──────────────────────────────────────────────
void simularWifi() {
  unsigned long agora = millis();
  if (agora - ultimoToggleWifi >= INTERVALO_TOGGLE) {
    wifiConectado = !wifiConectado;
    ultimoToggleWifi = agora;
    Serial.print(F("\n[WIFI] Status alterado → "));
    Serial.println(wifiConectado ? F("CONECTADO ✓") : F("DESCONECTADO ✗"));
  }
}

// ──────────────────────────────────────────────
//  LOOP PRINCIPAL
// ──────────────────────────────────────────────
void loop() {
  unsigned long agora = millis();

  // 1. Simula alternância de Wi-Fi (demo de resiliência)
  simularWifi();

  // 2. Calcula BPM a cada janela de 5 segundos
  int bpm = calcularBPM();

  // 3. Realiza leitura dos sensores no intervalo configurado
  if (agora - ultimaLeitura >= LEITURA_MS) {
    ultimaLeitura = agora;

    // Lê DHT22
    float temp = dht.readTemperature();
    float umid = dht.readHumidity();

    // Valida leitura (DHT pode retornar NaN em falha)
    if (isnan(temp) || isnan(umid)) {
      Serial.println(F("[ERRO] Falha na leitura do DHT22 – ignorando amostra."));
      return;
    }

    // Monta amostra
    Amostra amostra;
    amostra.timestamp   = agora;
    amostra.temperatura = temp;
    amostra.umidade     = umid;
    amostra.bpm         = bpm;
    amostra.alerta      = verificarAlerta(temp, umid, bpm);

    // 4. Decide: online → envia direto; offline → armazena no buffer
    if (wifiConectado) {
      // Primeiro sincroniza pendências do buffer
      sincronizarNuvem();

      // Envia leitura atual diretamente (simula MQTT publish)
      Serial.print(F("[MQTT→NUVEM] {"));
      Serial.print(F("\"ts\":"));    Serial.print(amostra.timestamp);
      Serial.print(F(",\"temp\":"));  Serial.print(amostra.temperatura, 1);
      Serial.print(F(",\"umid\":"));  Serial.print(amostra.umidade, 1);
      Serial.print(F(",\"bpm\":"));   Serial.print(amostra.bpm);
      Serial.print(F(",\"alerta\":")); Serial.print(amostra.alerta ? "true" : "false");
      Serial.println(F("}"));
    } else {
      // Modo offline: armazena no buffer circular
      adicionarAoBuffer(amostra);
      imprimirLeituraSerial(amostra);
    }
  }
}