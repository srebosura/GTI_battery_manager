    //GTI Battery Disconnect/Reconnect Control
    //by Samuel Rebosura
    
    #include <EEPROM.h> 
    #include <LiquidCrystal.h>
    LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
    
    //buttons variables
    float sw_version = 1.1;
    const int buttonPinLeft = 10;      // pin for the Up button
    const int buttonPinRight = 11;    // pin for the Down button
    const int buttonPinEnter = 12;   // pin for the Enter button

    int lastButtonPushed = 1;

    int lastButtonEnterState = HIGH;   // the previous reading from the Enter input pin
    int lastButtonholdEnterState = HIGH;   // the previous reading from the Esc input pin
    int lastButtonLeftState = HIGH;   // the previous reading from the Left input pin
    int lastButtonRightState = HIGH;   // the previous reading from the Right input pin
    int lastButtonholdRightState = HIGH;

    long lastEnterDebounceTime = 0;  // the last time the output pin was toggled
    long lastholdEnterDebounceTime = 0;  // the last time the output pin was toggled
    long lastLeftDebounceTime = 0;  // the last time the output pin was toggled
    long lastRightDebounceTime = 0;  // the last time the output pin was toggled
    long lastholdRightDebounceTime = 0; 
    long debounceDelay = 75;    // the debounce time
    
    
    long holddebounceDelay = 3000; //set_button_active delay
    
    
    const int run_ledPin =  13;
    //int run_ledState = 0; 
    const int relayPin = 3; //battery connect/disconnect pin
    int relayState = 0; //initial relay state
    
    
    int menu_display = 0;//0-default display 1-BRV,2-LVD,3-HVD menu display flag
    int last_menu=0; //last displayed menu flag
    int manage_status = 0;
    int manage_mode = LOW; //high = auto , low = manual
    float EEPROM_read_LVD = 0.0f; // variable for Low Voltage Disconnect
    float EEPROM_read_BRV = 0.0f; // variable for Battery Voltage Reconect
    float EEPROM_read_HVD = 0.0f; // variable for High Voltage Disconnect
    float EEPROM_read_R1 = 0.0f; // variable for Battery Voltage Reconect
    float EEPROM_read_R2 = 0.0f; // variable for High Voltage Disconnect
    
    //Battery variables
    const int analogInput = 0;
    float hvd_var = 0.0f;
    float lvd_var = 0.0f;
    float brv_var = 0.0f;
    float SoC = 0.0;
    float vout = 0.0;
    float vin = 0.0;
    float R1 = 0.0f; // resistance of R1 (100K)
    float R2 = 0.0f; // resistance of R2 (10K)
    int value = 0;
    int EE_addr1 = 0;
    int EE_addr2 = 4;
    int EE_addr3 = 8;
    int EE_addr4 = 16;
    int EE_addr5 = 32;
    void setup(){
      

       pinMode(buttonPinLeft, INPUT_PULLUP);
       pinMode(buttonPinRight, INPUT_PULLUP);
       pinMode(buttonPinEnter, INPUT_PULLUP);
       pinMode(analogInput, INPUT);
       pinMode(run_ledPin, OUTPUT);
       pinMode(relayPin, OUTPUT);
       
       if (digitalRead(buttonPinEnter)==0)  {
        delay(2000);
        R1 = 100380.0; // actual resistance of R1 (100K) default
        R2 = 9940.0; // actual resistance of R2 (10K) default
        EEPROM.put(EE_addr4, R1);
        EEPROM.put(EE_addr5, R2);
        
        auto_detect_voltage();
        }


      //read stored settings
      EEPROM.get(EE_addr1,EEPROM_read_LVD);
      lvd_var = EEPROM_read_LVD;
      EEPROM.get(EE_addr2,EEPROM_read_BRV);
      brv_var = EEPROM_read_BRV;
      EEPROM.get(EE_addr3,EEPROM_read_HVD);
      hvd_var = EEPROM_read_HVD;
      EEPROM.get(EE_addr4,EEPROM_read_R1);
      R1 = EEPROM_read_R1;
      EEPROM.get(EE_addr5,EEPROM_read_R2);
      R2 = EEPROM_read_R2;
      
       //lcd.begin(16, 2);
       //lcd.print("Batt Man v1.1");
       scroll("Batt Man v1.1");
       lcd.begin(16, 2);
       lcd.print("USER Settings");
       lcd.setCursor(0, 1);
       lcd.print("BRV=");
       lcd.print(brv_var,1);
       lcd.print("LVD=");
       lcd.print(lvd_var,1);
       read_analog_in();
       if (abs(hvd_var-vin) > 10) {
        auto_detect_voltage();
       }
       delay(3000);
       
    }


