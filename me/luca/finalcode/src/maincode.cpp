#include <Arduino.h>
#include "Servo.h"
#include "Wire.h"

Servo servo;

boolean GAMESTATUS = false;
int GAMES_LEFT = 0;

const byte start = 0;
const byte stop = 1;
const long One_Second_Pause = 1000;
const long Half_Second_Pause = 500;
unsigned long previousMillis = 0;

void transmit_inst(byte b);
void transmit_aryInst(byte b[2], byte inst);
boolean check_start_init();
boolean check_game_over();

void setup() {
    Serial.begin(9600);
    Wire.begin();

    pinMode(3, INPUT_PULLUP); //IR 1
    pinMode(4, INPUT_PULLUP); //IR 2
    pinMode(5, INPUT_PULLUP); //Game-Over IR


    servo.attach(9, 440, 1900);
    servo.write(0);
    pinMode(13, OUTPUT); //Sneaky workaround pin
    digitalWrite(13, HIGH);
}

void loop() {
    while (!GAMESTATUS){
        GAMESTATUS = check_start_init();
        delay(15);
    }
        GAMESTATUS = check_game_over();
    delay(15);
}


boolean check_start_init(){
    boolean money = GAMES_LEFT > 0;
    if(digitalRead(3) == 0){ // 50 CENT
        if(!money)
            money = true;
        GAMES_LEFT+=1;
    }
    if(digitalRead(4) == 0){ // 1 EUR
        if(!money)
            money = true;
        GAMES_LEFT+=2;
    }

    if(money){
        /*FIXME Zwei Sekunden Delay */
        previousMillis = millis();
        while(millis()- previousMillis < 2*One_Second_Pause){
            if(digitalRead(3) == 0){ // 50 CENT
                GAMES_LEFT+=1;
            }
            if(digitalRead(4) == 0){ // 1 EUR
                GAMES_LEFT+=2;
            }
            delay(10);
        }

        /*FIXME Servo move phase*/
        servo.write(0);
        for(int i = 0; i<90; i++){
            servo.write(i);

            previousMillis = millis();
            while(millis() - previousMillis < 2){
                if(digitalRead(3) == 0){ // 50 CENT
                    GAMES_LEFT+=1;
                }
                if(digitalRead(4) == 0){ // 1 EUR
                    GAMES_LEFT+=2;
                }
            }
            delay(9);
        }
        for(int i = 90; i>0; i--){
            servo.write(i);
            previousMillis = millis();

        }

        byte instruction[2] = {(byte) 0, (byte) GAMES_LEFT};
        transmit_aryInst(instruction, start);
        /*FIXME Com phase without wire!*/
        Serial.println("[Log] Starting Game!");
        GAMES_LEFT--;
        digitalWrite(13, LOW);
        return true;
    }
    return false;
}

boolean check_game_over(){
    if(!GAMESTATUS)
        return false;

    boolean gmovr = digitalRead(5) == 0;
    if(!gmovr){
        if(digitalRead(3) == 0){ // 50 CENT
            GAMES_LEFT+=1;
        }
        if(digitalRead(4) == 0){ // 1 EUR
            GAMES_LEFT+=2;
        }
        return true;
    }

    byte inst[2] = {stop, (byte) GAMES_LEFT};
    transmit_aryInst(inst, stop);
    Serial.println("[Log] Ending Game!");
    digitalWrite(13, HIGH);
    return false;
}

void transmit_inst(byte b){
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
