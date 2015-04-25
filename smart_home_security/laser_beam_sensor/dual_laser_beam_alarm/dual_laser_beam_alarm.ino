#define rxPin     0
#define txPin     1

#define laserPin  2
#define redPin    3
#define greenPin  4

#define laserPin_b 5
#define redPin_b   6
#define greenPin_b 7


#define laserPin_c 8
#define greenPin_c 9
#define redPin_c   10

#define alarmLightPin 11
#define alarmSoundPin 12

#define blinkPin 13

int debugMode = 1;

long previousLaserDuration = 0;
int firstLoop = 1;
long lastGreenMillisStamp = 0;
int photoCellInput = 0;
int previousPhotoCellInput = 0;
int photoCellDiff = 0;

int STATUS_STEADY_BEAM = 0; /*Beam detected steadily for 1 second or more*/
int STATUS_UNSTABLE_BEAM = 1; /*Beam detected for less than one second*/
int STATUS_ABSENT_BEAM = 2; /*Beam not detected for 1 second or more*/
int currentSensorStatus = 0;
int currentSensorStatus_b = 0;

long steadyBeamInitMillis = 0;
long absentBeamInitMillis = 0;

long steadyBeamInitMillis_b = 0;
long absentBeamInitMillis_b = 0;

long readings_Count = 0;
long readingsInSpec_Count = 0;


long readings_Count_b = 0;
long readingsInSpec_Count_b = 0;

long laserDurationSpec_min = 1000;
long laserDurationSpec_max = 4000;
long alarmMinTime = 30000;

String sensorStatus = "01-UNSTABLE_GREEN"; /*01-UNSTABLE_GREEN, 02-STABLE_GREEN, 03-RED*/



void setup() {
  
  
    Serial.begin (9600);
    
 
    
    sendDataToPVCloud("SKETCH_FLOW", "LASER BEAM SENSOR STARTED");
    if(debugMode==1) Serial.println("Initiating Sketch...");
    if(debugMode==1) Serial.println("Setting Pin Modes...");
    setPinModes();
    
    if(debugMode==1) Serial.println("Init Signal...");
    initSignal();

    if(debugMode==1) Serial.println("Getting PIN13 Value from PVCLOUD");    
    
    String pin13Command = getPin13CommandFromPVCloud();
    if(pin13Command == "HIGH"){
      digitalWrite(blinkPin, HIGH);
    } else if (pin13Command == "LOW") {
      digitalWrite(blinkPin, LOW);
    }  
  
    if(debugMode==1) Serial.println("PIN13 Value retrieval complete");      
    
    String thisIsGood = readFileValue();
    Serial.println("THE STRING IS: ");
    Serial.println(thisIsGood);  
    if(debugMode==1) Serial.println("Setup Complete!");
}

void loop() {  
return;  
    if(firstLoop == 1 && debugMode==1) Serial.print("Beginning Loop...");
    if(debugMode==1) Serial.println(micros());
    
    determineSensorStatus(); setSensorStatusPins();
    
    determineSensorStatus_b(); setSensorStatusPins_b();    
    
    setAlarmMode();
      
    if(firstLoop == 1 && debugMode == 1) {
      Serial.println("First Loop Complete!");
      firstLoop = 0;
    }    
}

int determineSensorStatus(){
    long laserDuration;
    
    laserDuration = getLaserDurationFiveInARow();    
    if(debugMode==1) Serial.print("Laser Duration: ");
    if(debugMode==1) Serial.println(laserDuration);
    
    if(isLaserDurationWithinSpec(laserDuration)){   
      absentBeamInitMillis = 0;
      if(steadyBeamInitMillis==0) steadyBeamInitMillis = millis();
      else if(millis()-steadyBeamInitMillis > 1000) {
        //BEAM IS STEADY
        currentSensorStatus =  STATUS_STEADY_BEAM;
      }
       
    } else {
      //BEAM IS NOT STEADY
      steadyBeamInitMillis = 0;
      currentSensorStatus = STATUS_UNSTABLE_BEAM;
      if(absentBeamInitMillis==0) absentBeamInitMillis = millis();
      else if (millis()-absentBeamInitMillis > 1000){
        currentSensorStatus =  STATUS_ABSENT_BEAM;
      } 
    } 
    
    return currentSensorStatus;
}

