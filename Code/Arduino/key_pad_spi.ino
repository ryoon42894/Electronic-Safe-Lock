// include the library code:
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Servo.h>
/*Poles 'dav' signal, which comes from the FPGA slave. Once the active high 'dav' signal assertion 
 * is detected, Arduino master will decode the binary key press value, 'key_digit' with the respective char value into 'key_code[5]'
 * corresponding to the 4x4 matrix below:
 *        1 |  2  |  3  | A
 *        4 |  5  |  6  | B
 *        7 |  8  |  9  | C
 *      CLR |  0  | ENT | D
 * Depending on the state machine, it'll go through the different states:
 * STATE 0 : Set's the 5 pin passcode
 * STATE 1 : Verifies the passcode by prompting user to re-enter passcode entered
 * STATE 2 : Main Menu where user can select to unlock or change passcode
 * STATE 3 : User has 3 guesses to enter the correct passcode for both unlock and change passcode
 * STATE 4 : Lock is open and pressing any guy will re-engage the lock. 
*/ 

//variables for pins
const int rs = 3, en = 4, d4 = 5, d5 = 6, d6 = 7, d7 = 8; //LCD pin #'s
const int dav = 9, lock = 11, alarm = 12, slavepin = 13;
int lcd_indicator = 5, i = 0, STATE = 0, guesses = 3, UnlockorChange = 0, alarm_on_off = 0;//i is the index of arrays, UnlockorChange: 0 to unlock, 1 to change passcode 
byte key_digit;                               //variable for the received raw value of key press from FPGA slave
char key_code[5], guess_code[5], options[2];;//user's passcode is key_code, entered guesses are stored in guess_code, entered choices for unlocking or change passcode is options
Servo servo_lock;                           //declare servo object 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);  //declare lcd object

