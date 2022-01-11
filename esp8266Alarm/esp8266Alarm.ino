#include <Ultrasonic.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/************************ Configurações do Adafruit IO *******************************/
//Insira seu nome de usuário e chave. Você encontra essas informações acessando
//sua conta no Adafruit IO
#define IO_USERNAME ""
#define IO_KEY ""

//Insira o SSID e Senha da rede WIFI a qual você irá conectar
#define WIFI_SSID ""
#define WIFI_PASS ""
 
const char* ssid = "";
const char* password = "";

#include "AdafruitIO_WiFi.h"
 
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

#define ALARME false   
/************************ Configuração dos tópicos *******************************/
 
//configura o tópico "fs_embarcados/feeds/alarme"
AdafruitIO_Feed *feedRele1 = io.feed("alarme");

#define LED 16 // D0 Proprio led da placa
#define SDA_PIN 4      // D2
#define RST_PIN 2      // D4
#define MOSI 13        // D7
#define MISO 12        // D6
#define SCK 14         // D5
#define pino_trigger 0 // D3
#define pino_echo 5    // D1
#define porta_rele1 15 // D8
Ultrasonic ultrasonic(pino_trigger, pino_echo);
MFRC522 mfrc522(SDA_PIN, RST_PIN); // Create MFRC522 instance.

long distancia;
bool alarme_status = false;
bool disparo = false;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(porta_rele1, OUTPUT);
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  alarme_status = false;
  mfrc522.PCD_DumpVersionToSerial();
  conectaBroker(); //função para conectar ao broker
  delay(1000);
  Serial.println("Aproxime o seu cartao do leitor...");
  
}

void loop()
{

  // processa as mensagens e mantêm a conexão ativa
  byte state = io.run();
 
  //verifica se está conectado
  if(state == AIO_NET_DISCONNECTED | state == AIO_DISCONNECTED){
    conectaBroker(); //função para conectar ao broker
  }

  if (mfrc522.PICC_IsNewCardPresent())
  {
    Serial.println("Cartão presente!");
    if (alarme_status == false)
    {
      alarme_status = true;
      Serial.println("Alarme ligado!");
      delay(2000);
    }
    else if (alarme_status == true)
    {
      alarme_status = false;
      Serial.println("Alarme desligado!");
      delay(1000);
    }
  }

  if (alarme_status == false)
  {
    rotinaAlarmeDesligado();
  }
  else
  {
    digitalWrite(LED, HIGH);
    disparo = rotinaAlarmeLigado();
  }
}

bool rotinaAlarmeLigado()
{
  digitalWrite(LED, HIGH);
  Serial.println("Alarme ligado.");
  delay(200);
  long medidas[10];
  for (int i = 0; i < 10; i++)
  {
    distancia = ultrasonic.Ranging(CM);
    Serial.print("Distância: ");
    Serial.println(distancia);
    delay(100);
    medidas[i] = distancia;
  }
  if (medidas[0] > 14 && medidas[1] > 14 && medidas[2] > 14 && medidas[3] > 14 && medidas[4] > 14 && medidas[5] > 14 && medidas[6] > 14 && medidas[7] > 14 && medidas[8] > 14 && medidas[9] > 14)
  {
    disparo = true;
    tocarAlarme();
    Serial.println("Disparo do alarme!");
  }
  return disparo;
}

void rotinaAlarmeDesligado()
{
  desligarAlarme();
  delay(1000);
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
}

void tocarAlarme()
{
  delay(50);
  digitalWrite(porta_rele1, HIGH); //Liga rele 1
}

void desligarAlarme()
{
  digitalWrite(porta_rele1, LOW); //Desliga rele 1
}

/****** Função de tratamento dos dados recebidos em L1***************/
 
void handleRele1(AdafruitIO_Data *data) {
 
  // Mensagem
  Serial.print("Recebido  <- ");
  Serial.print(data->feedName());
  Serial.print(" : ");
  Serial.println(data->value());
 
  //Aciona saída conforme dado recebido
  if(data->isTrue())
    alarme_status = true;
  else
    alarme_status = false;
}
 
 
/****** Função para conectar ao WIFI e Broker***************/
 
void conectaBroker(){
 
  //mensagem inicial
  Serial.print("Conectando ao Adafruit IO");
 
  // chama função de conexão io.adafruit.com
  io.connect();
 
   // instancia um novo handler para recepção da mensagem do feed Rele
  feedRele1->onMessage(handleRele1);
 
  // Aguarda conexação ser estabelecida
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  // Conectado
  Serial.println();
  Serial.println(io.statusText());
 
  // certifique-se de que todos os feeds obtenham seus valores atuais imediatamente
  feedRele1->get();
}
