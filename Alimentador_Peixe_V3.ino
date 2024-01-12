#include <Wire.h>
#include <LiquidCrystal_I2C.h>       
#include <Stepper.h> //para o motor de passo
#include <virtuabotixRTC.h>
virtuabotixRTC myRTC(6, 7, 8);
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Define as entradas onde o motor de passo está conectado
#define in1 2
#define in2 3
#define in3 4
#define in4 5

//Quantidade de passos que o motor é capaz de dar em cada acionamento
int passosPorAcionamento=32; 

//Quantidade de passos para cada alimentacao
//Ajuste o numero necessario
int passosRefeicao=4; 

//Define o motor de passo
// ordenar as entradas em in1, in3, in2, in4
Stepper mp(passosPorAcionamento, in1, in3, in2, in4); 

#include <EEPROM.h>

const int btnChange = 10;
const int btnOK = 11;
const int btnMode = 12;

int alarmHour = 00;
int alarmMinutes = 00;
boolean alarmOn = 1;
int M = 01;
int D = 04;
int Y = 2024;
int day = 5;
int h = 23;
int m = 58;

int addrH[] = {0, 1, 2, 3, 4};
int addrM[] = {5, 6, 7, 8, 9};
int addrActive[] = {10, 11, 12, 13, 14};

int mode = 1;
int submode = 1;
int alimentacao = 0;

byte clockChar[] = {B00000, B01110, B10101, B10101, B10111, B10001, B01110, B00000};
byte alarmChar[] = {B00100, B01110, B01110, B01110, B11111, B00000, B00100, B00000};
byte dateChar[] = {B11111, B00000, B01100, B01010, B01010, B01100, B00000, B11111};

void setup()  {

   //Define a velocidade do motor
  mp.setSpeed(500); 

  pinMode(btnChange, INPUT_PULLUP);
  pinMode(btnOK, INPUT_PULLUP);
  pinMode(btnMode, INPUT_PULLUP);


  lcd.begin(16,2);         // initialize the LCD
  lcd.backlight();    // Turn on the blacklight
  lcd.createChar(0, clockChar);
  lcd.createChar(1, alarmChar);
  lcd.createChar(2, dateChar);

  Serial.begin(9600);

}

///   void LOOP  ///

void loop()  {
  
  if (digitalRead(btnMode) == LOW) {
    mode++;
    Serial.println("btnMode press");
    if (mode == 4) {
      mode = 1;
    }
    submode = 0;
    lcd.clear();
    delay(200);
  }

  if (mode == 1) {
    clockDisplay();
    Serial.println(mode);
    //    delay(200);
  }

  if (mode == 2) {
    alarmMode();
    Serial.println(mode);
    //    delay(200);
  }

  if (mode == 3) {
    updateRTC();
    Serial.println(mode);
    //    delay(200);
  }
  if (mode == 0) {
    alimentarPeixe();
    Serial.println(mode);
    EEPROM.write(addrActive, 0);
    //    delay(200);
  }
  for (int x = 0; x < 2; x++) {
    if (EEPROM.read(addrActive[x]) == 1) {
      if ( myRTC.hours == EEPROM.read(addrH[x]) && myRTC.minutes == EEPROM.read(addrM[x])) {
        EEPROM.write(addrActive[x], 0);
        mode = 0;
      }
    }
  }
}

//Funcao auxiliar que comanda o motor de passo e realiza a alimentacao
void alimentarPeixe() {
  //Movimenta o motor de passo
  for(int i=0; i<passosRefeicao; i++){
  mp.step(passosPorAcionamento); 
  }
  //Desliga qualquer bobina do motor que possa ter ficado acionada
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  mode=1;
  delay(1000); 
}

///  HORA NA TELA  ///