void read_buttons() {
 // read the state of the switch into a local variable:
  int reading;
  int buttonEnterState=HIGH;             // the current reading from the Enter input pin
  int buttonholdEnterState=HIGH;             // the current reading from the input pin
  int buttonLeftState=HIGH;                  // the current reading from the input pin
  int buttonholdLeftState=HIGH;                  // the current reading from the input pin
  int buttonRightState=HIGH;             // the current reading from the input pin
  int buttonholdRightState=HIGH;  
  
  //Enter button
                  // read the state of the switch into a local variable:
                  reading = digitalRead(buttonPinEnter);

                  // check to see if you just pressed the enter button
                  // (i.e. the input went from LOW to HIGH),  and you've waited
                  // long enough since the last press to ignore any noise: 
               
                  // If the switch changed, due to noise or pressing:
                  if (reading != lastButtonEnterState) {
                    // reset the debouncing timer
                    lastEnterDebounceTime = millis();
                  }
                 
                  if ((millis() - lastEnterDebounceTime) > debounceDelay) {
                    // whatever the reading is at, it's been there for longer
                    // than the debounce delay, so take it as the actual current state:
                    buttonEnterState=reading;
                    lastEnterDebounceTime=millis();
                  }
                 
                  // save the reading.  Next time through the loop,
                  // it'll be the lastButtonState:
                  lastButtonEnterState = reading;
                 

    // Enter + hold button              
                 
                  reading = digitalRead(buttonPinEnter);

                  
                  if (reading != lastButtonholdEnterState) {
                    
                    lastholdEnterDebounceTime = millis();
                  }
                 
                  if ((millis() - lastholdEnterDebounceTime) > holddebounceDelay) {
                    
                    buttonholdEnterState = reading;
                    lastholdEnterDebounceTime=millis();
                  }
                 
                  
                  lastButtonholdEnterState = reading;               
                                 
   //Down button              
                  
                  reading = digitalRead(buttonPinRight);
                 
                  if (reading != lastButtonRightState) {                  
                    lastRightDebounceTime = millis();
                  }
                  if ((millis() - lastRightDebounceTime) > debounceDelay) {                 
                    buttonRightState = reading;
                   lastRightDebounceTime =millis();
                  }
                                 
                  lastButtonRightState = reading;                 

 //Down button plus hold             
                  
                  reading = digitalRead(buttonPinRight);                 
                  if (reading != lastButtonholdRightState) {
                    lastholdRightDebounceTime = millis();
                  }
                 
                  if ((millis() - lastholdRightDebounceTime) > holddebounceDelay) {
                    buttonholdRightState = reading;
                   lastholdRightDebounceTime =millis();
                  }                             
                  lastButtonholdRightState = reading;                 
                 
    //Up button              
                  reading = digitalRead(buttonPinLeft);

                  
                  if (reading != lastButtonLeftState) {
                   
                    lastLeftDebounceTime = millis();
                  }                
                  if ((millis() - lastLeftDebounceTime) > debounceDelay) {
                    buttonLeftState = reading;
                    lastLeftDebounceTime=millis();;
                  }                               
                  lastButtonLeftState = reading; 
                  //menu actions

                  if(buttonholdEnterState==LOW && menu_display==2){
                    lastButtonPushed=5;
                  }else if(buttonholdEnterState==LOW && menu_display==3){
                    lastButtonPushed=6;
                  }else if(buttonholdEnterState==LOW && menu_display==4){
                    lastButtonPushed=7;
                  }else if(buttonholdEnterState==LOW && menu_display==1){
                    lastButtonPushed=10;
                  }else if(buttonholdEnterState==LOW && menu_display==11){
                    lastButtonPushed=12;                                      
                  }else if(buttonRightState==LOW && menu_display==2){
                    lastButtonPushed=3;
                  }else if(buttonRightState==LOW && menu_display==3){
                    lastButtonPushed=4;
                  }else if(buttonRightState==LOW && menu_display==4){
                    lastButtonPushed=11;
                  }else if(buttonRightState==LOW && menu_display==11){
                    lastButtonPushed=14;
                  }else if(buttonRightState==LOW){
                    lastButtonPushed=2;
                  }else if(buttonholdRightState==LOW){
                    lastButtonPushed=9;
                  }else if(buttonLeftState==LOW && (menu_display==2 || menu_display==3  || menu_display==4 || menu_display==11 || menu_display==14)){
                    lastButtonPushed=1;
                  }else if (buttonEnterState==LOW && (menu_display==2 || menu_display==3  || menu_display==4 || menu_display==11 || menu_display==14)){
                    lastButtonPushed=9;
                  }else if (buttonEnterState==LOW && menu_display==1){
                    lastButtonPushed=8;
                  }else if(buttonEnterState==LOW){
                    lastButtonPushed=13;                     
                  
                  }                  
 }
 
