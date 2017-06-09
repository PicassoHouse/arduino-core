
#include <LiquidCrystal.h>
#include <Keypad.h>

#include <Ethernet.h>
#include "RestClient.h"
#include <PubSubClient.h>
#include <aREST.h>

#include <Servo.h>

//Portas correspondentes aos sensores de movimento
#define S_MOV_1 38 //IN2
#define S_MOV_2 39 //IN4
#define S_MOV_3 40 //IN1
#define S_MOV_4 41 //IN3

//Portas correspondentes ao controlador dos reles
#define IN_1 47 //room_id 1 - sala
#define IN_2 46 //room_id 2 - garage
#define IN_3 49 //room_id 3 - cozinha
#define IN_4 48 //room_id 4 - quarto

//Portas correspondentes aos sensores de movimento
#define S_RAIN_ANALOG A15
boolean hasDetectedRain = false;

//Portas correspondentes ao buzzer
#define BUZZER_P 45

//Portas correspondentes aos servos motores
#define SERVO_GARAGE 34
#define SERVO_WINDOW 35
Servo garage;
Servo window;

//Mapeamento das teclas
char keyMap[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
// quais pinos conectamos as linhas e colunas do teclado.
byte rowPins[4] = { 24, 25, 26, 27 };
byte colPins[4] = { 28, 29, 30, 31 };

Keypad keypad = Keypad(makeKeymap(keyMap), rowPins, colPins, 4, 4);
String keyboardInput = "";

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//Server
RestClient apiService = RestClient("192.168.1.10", 3000);
EthernetServer server(80);
String response;
//Clients
EthernetClient ethClient;
PubSubClient client(ethClient);
aREST rest = aREST(client); // Create aREST instance

char* device_id = "123456";

int windowIsopen = false;
int garageIsOpen = false;

// ------------------------------------------------
// Setup
// ------------------------------------------------
void setup() {

  Serial.begin(115200);
  
  //Inicializa a interface de rede
  setupClient();
  setupServer();

  //out
  setupLcd();
  setupBuzzer();
  setupServos();
  //in
  setupMoveSensors();
  setupRainSensor();

  setupRelay();
}


// ------------------------------------------------
// LOOP
// ------------------------------------------------
void loop() {
  //aREST
  EthernetClient client = server.available();
  rest.handle(client);
  
  //Keyboard
  lerTeclado();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Picasso House");
  lcd.setCursor(0,1);
  lcd.print("Senha: ");
  lcd.print(keyboardInput);
  
  //detectar se pressionou a tecla para enviar
  if(keyboardInput.indexOf('#') > 0) {
    String url = "/api/house/hasReceivedAuthCode?device_id=123456&auth_code=" + keyboardInput;
    apiService.get(url.c_str(), &response);
    keyboardInput = "";
  }
  
  //Move sensors
  readMoveSonsors();

  // rain sensors
  readRainSensor();
  
  delay(100);
  
}


// ------------------------------------------------
// Setups
// ------------------------------------------------
void setupClient() {
  apiService.dhcp();
  server.begin();

  Serial.println(Ethernet.localIP());
}

void setupServer() {
  // Set callback
  client.setCallback(callback);

  // Give name & ID to the device
  rest.set_id(device_id);
  rest.set_name("ethernet");

  //API Interface
  rest.function("turn_ligth_on", turn_ligth_on);
  rest.function("set_buzzer_on", set_buzzer_on);
  rest.function("open_garage", open_garage);
  rest.function("open_window", open_window);
}

void setupLcd() {
  lcd.begin(16, 2);
}

void setupBuzzer() {
  pinMode(BUZZER_P, OUTPUT);
}

void setupMoveSensors() {
  pinMode(S_MOV_1, INPUT);
}

void setupRainSensor() {
  pinMode(S_RAIN_ANALOG, INPUT); //analog sensor
}

void setupRelay() {
  pinMode(IN_1, OUTPUT); 
  pinMode(IN_2, OUTPUT);
  pinMode(IN_3, OUTPUT); 
  pinMode(IN_4, OUTPUT);
  
  digitalWrite(IN_1, HIGH);
  digitalWrite(IN_2, HIGH);
  digitalWrite(IN_3, HIGH);
  digitalWrite(IN_4, HIGH);
}

void setupServos() {
  garage.attach(SERVO_GARAGE);
  garage.write(180);
  
  window.attach(SERVO_WINDOW);
  window.write(180);
}

// ------------------------------------------------
// Components Listeners
// ------------------------------------------------
void lerTeclado() {
  char key = keypad.getKey();

  // Se foi pressionada uma tecla, mostramos qual foi.
  if (key != NO_KEY) {
    keyboardInput += key;
  }
}

void readMoveSonsors() {
  int act,actGaragem,actQuarto,actCozinha,actSala;
  
//   act = digitalRead(S_MOV_1); //garagem   
//   if(act != actGaragem){
//     actGaragem = act;
//     if(actGaragem == HIGH) { 
//      //apiService.get("/api/house/hasDetectedPresence?device_id=123456&room_id=2&on=true", &response);
//     } else {
//      //apiService.get("/api/house/hasDetectedPresence?device_id=123456&room_id=2&on=false", &response);
//     }
//   }

//   act = digitalRead(S_MOV_2); //quarto
//   Serial.println(act);
//   if(act != actQuarto){
//     Serial.println("Entrei");
//     if(act == HIGH) { 
//      Serial.println("Ligou");
//      actQuarto = HIGH;
//      apiService.get("/api/house/hasDetectedPresence?device_id=123456&room_id=4&on=true", &response);
//     } else {
//      Serial.println("Desligou");
//      actQuarto = LOW;
//      apiService.get("/api/house/hasDetectedPresence?device_id=123456&room_id=4&on=false", &response);
//     }
//   }

//   act = digitalRead(S_MOV_3); //sala
//   if(act != actCozinha){
//     actCozinha = act;
//     if(actCozinha == HIGH) { 
//      //apiService.get("/api/house/hasDetectedPresence?device_id=123456&room_id=1&on=true", &response);
//     } else {
//     }
//   }
//
//   act = digitalRead(S_MOV_4); //cozinha
//   if(act != actSala){
//     actSala = act;
//     if(actSala == HIGH) { 
//      //apiService.get("/house/hasDetectedPresence?device_id=123456&room_id=3&on=true", &response);
//     } else {
//     }
//   }
}

void readRainSensor() {
  int analogico = analogRead(S_RAIN_ANALOG);

  if(analogico > 900) {
    hasDetectedRain = false;
  }

  if (!hasDetectedRain && analogico > 0 && analogico < 400) { // intensidade alta
    hasDetectedRain = true;
    String url = "/api/house/hasDetectedRain?device_id=123456";
    apiService.get(url.c_str(), &response);
  }
  
}

// ------------------------------------------------
// API Interface
// ------------------------------------------------
// Handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
  rest.handle_callback(client, topic, payload, length);
}