void clockDisplay() {
  myRTC.updateTime();
  //  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write(0);

  lcd.setCursor(2, 1);
  if (myRTC.dayofmonth < 10) {
    lcd.print(" ");
    lcd.print(myRTC.dayofmonth);
  }
  else{
    lcd.print(myRTC.dayofmonth);
  } 

   lcd.print("/");

  switch (myRTC.month) {
    case 1: lcd.print("01"); break;
    case 2: lcd.print("02"); break;
    case 3: lcd.print("03"); break;
    case 4: lcd.print("04"); break;
    case 5: lcd.print("05"); break;
    case 6: lcd.print("06"); break;
    case 7: lcd.print("07"); break;
    case 8: lcd.print("08"); break;
    case 9: lcd.print("09"); break;
    case 10: lcd.print("10"); break;
    case 11: lcd.print("11"); break;
    case 12: lcd.print("12"); break;
  }

  lcd.print("/");
  lcd.print(myRTC.year);

  lcd.setCursor(13, 1);
  switch (myRTC.dayofweek) {
    case 2: lcd.print("Seg"); break;
    case 3: lcd.print("Ter"); break;
    case 4: lcd.print("Qua"); break;
    case 5: lcd.print("Qui"); break;
    case 6: lcd.print("Sex"); break;
    case 7: lcd.print("Sab"); break;
    case 1: lcd.print("Dom"); break;
  }
  lcd.setCursor(3, 0);
  if ((myRTC.hours >= 0 && myRTC.hours < 10)) {
    lcd.print("0");
    lcd.print(myRTC.hours);
  }
  
  else if (myRTC.hours <= 23) {
    lcd.print(myRTC.hours);
  }

 // repete alimentação, quando relógio marca 23:59 ativa repetição
   for (int x = 0; x < 2; x++) {
    if (EEPROM.read(addrActive[x]) == 0 && (myRTC.hours == 23 && myRTC.minutes == 59)) {
        EEPROM.write(addrActive[alimentacao],x);
        EEPROM.write(addrActive[x], 1);
        alarmOn = 1;
    }
  }
  
  lcd.print(":");
  if (myRTC.minutes < 10) {
    lcd.print("0");
  }
  lcd.print(myRTC.minutes);
  lcd.print(":");
  if (myRTC.seconds < 10) {
    lcd.print("0");
  }
  lcd.print(myRTC.seconds);
  lcd.print("   ");
}

