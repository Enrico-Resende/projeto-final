//incluir bibliotecas

#include <Arduino.h>
#include <HCSR04.h>



//definir pinos e valores iniciais

#define TRIGGER_PIN 2 //pino trigger sensor
#define ECHO_PIN 3 //pino eco sensor
#define MAX_DISTANCE 200 //dist. maxima sensor (cm)

const int buz=9; //pin do buzzer
const int da=10; //distância de aticação
const int but=7; //pin do botão
const int ps=6;  //pin da senha
const int pinled=10; //pin do LED
const int db=50; //debounce

int f; //frequência do buzzer
int on=0; //variável ligado/desligado
int leds=LOW; //estado do LED
int old_on=HIGH; //variável estado antigo do botão
int dist; //variável distancia
int des; //estado desbloqueado
int old_d; //variável distância antiga
unsigned long tempS=0; //tempo decorrido apos a senha
unsigned long lastdbb=0; //tempo após o ultimo debounce do botão
unsigned long lastdbs=0; //tempo apos o ultimo debounce da senha


UltraSonicDistanceSensor sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //define o sensor



void setup(){  //define pinModes e começa o serial
  pinMode(buz, OUTPUT);
  pinMode(ps, INPUT_PULLUP);
  pinMode(pinled, OUTPUT);
  pinMode(but, INPUT_PULLUP);
  Serial.begin(9600);
}



void soarAlarme(){
  while (des != 1) {
    for (int f = 1000; f < 5000; f += 100) {
      if (des == 1) {
        noTone(buz);
        return;
      }
      tone(buz, f);
      delay(1);
    }
  }
  noTone(buz);
}


void toggle(){  //transforma o botão em uma switch
  if(digitalRead(but)==LOW && old_on==HIGH && (millis()-lastdbb)>db){
    if (on==1){
      on=0;
      leds=LOW;
    }
    else{
      on=1;
    }
    lastdbb=millis();
  }
  old_on=digitalRead(but);
}



void led(){
  if (dist<=da && dist>0 && old_d>10){
    if (des==1){
        if (leds==LOW){
        leds=HIGH;
      }
      else{
        leds=LOW;
      }
    }
    else{
      soarAlarme();
    }
  }
}




void senha(){
  if (digitalRead(ps)==LOW && millis()-lastdbs>db){
    des=1;
    tempS=millis();
    lastdbs=millis();
  }
}



void resetSenha(){
  if (millis()-tempS>15000 && digitalRead(ps)==HIGH){
    des=0;
  }
}



void loop() {
  dist=sonar.measureDistanceCm(); //define a variável d como sitancia que o sensor lê
  toggle();
  if (on==1){
    senha(); //quando senha correta desbloqueia
    led(); //liga o LED com logica de toggle quando o sensor lê uma distancia menor que a variavel "da" em cm
    resetSenha(); //reseta o estado de desbloqueado para 0 após 15seg
  }
  digitalWrite(pinled, leds); //liga o LED dependendo da variável do estado dele


  //printa no terminal a distancia e se o sistema está ligado ou desligado
  Serial.print("Distance: ");
  Serial.println(dist);
  Serial.print("               On: ");
  Serial.println(on);
  Serial.print("                         led: ");
  Serial.println(leds);
  old_d=dist;
}