void lcd_display(int col, int row, String msg1, String msg2)  {
      lcd.setCursor(col, row);
      lcd.print(msg1);
      lcd.print(msg2); 
  }
  
void set_BRV() {
  
    EEPROM.put(EE_addr2, brv_var);
    lcd.clear();
    EEPROM.get(EE_addr1,EEPROM_read_LVD);
    lvd_var = EEPROM_read_LVD;
    EEPROM.get(EE_addr2,EEPROM_read_BRV);
    brv_var = EEPROM_read_BRV;
    EEPROM.get(EE_addr3,EEPROM_read_HVD);
    hvd_var = EEPROM_read_HVD;
  
    lcd_display(0,0,"Settings Saved!"," ");
    delay(2000);
    lastButtonPushed=2; //reset the lastButtonPushed variable
    
    }
  
void set_LVD() {
    
   EEPROM.put(EE_addr1, lvd_var);
   lcd.clear();
   EEPROM.get(EE_addr1,EEPROM_read_LVD);
   lvd_var = EEPROM_read_LVD;
   EEPROM.get(EE_addr2,EEPROM_read_BRV);
   brv_var = EEPROM_read_BRV;
   EEPROM.get(EE_addr3,EEPROM_read_HVD);
   hvd_var = EEPROM_read_HVD;
 
   lcd_display(0,0,"Settings Saved!"," ");
   delay(2000);
   lastButtonPushed=3; //reset the lastButtonPushed variable
   }


void set_HVD() {
  
  EEPROM.put(EE_addr3, hvd_var);
  lcd.clear();
  EEPROM.get(EE_addr1,EEPROM_read_LVD);
  lvd_var = EEPROM_read_LVD;
  EEPROM.get(EE_addr2,EEPROM_read_BRV);
  brv_var = EEPROM_read_BRV;
  EEPROM.get(EE_addr3,EEPROM_read_HVD);
  hvd_var = EEPROM_read_HVD;
  
  lcd_display(0,0,"Settings Saved!"," ");
  delay(2000);
  lastButtonPushed=4; //reset the lastButtonPushed variable
  }


  