int determineSensorStatus_b(){
    long laserDuration;
    
    laserDuration = getLaserDurationFiveInARow_b();    
    if(debugMode==1) Serial.print("Laser Duration (B): ");
    if(debugMode==1) Serial.println(laserDuration);
    
    if(isLaserDurationWithinSpec(laserDuration)){   
      absentBeamInitMillis_b = 0;
      if(steadyBeamInitMillis_b==0) steadyBeamInitMillis_b = millis();
      else if(millis()-steadyBeamInitMillis_b > 1000) {
        //BEAM IS STEADY
        currentSensorStatus_b =  STATUS_STEADY_BEAM;
      }
       
    } else {
      //BEAM IS NOT STEADY
      steadyBeamInitMillis_b = 0;
      currentSensorStatus_b = STATUS_UNSTABLE_BEAM;
      if(absentBeamInitMillis_b==0) absentBeamInitMillis_b = millis();
      else if (millis()-absentBeamInitMillis_b > 1000){
        currentSensorStatus_b =  STATUS_ABSENT_BEAM;
      } 
    } 
    
    return currentSensorStatus_b;
}

void setSensorStatusPins(){
    switch(currentSensorStatus){
       case 0: //STATUS_STEADY_BEAM
          setGreenLight_ON();
          setRedLight_OFF();      
          break;   
       case 1: //STATUS_UNSTABLE_BEAM
          setGreenLight_OFF();
          break;          
       case 2: //STATUS_ABSENT_BEAM
          setRedLight_ON();
          break;
    }
}

void setSensorStatusPins_b(){
    switch(currentSensorStatus_b){
       case 0: //STATUS_STEADY_BEAM
          setGreenLight_b_ON();
          setRedLight_b_OFF();      
          break;   
       case 1: //STATUS_UNSTABLE_BEAM
          setGreenLight_b_OFF();
          break;          
       case 2: //STATUS_ABSENT_BEAM
          setRedLight_b_ON();
          break;
    }
}

