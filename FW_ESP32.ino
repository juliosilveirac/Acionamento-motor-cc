#include "esp32-hal-timer.h"
#include <ModbusSerial.h>

#define _1MHz 1000000

#define IN_Ana_Tensao_Arm 36
#define IN_Ana_Corrente_Arm 39
#define IN_Ana_Tensao_Campo 34
#define IN_Ana_Corrente_Campo 35
#define IN_Ana_Tacogerador 32

#define IN_Sinc_R 33
#define IN_Sinc_S 25
#define IN_Sinc_T 26

#define OUT_Armadura_R 19
#define OUT_Armadura_S 18
#define OUT_Armadura_T 5

#define OUT_Campo_R 17
#define OUT_Campo_S 16
#define OUT_Campo_T 4

#define LED_PIN 2



// Used Pins
const int TxenPin = -1; // -1 disables the feature, change that if you are using an RS485 driver, this pin would be connected to the DE and /RE pins of the driver.

const byte SlaveId = 1;

#define MySerial Serial0 // define serial port used, Serial most of the time, or Serial1, Serial2 ... if available
const unsigned long Baudrate = 9600;

// ModbusSerial object
ModbusSerial mb (MySerial, SlaveId, TxenPin);

TaskHandle_t Core0Task;
TaskHandle_t Core1Task;

//---Variáveis Globais Interrupção ---//
hw_timer_t *timerR = NULL;  //Variável Timer interrupcao Fase R
hw_timer_t *timerS = NULL;  //Variável Timer interrupcao Fase S
hw_timer_t *timerT = NULL;  //Variável Timer interrupcao Fase T

bool timerFlag = false;
float tensaoArmadura,correnteArmadura,tensaoCampo,correnteCampo,tacogeradorRPM;
int anguloDisparoArmadura, anguloDisparoCampo;
float tensaoMaximaArmadura, correnteMaximaArmadura, tensaoMaximaCampo, tensaoMinimaCampo, correnteMaximaCampo, velocidadeMaxima;
int tipoControleArmadura, tipoControleCampo;
float setpointTensaoArmadura, setpointTensaoCampo;
int Liga, Rampa, Malha_fechada_Ligada, Comunicacao, Setpoint_Velocidade, Ganho_P, Ganho_I, Ganho_D;


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void ARDUINO_ISR_ATTR sincR(){
  //Serial.println("Passagem por zero R");
  timerWrite(timerR, 0);
}

void ARDUINO_ISR_ATTR sincS(){
  //Serial.println("Passagem por zero S");
  timerWrite(timerS, 0);
}

void ARDUINO_ISR_ATTR sincT(){
  //Serial.println("Passagem por zero T");
  timerWrite(timerT, 0);
}

void ARDUINO_ISR_ATTR interrupcaoR(){
  digitalWrite(OUT_Armadura_R,false);//Desliga o MOC
  digitalWrite(OUT_Campo_R,false);//Desliga o MOC

}

void ARDUINO_ISR_ATTR interrupcaoS(){
  digitalWrite(OUT_Armadura_S,false);//Desliga o MOC
  digitalWrite(OUT_Campo_S,false);//Desliga o MOC

}

void ARDUINO_ISR_ATTR interrupcaoT(){
  digitalWrite(OUT_Armadura_T,false);//Desliga o MOC
  digitalWrite(OUT_Campo_T,false);//Desliga o MOC

}

void shoot(int angle, int pin, uint64_t timer)
{
  int time_shoot;
  if(angle < 30)  {
    time_shoot = 30*8330/180; //Calcula o tempo de disparo
  }
  else{
    time_shoot = angle*8330/180; //Calcula o tempo de disparo
  }
  if(timer >= time_shoot && timer <= 6900 && Liga)
  {
    digitalWrite(pin,true);//Liga o MOC
  }
  else
  {
    digitalWrite(pin,false);//Desliga o MOC
  }
}