void read_analog_in() { // read the value at analog input

       value = analogRead(analogInput);
       vout = ((value) * 5.0) / 1024.0; 
       vin = vout / (R2/(R1+R2)); 
       if (vin<0.09) {
        vin=0.0;//statement to junk undesired reading !
          
          }
}

 void read_user_set() {
  //read stored settings
      EEPROM.get(EE_addr1,EEPROM_read_LVD);
      lvd_var = EEPROM_read_LVD;
      EEPROM.get(EE_addr2,EEPROM_read_BRV);
      brv_var = EEPROM_read_BRV;
      EEPROM.get(EE_addr3,EEPROM_read_HVD);
      hvd_var = EEPROM_read_HVD;
      EEPROM.get(EE_addr4,EEPROM_read_R1);
      R1 = EEPROM_read_R1;
      EEPROM.get(EE_addr5,EEPROM_read_R2);
      R2 = EEPROM_read_R2;
 }

  void default_display()  {
      String run_mode;
      String run_status;
      lcd.setCursor(0, 0);
        if (manage_mode == LOW)  {
          run_mode = "Manual   ";
        }
        else if (manage_mode == HIGH) {
         run_mode = "AUTO     ";
        }
      
      lcd.print(run_mode);
        if (relayState == 0)  {
          run_status = "OFFLINE";
        }
        else if (relayState == 1) {
         run_status = " ONLINE";
        }
      lcd.print(run_status);
      lcd.setCursor(0, 1);
      lcd.print("Bat=");
      lcd.print(vin,1);
      
      SoC = ((vin-lvd_var)/(hvd_var-lvd_var))*100;
       if (SoC<0.1) {
        SoC=0;
       }
       if (SoC>100.1) {
        SoC=100;
       }
      lcd.print(" SoC=");
      lcd.print(SoC,0);
      if (SoC<9) {
        lcd.print(" ");
       }
      lcd.print("%");
      delay(300);
      menu_display = 1;
  }
  
  void set_menu_BRV() {
      lcd.setCursor(0, 0);
      lcd.print("HIT  ENT to SAVE");
      lcd.setCursor(0, 1);
      lcd.print("BRV       V=");
      lcd.print(EEPROM_read_BRV,1);
      brv_var = EEPROM_read_BRV;
      delay(2000);
      while (lastButtonPushed==2 ||lastButtonPushed==3) {
        read_buttons();
        if (digitalRead(buttonPinLeft)== 0) {
          delay(250);
          brv_var = brv_var+0.1;
        }
        if (digitalRead(buttonPinRight)== 0) {
          delay(250);
          brv_var = brv_var-0.1;
      }
      lcd.setCursor(0, 1);   
      lcd.print("BRV       V=");
      lcd.print(brv_var,1);
    }
    set_BRV();
    
  }
  
  void set_menu_LVD() {
      lcd.setCursor(0, 0);
      lcd.print("HIT  ENT to SAVE");
      lcd.setCursor(0, 1);
      lcd.print("LVD       V=");
      lcd.print(EEPROM_read_LVD,1);
      lvd_var = EEPROM_read_LVD;
      delay(2000);
      while (lastButtonPushed==2 ||lastButtonPushed==3) {
        read_buttons();
        if (digitalRead(buttonPinLeft)== 0) {
          delay(500);
          lvd_var = lvd_var+0.1;
        }
        if (digitalRead(buttonPinRight)== 0) {
          delay(500);
          lvd_var = lvd_var-0.1;
      }
      lcd.setCursor(0, 1);   
      lcd.print("LVD       V=");
      lcd.print(lvd_var,1);
    }
    set_LVD();
  }
  void set_menu_HVD() {
      lcd.setCursor(0, 0);
      lcd.print("HIT  ENT to SAVE");
      lcd.setCursor(0, 1);
      lcd.print("HVD       V=");
      lcd.print(EEPROM_read_HVD,1);
      hvd_var = EEPROM_read_HVD;
      delay(2000);
      while (lastButtonPushed==2 ||lastButtonPushed==3) {
        read_buttons();
        if (digitalRead(buttonPinLeft)== 0) {
          delay(250);
          hvd_var = hvd_var+0.1;
        }
        if (digitalRead(buttonPinRight)== 0) {
          delay(250);
          hvd_var = hvd_var-0.1;
      }
      lcd.setCursor(0, 1);   
      lcd.print("HVD       V=");
      lcd.print(hvd_var,1);
    }
    set_HVD();
  } 
  void set_menu_R1() {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("VBat=");
      lcd.print(vin,1);
      lcd.setCursor(0, 1);
      lcd.print("R1     =");
      lcd.print(EEPROM_read_R1,1);
      R1 = EEPROM_read_R1;
      delay(2000);
      while (lastButtonPushed==2 ||lastButtonPushed==3) {
        read_buttons();
        if (digitalRead(buttonPinLeft)== 0) {
          delay(250);
          R1 = R1+1.0;
        }
        if (digitalRead(buttonPinRight)== 0) {
          delay(250);
          R1 = R1-1.0;
      }
      read_analog_in();
      lcd.setCursor(0, 0);
      lcd.print("VBat=");
      lcd.print(vin,2);
      lcd.setCursor(0, 1);   
      lcd.print("R1     =");
      lcd.print(R1,1);
      delay(100);
    }
    set_R1();
    set_menu_R2();
  } 
 void set_menu_R2() {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("VBat=");
      lcd.print(vin,1);
      lcd.setCursor(0, 1);
      lcd.print("R2      = ");
      lcd.print(EEPROM_read_R2,1);
      R2 = EEPROM_read_R2;
      delay(2000);
      while (lastButtonPushed==2 ||lastButtonPushed==3) {
        read_buttons();
        if (digitalRead(buttonPinLeft)== 0) {
          delay(250);
          R2 = R2+1.0;
        }
        if (digitalRead(buttonPinRight)== 0) {
          delay(250);
          R2 = R2-1.0;
      }
      read_analog_in();
      lcd.setCursor(0, 0);
      lcd.print("VBat=");
      lcd.print(vin,2);
      lcd.setCursor(0, 1);   
      lcd.print("R2      = ");
      lcd.print(R2,1);
      delay(100);
    }
    set_R2();
  } 

  void set_R1() {
    EEPROM.put(EE_addr4, R1);
    lcd.clear();
    EEPROM.get(EE_addr4,EEPROM_read_R1);
    R1 = EEPROM_read_R1;
  
    lcd_display(0,0,"Settings Saved!"," ");
    delay(2000);
    lastButtonPushed=2; //default display
  }
  void set_R2() {
    EEPROM.put(EE_addr5, R2);
    lcd.clear();
    EEPROM.get(EE_addr5,EEPROM_read_R2);
    R2 = EEPROM_read_R2;
  
    lcd_display(0,0,"Settings Saved!"," ");
    delay(2000);
    lastButtonPushed=1; //default display
  }
  void auto_detect_voltage() {
      read_analog_in();
      lcd.begin(16, 2);
      lcd_display(0,0,"Vbat ="," ");
      lcd.print(vin);
      delay(2000);
      
      lcd_display(0,0,"Auto Detect","     ");
      lcd_display(0,1,"Activated ....."," ");
      delay(2500);
      
    if (vin < 19.0)  {
      lvd_var = 12.0;
      hvd_var = 13.2;
      brv_var = 12.5;
      EEPROM.put(EE_addr1, lvd_var);
      EEPROM.put(EE_addr2, brv_var);
      EEPROM.put(EE_addr3, hvd_var);
      lcd.clear();
      lcd_display(0,0,"12V Settings","    ");
      lcd_display(0,1,"Default loaded.."," ");
      delay(2500); 
      read_user_set();
      return;
      }
      
    if (vin < 33.0)  {
      lvd_var = 24.0;
      hvd_var = 26.4;
      brv_var = 25.0;
      EEPROM.put(EE_addr1, lvd_var);
      EEPROM.put(EE_addr2, brv_var);
      EEPROM.put(EE_addr3, hvd_var);      
      lcd.clear();
      lcd_display(0,0,"24V Settings","    ");
      lcd_display(0,1,"Default loaded.."," ");
      delay(2500); 
      read_user_set();
      return;
      }
    if (vin > 33.1)  {  
     lvd_var = 48.0;
     hvd_var = 54.4;
     brv_var = 50.0;
     EEPROM.put(EE_addr1, lvd_var);
     EEPROM.put(EE_addr2, brv_var);
     EEPROM.put(EE_addr3, hvd_var);
     lcd_display(0,0,"48V Settings","    ");
     lcd_display(0,1,"Default loaded.."," ");
     delay(2500); 
     read_user_set();
     return;
    }
    
  }
  void scroll (String msg) {
  lcd.clear();
  lcd.begin(16, 2);
  lcd.print(msg);
  delay(500);
    // scroll 13 positions (string length) to the left
  // to move it offscreen left:
  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
    // scroll one position left:
    lcd.scrollDisplayLeft();
    // wait a bit:
    delay(150);
  }

  // scroll 13 positions (string length + display length) to the right
  // to move it offscreen right:
  for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
    // scroll one position right:
    lcd.scrollDisplayRight();
    // wait a bit:
    delay(150);
  }
 
  // delay at the end of the full loop:
  delay(500);
  
}

  
  void manage_bat()  {
    if (vin <= lvd_var && manage_status==1) {
       relayState = 0;
       digitalWrite(relayPin, relayState);
       manage_status = relayState;
       digitalWrite(run_ledPin, relayState);
      }
    if (vin >= brv_var && manage_status==0 && manage_mode == HIGH ) {
      relayState = 1;
      digitalWrite(relayPin, relayState);
      manage_status = relayState;
      digitalWrite(run_ledPin, relayState);
    }
  }

  void set_relayPin()  {
    relayState = !relayState;   
    digitalWrite(relayPin, relayState);
    manage_status = relayState;
    manage_mode = !manage_mode;
    digitalWrite(run_ledPin, relayState);
    
  }
  
  void main_menu() {
  
   switch (lastButtonPushed) {
    case 1:
      default_display();
      menu_display = 1;
      last_menu = menu_display;
      break;
    
    case 2:
      
      lcd.setCursor(0, 0);
      lcd.print("GTI Batt MANAGER");
      lcd.setCursor(0, 1);
      lcd.print("BRV       V=");
      lcd.print(EEPROM_read_BRV,1);
      menu_display = 2;
      last_menu = menu_display;
      delay(150);
      break;
      
    case 3:
      
      lcd.setCursor(0, 0);
      lcd.print("GTI Batt MANAGER");
      lcd.setCursor(0, 1);
      lcd.print("LVD       V=");
      lcd.print(EEPROM_read_LVD,1);
      menu_display = 3;
      last_menu = menu_display;

      delay(150);
      break;
    
    case 4:
        
      lcd.setCursor(0,0);
      lcd.print("GTI Batt MANAGER");
      lcd.setCursor(0,1);
      lcd.print("HVD       V=");
      lcd.print(hvd_var,1);
      menu_display = 4; 
      last_menu = menu_display;

      delay(150);   
      break;  
      
    case 5:
      lastButtonPushed=2;
      menu_display = 5;      
      last_menu = menu_display;    
      delay(150);
      set_menu_BRV();
      break;  
    
    
    case 6:
      lastButtonPushed=2;
      menu_display = 6;
      last_menu = menu_display;     
      delay(150);   
      set_menu_LVD();
      
      break;
    case 7:
      lastButtonPushed=2;
      menu_display = 7;
      last_menu = menu_display;

      delay(150);   
      set_menu_HVD();   
      
      break;
    case 8:
      
      lcd_display(0,0,"Press for 3 sec","  ");
      lcd_display(0,1,"to START/STOP","   ");
      delay(3000);
      lastButtonPushed=1;
      menu_display=last_menu;         
      break;  
    case 9:
      lcd_display(0,0,"Hold ENT for 3s"," ");
      lcd_display(0,1,"  to change!","    ");
      //menu_display=last_menu+1;     
      break;
    case 10:
      lcd_display(0,0,"Setting system","  ");
      lcd_display(0,1,"to Manual/Auto!"," ");
      delay(2000);
      set_relayPin();
      lastButtonPushed=1;
      //menu_display=last_menu;     
      break;  

    case 11:
        
      lcd.setCursor(0,0);
      lcd.print("R1    = ");
      lcd.print(R1,1);
      lcd.setCursor(0,1);
      lcd.print("R2      = ");
      lcd.print(R2,1);
      menu_display = 11; 
      last_menu = menu_display;

      delay(150);   
      break;    
    case 12:
      lastButtonPushed=2;
      menu_display = 12;      
      last_menu = menu_display;    
      delay(150);
      set_menu_R1();
      break;

    
    case 14:
      lcd.clear();  
      lcd.setCursor(0,0);
      lcd.print("Firmware version");
      lcd.setCursor(0,1);
      lcd.print(sw_version,1);
      menu_display = 14; 
      last_menu = menu_display;

      delay(150);   
      break;       
   }
      
   //lastButtonPushed=1; //reset the lastButtonPushed variable
  }  

  void loop() {
   while (lastButtonPushed == 1)  {
    read_analog_in();
    default_display();
    manage_bat();
    read_buttons();    
   }
   read_buttons();
   main_menu();
  }


