#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Definição dos pinos usados para o LED RGB e servos
#define RED_PIN 16
#define GREEN_PIN 17
#define BLUE_PIN 18
#define SERVO_CORTINA_PIN 19
#define SERVO_JANELA_PIN 26
#define LED 33

// Credenciais WiFi e broker MQTT
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);

Servo cortinaServo;  // Servo para controlo da cortina
Servo janelaServo;   // Servo para controlo da janela

// Modos de funcionamento: manual ou automático para cada dispositivo
String modoCond = "manual";
String modoCortina = "manual";
String modoJanela = "manual";

// Estado atual do ar condicionado (para controlo da janela em automático)
String estadoCondicionador = "desligado";

// Função para publicar estado (string) num tópico MQTT
void publicarEstado(const char* topico, const char* chave, const char* valor) {
  StaticJsonDocument<100> doc;
  doc[chave] = valor;
  char buffer[100];
  serializeJson(doc, buffer);
  client.publish(topico, buffer);
}

// Função para publicar estado (inteiro) num tópico MQTT
void publicarEstado(const char* topico, const char* chave, int valor) {
  StaticJsonDocument<100> doc;
  doc[chave] = valor;
  char buffer[100];
  serializeJson(doc, buffer);
  client.publish(topico, buffer);
}

// Função para ligar ao WiFi
void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi");
}

// Funções para controlar o LED RGB que indica o estado do ar condicionado

void desligarAr() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  estadoCondicionador = "desligado";
  publicarEstado("status/condicionador", "condicionador", "desligado");
}

void aquecer() {
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  estadoCondicionador = "aquecendo";
  publicarEstado("status/condicionador", "condicionador", "aquecendo");
}

void arrefecer() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, HIGH);
  estadoCondicionador = "arrefecendo";
  publicarEstado("status/condicionador", "condicionador", "arrefecendo");
}

void ventilando() {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
  estadoCondicionador = "ventilando";
  publicarEstado("status/condicionador", "condicionador", "ventilando");
}

void dry() {
  digitalWrite(RED_PIN, HIGH);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
  estadoCondicionador = "dry";
  publicarEstado("status/condicionador", "condicionador", "dry");
}

