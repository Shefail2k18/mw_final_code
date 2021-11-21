#include <SoftwareSerial.h>
#include <Keypad.h>

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {11, 10, 9, 8}; 
byte colPins[COLS] = {7, 6, 5, 4};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(3, 2); //SIM800L Tx & Rx is connected to Arduino #3 & #2

String number="";
String msg="";
String instr="";
String str_sms="";
String str1="";
int ring=0;
int prevring=0;
int i=0,temp=0;
int sms_flag=0;
char sms_num[3];
int rec_read=0;
int temp1=0;
int callPin=13;

void message(){
  Serial.println("---------------------------------------------------------------------");
  Serial.println("Press '1' to Call");
  Serial.println("Press '2' to send message");
}

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Configuring Module.Please wait...");
  boolean at_flag=1;
  while(at_flag)
  {
    mySerial.println("AT");
    while(mySerial.available()>0)
    {
      if(mySerial.find("OK"))
      at_flag=0;
    }
    
    delay(1000);
  }

  delay(100);


  mySerial.println("AT"); //handshake
  delay(1000);
  mySerial.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  delay(1000);
  mySerial.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  delay(1000);
  mySerial.println("AT+CREG?"); //Check whether it has registered in the network
  Serial.println("Almost done..."); 
  delay(1000);
  
  mySerial.println("AT+CNMI=1,2,0,0,0");   //sim receiving mode 
  Serial.println("\nConnected Successfully\n");

  message();

  pinMode(callPin,OUTPUT);
}


void loop()
{
  if(mySerial.available()){
    while(mySerial.available()){
      char ch=mySerial.read();
      instr+=ch;
      i++;
      if(instr[i-4] == 'R' && instr[i-3] == 'I' && instr[i-2] == 'N' && instr[i-1] == 'G' )
      {
          ring=1;
      }
  
      if(instr.indexOf("NO CARRIER")>=0)
      {
         ring=0;
         i=0;
      }
      if(instr.indexOf("+CMT: ")>=0)
      {
        sms_flag=1;
      }
    }
  }
  

  if(prevring != ring){
    if(ring==1){
      digitalWrite(callPin, HIGH);
      Serial.println("\nCall Received");
      Serial.println("A: pick up     B: hang up");
      char customKey = NULL;
      while(!customKey){
        customKey = customKeypad.getKey();
      }
      while(1){
        if(customKey=='A') {
          mySerial.println("ATA");
          Serial.println("Call Connected");
          Serial.println("---------------------------------------------------------------------");
          Serial.println("C : End Call");
          Serial.println("D : Exit");
          
          char customKey = NULL;
          while(!customKey){
            customKey = customKeypad.getKey();
          }
          while(1){
            if(customKey=='C'){
              mySerial.println("ATH");
              Serial.println("Ended");
              break;
            }
            else if(customKey=='D') break;
          }
          break;
        }
        else if(customKey=='B') {
          mySerial.println("ATH");
          Serial.println("Hanging up");
          break;
        }
        customKey = customKeypad.getKey();
      }
    }
    else{
      digitalWrite(callPin, LOW);
      Serial.println("Call ended");
    }
    message();
  }
  prevring=ring;

  
  if(sms_flag==1){
    Serial.println("\nNew Message Received");
    int ind=instr.indexOf("+CMT: ");
    ind+=7;
    sms_flag=0;
    
    Serial.print("Message from : ");
    for(int k=ind;k<ind+13;k++){
        Serial.print(instr[k]);
    }
    Serial.print("\nDate         : ");
    for(int k=ind+19;k<ind+36;k++){
        Serial.print(instr[k]);
    }
    Serial.print("\nContent      : ");

    for(int k=ind+42;k<instr.length();k++){
        Serial.print(instr[k]);
    }
    Serial.println();

    message();
    
    delay(1000);
    instr="";
    rec_read=1;
    temp1=1;
    i=0;
  }

  char key=customKeypad.getKey();
  if (key){
    Serial.println(key);
    
    if(key=='1'){
      call();
    }
    else if(key=='2'){
      sms();
    }
    
  }





  delay(100);
  
}

void call(){
  Serial.println("Calling, enter Phone number :");
  Serial.println("'C' : Call     '*' : Cancel");
  number="+91";
  while(1){
    char key=customKeypad.getKey();
    if(!key) continue;
    
    if(key=='C'){
      delay(300);
      Serial.println("\nCalling...");
      mySerial.print("ATD");
      mySerial.print(number);
      mySerial.println(";");

      int ans=1;
        while(ans==1)
        { 
          while(mySerial.available()>0)
          {
            if(mySerial.find("OK"))
            {
              Serial.println("Press 'D' to disconnect");
              int l=0;
              str1="";
              while(ans==1)
              {
                while(mySerial.available()>0)
                {
                  char ch=mySerial.read();
                  str1+=ch;
                  if(str1.indexOf("NO CARRIER")>0)
                  {
                    Serial.println("Call Ended\n");
                    ans=0;
                    message();
                    return;
                  }
                 }
                  char key=customKeypad.getKey();
                   if(key == 'D')
                  {
                    Serial.println("Call Ended");
                    mySerial.println("ATH");
                    delay(2000);
                    ans=0;
                    message();
                    return;
                  }
                   if(ans==0)
                   break;
                }
             }  
           }
        } 
      
    }
    else if(key=='*') {
      Serial.println("cancelled\n");
      message();
      return;
    }
    else{
      number+=key;
      Serial.print(key);
    }
  }
}

void sms(){
  //get number
  String num="+91";
  Serial.println("Enter receiver's number\n'*' : cancel   'C' : next");
  while(1){
    char key=customKeypad.getKey();
    if(!key) continue;
    
    if(key=='*'){
      Serial.println("Cancelled\n");
      message();
      return;
    }
    else if(key=='C'){
      break;
    }
    else{
      num+=key;
      Serial.print(key);
    }
  }

  //taking mssg input 
  Serial.println("\nEnter message content : (ends with *)");
  String mssg="";
  while(1){
    delay(10);
    if(!Serial.available()) continue;
    
    char ch = Serial.read();
    if(ch=='*') break;
    else {
      mssg+=ch;
    }
  }
  Serial.print("\nSending.");
  
  mySerial.println("AT+CMGF=1"); 
  delay(2000);
  Serial.print("..");
  mySerial.println("AT+CMGS=\""+num+"\"");
  delay(500);
  Serial.print(".");

  mySerial.print(mssg);
  Serial.print("...");
  mySerial.write(26);
  Serial.println("\nSent\n");
  mySerial.println("AT+CMGF=0"); 
  mySerial.println("AT+CNMI=1,2,0,0,0");
  message();
  return;
}

void updateSerial()
{
  delay(1000);
  if(Serial.available()){
    while (Serial.available()) 
    {
      mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
    }
    while(mySerial.available()) 
    {
      Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
    }
  }
}
