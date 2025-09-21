//incluir bibliotecas

#include <Arduino.h>
#include <HCSR04.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>



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
const int senhaC=13; //numero senha correta

int lastdes=0; //variável estado bloqueado antiga
int errcount=0; //contador de erros da senha
int inputS; //variável numero da senha
int f; //frequência do buzzer
int fa=3000; //frequência alarme
int on=0; //variável ligado/desligado
int leds=LOW; //estado do LED
int old_on=HIGH; //variável estado antigo do botão
int dist; //variável distancia
int des=0; //estado desbloqueado
int old_d; //variável distância antiga
int olderrcount=0; //variável contador de erros antigo
int c; //variável de controle do erro ao setar alarme
int c1; //variável de controle do erro ao setar alarme mas com 1 na frente
int lcdOverride=0; //variável de controle do lcd
int lastalarme=0; //variável estado alarme antigo
int laston=0; //variável estado on antigo
int lastInputS=0; //variável estado senha antigo
unsigned long tempL=0; //tempo do LED
unsigned long lastdbb=0; //tempo após o ultimo debounce do botão
unsigned long lastdbs=0; //tempo apos o ultimo debounce da senha
bool alarmeativo=false; //estado do alarme

//variaveis de timing, usadas no beep handler para evitar delay()
unsigned long desTimer=0; //tempo desbloqueado
unsigned long errTimer=0; //tempo de erro
unsigned long blockTimer=0; //tempo do bloqueio
unsigned long onTimer=0; //tempo do botao on
unsigned long senhaTimer=0; //tempo decorrido apos a senha
unsigned long lcdTimer=0; //timer do lcd



UltraSonicDistanceSensor sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); //define o sensor
LiquidCrystal_I2C lcd(0x27, 20, 4);  //define o LCD


void setup(){  //define pinModes e começa o serial
  pinMode(buz, OUTPUT);
  pinMode(ps, INPUT_PULLUP);
  pinMode(pinled, OUTPUT);
  pinMode(but, INPUT_PULLUP);

  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();

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
  onTimer=millis();
}



void led(){
  if (dist<=da && dist>0 && (old_d>10 or old_d==-1)){ //usa o sensor como "switch"
    if (des==1){
        if (tempL<=millis()-tminled && leds==LOW){ //garante temp. minimo led
        leds=HIGH;
        tempL=millis();
      }
      else if(tempL<=millis()-tminled && leds==HIGH){
        leds=LOW;
        tempL=millis();
      }
    }
    else{
      alarmeativo = true;
    }
  }
  old_d=dist;
}



void senha(){
  if (digitalRead(ps)==LOW && blockTimer<=millis()-1000 && desTimer<=millis()-1000){ //se o botao senha for pressionado e ja tiver passado 700ms do bloqueio (tempo do usuario tirar o dedo)
    if (senhaTimer==0){senhaTimer=millis();} 
    if (inputS==senhaC && des==0 && millis()-lastdbs>db && millis()-senhaTimer<=500){
      des=1;
      desTimer=millis();
      alarmeativo = false;
      errcount=0;
    } 
    if(inputS!=senhaC && des==0 && millis()-lastdbs>db && alarmeativo==false && c==0 && millis()-senhaTimer<=10){ //se a senha estiver errada
      errTimer=millis();
      errcount+=1; //incrementa o contador de erros
      if (errcount == 3) { //se errar 3 vezes, ativa o alarme
        alarmeativo = true;
        errcount = 0; //reseta o contador de erros
      }
      c=1;
    }
    if (des==1 && millis()-senhaTimer>=2000 && millis()-senhaTimer<=2100) {//se o botao senha for segurado por 2 segundos
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
    lastdes=des;
    lastalarme=alarmeativo;
  }
}



void beepHandler(){
  if (lastdes==0 && des==1 && on==1){ //se foi desbloqueado
    lcdOverride=1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sist Desboqueado");
    lcdTimer=millis();
    f=1000;
    noTone(buz);
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



  if(lastdes==1 && des==0 && on==1){ //se foi bloqueado
    lcdOverride=1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcdTimer=millis();
    lcd.print("Sist Bloqueado");
    f=2000;
    noTone(buz);
    for(int i=0;i<20;i++){ //bipe de bloqueio
      tone(buz, f);
      while(blockTimer>=millis()-10){}
      f+=50;
      blockTimer=millis();
    }
    noTone(buz);
  }



  if ((olderrcount!=errcount && errcount!=0) or (c1==1)){  //se houve erro na senha ou erro ao settar alarme
    lcdOverride=1;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcdTimer=millis();
    lcd.print("Senha incorreta.");
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
    if (fa > 4000){
      fa = 3000;
    }
    leds=LOW; //desliga o LED
    lcdOverride=1;
    lcd.setCursor(0, 0);
    lcd.print("!ALARME ATIVADO!");
    if (inputS<10 && lastInputS==10){
      lcd.setCursor(8, 1);
      lcd.print(" ");
    }
    lcd.setCursor(0, 1);
    lcd.print("Senha: ");
    lcd.print(inputS);
    lastInputS=inputS;
  }else if (lastalarme==true){ //se o alarme foi desativado
    noTone(buz);
    fa=3000;
  }


  if (laston!=on && alarmeativo==false){ //se foi ligado ou desligado
    if (on==1){ //bipe de olá e "ola" no lcd
      lcdOverride=1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcdTimer=millis();
      lcd.print("Ola!");
      tone(buz, 1000);
      delay(200);
      tone(buz, 1500);
      delay(400);
      noTone(buz);
      delay(200);
      lcdOverride=0;
    }
    if (on==0){ //bipe de tchau e "tchau" no lcd
      lcd.clear();
      lcdOverride=1;
      lcd.setCursor(0, 0);
      lcdTimer=millis();
      lcd.print("Tchau!");
      tone(buz, 1500);
      delay(200);
      tone(buz, 1000);
      delay(400);
      noTone(buz);
    }
  }
  laston=on;

  if (lcdTimer<=millis()-2000 && lcdTimer>=millis()-2100){ //reseta o lcd após 2 segundos.
    lcd.clear();
    lcdOverride=0;
  }
}



void loop() {
  //define a variável d como sitancia que o sensor lê
  dist=sonar.measureDistanceCm();

  //lê o potenciometro e mapeia para 1-20
  inputS=map(analogRead(pot), 0, 1023, 1, 21); 
  toggle();
  if (on==1){
    senha();
    led();
  }
  beepHandler();
  digitalWrite(pinled, leds); //controla LED

  //printa no terminal
  Serial.print("\tOn: ");
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
  Serial.println(errcount);

  //printa no LCD input estado
  if (lcdOverride==0 && on==1){
    if (laston!=on){
      lcd.clear();
    }
    if (inputS<10 && lastInputS==10){
      lcd.setCursor(8, 1);
      lcd.print(" ");
    }
    lcd.setCursor(0, 1);
    lcd.print("Senha: ");
    lcd.print(inputS);
    lcd.setCursor(0, 0); 
    if (des==1){
      lcd.print("Desbloqueado");
    } else {
      lcd.print("Bloqueado");
    }
    lastInputS=inputS;
  }
}