void leiturasAnalogicas(){
  int cont, amostras;
  float leitura1,leitura2,leitura3,leitura4,leitura5,EntradaAD;
  leitura1 = 0;
  leitura2 = 0;
  leitura3 = 0;
  leitura4 = 0;
  leitura5 = 0;
  cont = 0;
  amostras = 200;
  while (cont < amostras) { //inicia um ciclo de amostragem até que i alcance o número de amostras
    leitura1 = leitura1 + mapfloat(analogRead(IN_Ana_Tensao_Arm),0,4095,0,330);
    cont++; //incrementa o iterador
  }
  if((leitura1/amostras)>8)
  {
    tensaoArmadura   = (leitura1/amostras)*1.1054-6,77;
  }
  else
  {
    tensaoArmadura   = (leitura1/amostras);
  }
  
  cont = 0;
  while (cont < amostras) { //inicia um ciclo de amostragem até que i alcance o número de amostras
    leitura2 = leitura2 + mapfloat(analogRead(IN_Ana_Corrente_Arm),0,4095,0,1200);
    cont++; //incrementa o iterador
  }
  if((leitura2/amostras)>8)
  {
    correnteArmadura = (leitura2/amostras)*1-0;
  }
  else
  {
    correnteArmadura = (leitura2/amostras);
  }
  

  cont = 0;
  while (cont < amostras) { //inicia um ciclo de amostragem até que i alcance o número de amostras
    leitura3 = leitura3 + mapfloat(analogRead(IN_Ana_Tensao_Campo),0,4095,0,330);
    cont++; //incrementa o iterador
  }
  if((leitura3/amostras)>8)
  {
    tensaoCampo      = (leitura3/amostras)*1.2558-5.82;
  }
  else
  {
    tensaoCampo      = (leitura3/amostras);
  }  
  
  cont = 0;
  while (cont < amostras) { //inicia um ciclo de amostragem até que i alcance o número de amostras
    leitura4 = leitura4 + mapfloat(analogRead(IN_Ana_Corrente_Campo),0,4095,0,360);
    cont++; //incrementa o iterador
  }
  if((leitura4/amostras)>7)
  {
    correnteCampo    = (leitura4/amostras)*0.9159-5.78;
  }
  else
  {
    correnteCampo    = (leitura4/amostras);
  }
  

  cont = 0;
  while (cont < amostras) { //inicia um ciclo de amostragem até que i alcance o número de amostras
    leitura5 = leitura5 + mapfloat(analogRead(IN_Ana_Tacogerador),0,4095,0,8250);
    cont++; //incrementa o iterador
  }
  if((leitura5/amostras)>100)
  {
    tacogeradorRPM   = (leitura5/amostras)*1.0777+125;
  }
  else
  {
    tacogeradorRPM   = (leitura5/amostras);
  }
  
}

