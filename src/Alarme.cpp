//incluir bibliotecas

#include <Arduino.h>
#include <HCSR04.h>



//definir pinos e valores iniciais

#define TRIGGER_PIN 2 //pino trigger sensor
#define ECHO_PIN 3 //pino eco sensor
#define MAX_DISTANCE 200 //dist. maxima sensor (cm)

const int buz=9; //pin do buzzer
const int but=7; //pin do botão
const int ps=6;  //pin da senha
const int pinled=10; //pin do LED
const int db=50; //debounce
const int da=10; //distância de ativação (cm)
const int tminled=3000; //tempo minimo do LED ligado (ms)

int f; //frequência do buzzer
int fa=2000; //frequência alarme
int on=0; //variável ligado/desligado
int leds=LOW; //estado do LED
int old_on=HIGH; //variável estado antigo do botão
int dist; //variável distancia
int des; //estado desbloqueado
int old_d; //variável distância antiga
unsigned long tempS=0; //tempo decorrido apos a senha
unsigned long tempL=0; //tempo do LED
unsigned long lastdbb=0; //tempo após o ultimo debounce do botão
unsigned long lastdbs=0; //tempo apos o ultimo debounce da senha
bool alarmeativo = false; //estado do alarme

UltraSonicDistanceSensor sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //define o sensor



void setup(){  //define pinModes e começa o serial
  pinMode(buz, OUTPUT);
  pinMode(ps, INPUT_PULLUP);
  pinMode(pinled, OUTPUT);
  pinMode(but, INPUT_PULLUP);
  Serial.begin(9600);
}



void toggle(){  //transforma o botão em uma switch
  if(digitalRead(but)==LOW && old_on==HIGH && (millis()-lastdbb)>db){
    if (on==1){
      on=0;
      leds=LOW;
      des=0;
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
        tempL=millis();
      }
      else if(tempL<=millis()-tminled && leds==HIGH){
        leds=LOW;
      }
    }
    else{
      alarmeativo = true;
    }
  }
}




void senha(){
  if (digitalRead(ps)==LOW){
    if (tempS==0){
      tempS=millis();
    }
    if (des==0 && millis()-lastdbs>db) {
      f=1000;
      for(int i=0;i<3;i++){ //bipe de confirmação
        tone(buz, f);
        delay(80);
        noTone(buz);
        f+=500;
      }
    des=1;
    alarmeativo = false;
    }
    if (des==1 && millis()-tempS>=2000) {//se o botao senha for segurado por 2 segundos, bloqueia
      des=0;
      tone(buz, 500); //bipe set alarme
      delay(100);
      noTone(buz);
      delay(50);
      tone(buz, 500);
      delay(100);
      noTone(buz);
      delay(500); //delay para dar tempo do usuario tirar o dedo do botao
      tempS=0;
      return;
    }
    lastdbs=millis();
  }
  else{
    tempS=0;
  }
}



void loop() {
  dist=sonar.measureDistanceCm(); //define a variável d como sitancia que o sensor lê
  toggle();
  if (on==1){
    senha(); //quando senha correta desbloqueia
    led(); //liga o LED com logica de toggle quando o sensor lê uma distancia menor que a variavel "da" em cm
  }
  digitalWrite(pinled, leds); //liga o LED dependendo da variável do estado dele

  if (alarmeativo && des != 1) { //varre frequencias do buzzer
    tone(buz, fa);
    fa += 100;
    if (fa > 3000){
      fa = 2000;
    }
    leds=LOW; //desliga o LED
  }
  //printa no terminal a distancia, estado do sistema, estado do LED, estado do alarme e estado da senha
  Serial.print("Dist: ");
  Serial.print(dist);
  Serial.print(" cm\tOn: ");
  Serial.print(on);
  Serial.print("\tLED: ");
  Serial.print(leds);
  Serial.print("\tAlarme: ");
  Serial.print(alarmeativo);
  Serial.print("\tSenha: ");
  Serial.println(des);

  old_d=dist;
}