///   FUNÇÃO DE ALARME  ///
void alarmMode() {
  if (submode == 0) {
    lcd.setCursor(0, 0);
    lcd.write(1);
   

    if (digitalRead(btnChange) == LOW) {
      alimentacao++;
      if (alimentacao > 1 ) {
        alimentacao = 0;
      }
      delay(200);
    }
    lcd.setCursor(2, 0);
    lcd.print("Alimentacao#");
    lcd.print(alimentacao + 1);
    lcd.setCursor(0, 1);
    if (EEPROM.read(addrH[alimentacao]) > 23) {
      EEPROM.write(addrH[alimentacao], 0);
    }
    if (EEPROM.read(addrM[alimentacao]) > 59) {
      EEPROM.write(addrM[alimentacao], 0);
    }
    if (EEPROM.read(addrActive[alimentacao]) > 1) {
      EEPROM.write(addrActive[alimentacao], 0);
    }
    if ((EEPROM.read(addrH[alimentacao]) >= 0 && EEPROM.read(addrH[alimentacao]) < 10)) {
      lcd.setCursor(0, 1);
      lcd.print("0");
      lcd.print(EEPROM.read(addrH[alimentacao]));
    }
   
    else if (EEPROM.read(addrH[alimentacao]) <= 23) {
      lcd.print(EEPROM.read(addrH[alimentacao]));
      alarmHour = EEPROM.read(addrH[alimentacao]);
    }
   
    lcd.print(":");
    if (EEPROM.read(addrM[alimentacao]) < 10) {
      lcd.print("0");
    }
    lcd.print(EEPROM.read(addrM[alimentacao]));
    alarmMinutes = EEPROM.read(addrM[alimentacao]);
    lcd.print(" ");
    
    lcd.print(" ");
    if (EEPROM.read(addrActive[alimentacao]) == 1 ) { 
      lcd.print(" ON ");
    }
    if (EEPROM.read(addrActive[alimentacao]) == 0) {
      lcd.print(" OFF ");
      alarmOn = 0;
    }
  }
  else {
    lcd.setCursor(0, 0);
    lcd.write(1);
    lcd.setCursor(2, 0);
    lcd.print("Ajustar");
    lcd.setCursor(0, 1);
    lcd.print("#");
    lcd.print(alimentacao+1);
    lcd.setCursor(11, 1);
    if (alarmHour < 10) {
      lcd.print("0");
      lcd.setCursor(12, 1);
    }
    lcd.print(alarmHour);
    lcd.setCursor(13, 1);
    lcd.print(":");
    lcd.setCursor(14, 1);
    if (alarmMinutes < 10) {
      lcd.print("0");
      lcd.setCursor(15, 1);
    }
    lcd.print(alarmMinutes);
    
  }
  //  lcd.print(alarmMinutes);
  if (digitalRead(btnOK) == LOW) {
    submode++;
    if (submode > 4) {
      submode = 0;
    }
    delay(200);
    lcd.clear();
  }
  if (submode == 1) {
    lcd.setCursor(11, 0);
    lcd.write(1);
    lcd.write(1);
    if (digitalRead(btnChange) == LOW) {
      alarmHour++;
      if (alarmHour > 23) {
        alarmHour = 0;
      }
      delay(500);
    }
  }
  if (submode == 2) {
    lcd.setCursor(14, 0);
    lcd.write(1);
    lcd.write(1);
    if (digitalRead(btnChange) == LOW) {
      alarmMinutes++;
      if (alarmMinutes >= 60) {
        alarmMinutes = 0;
      }
      delay(500);
    }
  }
  
  while (submode == 3) {
    if(alarmOn==1){
    lcd.setCursor(1, 0);
    lcd.print("Alimentacao#");
    lcd.print(alimentacao + 1);
    lcd.setCursor(5, 1);
    lcd.print(" ON ");
    }
    else{
    lcd.setCursor(1, 0);
    lcd.print("Alimentacao#");
    lcd.print(alimentacao + 1);
    lcd.setCursor(5, 1);
    lcd.print(" OFF ");
    }
    
    if (digitalRead(btnChange) == LOW) {
      alarmOn = !alarmOn;
      delay(200);
    }

    if (digitalRead(btnOK) == LOW && alarmOn == 1) {
      lcd.clear();
      lcd.setCursor(0, 1);
      lcd.print("Salvando.");
      delay(200);
      lcd.print(".");
      delay(200);
      lcd.print(".");
      delay(200);
      lcd.print(".");
      delay(200);
      lcd.print(".");
      delay(200);
      lcd.print(".");
      delay(200);
      lcd.print(".");
      delay(200);
      lcd.print(".");
      delay(200);
      lcd.print(".");
      if (alarmHour != 12) {
        EEPROM.write(addrH[alimentacao], alarmHour);
      }
      else if ( alarmHour == 23) {
        EEPROM.write(addrH[alimentacao], 0);
      }
      else {
        EEPROM.write(addrH[alimentacao], alarmHour);
      }

      EEPROM.write(addrM[alimentacao], alarmMinutes);
      EEPROM.write(addrActive[alimentacao], 1);
      submode = 0;
      mode = 1;
      lcd.clear();
    }
    if (digitalRead(btnOK) == LOW && alarmOn == 0)  {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Alimentacao#");
      lcd.print(alimentacao + 1);
      lcd.setCursor(0, 1);
      lcd.print(" OFF ");
      
      EEPROM.write(addrActive[alimentacao], 0);
      submode = 0;
      mode = 1;
      
      delay(2000);
      lcd.clear();
    }
  }
}

///   CONFIGURAÇÃOES DE DATA E HORA  ///