//initializes the various I/O's
void setup() {  
  lcd.begin(20, 4);                 //sets lcd to be 4 rows (0-3), 20 columns (0-19)
  pinMode(dav, INPUT_PULLUP);       //sets 'dav' pin 9 to be input with internal pull up resistors enabled
  
  lcd.clear();                      //clears lcd screen
  lcd.print(" Set 5-pin passcode ");//display's "Set 5-pin passcode"
  lcd.setCursor(lcd_indicator,1);   //sets cursor on row 1, column

  //pinMode(lock, OUTPUT);
  //digitalWrite(lock, LOW);
  servo_lock.attach(11);            //sets servo control pin to 11
  servo_lock.write(90);             //sets initial (locked) value of, servo angle to 90 deg
  
  pinMode(alarm, OUTPUT);           //sets alarm (buzzer) pin 12 as output
  digitalWrite(alarm, LOW);         //outputs initial value of (off), 0 to alarm
  pinMode(slavepin, OUTPUT);        //sets slavepin (of SPI) pin 13 as output
  digitalWrite(slavepin, HIGH);     //outputs initial value HIGH to slavepin
  SPI.begin();                      //initializes SPI in arduino from SPI library
  pinMode(SS, OUTPUT);              //sets SS from SPI as OUTPUT so that the arduino is in Master mode
}
//poles 'dav', then encodes the received value from FPGA, then goes into the respective STATE
void loop() {
  if (digitalRead(dav)){                    //wait for 'dav' to go HIGH             
    SPI.beginTransaction(SPISettings(1000000,MSBFIRST,SPI_MODE0));//once active high 'dav' is asserted, begin SPI with 1 MHz speed, MSB received first, mode0 
    digitalWrite(slavepin, LOW);            //pull slavepin low to begin SPI transmission
    key_digit = SPI.transfer(255);          //received value (MISO) is stored into 'key_digit'
    digitalWrite(slavepin, HIGH);           //pull slavepin high
    SPI.endTransaction();                   //to end SPI transmission
    //delayMicroseconds(5);
    if (key_digit == 12 && STATE != 4){  //if 'key_digit' is 12, then user wants to clear their inputted digits, STATE4 is excluded b/c STATE4 is when LOCK is open, and any key press including clear will re-engage the lock
      i = 0;                             //reset i to 0
      if (STATE < 2){                    //if STATE0 or STATE1, then..
        lcd_indicator = 5;               //reset lcd_indicator back to 5
        lcd.setCursor(0,1);              //set cursor on lcd to row 1, column 0
        lcd.print("                    ");//'clear' row 1 by printing out spaces
        lcd.setCursor(lcd_indicator,1);  //set cursor on lcd to row 1, column 5
        if (STATE == 0){                 //if STATE0
          memset(key_code, '\0', sizeof(key_code));//reset all elements in key_code array
        }
        else if (STATE == 1){            //if STATE1
          memset(guess_code, '\0', sizeof(guess_code));//reset all elements in guess_code array
        }
      }
      else if (STATE == 2){              //if in STATE2 and 'clear' was pressed...
        i = 0;                         //set i to 0
        lcd_indicator = 9;             //set lcd_indicator to 9
        if (UnlockorChange == 0){        //if UnlockorChange is 0 then...
          lcd.setCursor(0,3);            //set cursor on lcd to row 0, column 3
          lcd.print("                    "); //'clear' row 3 by printing spaces
          lcd.setCursor(9,3);            //set cursor on lcd to row 3, column 9
          memset(guess_code, '\0', sizeof(guess_code));//reset all elements in guess_code array
        }
        else if (UnlockorChange == 1){  //if UnlockorChange is 1 then...
          lcd.setCursor(0,2);           //set cursor on lcd to row 0, column 2
          lcd.print("                    ");//'clear' row 2 by printing spaces
          lcd.setCursor(9,2);               //set cursor on lcd to row 2, column 9
          memset(options, '\0', sizeof(options));//reset all elements in options array           
        }
      }
      else if (STATE == 3){                //if STATE3 then...
          i = 0;                           //set i to 0
          lcd_indicator = 5;               //set lcd_indicator to 5
          lcd.setCursor(0,3);              //set cursor on lcd to row 3, column 0
          lcd.print("                    ");//'clear' row 3 by printing spaces
          lcd.setCursor(5,3);              //set cursor on lcd to row 3, column 5
          memset(guess_code, '\0', sizeof(guess_code));//reset all elements in guess_code array  
      }      
    }
    else if (STATE == 0){               //if 'key_digit' is not clear, and state is in STATE0 then...
      key_code[i] = Decoder(key_digit); //start and set the passcode by storing the user inputted 5 digits onto key_code array     
      Enter_KeyCode();                  //using Enter_KeyCode() function to display the digits the user inputted
    }
    else if (STATE == 1){                //if 'key_digit' is not clear, and state is in STATE1 then...
      guess_code[i] = Decoder(key_digit);//store the verifying passcode in guess_code array
      Check_Entering_KeyCode_1();        //using Enter_KeyCode() function verify if passcodes match, also display the current inputs
    }
    else if (STATE == 2){   //if 'key_digit' is not clear and state is in STATE2, then display Main Menu, awaiting user either to unlock or change passcode
     if (UnlockorChange == 0){//if UnlockorChange is 0 then user wants to unlock
      guess_code[i] = Decoder(key_digit);//store user inputs into guess_code array
     }
     else if (UnlockorChange == 1){//Confirm the prior decision//if UnlockorChange is 1 then user wants to change passcode
      options[i] = Decoder(key_digit);  //store user inputs into options array
     }
     Check_User_Options();  //using Check_User_Options() function and depending on UnlockorChange, verify user's prior decision
  }                                                                                                                     
  else if (STATE == 3){                 //if 'key_digit' is not clear and state is in STATE3, then...
    guess_code[i] = Decoder(key_digit); //store user inputs of passcode into guess_code array
    Check_Entering_KeyCode_2();         //verify if passcode is correct, this applies to both unlocking or changing passcode
  }
  else if (STATE == 4){                 //if in STATE4, lock is open and any user key press re-engages the lock
    STATE = 2;                          //go back to STATE2, 'Main Menu'
    //digitalWrite(lock, HIGH); 
    servo_lock.write(90);               //re-engage the lock, servo angle 90 deg
    lcd.clear();                        //clear lcd screen
    lcd.print("    Safe  locked    ");  //output "Safe locked" on row 0
    lcd.setCursor(0,1);                 //set cursor on lcd to row 1
    lcd.print("A to unlock         ");  //out "A to unlock" on row 1
    lcd.setCursor(0,2);                 //set cursor on lcd to row 2
    lcd.print("B to change passcode");  //out "B to change passcode" on row 2
    lcd.setCursor(9,3);                 //set cursor on row 3, column 9
    i = 0;                              //set i to 0
    lcd_indicator = 9;                  //set lcd_indicator to 9
  }
}
}
//function that decodes the received key pressed value into a char
char Decoder(byte key_digit){
  switch (key_digit) {
    case 0 :    //if received key press is 4'b0000
      return 49;//then, key press is 1
    case 1 :    //if received key press is 4'b0001
      return 50;//then, key press is 2
    case 2 :    //if received key press is 4'b0010
      return 51;//then, key press is 3
    case 3 :    //if received key press is 4'b0011
      return 'A';//then, key press is A
    case 4 :    //if received key press is 4'b0100
      return 52;//then, key press is 4
    case 5 :    //if received key press is 4'b0101
      return 53;//then, key press is 5
    case 6 :    //if received key press is 4'b0110
      return 54;//then, key press is 6
    case 7 :    //if received key press is 4'b0111
      return 'B';//then, key press is B
    case 8 :    //if received key press is 4'b1000
      return 55;//then, key press is 7
    case 9 :    //if received key press is 4'b1001
      return 56;//then, key press is 8
    case 10 :   //if received key press is 4'b1010
      return 57;//then, key press is 9
    case 11 :   //if received key press is 4'b1011
      return 'C';//then, key press is C
    case 12 :   //if received key press is 4'b1100
      return 58;  //then, key press is CLEAR, but this is not used here as it is checked when the STATE value is checked
    case 13 :   //if received key press is 4'b1101
      return 48; //then, key press is 0
    case 14 :   //if received key press is 4'b1110
      return 16;  //then, key press is ENTER
    case 15:    //if received key press is 4'b1111
      return 'D';  //then, key press is D
    default:    //if the received key press is none of the above
      return 32;  //then, key press is blank
  }
}
//function used in STATE0 that user sets passcode, this is stored in key_code array, also displays the user inputs
void Enter_KeyCode(){
  if (i < 5){
    if (key_code[i] == 16){ //if less than 5 digits were inputted and then 'Enter'ed,
      lcd.setCursor(0,1);   //notify user to enter a 5-pin passcode. 
      lcd.print("    Please enter    "); //print "Please enter" on row 1
      lcd.setCursor(0,2);
      lcd.print("      5-digits      "); //print "5-digits" on row 2
      delay(2000);                       //hold this message for 2 seconds

      lcd.setCursor(0,1);                //then clear the message by printing spaces
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("                    ");

      lcd_indicator = 5;      //set lcd_indicator to 5
      for (int j=0;j<i;j++){  //display the less than 5 pin passcode user inputted by going through the array
        lcd.setCursor(lcd_indicator,1); 
        lcd.print(key_code[j]);
        lcd_indicator+=2;     //print the digits of the passcode on every other block starting at row 1, column 5
      }
    }
    else{                               //if less than 5-pin code was inputted, but not 'Enter'ed
        lcd.setCursor(lcd_indicator,1); //then display the passcode on 2nd row of LCD, but only a maximum of 5 digits
        lcd.print(key_code[i]);
        i++;                  //increment i so that the next input key press is stored onto the next index of the key_code array
        lcd_indicator+=2;     //print the digits of the passcode on every other block starting at row 1, column 5
    }
 }
 else if (i == 5){          //if 5-pins were inputted AND 'Enter'ed,
  if (key_code[i] == 16) {  //then move onto STATE 1, to have the user confirm the 'Enter'ed passcode
    STATE = 1;              
    lcd.clear();            //clear lcd to print a new menu screen that say prompts user to "Re-enter 5 pin"
    lcd.setCursor(0,0);     
    lcd.print("   Re-enter 5 pin   ");    
    i = 0;                  //set i back to 0
    lcd_indicator = 5;      //set lcd_indicator back to 5
    lcd.setCursor(lcd_indicator,1);//set lcd cursor to row 1, column 5
  }
 }
}
//function used in STATE1 where user re-enters 5-pin passcode to verify if they match, re-entered passcode stored in guess_code array, also prints the digits
void Check_Entering_KeyCode_1(){
  if (guess_code[i] == 16){                       //if 'Enter'ed, then check 
    for (int k=0;k<5;k++){                        //to see if the 'Enter'ed code matches
      if ((guess_code[k] != key_code[k]) || i < 5){ //if re-entered passcode does not match previous entered passcode OR re-entered passcode is less than 5-pins then notify user
        //delay(2000);                            //and go back to State 0.
        lcd.clear();                      //clear lcd screen
        lcd.print("   5-pin passcode   ");//print "5-pin passcode" on row 0
        lcd.setCursor(0,1);
        lcd.print("   did not match    ");//print "did not match" on row 1

        delay(2000);                      //hold message for 2 seconds
        
        lcd.clear();                      //clear lcd screen
        lcd.print(" Set 5-pin passcode ");//print "Set 5-pin passcode" on row 0
        i = 0;                            //reset i to 0
        lcd_indicator = 5;                //reset lcd_indicator to 5
        lcd.setCursor(lcd_indicator,1);   //set cursor on lcd to row 1, column 5
        STATE = 0;
        memset (key_code, '\0', sizeof(key_code));//reset all elements in key_code array
        memset (guess_code, '\0', sizeof(guess_code));//reset all elements in guess_code array
        return;                           //'break'/end the function
      }
    }
      //else if (i == 5){                //if the 'Enter'ed 5-pin passcode matches, notify user 
        lcd.clear();                      //and then move onto State 2
        lcd.print("    Safe  locked    ");//print "Safe locked" on row 0
        lcd.setCursor(0,1);               
        lcd.print("A to unlock         ");//print "A to unlock" on row 1
        lcd.setCursor(0,2);
        lcd.print("B to change passcode");//print "B to change passcode" on row 2
        lcd.setCursor(9,3);               //set cursor on lcd to row 3, column 9
        i = 0;                            //reset i to 0
        lcd_indicator = 9;                //set lcd_indicator to 9
        STATE = 2;                        //move onto STATE2
        memset (guess_code, '\0', sizeof(guess_code));//reset all elements in guess_code array
      //}
   
  }
  else if (i < 5){      //if less than 5-pins inputted but NOT 'Enter'ed, 
      lcd.setCursor(lcd_indicator,1); //then display them on 2nd row of LCD
      lcd.print(guess_code[i]);
      i++;
      lcd_indicator+=2;      
  }
}
//Function used in STATE2 to allow user to either unlock or change passcode ('A' to unlock, 'B' to change passcode and then verify their answer, '1' for Yes, '2' for No
void Check_User_Options(){
  if (i == 1){                                        //if i is 1, then need to check index 0 of guess_code
    if (guess_code[i] == 16 && UnlockorChange == 0){  //guess_code[1] is 'Enter' and UnlockorChange = 0, then User entered an option to either unlock or change passcode
      i = 0;                                          //reset i to 0
      if (guess_code[0] == 'A'){                      //if guess_code[0] is 'A', then user wants to unlock
        lcd.clear();                                  //clear screen and prompt user to confirm their decision
        lcd.print("      Unlock?       ");            //'1' for Yes, '2' for No
        lcd.setCursor(0,1);
        lcd.print("1 for Yes, 2 for No ");
        lcd.setCursor(9,2);
        UnlockorChange = 1;                           //set UnlockorChange to 1   
      }
      else if (guess_code[0] == 'B'){                 //if guess_code[0] is 'B', then user wants to change passcode
        lcd.clear();                                  //clear screen and prompt user to confirm their decision
        lcd.print("  Change passcode?  ");            //'1' for Yes, '2' for No
        lcd.setCursor(0,1);
        lcd.print("1 for Yes, 2 for No ");
        lcd.setCursor(9,2);
        UnlockorChange = 1;
      }
      else{                                           //if guess_code[0] is neither 'A' or 'B', then user entered an invalid input
        lcd.setCursor(0,3);                           //notify the user of the invalid input and keep the message up for 2 seconds
        lcd.print("  Invalid Response  ");

        delay(2000);

        lcd.setCursor(0,3);                           //clear row3, column0 and set cursor on lcd to row3, column9
        lcd.print("                    ");
        lcd.setCursor(9,3); 
        lcd_indicator = 9;
        UnlockorChange = 0;  
        memset (guess_code, '\0', sizeof(guess_code));//clear all elements in guess_code
      }
    }
    else if (options[i]== 16 && UnlockorChange == 1){//if options[1] is 'Enter' and UnlockorChange = 1, then verify if user wants to unlock or change passcode
      if (options[0] == 50){                         //if options[0] is 50, User picked 'NO'
        lcd.clear();                                 //clear lcd, and display Main menu
        lcd.print("    Safe  locked    ");           //prompting user to either Unlock or Change PW
        lcd.setCursor(0,1);
        lcd.print("A to unlock         ");
        lcd.setCursor(0,2);
        lcd.print("B to change passcode");
        i = 0;
        lcd_indicator = 9;
        lcd.setCursor(lcd_indicator,3);
        UnlockorChange = 0;                           //reset UnlockorChange back to 0
        memset (guess_code, '\0', sizeof(guess_code));//clear all elements in guess_code array
        memset (options, '\0', sizeof(options));      //clear all elements in options array
      }
      else if (options[0] == 49){  //if options[0] is 49, User picked 'Yes', check what the YES is to, Unlock or Change PW
        lcd.clear();               //clear lcd screen
        i = 0;                     //reset i back to 0
        lcd_indicator = 5;         //set lcd_indicator to 5
        STATE = 3;                 //set STATE to STATE3
        memset (options, '\0', sizeof(options));               
        if (guess_code[0] == 'A'){  //if previous input, guess_code[0], which was stored in guess_code array is 'A', then user confirms 'YES' to Unlock
          lcd.print("Enter 5-pin passcode");//prompt user to enter the 5 pin passcode while also displaying the 3 guess attempts they have to do so
          lcd.setCursor(0,1);
          lcd.print("     to unlock      ");
          lcd.setCursor(0,2);
          lcd.print("  "+ String(guesses) + " attempts remain ");
          lcd.setCursor(lcd_indicator,3);     
          UnlockorChange = 3;                          //set UnlockorChange to 3
          memset(guess_code, '\0', sizeof(guess_code));//clear all elements in guess_code array                          
        }
        else if (guess_code[0] == 'B'){ //if previous input, guess_code[0], which was stored in guess_code array is 'B', then user confirms 'YES' to change passcode
          lcd.print(" Enter current code ");//prompt user to enter the 5 pin passcode, but also an option to cancel the change passcode process
          lcd.setCursor(0,1);               //also display the 3 guess attempts they have to do so
          lcd.print("    C to cancel     ");
          lcd.setCursor(0,2);
          lcd.print("  "+ String(guesses) + " attempts remain ");         
          lcd.setCursor(lcd_indicator,3);
          UnlockorChange = 4;                           //set UnlockorChange to 4
          memset(guess_code, '\0', sizeof(guess_code)); //clear all elements in guess_code array                       
        }
      }
      else{                                   //if options[0] is neither '1' or '2', then user entered an invalid input
          lcd.setCursor(0,3);                 //notify the user of the invalid input and keep the message up for 2 
          lcd.print("  Invalid response  ");

          delay (2000);
          
          lcd.setCursor(0,3);                 //clear row3, column0 and set cursor on lcd to row3, column9
          lcd.print("                    ");
          i = 0;
          lcd_indicator = 9;
          lcd.setCursor(lcd_indicator,2);
          memset(options, '\0', sizeof(options));//clear all elements in options array
      }              
    }
  }
  else{                                             //if i is not 1
   if (UnlockorChange == 0 && guess_code[i] != 16){ //if UnlockorChange is 0, guess_code is NOT 'Enter'
    lcd.print(guess_code[i]);                       //then print user input onto lcd
    i++;
    lcd_indicator+=2;     
   }
   else if (UnlockorChange == 1 && options[i]!= 16){//if UnlockorChange is 1, options is NOT 'Enter'
    lcd.print(options[i]);                          //then print user input onto lcd
    i++;
    lcd_indicator+=2;         
   }
  }  
}