// GET /turn_ligth_on?param=_{room_id}&{is_on}
void turn_ligth_on(String message) {
  int room_id, is_on;
  int IN_X;

  int i = message.indexOf('&');
  room_id = message.substring(0,i).toInt();
  is_on = message.substring(i+1).toInt();

  switch(room_id) {
    case 1: IN_X = IN_1; break;
    case 2: IN_X = IN_2; break;
    case 3: IN_X = IN_3; break;
    case 4: IN_X = IN_4; break;
  }
  //ligar ou desligar a luz
  digitalWrite(IN_X, is_on ? LOW : HIGH);
}

// GET /set_buzzer_on?param=_{is_on}
void set_buzzer_on(String message) {
  int is_on = message.toInt();
  digitalWrite(BUZZER_P, is_on ? HIGH : LOW);
}
// GET /open_garage?param=_{is_on}
void open_garage(String message) {
  garageIsOpen = message.toInt();
  if(!garageIsOpen){
    for(int i=90; i<180; i++){
      garage.write(i);
      delay(25);
    }
  } else {
    for(int i=180; i>=100; i--){
      garage.write(i);
      delay(25);
    }
  }
}

// GET /open_window?param=_{room_id}&{is_open}
void open_window(String message) {
  
  int room_id, is_open;
  
  int i = message.indexOf('&');
  room_id = message.substring(0,i).toInt();
  windowIsopen = message.substring(i+1).toInt();

  if(!windowIsopen){
    for(int i=180; i>0; i--){
      window.write(i);
      delay(25);
    }
  } else {
    for(int i=0; i<180; i++){
      window.write(i);
      delay(25);
    }
  }
}


