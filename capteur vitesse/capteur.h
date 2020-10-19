
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//variables programme
int capteur_str = A0;
int capteur_end = A1;
int capteur_bpt = 2;
int var_dst     = 1;
int var_ses     = 150;

// variables de travail
int chrono      = 0;
int temps       = 0;
int boucle      = false;
int var_top     = false;
boolean bpt_1;

LiquidCrystal_I2C lcd(0x20,16,2);

void setup(){
  // initialisation E/S
  pinMode(capteur_bpt, INPUT);
  
  // initialisation LCD
  lcd.init();
  lcd.backlight();
}


void loop(){

      bpt_1 = digitalRead(capteur_bpt);
      
      if(bpt_1 = 1){
        
      // Effacement variables
      chrono  = 0;
      temps   = 0;
      boucle  = true;
      var_top = false;
      
      // Effacement LCD
      lcd.clear();
      lcd.setCursor(1,4);
      lcd.print("Ready");
      
      }

      // Boucle mesure
      while (boucle == true){
      int top_0 = analogRead(capteur_str);
      int top_1 = analogRead(capteur_end);

      //
      if (var_top == false and top_0 > var_ses){
        chrono = millis();
        var_top = true;
      }
        

      if (var_top == true and top_1 > 150){
        int chrono_stop = millis();
        temps = (chrono_stop - chrono)/1000;
        boucle = false;

        // calcul vitesse
        int m_s = var_dst/temps;
        int km_h = m_s*3.6;

        // Affichage LCD
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Vitesse :");
        lcd.setCursor(1,0);
        lcd.print(m_s);
        lcd.setCursor(2,0);
        lcd.print(km_h);
        break;
      }
      }


}