void updateRTC() {
  lcd.setCursor(0, 0);
  lcd.write(2);
  if (digitalRead(btnOK) == LOW) {
    submode++;
    if (submode > 7) {
      submode = 0;
    }
    delay(200);
    lcd.clear();
  }

  if (submode == 0) {
    lcd.setCursor(2, 0);
    lcd.print(" Ajustar Data/Hora?");
    lcd.setCursor(0, 1);
    lcd.print("Tecle OK");
  }
  if (submode == 1) {
    lcd.setCursor(2, 0);
    lcd.print("Ajustar Mês");
    lcd.setCursor(0, 1);
    lcd.print("--> ");
    if (digitalRead(btnChange) == LOW) {
      M++;
      if (M == 13) {
        M = 1;
      }
      delay(200);
    }
    switch (M) {
      case 1: lcd.print("Janeiro     ");
        break;
      case 2: lcd.print("Fevereiro    ");
        break;
      case 3: lcd.print("Marco       ");
        break;
      case 4: lcd.print("Abril       ");
        break;
      case 5: lcd.print("Maio        ");
        break;
      case 6: lcd.print("Junho        ");
        break;
      case 7: lcd.print("Julho        ");
        break;
      case 8: lcd.print("Agosto      ");
        break;
      case 9: lcd.print("Setembro   ");
        break;
      case 10: lcd.print("Outubro     ");
        break;
      case 11: lcd.print("Novembro    ");
        break;
      case 12: lcd.print("Dezembro    ");
        break;
    }
  }
  if (submode == 2) {
    lcd.setCursor(2, 0);
    lcd.print("Ajustar Dia");
    lcd.setCursor(0, 1);
    lcd.print("--> ");
    if (digitalRead(btnChange) == LOW) {
      D++;
      if (D == 32) {
        D = 1;
      }
      delay(200);
    }
    lcd.print(D);
    lcd.print(" ");
  }

  if (submode == 3) {
    lcd.setCursor(2, 0);
    lcd.print("Ajustar Ano");
    lcd.setCursor(0, 1);
    lcd.print("--> ");
    if (digitalRead(btnChange) == LOW) {
      Y++;
      if (Y == 2099) {
        Y = 2000;
      }
      delay(200);
    }
    lcd.print(Y);
  }
  if (submode == 4) {
    lcd.setCursor(2, 0);
    lcd.print("Ajustar Dia da semana");
    lcd.setCursor(0, 1);
    lcd.print("--> ");
    if (digitalRead(btnChange) == LOW) {
      day++;
      if (day == 8) {
        day = 1;
      }
      delay(200);
    }
    switch (day) {
      case 1: lcd.print("Domingo      ");
        break;
      case 2: lcd.print("Segunda-Feira      ");
        break;
      case 3: lcd.print("Terca-Feira     ");
        break;
      case 4: lcd.print("Quarta-Feira   ");
        break;
      case 5: lcd.print("Quinta-Feira    ");
        break;
      case 6: lcd.print("Sexta-Feira      ");
        break;
      case 7: lcd.print("Sabado    ");
        break;
    }
  }
  if (submode == 5) {
    lcd.setCursor(2, 0);
    lcd.print("Ajustar Hora");
    lcd.setCursor(0, 1);
    lcd.print("--> ");
    if (digitalRead(btnChange) == LOW) {
      h++;
      if (h == 24) {
        h = 0;
      }
      delay(200);
    }
    else {
      lcd.print(h);
      lcd.print(" ");

    }

  }
  if (submode == 6) {
    lcd.setCursor(2, 0);
    lcd.print("Ajustar Minutos");
    lcd.setCursor(0, 1);
    lcd.print("--> ");
    if (digitalRead(btnChange) == LOW) {
      m++;
      if (m == 60) {
        m = 0;
      }
      delay(200);
    }
    lcd.print(m);
    lcd.print(" ");
  }
  if (submode == 7) {
    lcd.setCursor(2, 0);
    lcd.print("Data & Hora");
    lcd.setCursor(0, 1);
    lcd.print("Salvando");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    delay(200);
    lcd.print(".");
    myRTC.setDS1302Time(00, m, h, day, D, M, Y);
    delay(200);
    lcd.clear();
    mode = 1;
  }
}
