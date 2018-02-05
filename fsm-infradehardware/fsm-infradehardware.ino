// Includes
#include "fsm_config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Definições
#define DHTPIN D3     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)

// Constantes
const int buttonPin = D5;    // definicao do pino utilizado pelo botao
const int ledPin = D7;       // definicao do pino utilizado pelo led
const char* ssid = "redimi 4x"; // nome da rede wi-fi
const char* password = "12345678"; // senha da rede wi-fi
const char* mqtt_server = "things.ubidots.com"; // url do servidor

// Variáveis
int buttonState = LOW;             // armazena a leitura atual do botao
int lastButtonState = LOW;         // armazena a leitura anterior do botao
unsigned long lastDebounceTime = 0;  // armazena a ultima vez que a leitura da entrada variou
unsigned long debounceDelay = 50;    // tempo utilizado para implementar o debounce
long lastMsg = 0;
char msg[50];
int value = 0;
float hum;  //Stores humidity value
float temp; //Stores temperature value
char umidade[50];
char temperatura[50];

// Objetos
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);


// definicao das funcoes relativas a cada estado
event connect_state(void) {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client","A1E-SXNsNzPsH1n7KQyjs75tCHlnK2NjVZ","")) {
      Serial.println("connected");
      hum = dht.readHumidity();
      temp = dht.readTemperature();
      Serial.print("Humidity: ");
      Serial.print(hum);
      Serial.print(" %, Temp: ");
      Serial.print(temp);
      Serial.println(" Celsius");
      // AQUI DÁ FATAL EXCEPTION 29, POR ISSO COMENTAMOS E BOTAMOS HARDCODED
//      snprintf(umidade, 50, "{\"value\":%d}", dtostrf(hum, 6, 2, NULL));
//      snprintf(temperatura, 50, "{\"value\":%d}", dtostrf(temp, 6, 2, NULL));
      // AQUI COMEÇAM OS TESTES, ELA ENVIA A UMIDADE
      client.publish("/v1.6/devices/mytemstatus/umidade", "{\"value\": 28}");
      //client.subscribe("inTopic");
      // AQUI A CONEXÃO É PERDIDA E A TEMPERATURA NÃO É ENVIADA, A ROTA ESTÁ CORRETA
      client.publish("/v1.6/devices/mytemstatus/temp", "{\"value\": 50}");
      //client.subscribe("inTopic");
      return empty;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  return action;
}

event idle_state(void) {
  int tempoComeco = millis();
  int tempoFinal = tempoComeco + 10000;
  while (tempoComeco < tempoFinal) {
    if(read_button()) {
      return action;
    }
    tempoComeco = millis();
  }
  return empty;
}

event send_data_state(void) {
  if (client.loop()) {
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    Serial.println("Button pressed");
    snprintf(umidade, 50, "{\"value\":%s}", dtostrf(hum, 6, 2, NULL));
    snprintf(temperatura, 50, "{\"value\":%s}", dtostrf(temp, 6, 2, NULL));
    client.publish("/v1.6/devices/mytemstatus/temp", umidade);
    client.publish("/v1.6/devices/mytemstatus/umidade", temperatura);
    client.subscribe("inTopic");
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    return empty;
  } else {
    return empty;
  }
  enviar();
}

event send_data_button_state(void) {
  if (client.loop()) {
    hum = dht.readHumidity();
    temp = dht.readTemperature();
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    Serial.println("Button pressed");
    snprintf(umidade, 50, "{\"value\":%s}", dtostrf(hum, 6, 2, NULL));
    snprintf(temperatura, 50, "{\"value\":%s}", dtostrf(temp, 6, 2, NULL));
    client.publish("/v1.6/devices/mytemstatus/temp", umidade);
    client.publish("/v1.6/devices/mytemstatus/umidade", temperatura);
    client.publish("/v1.6/devices/mytemstatus/button", "{\"value\":100}");
    client.subscribe("inTopic");
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    return empty;
  } else {
    return action;
  }
}

event no_connection_state(void) {
  return empty;
}

event end_state(void) {
  
}

// variaveis que armazenam estado atual, evento atual e funcao de tratamento do estado atual
state cur_state = ENTRY_STATE;
event cur_evt;
event (* cur_state_function)(void);

// implementacao de funcoes auxiliares
int read_button() {
  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  lastButtonState = reading;
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        return true;
      }
    }
  }
  return false;
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(ledPin, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(ledPin, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void enviar() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client","A1E-SXNsNzPsH1n7KQyjs75tCHlnK2NjVZ","")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("/v1.6/devices/mytemstatus/button", "{\"value\":100}");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  pinMode(ledPin, OUTPUT);
  dht.begin();
}

void loop() {
  cur_state_function = state_functions[cur_state];
  cur_evt = (event) cur_state_function();
  if (EXIT_STATE == cur_state)
    return;
  cur_state = lookup_transitions(cur_state, cur_evt);
}
