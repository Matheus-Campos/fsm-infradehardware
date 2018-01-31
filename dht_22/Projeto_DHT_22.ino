/* How to use the DHT-22 sensor with Arduino uno
   Temperature and humidity sensor
   More info: http://www.ardumotive.com/how-to-use-dht-22-sensor-en.html
   Dev: Michalis Vasilakis // Date: 1/7/2015 // www.ardumotive.com */

//Libraries
#include <DHT.h>

//Constants
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino


//Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value
int ledVermelho = 41;
int ledVerde = 43;
int ledAzul = 45;

void setup()
{
    Serial.begin(9600);
    pinMode(ledVermelho, OUTPUT);
    pinMode(ledVerde, OUTPUT);
    pinMode(ledAzul, OUTPUT);
    dht.begin();
}

void loop()
{
    //Read data and store it to variables hum and temp
    hum = dht.readHumidity();
    temp= dht.readTemperature();
    if (temp < 22) {
      digitalWrite(ledAzul, HIGH);
      digitalWrite(ledVermelho, LOW);
      digitalWrite(ledVerde, LOW);
    } else if (temp > 23) {
      digitalWrite(ledAzul, LOW);
      digitalWrite(ledVermelho, HIGH);
      digitalWrite(ledVerde, LOW);
    } else {
      digitalWrite(ledAzul, LOW);
      digitalWrite(ledVermelho, LOW);
      digitalWrite(ledVerde, HIGH);
    }
    //Print temp and humidity values to serial monitor
    Serial.print("Humidity: ");
    Serial.print(hum);
    Serial.print(" %, Temp: ");
    Serial.print(temp);
    Serial.println(" Celsius");
    delay(2000); //Delay 2 sec.
}

   
