#include <Arduino.h>
#include "Servo.h"
#include "SD.h"
#include "SPI.h"
#include "Wire.h"

const byte start = 0;
const byte stop = 1;
Servo servo;

boolean GAMESTATUS = false;
boolean check_start_init();
boolean check_game_over();

void transmit_inst(byte b);
void transmit_aryInst(byte b[2], byte inst);
int GAMES_LEFT;


void setup() {
    Wire.begin();
    Serial.begin(9600);


    pinMode(3, INPUT_PULLUP); //IR 1
    pinMode(4, INPUT_PULLUP); //IR 2
    pinMode(5, INPUT_PULLUP); //IR 3
    pinMode(6, INPUT_PULLUP); //Start Knopf

    servo.attach(9, 440, 1900);
    pinMode(10, INPUT_PULLUP); //TODO placeholder Game-Over IR
    pinMode(13, OUTPUT); //Sneaky workaround pin
    digitalWrite(13, HIGH);
}


void loop() {

    while(!GAMESTATUS){
        GAMESTATUS = check_start_init();
        delay(10);
    }

    GAMESTATUS = !check_game_over();

}


boolean check_start_init(){
    boolean money = GAMES_LEFT > 0;
    Serial.println(GAMES_LEFT);

    // boolean money = digitalRead(3) == 0 || digitalRead(4) == 0 || digitalRead(5) == 0; -> Alter Code, der nicht auf Geldmenge achtet
    if(digitalRead(3) == 0){ // 50 CENT
        money = true;
        GAMES_LEFT+=1;
    }
    if(digitalRead(4) == 0){ // 1 EUR
        money = true;
        GAMES_LEFT+=2;
    }
    if(digitalRead(5) == 0){ // 2 EUR
        money = true;
        GAMES_LEFT+=4;
    }

    boolean startbtn = digitalRead(6) == 0;

    if(money && startbtn){
        for (int i = 0; i < 91; ++i) { //Ballarm
            servo.write(i);
        }
        servo.write(0);
        byte instruction[2] = {(byte) 0, (byte) GAMES_LEFT};
        transmit_aryInst(instruction, start);

        Serial.println("[Log] Starting Game!");
        GAMES_LEFT--;
        digitalWrite(13, LOW);
        return true;
    }


    return false;
}


boolean check_game_over(){
   boolean gmovr = digitalRead(10) == 0;
   if(!gmovr)
       return false;

    byte inst[2] = {stop, (byte) GAMES_LEFT};
    transmit_aryInst(inst, stop);
    Serial.println("[Log] Ending Game!");
    digitalWrite(13, HIGH);
    return true;
}


void transmit_inst(byte b){ //Einzelübertragung (Deprecated)
    Wire.beginTransmission(1);
    Wire.write(b);
    Wire.endTransmission();
    Wire.beginTransmission(2);
    Wire.write(b);
    Wire.endTransmission();
}


void transmit_aryInst(byte b[2], byte inst){
    Wire.beginTransmission(1);
    Wire.write(b, sizeof(b));
    Wire.endTransmission();
    Wire.beginTransmission(2);
    Wire.write(inst);
    Wire.endTransmission();

}