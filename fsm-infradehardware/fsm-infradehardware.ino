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
const char* ssid = "Edna"; // nome da rede wi-fi
const char* password = "3dn4123@"; // senha da rede wi-fi
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
char button[50];

// Objetos
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

// variaveis que armazenam estado atual, evento atual e funcao de tratamento do estado atual
state cur_state = ENTRY_STATE;
event cur_evt;
event (* cur_state_function)(void);


// definicao das funcoes relativas a cada estado
event connect_state(void) {
  // Caso a WiFi não esteja conectada.
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }

  // Configura o servidor
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  Serial.println("Tentando se conectar ao servidor");
  client.connect("ESP8266Client","A1E-sai2K7TuqyU8xRbNNFPTGGMG5kKE6s",""); // Se conecta ao device no Ubidots

  // Se mesmo assim o client não estiver conectado
  if (!client.connected()) {
    return action; // Volta para o estado connect
  }
  return empty; // Vai para o estado idle
}

event idle_state(void) {
  Serial.println("Entrou no idle");
  int tempoInicio = millis();
  int tempoFinal = tempoInicio + 2000; // Tempo após 10 segundos
  
  while (tempoInicio < tempoFinal) {
    // Checa se o botão foi pressionado
    Serial.println("Entrou no loop");
    if(read_button()) {
      Serial.println("Apertou o botão");
      return action; // Vai para o estados send_data_button
    }
    tempoInicio = millis(); // Atualiza o tempoInicio
  }
  Serial.println("Saiu do loop");
  // Se passaram 10s, vai para o estado send_data
  return empty;
}

event send_data_state(void) {
  Serial.println("Entrou no send_data");
  ler_dados(0);
  // Checa se o client continua conectado
  if (client.loop()) {
    enviar_dados();
    piscar_led();
    return empty; // Vai para o estado idle
  } else {
    return action; // Vai para o estado no_connection
  }
}

event send_data_button_state(void) {
  Serial.println("Entrou no send_data_button");
  ler_dados(1);
  // Checa se o client continua conectado
  if (client.connected()) {
    enviar_dados();
    piscar_led();
    return empty; // Vai para o estado idle
  } else {
    return action; // Vai para o estado no_connection
  }
}

event no_connection_state(void) {
  Serial.println("Entrou no no_connection");
  // Apenas vai para o estado connect
  return empty;
}

event end_state(void) {
  
}

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

void piscar_led() {
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
}

void ler_dados(float button_state) {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
//  snprintf(umidade, 50, "{\"value\":%s}", dtostrf(hum, 5, 2, NULL));
//  snprintf(temperatura, 50, "{\"value\":%s}", dtostrf(temp, 5, 2, NULL));
//  snprintf(button, 50, "{\"value\":%s}", dtostrf(button_state, 1, 0, NULL));
//  Serial.println(button);
//  Serial.println(umidade);
//  Serial.println(temperatura);
}

void enviar_dados() {
  Serial.println("Enviou!");
  client.publish("/v1.6/devices/wemos-d1-r2-mini/temp", "{\"value\": 50}");
  client.publish("/v1.6/devices/wemos-d1-r2-mini/humidity", "{\"value\": 100}");
  client.publish("/v1.6/devices/wemos-d1-r2-mini/button", "{\"value\": 1}");
  delay(3000);
//  client.publish("/v1.6/devices/wemos-d1-r2-mini/temp", temperatura);
//  client.publish("/v1.6/devices/wemos-d1-r2-mini/humidity", umidade);
//  client.publish("/v1.6/devices/wemos-d1-r2-mini/button", button);
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

// NÃO ESTÁ SENDO CHAMADO (POR ENQUANTO)
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