//Function used to check the passcode entered by the user, if they match, it will go into the respective path, either to unlock or change passcode, if they don't match, the alarm is activated
void Check_Entering_KeyCode_2(){
  if (guess_code[i] == 16){           //if guess_code is 'Enter'
    for (int k=0;k<5;k++){            //go through each element in guess_code with a for loop
      if (guess_code[0] == 'C' && UnlockorChange == 4 && i == 1){ //if UnlockorChange is 4, which means its in the change passcode process, guess_code[0] is C, and i is 1, then user wants to cancel
        lcd.clear();                                              //the change passcode process, clear screen and display Main Menu, prompting user to unlock or change passcode
        lcd.print("    Safe  locked    ");                        
        lcd.setCursor(0,1);
        lcd.print("A to unlock         ");
        lcd.setCursor(0,2);
        lcd.print("B to change passcode");
        lcd.setCursor(9,3);
        i = 0;
        lcd_indicator = 9;
        UnlockorChange = 0;   
        STATE = 2;                                                //set STATE back to STATE2
        memset (guess_code, '\0', sizeof(guess_code));            //clear all elements in guess_code
        return;                                                   //end/'break' this function
      }
      else if ((guess_code[k] != key_code[k]) || i < 5){  //if the user inputted passcode does not match the passcode set from before OR less than 5 digits were entered..
        if (guesses > 1){                                 //if guesses are greater than 1, then display and decrement the guess attempts
          guesses--;
          lcd.setCursor(0,3);
          lcd.print("                    ");
          lcd.setCursor(0,2);
          lcd.print(" " + String(guesses) + " attempts remain  ");
          i = 0;
          lcd_indicator = 5;
          lcd.setCursor(lcd_indicator,3);
          memset(guess_code, '\0', sizeof(guess_code)); //clear all elements in guess_code array
          return;                                       //end/'break' function
        }
        else{                                           //if guesses is not greater than 1, than user has used all 3 attempts...
          digitalWrite(alarm, HIGH);                    //activate the active high alarm
          i = 0;
          lcd_indicator = 5;
          memset(guess_code, '\0', sizeof(guess_code)); //clear all elements in guess_code array        
          if (alarm_on_off == 0){                       //if alarm_on_off is 0 then display that the passcode entered was incorrect and alarm has been activated
            lcd.clear();                                
            lcd.print(" INCORRECT PASSCODE ");
            lcd.setCursor(0,1);
            lcd.print("  ALARM  ACTIVATED  ");
            lcd.setCursor(0,2);
            lcd.print(" INTRUDER  DETECTED ");
            alarm_on_off = 1;                           //set alarm_on_off to 1 so that, when the user inputs the passcode again, the menu keeps the texts
            lcd.setCursor(lcd_indicator,3);
          }
          else if (alarm_on_off == 1){                  //if alarm_on_off is 1, then...
            lcd.setCursor(0,3);                         //just clear row3, and set cursor on lcd back to row5, column5
            lcd.print("                    "); 
            lcd.setCursor(lcd_indicator,3);               
          }
          return;                                       //end/'break' function
       }
    }
    }
    //else if (k == 5){
      lcd.clear();          //if all elements of the user inputted passcode from guess_code matches set passcode from key_code then...
      guesses = 3;          //reset guesses back to 3
      i = 0;                //reset i back to 0
      lcd_indicator = 5;
      alarm_on_off = 0;     //reset alarm_on_or_off to 0
      digitalWrite(alarm, LOW);
      if (UnlockorChange == 3){//if in UnlockorChange3, then the lock is opened now
        //digitalWrite(lock, HIGH);
        servo_lock.write(165);  //set servo angle to open, 165 deg
        lcd.print("     LOCK  OPEN     ");//display that the lock is open and to re-engage the lock user has to press any button
        lcd.setCursor(0,1);
        lcd.print("  Press any key to  ");
        lcd.setCursor(0,2);
        lcd.print("  engage the lock   ");
        lcd.setCursor(lcd_indicator,2);
        STATE = 4;                      //set to STATE4 to wait for user to re-engage lock
        UnlockorChange = 0;             //reset UnlockorChange back to 0
        memset(guess_code, '\0', sizeof(guess_code));   //reset all elements in guess_code back to 0
      }
      else if (UnlockorChange == 4){      //if in UnlockorChange4, then the user can now change passcode
        lcd.print(" Enter new passcode ");//prompt user to enter a new passcode
        STATE = 0;                        //set STATE to STATE0
        UnlockorChange = 0;               //reset UnlockorChange back to 0
        lcd.setCursor(0,2);        
        memset(guess_code, '\0', sizeof(guess_code)); //clear all elements in guess_code array   
      }
    //}
    }  
  else if (i < 5){                    //if i is less than 5 and none of them is 'Enter' then...
      lcd.setCursor(lcd_indicator,3); //display the user inputted digits on lcd
      lcd.print(guess_code[i]);
      i++;
      lcd_indicator+=2;      
  }  
}
