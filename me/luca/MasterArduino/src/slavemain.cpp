#include <Arduino.h>
#include "Wire.h"
#include "SD.h"
#include "SPI.h"

#include "TimeLib.h"
#include "Stream.h"
#include "LiquidCrystal_I2C.h"
#include "NoDelay.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);


boolean GAMESTATUS = false;
boolean inst_start_game();
boolean inst_stop_game();

void onRecieve(int p);
void onRequest(int p);

File save;


int points = 0;
int GAME_NO = 0;
int PREPAY = 0;
int lcd_status = 0;

const long One_Second_Pause = 1000;
const long Half_Second_Pause = 500;

unsigned long previousMillis = 0;

void setup() {
    Serial.begin(9600);
    Wire.begin(1);
    Wire.onReceive(onRecieve);
    Wire.onReceive(onRequest);
    lcd.init();
    lcd.setBacklight(true);

    pinMode(3, INPUT_PULLUP); //Sneaky For-Loop block work around :DD (Ich wollte das hochzählen unbedingt haben.)

    if(!SD.begin(12)){
        Serial.println("[Severe Error] SD-Karte konnte nicht initalisiert werden!");
    }

    previousMillis = millis();
}

void loop() {
        while (!GAMESTATUS){
            if(digitalRead(3) == LOW) {
                //Pin 3 ist LOW wenn das game starten soll!
                GAMESTATUS = inst_start_game();
                continue;
            }

            previousMillis = millis();
            if(PREPAY > 0){
                lcd.clear();
                lcd.print("Knopf druecken!");
                lcd.setCursor(0, 1);
                lcd.print("Uebrige Spiele: "+ String(PREPAY));

            }else{
                lcd.clear();
                lcd.print("1.Muenze(n) einwerfen");
                lcd.setCursor(0, 1);
                lcd.print("2.Knopf druecken");
                previousMillis = millis();
                for(int i = 0; i<5; i++){
                    while(millis()-previousMillis > Half_Second_Pause){
                        if(digitalRead(3) == LOW) //Pin 3 ist LOW wenn das game starten soll!
                            GAMESTATUS = inst_start_game();
                        break;
                    }

                    lcd.scrollDisplayLeft();
                }
            }
        }


        switch (lcd_status) {
            case 0:
                for(int i = 0 ; i <13; ++i){
                    lcd.clear();
                    lcd.print("Ueberlebenszeit:");
                    lcd.setCursor(0,1);
                    lcd.print(String(minute()) +" min "+String(second())+" sek");

                    while(millis()-previousMillis > One_Second_Pause){
                        if(digitalRead(3) == HIGH){
                            //Pin 3 ist HIGH wenn das Game zu Ende ist!
                            GAMESTATUS = inst_stop_game();
                            break;
                        }
                    }
                }
                lcd_status++;
                break;
            case 1:
                lcd.clear();
                lcd.print("Spiel-Nr. "+String(GAME_NO));

                while(millis()-previousMillis > (13*One_Second_Pause)){
                    if(digitalRead(3) == HIGH){
                        //Pin 3 ist HIGH wenn das Game zu Ende ist!
                        GAMESTATUS = inst_stop_game();
                        break;
                    }
                }
                lcd_status++;
                break;
            case 2:
                lcd.clear();
                lcd.print("Uebrige Spiele: "+String(PREPAY));

                while(millis()-previousMillis > (6*One_Second_Pause)){
                    if(digitalRead(3) == HIGH){
                        //Pin 3 ist HIGH wenn das Game zu Ende ist!
                        GAMESTATUS = inst_stop_game();
                        break;
                    }
                }
                lcd_status = 0;
                break;
        }
    }


void onRecieve(int p){
    Serial.println("Receiving data.");

    byte com[2];
    Wire.readBytes(com, sizeof(com));

    PREPAY = int(com[1]); //Menge an übrigen Spielen

    if(com[0] == (byte) 1){
        GAMESTATUS = inst_stop_game();
        return;
    }else{
        GAMESTATUS = inst_start_game();
        return;
    }


}

void onRequest(int p){
   //Ungenutzt
}

boolean inst_stop_game(){
    randomSeed(millis());
    points = (minute()*60+second())*random(1,4);

    if(save){
        save = SD.open("data.txt", FILE_WRITE);
        if(GAME_NO <10)
            save.println("Game-Stats #0"+String(GAME_NO)+"; Points: "+ String(points)
                         +"; Hours: "+String(hour())+" Minutes: "+String(minute())+" Seconds: "+String(second()));

        else
            save.println("Game-Stats #"+String(GAME_NO)+"; Points: "+ String(points)
                         +"; Hours: "+String(hour())+" Minutes: "+String(minute())+" Seconds: "+String(second()));
        save.close();
    }

    points = 0;
    return false;
}

boolean inst_start_game(){
    Serial.println("[Log] Starting Game No. "+ String(++GAME_NO) + "!");
    setTime(0,0,0,0,0,0);
    if(PREPAY > 0)
        PREPAY--;

    return true;
}