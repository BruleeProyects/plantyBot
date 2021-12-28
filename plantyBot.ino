/*
 Name:        plantyBot.ino
 Created:     05/12/2021
 Author:      Javiera Hormazábal <javiera.hormazabal9@gmail.com>
 Description: Planty es un sistema de riego automático en base a humedad de suelo y
              luz. Comunicado a distancia a través de NodeMCU y TelegramBot para 
              informes de necesidad de riego con espera de confirmación por parte del usuario.
*/
#include <ESP8266WiFi.h>
#include "CTBot.h"
CTBot myBot;

String ssid  = "HUAWEI Mate 20 lite"; // Nombre de red WiFi.
String pass  = "javierah"; //Contraseña red WiFi.
String token = "5000748142:AAFIWszYm4yFFajH6oJ4gtV6o4L-wISR8qs"; // Token bot Telegram.
const int ledRojo = 5;
const int ledVerde = 4;
const int higrometro = 16;
const int foto = A0;
const int rele = 10; 

//Variables que almacenarán los valores del higrometro y fotoresistor.
int humedad;
int luz;
//Variables booleanas para evitar constante envío de mensajes.
bool pregunta = false;
bool alertDia = false;
bool alertHum = false;
bool alertInv = false;

//Método encargado de la conexión del ESP8266 a la red WiFi.
void setup_wifi() {
  
  delay(10);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi conectado!");
  }

void setup() {
  
  Serial.begin(115200);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(higrometro, INPUT);
  pinMode(rele, OUTPUT);
  pinMode(foto, INPUT);

  Serial.println("Iniciando conexión WiFi");
  setup_wifi();
  
  // Conecta el ESP8266 y el bot de Telegram a la red WiFi.
  myBot.wifiConnect(ssid, pass);
  myBot.setTelegramToken(token);
  Serial.println("Inicializando TelegramBot...");
  
  // Verifica la conexión
  if (myBot.testConnection()){
    Serial.println("TelegramBot conectado!");}
  else{
    Serial.println("Conexión fallida con bot");}
}


void loop() {
  
  humedad = digitalRead(higrometro); //Lectura del higrometro.
  digitalWrite(rele,HIGH); //Bomba desactivada.
  if(humedad == HIGH){ //Si la humedad es baja.
    
    luz = analogRead(foto); //Lectura de fotoresistor.
  
    if(luz < 100){ //Si la luz es menor a 100 (noche)...
  
      //Solicita el riego al usuario, ciclo if asegura que solo se pregunte 1 vez.
      if(pregunta == false){
        myBot.sendMessage(1615995844, "Hola! necesito que me riegen.  \nIngresa 2 si quieres que active el riego automático."); //Envío de mensaje al bot informando necesidad de riego.

        pregunta = true; //Evita que vuelva a preguntar lo mismo.
        alertDia = false;
        alertHum = false;
        alertInv = false;
        }
      
      TBMessage msg; //Variable que almacena el mensaje entrante.
  
      if(CTBotMessageText == myBot.getNewMessage(msg)){ //Si hay un mensaje entrante.
  
          if(msg.text.equals("2")){ // número 2 indica instrucción de riego desde Telegram.
          
            digitalWrite(ledVerde, LOW); 
            Serial.println("Regando...");
            myBot.sendMessage(1615995844, "Regando..."); //Envío mensaje a telegram.
            digitalWrite(ledRojo, HIGH); //Luz roja indica "Regando".
            digitalWrite(rele, LOW); //Bomba activada.
            delay(60000); //Riega por 1 min.
            
            digitalWrite(rele,HIGH); //Bomba desactivada.
            digitalWrite(ledRojo,LOW); //ya no riega.
            digitalWrite(ledVerde,HIGH); //En espera de siguiente lectura.
            myBot.sendMessage(1615995844, "Riego finalizado exitosamente!");//Envío mensaje a Telegram.
            Serial.println("Riego finalizado existosamente.");

            //Reinicia todas las variables de verificación.
            alertInv = false;
            pregunta = false;
            alertDia = false;
            alertHum = false;            
          }
          else if(!msg.text.equals("2")){ // si el texto ingresado es diferente a 2...
            
            //ciclo if asegura que solo se pregunte 1 vez.
            if(alertInv == false ){ 
              
              myBot.sendMessage(1615995844, "Instrucción no válida."); //Envío mensaje a telegram.
              alertInv = true; //Evita que vuelva a preguntar lo mismo.
              pregunta = false;
              alertDia = false;
              alertHum = false;
              }
            }
          }
    }else if(luz > 100) //Si aún es de día.
      {
        if(alertDia == false ){
            
            myBot.sendMessage(1615995844, "Hola! necesito que me rieges, pero aún es de día. \nEsperaré a que sea de noche."); //Envío mensaje a telegram.
            alertDia = true; //Evita que vuelva a preguntar lo mismo.
            pregunta = false;
            alertHum = false;
            alertInv = false;
          }
        
        Serial.println("Aún es de día");
        digitalWrite(ledVerde, HIGH); //Luz verde indica planta con humedad suficiente.
        digitalWrite(ledRojo, LOW);
        digitalWrite(rele,HIGH); //Bomba desactivada.
        }
    }
    else if(humedad == LOW){ //Si la humedad es adecuada.
  
      if(alertHum == false ){
          myBot.sendMessage(1615995844, "Planta en óptimas condiciones."); //Envío mensaje a telegram.
          
          alertHum = true; //Evita que vuelva a preguntar lo mismo.
          pregunta = false;
          alertDia = false;
          alertInv = false;

          }

           digitalWrite(ledVerde, HIGH); //Luz verde indica planta con humedad suficiente.
           digitalWrite(ledRojo,LOW);
           digitalWrite(rele,HIGH); //Bomba desactivada.
           Serial.println("Planta en estado óptimo");
      }
}
