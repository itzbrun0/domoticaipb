# Domótica – Internet das Coisas (IoT)

**Autores:**  
Bruno Nogueira (a56543), Jorge Costa (a56575)

---

## Descrição do Projeto

Este projeto de Internet das Coisas (IoT) consiste no desenvolvimento de um sistema integrado para automação e monitorização ambiental de um espaço interior. O sistema usa um microcontrolador ESP32, sensores ambientais, atuadores e armazenamento de dados em base temporal.

A comunicação entre os dispositivos e a plataforma central é feita via protocolo MQTT. A plataforma Node-RED gere os fluxos de dados e a visualização. Os dados ambientais — como temperatura, humidade, qualidade do ar, luminosidade, posição da cortina e janela, e estado do ar condicionado — são armazenados num banco de dados temporal InfluxDB, permitindo análises históricas e visualização através de dashboards.

O sistema opera em modos manual e automático, integrando também dados meteorológicos externos para otimizar o controlo do ambiente interior.

---

## Estrutura do Repositório

- `Firmware`  
  Código fonte do ESP32 desenvolvido para controlo dos sensores e atuadores, comunicação via MQTT.

- `Node-red-flows`  
  Exportação JSON dos fluxos Node-RED que orquestram os dados, integram APIs externas, controlam dispositivos e enviam dados para o InfluxDB.

- `Influxdb`  
  Scripts para configuração do InfluxDB, queries ou configurações relevantes.

- `README.md`  
  Este ficheiro com instruções e descrição do projeto.

---

## Tecnologias e Ferramentas

- **ESP32:** Microcontrolador principal do sistema.  
- **MQTT:** Protocolo de comunicação leve usado para troca de mensagens.  
- **Node-RED:** Plataforma para orquestração e controlo do sistema, criação dos dashboards e integração com APIs.  
- **InfluxDB:** Banco de dados temporal para armazenamento e análise histórica dos dados ambientais.  
- **Wokwi:** Ambiente online para desenvolvimento e simulação do firmware ESP32.

---

## Como executar o projeto

### 1. Firmware ESP32

- Fazer download do wowki_code.ino 
- Carregar o código no ESP32 via Arduino IDE ou plataforma compatível.  
- Configurar credenciais WiFi e endereço do broker MQTT (`broker.emqx.io`).

### 2. Node-RED

- Importar o fluxo JSON disponível na tua instância Node-RED.  
- Configurar os nodes MQTT para apontar para o broker MQTT público usado.  
- Configurar o node InfluxDB com as credenciais e bucket da tua base InfluxDB.

### 3. InfluxDB

- Caso uses InfluxDB local ou cloud, cria um bucket chamado **ProjetoFinal** para armazenar os dados ambientais.  
- Configurar as credenciais e a ligação no Node-RED para escrita e consulta dos dados neste bucket.  
- Para facilitar a visualização dos dados, a dashboard personalizada do InfluxDB foi exportada em formato JSON.  
- Para importar a dashboard:  
  1. Acede ao UI do InfluxDB.  
  2. Vai ao menu **Dashboards** → **Importar**.  
  3. Seleciona o ficheiro JSON disponibilizado.  
  4. A dashboard será carregada com todos os gráficos e configurações usados no projeto.


## Referências e Fontes

- [Projeto Wokwi: Simulação do ESP32 com servo](https://wokwi.com/projects/323706614646309460)  
- [Projeto Wokwi: Simulação do ESP32 com LED RGB](https://wokwi.com/projects/391299344938984449)  
- [ChatGPT - OpenAI](https://chat.openai.com/) para apoio informativo e desenvolvimento do projeto.  
- [OpenWeather API](https://openweathermap.org/api)  
- [Instituto Português do Mar e da Atmosfera – IPMA](https://api.ipma.pt/)  
- [Wikipedia](https://pt.wikipedia.org)  
- [Google Scholar](https://scholar.google.com/)

---