// Função callback para tratar mensagens recebidas via MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  // Copia o payload para uma string JSON terminada em \0
  char json[length + 1];
  memcpy(json, payload, length);
  json[length] = '\0';

  StaticJsonDocument<256> doc;
  // Deserializa o JSON recebido. Se falhar, imprime erro e sai.
  if (deserializeJson(doc, json)) {
    Serial.println("Erro ao processar JSON");
    return;
  }

  String topicoStr = String(topic);

  // Tratamento do modo do ar condicionado
  if (topicoStr == "condicionador/mode") {
    String novoModo = doc["modo"];
    if (novoModo == "manual" || novoModo == "auto") {
      modoCond = novoModo;
      Serial.print("Modo atual do condicionador: ");
      Serial.println(modoCond);
    }
  }

  // Tratamento do modo da cortina
  if (topicoStr == "cortinas/mode") {
    String novoModo = doc["modo"];
    if (novoModo == "manual" || novoModo == "auto") {
      modoCortina = novoModo;
      Serial.print("Modo atual da cortina: ");
      Serial.println(modoCortina);
    }
  }

  // Tratamento do modo da janela
  if (topicoStr == "janela/mode") {
    String novoModo = doc["modo"];
    if (novoModo == "manual" || novoModo == "auto") {
      modoJanela = novoModo;
      Serial.print("Modo atual da janela: ");
      Serial.println(modoJanela);
    }
  }

  // Controle do ar condicionado (modo manual)
  if (topicoStr == "condicionador/controle") {
    if (modoCond == "manual") {
      if (doc.containsKey("condicionador")) {
        String modo = doc["condicionador"].as<String>();
        Serial.print("Comando manual recebido: ");
        Serial.println(modo);

        // Liga ou desliga o ar condicionado conforme comando
        if (modo == "desligado") desligarAr();
        else if (modo == "aquecendo") aquecer();
        else if (modo == "arrefecendo") arrefecer();
        else if (modo == "ventilando") ventilando();
        else if (modo == "dry") dry();
      } else {
        Serial.println("❌ Campo 'condicionador' não encontrado no JSON!");
      }
    } else {
      Serial.println("Modo AUTO ativo — comandos manuais ignorados.");
    }
  }

  // Controle da cortina (modo manual)
  if (topicoStr == "cortinas/controle") {
    if (modoCortina == "manual") {
      int cortina = doc["cortina"];
      // Define posição do servo conforme comando: 0 (fechado), 90 (meio), 180 (aberto)
      if (cortina == 1) {
        cortinaServo.write(180);
        digitalWrite(LED, HIGH);
      } else if (cortina == 0) {
        cortinaServo.write(0);
        digitalWrite(LED, LOW);
      } else if (cortina == 2) {
        cortinaServo.write(90);
        digitalWrite(LED, HIGH);
      }
      publicarEstado("status/cortina", "cortina", cortina);
    } else {
      Serial.println("Modo automático ativo — comando manual ignorado (cortina).");
    }
  }

  // Controle da janela (modo manual)
  if (topicoStr == "janela/controle") {
    if (modoJanela == "manual") {
      int janela = doc["janela"];
      // Define posição do servo conforme comando
      if (janela == 1) {
        janelaServo.write(180);
      } else if (janela == 0) {
        janelaServo.write(0);
      } else if (janela == 2) {
        janelaServo.write(90);
      }
      publicarEstado("status/janela", "janela", janela);
    } else {
      Serial.println("Modo automático ativo — comando manual ignorado (janela).");
    }
  }

  // Receção dos dados do sensor de qualidade do ar
  if (topicoStr == "IPB/IoT/Lab/AirQuality") {
    int co2 = doc["co2_eqv"];
    float temp = doc["temp"];
    int hum = doc["hum"];

    Serial.print("CO2 recebido: ");
    Serial.println(co2);
    Serial.print("Temperatura recebida: ");
    Serial.println(temp);
    Serial.print("Humidade recebida: ");
    Serial.print(hum);
    Serial.println("%");

    // Verificação dos valores de CO2
    if (co2 > 1000) Serial.println("⚠️  Valor de CO₂ muito elevado!");
    else if (co2 < 400) Serial.println("🔵 Valor de CO₂ muito baixo.");
    else Serial.println("✅ Valor de CO₂ normal.");

    // Verificação da humidade relativa
    if (hum < 30) Serial.println("🔴 Humidade muito baixa.");
    else if (hum > 60) Serial.println("🔺 Humidade muito alta.");
    else if (hum >= 40 && hum <= 60) Serial.println("✅ Humidade ideal.");
    else Serial.println("🟡 Humidade aceitável.");

    // Lógica automática do ar condicionado consoante temperatura
    if (modoCond == "auto") {
      if (temp < 22.0) aquecer();
      else if (temp > 24.0) arrefecer();
      else desligarAr();
    } else {
      Serial.println("Modo manual ativo — sensores ignorados.");
    }
  }

  // Receção dos dados do vento externo
  if (topicoStr == "IPB/IoT/Lab/ExternalWind/response") {
    float velocidade = doc["intensidadeVento"];
    Serial.print("Velocidade do vento recebida: ");
    Serial.print(velocidade);
    Serial.println(" km/h");

    // Controle automático da janela conforme vento, apenas se ar condicionado desligado
    if (modoJanela == "auto") {
      if (estadoCondicionador != "desligado") {
        Serial.println("Condicionador ligado — janela não será aberta.");
        return;
      }

      if (velocidade > 20.0) {
        Serial.println("💨 Vento forte — a fechar janela.");
        janelaServo.write(0);
        publicarEstado("status/janela", "janela", 0);
      } else if (velocidade >= 5.0 && velocidade <= 20.0) {
        Serial.println("🌬️ Vento moderado — a abrir janela a meio.");
        janelaServo.write(90);
        publicarEstado("status/janela", "janela", 2);
      } else {
        Serial.println("🍃 Vento fraco — a abrir janela completamente.");
        janelaServo.write(180);
        publicarEstado("status/janela", "janela", 1);
      }
    } else {
      Serial.println("Modo manual da janela ativo — ignorar vento.");
    }
  }

  // Receção dos dados da luz externa
  if (topicoStr == "IPB/IoT/Lab/ExternalLux/response") {
    int lux = doc["lux"];
    Serial.print("Luz recebida (lux): ");
    Serial.println(lux);

    // Controle automático da cortina conforme luminosidade
    if (modoCortina == "auto") {
      if (lux <= 1000) {
        Serial.println("Luz baixa — cortina aberta.");
        cortinaServo.write(180);
        digitalWrite(LED, HIGH);
        publicarEstado("status/cortina", "cortina", 1);
      } else if (lux <= 10000) {
        Serial.println("Luz moderada — cortina semi-aberta.");
        cortinaServo.write(90);
        digitalWrite(LED, HIGH);
        publicarEstado("status/cortina", "cortina", 2);
      } else {
        Serial.println("Luz forte — cortina fechada (bloquear sol/calor).");
        cortinaServo.write(0);
        digitalWrite(LED, LOW);
        publicarEstado("status/cortina", "cortina", 0);
      }
    } else {
      Serial.println("Modo manual da cortina ativo — ignorar luz.");
    }
  }
}

// Função que garante ligação ao broker MQTT (reconexão automática)
void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      // Subscreve os tópicos importantes após ligação
      client.subscribe("condicionador/controle");
      client.subscribe("condicionador/mode");
      client.subscribe("cortinas/controle");
      client.subscribe("cortinas/mode");
      client.subscribe("janela/controle");
      client.subscribe("janela/mode");
      client.subscribe("IPB/IoT/Lab/ExternalWind/response");
      client.subscribe("IPB/IoT/Lab/ExternalLux/response");
    } else {
      delay(5000);  // Espera antes de tentar novamente
    }
  }
}

void setup() {
  Serial.begin(115200);
  // Configura os pinos dos LEDs como saída
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(LED, OUTPUT);

  // Associa os servos aos pinos e posiciona inicialmente fechados
  cortinaServo.attach(SERVO_CORTINA_PIN);
  cortinaServo.write(0);
  janelaServo.attach(SERVO_JANELA_PIN);
  janelaServo.write(0);

  // Conecta ao WiFi e configura MQTT
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // Garante ligação ao broker MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