void setup() {
  
  //Configuração Modbus
  MySerial.begin (Baudrate);
  mb.config (Baudrate);
  mb.setAdditionalServerData ("TEMP_SENSOR");
  //Adiciona 100 endereços modbus
  for (int i = 0; i < 100 ; i++){
    mb.addHreg (i);
  }

  //Atribuição das Entradas Analógicas
  pinMode(IN_Ana_Tensao_Arm, INPUT);
  pinMode(IN_Ana_Corrente_Arm, INPUT);
  pinMode(IN_Ana_Tensao_Campo, INPUT);
  pinMode(IN_Ana_Corrente_Campo, INPUT);
  pinMode(IN_Ana_Tacogerador, INPUT);

  //Atribuição das Entradas Digitais
  pinMode(IN_Sinc_R, INPUT);
  pinMode(IN_Sinc_S, INPUT);
  pinMode(IN_Sinc_T, INPUT);


  //Configuração timer para interrupção interna
  timerR = timerBegin(_1MHz);// tempo maximo em us                  
  timerAttachInterrupt(timerR, &interrupcaoR);  
  timerAlarm(timerR, 6900, false, 0);// tempo maximo em us 
  
  timerS = timerBegin(_1MHz);// tempo maximo em us
  timerAttachInterrupt(timerS, &interrupcaoS);  
  timerAlarm(timerS, 6900, false, 0);// tempo maximo em us 

  timerT = timerBegin(_1MHz);// tempo maximo em us                  
  timerAttachInterrupt(timerT, &interrupcaoT);  
  timerAlarm(timerT, 6900, false, 0);// tempo maximo em us 

  //Atribuição das Interrupções das Entradas Digitais
  attachInterrupt(digitalPinToInterrupt(IN_Sinc_R), sincR, FALLING);//borda de subida
  attachInterrupt(digitalPinToInterrupt(IN_Sinc_S), sincS, FALLING);//borda de subida
  attachInterrupt(digitalPinToInterrupt(IN_Sinc_T), sincT, FALLING);//borda de subida
  
  //Atribuição das Saídas Digitais
  pinMode(OUT_Armadura_R, OUTPUT);
  pinMode(OUT_Armadura_S, OUTPUT);
  pinMode(OUT_Armadura_T, OUTPUT);
  pinMode(OUT_Campo_R, OUTPUT);
  pinMode(OUT_Campo_S, OUTPUT);
  pinMode(OUT_Campo_T, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  mb.setHreg (57, 90);
  mb.setHreg (60, 90);

  xTaskCreatePinnedToCore(
    codeForCore0Task,
    "Core 0 task",
    10000,
    NULL,
    1,
    &Core0Task,
    0);


  // Set up Core 1 task handler
  xTaskCreatePinnedToCore(
    codeForCore1Task,
    "Core 1 task",
    10000,
    NULL,
    1,
    &Core1Task,
    1);
  disableCore0WDT();

}

void codeForCore0Task(void *parameter){
  for (;;)
  {
    mb.task();

    // atualiza dados modbus
    mb.setHreg (0, tensaoArmadura);
    mb.setHreg (1, correnteArmadura);
    mb.setHreg (2, tensaoCampo);
    mb.setHreg (3, correnteCampo);
    mb.setHreg (4, tacogeradorRPM);
    mb.setHreg (5, anguloDisparoArmadura);
    mb.setHreg (6, anguloDisparoCampo);

    // leitura dados modbus
    tensaoMaximaArmadura   = mb.hreg (50);
    correnteMaximaArmadura = mb.hreg (51);
    tensaoMaximaCampo      = mb.hreg (52);
    tensaoMinimaCampo      = mb.hreg (53);
    correnteMaximaCampo    = mb.hreg (54);
    velocidadeMaxima       = mb.hreg (55);
    tipoControleArmadura   = mb.hreg (56);
    anguloDisparoArmadura  = mb.hreg (57);
    setpointTensaoArmadura = mb.hreg (58);
    tipoControleCampo      = mb.hreg (59);
    anguloDisparoCampo     = mb.hreg (60);
    setpointTensaoCampo    = mb.hreg (61);
    Liga                   = mb.hreg (62);
    Rampa                  = mb.hreg (63);
    Comunicacao            = mb.hreg (64);
    Setpoint_Velocidade    = mb.hreg (65);
    Ganho_P                = mb.hreg (66);
    Ganho_I                = mb.hreg (67);
    Ganho_D                = mb.hreg (68);
    leiturasAnalogicas();
  }
}


void codeForCore1Task(void *parameter){
  for (;;)
  {
    uint64_t tReadR,tReadS,tReadT;
    tReadR = timerRead(timerR);
    tReadS = timerRead(timerS),
    tReadT = timerRead(timerT);
    
    shoot(anguloDisparoArmadura,OUT_Armadura_R,tReadR);
    shoot(anguloDisparoArmadura,OUT_Armadura_S,tReadS);
    shoot(anguloDisparoArmadura,OUT_Armadura_T,tReadT);
    shoot(anguloDisparoCampo,OUT_Campo_R,tReadR);
    shoot(anguloDisparoCampo,OUT_Campo_S,tReadS);
    shoot(anguloDisparoCampo,OUT_Campo_T,tReadT);
    
  }
}


void loop() {
  
  delay(100);

}