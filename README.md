# 🍷 Vinheria Agnello – IoT com ESP32 e MQTT (Checkpoint 05)

**FIAP – Engenharia de Software**
**Disciplina:** Edge Computing & Computer Systems
**Professor:** Lucas Demetrius Augusto
**Turma:** 1ESR | **Data da atividade:** 22/10/2026

---

## 👥 Equipe

| Nome | Rm |
|---|---|
| Esther dos Santos de Almeida Tozzo | 570860 |
| Felipe de Oliveira Zimmermann | 570863 |
| Izabela Pordeus de Almeida | 570316 |
| João Victor Santos Souza | 569949 |
| Matheus Lopes Lima | 571458 |

🔗 **Projeto no Wokwi:**[ [https://wokwi.com/projects/478608964354736129)](https://wokwi.com/projects/478608964354736129](https://wokwi.com/projects/478608964354736129)](https://wokwi.com/projects/478608964354736129)

---

## 📌 1. Descrição do Projeto

A Vinheria Agnello deseja evoluir para a Internet das Coisas (IoT) e expandir sua atuação no mercado europeu. Na fase anterior, o sistema avisava quando o ambiente estava muito claro. Agora os proprietários precisam saber **exatamente a temperatura e a umidade do depósito**, além da luminosidade, e ter esses dados enviados à internet para consulta, histórico e rastreabilidade.

Este checkpoint implementa a **camada de dispositivo (edge)** dessa solução: um **ESP32** lê os sensores **DHT** (temperatura e umidade) e **LDR** (luminosidade), marca cada leitura com **timestamp** e publica os dados via **Wi-Fi** usando o protocolo **MQTT**. O aplicativo **MyMQTT** é usado para **ler** os dados e **escrever** comandos (liga/desliga LED).

### Por que monitorar esses fatores?
| Fator | Impacto na qualidade do vinho | Faixa ideal |
|---|---|---|
| **Luminosidade** | Raios UV alteram compostos orgânicos e geram sabores desagradáveis (brancos e espumantes sofrem mais) | Penumbra |
| **Temperatura** | Calor excessivo encurta a vida do vinho; oscilações > 3 °C geram aromas indesejados | ~13 °C |
| **Umidade** | Pouca umidade resseca a rolha (oxidação); muita danifica rótulos e favorece fungos | ~70 % (60–80 %) |

---

## ✅ 2. Requisitos Atendidos

| # | Requisito do enunciado | Como foi atendido |
|---|---|---|
| 1 | Enviar dados dos sensores via **MQTT** | Biblioteca PubSubClient publica em `/TEF/<ID>/attrs` |
| 2 | Incluir **timestamp** em cada leitura | Hora obtida por NTP e enviada no campo `ts` (ISO 8601) |
| 3 | MCU envia dados à internet por **Wi-Fi** | ESP32 em modo estação (WiFi.h) |
| 4 | Interface compatível com **NGSI** | Tópicos e payload no padrão do *IoT Agent MQTT (Ultralight)* do FIWARE, que converte para NGSIv2 (ver seção 9) |
| – | Leitura de temperatura/umidade (DHT) e luminosidade (LDR) | Funções `dht.read*()` e `analogRead()` |
| – | Leitura e escrita pelo **MyMQTT** | Assinatura dos tópicos `attrs` e publicação em `cmd` |

---

## 🏗️ 3. Arquitetura

```
 ┌─────────────────────┐   Wi-Fi    ┌──────────────┐    MQTT     ┌──────────┐
 │ ESP32               │──────────▶│ Broker MQTT  │◀──────────▶│ MyMQTT   │
 │  ├─ DHT (T e U)     │  publish  │ (porta 1883) │  subscribe  │ (celular)│
 │  ├─ LDR (luz)       │◀──────────│              │  publish    └──────────┘
 │  └─ LED (atuador)   │  comandos └──────────────┘
 └─────────────────────┘
```

Fluxo: o ESP32 lê os sensores a cada 5 s → monta o payload com timestamp → publica no broker → o MyMQTT, assinante dos tópicos, exibe os dados. Comandos publicados pelo MyMQTT chegam ao ESP32, que aciona o LED e confirma em `cmdexe`.

---

## 🧰 4. Materiais e Software

**Hardware (versão física):** ESP32 DevKit (ESP32-WROOM-32), sensor DHT11, LDR, LED + resistor 220 Ω, cabo micro-USB.
**Simulador (esta versão):** [Wokwi](https://wokwi.com) – ESP32 DevKit V1, DHT22*, módulo fotorresistor, LED e resistor.
**Bibliotecas:** `PubSubClient`, `DHT sensor library`, `Adafruit Unified Sensor`.
**Aplicativo:** MyMQTT (Android/iOS).

\* O Wokwi simula o DHT22. No hardware real do enunciado (DHT11), altere `#define DHT_TYPE DHT22` para `DHT11`.

---

## 🔌 5. Ligações (conforme montagem do enunciado)

| Componente | Pino do componente | Pino do ESP32 |
|---|---|---|
| DHT | DATA / SDA | **D4** |
| LDR (módulo) | AO | **D34** |
| LED (via resistor 220 Ω) | Anodo (A) | **D2** |
| Alimentação | VCC | **3V3** |
| Terra | GND | **GND** |

O circuito completo está em `diagram.json`.

---

## 📁 6. Arquivos do Projeto

| Arquivo | Função |
|---|---|
| `sketch.ino` | Firmware do ESP32 (leitura dos sensores, Wi-Fi, NTP, MQTT) |
| `diagram.json` | Circuito do Wokwi |
| `libraries.txt` | Bibliotecas necessárias |
| `README.md` | Esta documentação |

---

## ▶️ 7. Como Replicar o Projeto (passo a passo)

### 7.1 No Wokwi
1. Acesse [wokwi.com](https://wokwi.com), faça login e clique em **New Project → ESP32**.
2. Substitua o conteúdo de `sketch.ino`, `diagram.json` e `libraries.txt` pelos arquivos deste repositório.
3. No `sketch.ino`, altere `DEVICE_ID` para um valor **único do grupo** (ex.: `adega_g7_001`). Troque só o trecho `GRUPOX`.
4. Clique em ▶️ **Start Simulation**.
5. Confira no Serial Monitor: `Wi-Fi OK` → `Hora sincronizada via NTP` → `Conectando ao broker MQTT... OK` → leituras a cada 5 s, como:
   `[2026-10-22T10:00:00-03:00] T=13.0 C | U=70.0 % | L=12 %`
6. Clique no sensor DHT e no LDR para alterar temperatura, umidade e luz pelos controles deslizantes.

Configurações do código (topo do `sketch.ino`):

| Parâmetro | Valor |
|---|---|
| `WIFI_SSID` / `WIFI_PASS` | `Wokwi-GUEST` / vazio |
| `MQTT_BROKER` | `broker.hivemq.com` |
| `MQTT_PORT` | `1883` |
| `DEVICE_ID` | ID único do grupo |

### 7.2 Em hardware real (alternativa)
1. Monte o circuito conforme a seção 5.
2. Na Arduino IDE 2.x, instale a placa *esp32 by Espressif* e as bibliotecas citadas.
3. Altere `WIFI_SSID`, `WIFI_PASS` (rede 2,4 GHz), `MQTT_BROKER` (IP do servidor de IoT) e `DHT_TYPE` para `DHT11`.
4. Selecione *ESP32 Dev Module*, a porta COM, faça o upload e abra o Serial Monitor em 115200 baud.

---

## 📱 8. Leitura e Escrita com o MyMQTT

Exemplos com `DEVICE_ID = adega_g7_001`.

### Configuração
1. Abra **Settings**: Broker = `broker.hivemq.com`, Port = `1883`, sem usuário/senha.

### Leitura (Subscribe)
Assine:
- `/TEF/adega_g7_001/attrs/#` → temperatura (`t`), umidade (`h`), luminosidade (`l`) e a mensagem completa com timestamp.
- `/TEF/adega_g7_001/cmdexe` → confirmações de comandos.

### Escrita (Publish)
| Topic | Message | Resultado |
|---|---|---|
| `/TEF/adega_g7_001/cmd` | `adega_g7_001@on\|` | LED acende |
| `/TEF/adega_g7_001/cmd` | `adega_g7_001@off\|` | LED apaga |

A confirmação (`adega_g7_001@on|LED ligado`) chega em `/TEF/adega_g7_001/cmdexe`.

### Tópicos e formato dos dados
| Tópico | Direção | Conteúdo |
|---|---|---|
| `/TEF/<ID>/attrs` | ESP32 → broker | `t\|13.0\|h\|70.0\|l\|12\|ts\|2026-10-22T10:00:00-03:00` |
| `/TEF/<ID>/attrs/t` · `/h` · `/l` | ESP32 → broker | valor isolado (°C, %, %) |
| `/TEF/<ID>/cmd` | broker → ESP32 | `<ID>@on\|` ou `<ID>@off\|` |
| `/TEF/<ID>/cmdexe` | ESP32 → broker | confirmação do comando |

Legenda dos atributos: `t` = temperatura (°C), `h` = umidade relativa (%), `l` = luminosidade (0–100 %), `ts` = timestamp ISO 8601 (UTC-3).

---

## 🌍 9. Integração com FIWARE / NGSI (contexto do projeto)

O enunciado prevê o **FIWARE** como back-end (Orion Context Broker via NGSIv2, IoT Agent MQTT, STH-Comet/MongoDB para histórico) e o uso de **Smart Data Models** para interoperar com o ecossistema europeu.

Este checkpoint cobre a camada de dispositivo e a comunicação MQTT. O payload e os tópicos seguem o padrão **Ultralight do IoT Agent MQTT** (`/TEF/<device>/attrs`), de modo que, ao apontar o `MQTT_BROKER` para o servidor FIWARE e provisionar o dispositivo, o IoT Agent converte os dados em entidades **NGSIv2** no Orion, e o histórico passa a ser gravado pelo STH-Comet. Na demonstração, o broker público é usado apenas para simulação.

---

## 🧪 10. Testes Realizados

| Teste | Ação | Resultado esperado |
|---|---|---|
| Luminosidade | Aumentar/diminuir a luz no LDR | `l` varia de 0 a 100 % |
| Temperatura | Alterar o slider do DHT | `t` muda no Serial e no MyMQTT |
| Umidade | Alterar o slider do DHT | `h` muda no Serial e no MyMQTT |
| Timestamp | Observar cada leitura | Campo `ts` com data/hora corretas |
| Leitura MQTT | Assinar `attrs/#` | Mensagens a cada 5 s |
| Escrita MQTT | Publicar `@on\|` / `@off\|` | LED muda e `cmdexe` confirma |


---

## 🛠️ 11. Problemas Comuns

| Sintoma | Causa provável | Solução |
|---|---|---|
| Não conecta ao broker (`rc=-2`) | Broker fora do ar ou porta errada | Testar `test.mosquitto.org`, porta 1883 |
| Recebe dados de outros grupos | `DEVICE_ID` repetido | Usar ID único |
| `Falha na leitura do DHT` | Pino incorreto ou alimentação | Conferir SDA em D4 e 3V3 |
| LED não reage | ID ou tópico diferente do código | Conferir `/TEF/<ID>/cmd` e o `\|` final |
| Wi-Fi real não conecta | Rede 5 GHz | Usar 2,4 GHz |

---

## 📝 12. Observações
- O LDR é convertido para 0–100 %; pode ser necessário calibrar conforme o módulo.
- O broker público é aberto: não use dados sensíveis.
- Projeto acadêmico desenvolvido para o Checkpoint 05 da FIAP.
