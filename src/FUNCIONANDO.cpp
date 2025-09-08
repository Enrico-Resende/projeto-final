//incluir bibliotecas

#include <Arduino.h>
#include <NewPing.h>



//definir pinos e valores iniciais

#define TRIGGER_PIN 2 //pino trigger sensor
#define ECHO_PIN 3 //pino eco sensor
#define MAX_DISTANCE 200 //dist. maxima sensor (cm)

const int da=10; //distância de aticação
const int but=7; //pin do botão
const int senha=6;  //pin da senha
const int pinled=10; //pin do LED

int on=0; //variável ligado/desligado
int leds=LOW; //estado do LED
int old_d; //variável distância antiga
int old_on=HIGH;
int d; //variável distancia



NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //define o sensor como sonar



void setup() {  //define pinModes e começa o serial
  pinMode(senha, INPUT_PULLUP);
  pinMode(pinled, OUTPUT);
  pinMode(but, INPUT_PULLUP);
  Serial.begin(9600);
}



void led(){  //função que liga o LED com logica de toggle quando o sensor lê uma distancia menor que a variavel "da" em cm
  if (on==1 && d<=da && d>0 && old_d>10){
    if (leds==LOW){
      leds=HIGH;
    }
    else{
      leds=LOW;
    }
  }
}



void toggle(){  //transforma o botão em uma switch
  if(digitalRead(but)==LOW && old_on==HIGH){
    if (on==1){
      on=0;
      leds=LOW;
    }
    else{
      on=1;
    }
  }
  old_on=digitalRead(but);
}



void loop() {
  d=sonar.ping_cm(); //define a variável d como sitancia que o sensor lê
  toggle();
  led();
  digitalWrite(pinled, leds); //liga o LED dependendo da variável do estado dele

  //printa no terminal a distancia e se o sistema está ligado ou desligado
  Serial.print("Distance: ");
  Serial.println(d);
  Serial.print("               On: ");
  Serial.println(on);
  Serial.print("                         led: ");
  Serial.println(leds);
  old_d=d;
}