void setPinModes(){
    pinMode(laserPin, INPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    
    pinMode(laserPin_b, INPUT);
    pinMode(greenPin_b, OUTPUT);
    pinMode(redPin_b, OUTPUT);    
    
    pinMode(alarmLightPin, OUTPUT);    
    pinMode(alarmSoundPin, OUTPUT);    
    
   digitalWrite(alarmLightPin,LOW);
   digitalWrite(alarmSoundPin,LOW);       
}

String previousAlarmMode = "";
void setAlarmMode(){
 
   if(currentSensorStatus ==  STATUS_ABSENT_BEAM && currentSensorStatus_b == STATUS_ABSENT_BEAM){
       
       Serial.println("Alarm Condition Detected!");
       sendAlarmStatusChangeToPVCloud("ALARM");
       
       long alarmStopMillis = millis()+alarmMinTime/2;
       do{
         Serial.print("Current Millis:");
         Serial.println(millis());
         
         Serial.print("Alarm Stop Millis:");
         Serial.println(alarmStopMillis);         
         digitalWrite(alarmLightPin,HIGH);
         digitalWrite(alarmSoundPin,HIGH);
         delay(200);
         digitalWrite(alarmLightPin,LOW);
         delay(100);
       } while (millis()< alarmStopMillis);
       
       
   } else if((currentSensorStatus ==  STATUS_UNSTABLE_BEAM || currentSensorStatus ==  STATUS_ABSENT_BEAM) && (currentSensorStatus_b ==  STATUS_UNSTABLE_BEAM || currentSensorStatus_b ==  STATUS_ABSENT_BEAM) ){
      
       sendAlarmStatusChangeToPVCloud("WARNING");  
       
       digitalWrite(alarmLightPin,HIGH);
       digitalWrite(alarmSoundPin,HIGH);
       delay(50);
       digitalWrite(alarmSoundPin,LOW);
       
   } else {

       sendAlarmStatusChangeToPVCloud("STABLE"); 
       digitalWrite(alarmLightPin,LOW);
       digitalWrite(alarmSoundPin,LOW);     
   }
}
void initSignal(){
    for (int i=1; i<4; i++){
        if(debugMode==1) { Serial.print("Init Signal Count: ");}
        if(debugMode==1) Serial.println(i);
        digitalWrite(greenPin, HIGH);  
        digitalWrite(blinkPin, HIGH);  
        digitalWrite(redPin, HIGH);  
        digitalWrite(greenPin_b, HIGH);  
        digitalWrite(blinkPin, HIGH);  
        digitalWrite(redPin_b, HIGH);          
        delay(1000);
        
        digitalWrite(greenPin, LOW);
        digitalWrite(blinkPin, LOW);  
        digitalWrite(redPin, LOW);     
   
        digitalWrite(greenPin_b, LOW);
        digitalWrite(blinkPin, LOW);  
        digitalWrite(redPin_b, LOW);       
        delay(1000);
    }
}





long getLaserDurationFiveInARow(){
    int laserDurationInSpec_Count = 0;

    long measuredDuration = 0;
    long laserDuration = 0;
    
    for(int i=0; i<10; i++){
       readings_Count ++;
       laserDuration = pulseIn(laserPin, LOW, 5000);
       
       if(isLaserDurationWithinSpec(laserDuration)) {
         laserDurationInSpec_Count ++; 
         readingsInSpec_Count ++;
         measuredDuration = laserDuration;
       }
       
       if(debugMode==1) {
         Serial.print("Laser Duration # ");
         Serial.print(i);Serial.print(": ");
         Serial.println(laserDuration);
                
       }
    }
    
 
     if(debugMode==1) {Serial.print("Measured Duration: "); Serial.println(measuredDuration); } 
   
     if(readings_Count==1000) {
         Serial.println("--------------------------------------------------");
         Serial.println("--------------------------------------------------");
         Serial.println("--------------------------------------------------");
         Serial.print("----- AVERAGE SAMPLES IN SPEC: ");
         Serial.print(readingsInSpec_Count);
         Serial.print(" OF ");
         Serial.print(readings_Count);
         readings_Count=0;
         readingsInSpec_Count=0;
     }
     
     if(laserDurationInSpec_Count >= 3) {
        if(debugMode==1) Serial.println("Returned MD");
        return measuredDuration;
    } else {
        if(debugMode==1) Serial.println("Returned 0");
        return 0;
    }
}

long getLaserDurationFiveInARow_b(){
    int laserDurationInSpec_Count = 0;

    long measuredDuration = 0;
    long laserDuration = 0;
    
    for(int i=0; i<10; i++){
       readings_Count_b ++;
       laserDuration = pulseIn(laserPin_b, LOW, 5000);
       
       if(isLaserDurationWithinSpec(laserDuration)) {
         laserDurationInSpec_Count ++; 
         readingsInSpec_Count_b ++;
         measuredDuration = laserDuration;
       }
       
       if(debugMode==1) {
         Serial.print("Laser Duration # ");
         Serial.print(i);Serial.print(": ");
         Serial.println(laserDuration);
                
       }
    }
    
 
     if(debugMode==1) {Serial.print("Measured Duration: "); Serial.println(measuredDuration); } 
   
     if(readings_Count_b==1000) {
         Serial.println("--------------------------------------------------");
         Serial.println("--------------------------------------------------");
         Serial.println("--------------------------------------------------");
         Serial.print("----- AVERAGE SAMPLES IN SPEC: ");
         Serial.print(readingsInSpec_Count_b);
         Serial.print(" OF ");
         Serial.print(readings_Count_b);
         readings_Count_b=0;
         readingsInSpec_Count_b=0;
     }
     
     if(laserDurationInSpec_Count >= 3) {
        if(debugMode==1) Serial.println("Returned MD");
        return measuredDuration;
    } else {
        if(debugMode==1) Serial.println("Returned 0");
        return 0;
    }
}

void setGreenLight_ON(){
    digitalWrite(greenPin, HIGH);  
}

void setGreenLight_OFF(){
    digitalWrite(greenPin, LOW);  
}

void setRedLight_ON(){
    digitalWrite(redPin, HIGH);
}

void setRedLight_OFF(){
    digitalWrite(redPin, LOW);  
}

void setGreenLight_b_ON(){
    digitalWrite(greenPin_b, HIGH);  
}

void setGreenLight_b_OFF(){
    digitalWrite(greenPin_b, LOW);  
}

void setRedLight_b_ON(){
    digitalWrite(redPin_b, HIGH);
}

void setRedLight_b_OFF(){
    digitalWrite(redPin_b, LOW);  
}


boolean isLaserDurationWithinSpec(long duration){
  return duration > laserDurationSpec_min &&  duration < laserDurationSpec_max;
}


void sendDataToPVCloud(String label, String value){
  String pvcloudCommand = "node /home/root/pvcloud_api.js action='add_value' value='" + value + "' value_type='JSON_TH' value_label='" + label + "' captured_datetime='2015-03-09+21:00' >> pvcloud_log.txt &";
  system ( pvcloudCommand.buffer );
}

void sendAlarmStatusChangeToPVCloud(String newAlarmStatus){
     if(previousAlarmMode!=newAlarmStatus){
       previousAlarmMode = newAlarmStatus;
       sendDataToPVCloud("LSM ALARM MODE:",newAlarmStatus);
     }  
}

String getPin13CommandFromPVCloud(){
  String pvcloudCommand = "node /home/root/pvcloud_api.js action='get_last_value_simple' value_label='PIN_13_STATUS' captured_datetime='2015-03-09+21:00' > pvcloud_pin13_command.txt";
  system ( pvcloudCommand.buffer );  
  Serial.println("PIN 13 COMMAND READ!");
  return "DONE";
}


String readFileValue(){
  FILE *filePointer;
  filePointer = fopen("/pvcloud_pin13_command.txt","r");
  char fileContent[100];
  fgets (fileContent , 400 , filePointer);
  Serial.println(fileContent);
  
  return fileContent;
}

