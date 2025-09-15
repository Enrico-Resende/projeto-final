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
const int pot=A0; //pin do potenciometro
const int pinled=10; //pin do LED
const int db=50; //debounce
const int da=10; //distância de ativação (cm)
const int tminled=3000; //tempo minimo do LED ligado (ms)

int lastdes=0; //variável estado bloqueado antiga
int errcount=0; //contador de erros da senha
int senhaC=13; //numero senha correta
int inputS; //variável numero da senha
int f; //frequência do buzzer
int fa=2000; //frequência alarme
int on=0; //variável ligado/desligado
int leds=LOW; //estado do LED
int old_on=HIGH; //variável estado antigo do botão
int dist; //variável distancia
int des=0; //estado desbloqueado
int old_d; //variável distância antiga
int olderrcount=0; //variável contador de erros antigo
int c; //variável de controle do erro ao setar alarme
int c1; //variável de controle do erro ao setar alarme mas com 1 na frente
unsigned long senhaTimer=0; //tempo decorrido apos a senha
unsigned long tempL=0; //tempo do LED
unsigned long lastdbb=0; //tempo após o ultimo debounce do botão
unsigned long lastdbs=0; //tempo apos o ultimo debounce da senha
bool alarmeativo=false; //estado do alarme

//variaveis de timing, usadas no beep handler para evitar delay()
unsigned long desTimer=0; //tempo desbloqueado
unsigned long errTimer=0; //tempo de erro
unsigned long blockTimer=0; //tempo do bloqueio

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
  old_d=dist;
}



void senha(){
  if (digitalRead(ps)==LOW && blockTimer<=millis()-500){ //se o botao senha for pressionado e ja tiver passado 500ms do bloqueio (tempo do usuario tirar o dedo)
    if (senhaTimer==0){senhaTimer=millis();} 
    if (inputS==senhaC && des==0 && millis()-lastdbs>db){
      des=1;
      desTimer=millis();
      alarmeativo = false;
      errcount=0;
    } 
    else if(inputS!=senhaC && des==0 && millis()-lastdbs>db && alarmeativo==false && c==0){ //se a senha estiver errada
      errTimer=millis();
      errcount+=1; //incrementa o contador de erros
      if (errcount == 3) { //se errar 3 vezes, ativa o alarme
        alarmeativo = true;
        errcount = 0; //reseta o contador de erros
      }
      c=1;
    }
    else if (des==1 && millis()-senhaTimer>=2000) {//se o botao senha for segurado por 2 segundos
      if(inputS==senhaC){
        errcount=0;
        des=0;
        blockTimer=millis();
      }
      else if(c==0){
        errTimer=millis();
        c1=1;
        c=1;
      }
    }
    lastdbs=millis();
}else{
  senhaTimer=0;
  c=0;
}
}



void beepHandler(){
  if (lastdes==0 && des==1){  //se foi desbloqueado
    f=1000;
    fa=2000;
    for(int i=0;i<3;i++){ //bipe de confirmação
      while (desTimer>=millis()-50){
        tone(buz, f);
      }
      desTimer=millis();
      while (desTimer>=millis()-10){
        noTone(buz);
      }
      desTimer=millis();
      f+=500;
    }
    noTone(buz);
    }



  if(lastdes==1 && des==0){ //se foi bloqueado
    fa=2000;
    f=2000;
    for(int i=0;i<20;i++){ //bipe de bloqueio
      tone(buz, f);
      while(blockTimer>=millis()-10){} //equivalente a delay() sem travar o loop
      f+=50;
      blockTimer=millis();
    }
    noTone(buz);
    blockTimer=millis();
    }



  if ((olderrcount!=errcount && errcount!=0) or (c1==1)){  //se houve erro na senha ou erro ao settar alarme
    for(int i=0;i<2;i++) {
      while (errTimer>=millis()-100){ //bipe de erro
        tone(buz, 500);
      }
      errTimer=millis();
      while (errTimer>=millis()-50){
        noTone(buz);
      }
      errTimer=millis();
    }
    noTone(buz);
    olderrcount=errcount;
    c1=0;
  }



  if (alarmeativo==true){ //se o alarme estiver ativo
    tone(buz, fa);
    fa += 200;
    if (fa > 3000){
      fa = 2000;
    }
    leds=LOW; //desliga o LED
  }else{
    noTone(buz);
  }
  lastdes=des;
}



void loop() {
  dist=sonar.measureDistanceCm(); //define a variável d como sitancia que o sensor lê
  inputS=map(analogRead(pot), 0, 1023, 0, 20); //lê o potenciometro e mapeia para 0-100
  toggle();
  if (on==1){
    senha(); //quando senha correta desbloqueia
    led(); //liga o LED com logica de toggle quando o sensor lê uma distancia menor que a variavel "da" em cm
  }
  beepHandler(); //trata os bipes
  digitalWrite(pinled, leds); //liga o LED dependendo da variável do estado dele
  //printa no terminal a distancia, estado do sistema, estado do LED, estado do alarme e estado da senha
  Serial.print("Dist: ");
  Serial.print(dist);
  Serial.print("cm\tOn: ");
  Serial.print(on);
  Serial.print("\tLED: ");
  Serial.print(leds);
  Serial.print("\tAlarme: ");
  Serial.print(alarmeativo);
  Serial.print("\tDesbloqueado: ");
  Serial.print(des);
  Serial.print("\tSenha: ");
  Serial.print(inputS);
  Serial.print("\tErros: ");
  Serial.print(errcount);
  Serial.print("\tc: ");
  Serial.println(c);
}