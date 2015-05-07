// DIGITAL PINS
#define pin_rx              0
#define pin_tx              1

#define pin_laser_01        2
#define pin_laser_01_red    3
#define pin_laser_01_green  4

#define pin_laser_02        5
#define pin_laser_02_red    6
#define pin_laser_02_green  7

#define pin_usonic_trigger  8
#define pin_usonic_echo     9

#define pin_alarm_light    11
#define pin_alarm_buzzer   12

#define pin_blink 13

//ANALOG PINS
#define pin_photocell       0
#define pin_temperature     1
#define pin_absence         2
#define pin_presence        3

#define LASER_BEAM_STEADY   0
#define LASER_BEAM_UNSTABLE 1
#define LASER_BEAM_ABSENT   2

#define ALARM_STATUS_CLEAR   0
#define ALARM_STATUS_WARNING 1
#define ALARM_STATUS_PANIC   2

#define alarmDurationMillis 3000

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699

boolean  debugMode          = false; /*Use true for DEBUG MODE ACTIVE, and false for DEBUG MODE INACTIVE*/

int DistanceInCM = -1;
int LightSensor = -1;
int TemperatureInCelsius = -1;
boolean AbsenseSensor = true;
boolean PresenseSensor = false;
int Laser1DurationInMilliseconds = 0;
int Laser2DurationInMilliseconds = 0;
int Laser1Status = 1;
int Laser2Status = 1;

long previousLaserDuration = 0;

long lastGreenMillisStamp = 0;
int photoCellInput = 0;
int previousPhotoCellInput = 0;
int photoCellDiff = 0;


long readings_Count = 0;
long readingsInSpec_Count = 0;


long readings_Count_b = 0;
long readingsInSpec_Count_b = 0;

long laserDurationSpec_min = 1000;
long laserDurationSpec_max = 4000;
long alarmMinTime = 30000;

String sensorStatus = "01-UNSTABLE_GREEN"; /*01-UNSTABLE_GREEN, 02-STABLE_GREEN, 03-RED*/



void setup() {
    tune();
    delay(2000);
    Serial.begin (9600);
    Serial.println("SETTING SKETCH UP...");
    Serial.println("Sending START data to pvCloud...");
    sendDataToPVCloud("SKETCH_FLOW", "HOME SECURITY SKETCH STARTED", false);
    
    if(debugMode) Serial.println("Setting Pin Modes...");
    setPinModes();
    
    if(debugMode) Serial.println("Init Signal...");
    initSignal();
   
    if(debugMode) Serial.println("Setup Complete!");
}

/*-------------------------------------------------------------------*/
/*                         LOOP FUNCTION GLOBALS                     */
/*-------------------------------------------------------------------*/
boolean isFirstLoop = true;
int alarmStatus_PREV = -1;
String currentFrontDoorStatus="H";
long frontDoorStatus_diffLastDetectedMillis = 0;
long lastPVCloudUpdateMillis = 0;

/*-------------------------------------------------------------------*/
/*                         LOOP FUNCTION                             */
/*-------------------------------------------------------------------*/
void loop(){
  if(isFirstLoop) processInitialLoop();
  
  readSensors(false); 
  
  String laserStatus = (Laser1Status == LASER_BEAM_ABSENT && Laser2Status == LASER_BEAM_ABSENT)?"A":"S";
  String z1Status = AbsenseSensor==false?"L":"H"; 
  String z2Status = PresenseSensor?"L":"H";
  String houseStatus = "C";
  String frontDoorStatus = DistanceInCM < 70?"L":"H";     
  String lightStatus = LightSensor > 500? "L": "H";
  String temperatureStatus = TemperatureInCelsius > 30? "L": "H";
  
  int alarmStatus = determineAlarmStatus();
    
  processAlarmStatusOnPVCloud(alarmStatus, laserStatus, z1Status, z2Status, houseStatus, frontDoorStatus, temperatureStatus, lightStatus);
    
  executeAlarmStatus(alarmStatus);  
  
  currentDoorStatusCheckpoint(laserStatus, z1Status, z2Status, houseStatus, frontDoorStatus, temperatureStatus, lightStatus);
  
  minuteCheckpoint(laserStatus, z1Status, z2Status, houseStatus, frontDoorStatus, temperatureStatus, lightStatus);
  
  if(isFirstLoop){
    clearInitialLoopCondition();
  }  
}

void clearInitialLoopCondition(){
    Serial.println("INITIAL LOOP END");
    isFirstLoop = false;
}
void processInitialLoop(){
    Serial.println("INITIAL LOOP BEGIN");
    String jsonvalue = buildPVCloudValue("S", "H","H", "C", "H","H","H");
    sendDataToPVCloud("HOUSE_STATUS",jsonvalue, false);
}

void processAlarmStatusOnPVCloud(int alarmStatus, String laserStatus, String z1Status, String z2Status, String houseStatus, String frontDoorStatus, String temperatureStatus, String lightStatus){
   String jsonvalue = "";
   if (alarmStatus != alarmStatus_PREV && alarmStatus_PREV!=-1){
      Serial.println("ALARM STATUS CHANGED!");
      alarmStatus_PREV = alarmStatus;
  
      
      switch(alarmStatus){
        case ALARM_STATUS_CLEAR:
          Serial.println("ALARM STATUS: CLEAR");
          houseStatus="C";
          jsonvalue = buildPVCloudValue(laserStatus, z1Status,z2Status, houseStatus, frontDoorStatus, temperatureStatus, lightStatus);
          sendDataToPVCloud("HOUSE_STATUS",jsonvalue, false);
          lastPVCloudUpdateMillis = millis();
          break;
        case ALARM_STATUS_WARNING:
          Serial.println("ALARM STATUS: WARNING!");
          break;
        case ALARM_STATUS_PANIC:
          Serial.println("ALARM STATUS: PANIC!!!!!");
          houseStatus="U";
          jsonvalue = buildPVCloudValue(laserStatus, z1Status,z2Status, houseStatus, frontDoorStatus, temperatureStatus, lightStatus);
          sendDataToPVCloud("HOUSE_STATUS",jsonvalue, false);
          lastPVCloudUpdateMillis = millis();
          break;
      }  
    } else if ( alarmStatus_PREV == -1 ) {//Home109080363
      alarmStatus_PREV = alarmStatus;
      
    }
}

void currentDoorStatusCheckpoint(String laserStatus, String z1Status, String z2Status, String houseStatus, String frontDoorStatus, String temperatureStatus, String lightStatus){
  String jsonvalue = "";
  if(currentFrontDoorStatus == ""){
     Serial.print("INITIALIZING FRONTDOOR PREV: ");
     Serial.println(frontDoorStatus);
     currentFrontDoorStatus = frontDoorStatus;
  } else if (frontDoorStatus != currentFrontDoorStatus){
    if((frontDoorStatus=="L" && frontDoorStatus_diffLastDetectedMillis!=0 && millis() - frontDoorStatus_diffLastDetectedMillis > 300)||(frontDoorStatus=="H" && frontDoorStatus_diffLastDetectedMillis!=0 && millis() - frontDoorStatus_diffLastDetectedMillis > 10000)){
       if(frontDoorStatus=="L") tune();
       Serial.print("CHANGE ON FRONTDOOR: ");
       Serial.println(frontDoorStatus);
       
       currentFrontDoorStatus = frontDoorStatus;
       jsonvalue = buildPVCloudValue(laserStatus, z1Status,z2Status, houseStatus, currentFrontDoorStatus, temperatureStatus, lightStatus);
       sendDataToPVCloud("HOUSE_STATUS",jsonvalue, false);
       lastPVCloudUpdateMillis = millis();      
    } else if (frontDoorStatus_diffLastDetectedMillis==0) {
      frontDoorStatus_diffLastDetectedMillis = millis();
    }
  } else { //NOT DIFFERENT STATUS
       frontDoorStatus_diffLastDetectedMillis = millis();
  }  
}

void minuteCheckpoint(String laserStatus, String z1Status, String z2Status, String houseStatus, String frontDoorStatus, String temperatureStatus, String lightStatus){
  
  if(millis() - lastPVCloudUpdateMillis > 60000){
     String jsonvalue = buildPVCloudValue(laserStatus, z1Status,z2Status, houseStatus, frontDoorStatus, temperatureStatus, lightStatus);
     sendDataToPVCloud("HOUSE_STATUS",jsonvalue, false);   
     lastPVCloudUpdateMillis = millis(); 
  }
}

void initSignal(){
    if(debugMode) {Serial.println("Init Signal Begins...");}
    for (int i=1; i<=20; i++){
        if(debugMode) {Serial.print("Init Signal Count: ");}
        if(debugMode) {Serial.println(i);}
        
        digitalWrite(pin_blink, HIGH);  
        digitalWrite(pin_laser_01_green, HIGH);  
        digitalWrite(pin_laser_01_red, HIGH);  
        digitalWrite(pin_laser_02_green, HIGH);  
        digitalWrite(pin_laser_02_red, HIGH);                      
        digitalWrite(pin_alarm_light, HIGH); 
        digitalWrite(pin_alarm_buzzer, HIGH); 
        delay(5);
        
        digitalWrite(pin_blink, LOW);  
        digitalWrite(pin_laser_01_green, LOW);  
        digitalWrite(pin_laser_01_red, LOW);  
        digitalWrite(pin_laser_02_green, LOW);  
        digitalWrite(pin_laser_02_red, LOW);  
        digitalWrite(pin_alarm_light, LOW); 
        digitalWrite(pin_alarm_buzzer, LOW);    
        delay(300);
        
        readSensors(true);        
    }
}

void readSensors( bool initial ){
  if(debugMode || initial) Serial.println("----------------------------------------------");
  if(debugMode || initial) Serial.print("MILLIS: ");
  if(debugMode || initial) Serial.println(millis());
  DistanceInCM = getDistance();
  if(debugMode || initial) Serial.print("INI - DISTANCE:  ");
  if(debugMode || initial) Serial.print(DistanceInCM);
  if(debugMode || initial) Serial.println("cm");
  
  LightSensor = readLightSensor();
  if(debugMode || initial) Serial.print("INI - LIGHT:     ");
  if(debugMode || initial) Serial.println(LightSensor);  
  
  TemperatureInCelsius = readTemperatureSensor() ;
  if(debugMode || initial) Serial.print("INI - TEMP:      ");
  if(debugMode || initial) Serial.println(TemperatureInCelsius);  
  
  
  AbsenseSensor = readAbsenceSensor()>500?true:false;
  if(debugMode || initial) Serial.print("INI - ABSENCE:   ");
  if(debugMode || initial) Serial.println(AbsenseSensor);  
  
  PresenseSensor = readPresenceSensor()>500?true:false;
  if(debugMode || initial) Serial.print("INI - PRESENCE:  ");
  if(debugMode || initial) Serial.println(PresenseSensor);   
  
  Laser1DurationInMilliseconds = getLaserDurationFiveInARow();
  if(debugMode || initial) Serial.print("INI - LASER1 PW: ");
  if(debugMode || initial) Serial.println(Laser1DurationInMilliseconds);   
 
  Laser2DurationInMilliseconds = getLaserDurationFiveInARow_b();
  if(debugMode || initial) Serial.print("INI - LASER2 PW: ");
  if(debugMode || initial) Serial.println(Laser2DurationInMilliseconds);   
  
  Laser1Status =   Laser_DetermineStatus(Laser1DurationInMilliseconds,0);
  if(debugMode || initial) Serial.print("INI - LASER1 ST: ");
  if(debugMode || initial) Serial.println(Laser1Status);   
  
  Laser_SetStatusLEDs(0,Laser1Status);
  
  Laser2Status =   Laser_DetermineStatus(Laser2DurationInMilliseconds,1);
  if(debugMode || initial) Serial.print("INI - LASER2 ST: ");
  if(debugMode || initial) Serial.println(Laser2Status);   
  Laser_SetStatusLEDs(1,Laser2Status);
  
  if(debugMode || initial) Serial.print("MILLIS: ");
  if(debugMode || initial) Serial.println(millis());
  
}

int SteadyBeamInitMillis[] = {0,0};
int AbsentBeamInitMillis[] = {0,0};
int Laser_DetermineStatus(int laserDuration, int laserIndex){
    int currentSensorStatus =  LASER_BEAM_UNSTABLE;
    if(isLaserDurationWithinSpec(laserDuration)){   
      AbsentBeamInitMillis[laserIndex] = 0;
      if(SteadyBeamInitMillis[laserIndex]==0) {
        SteadyBeamInitMillis[laserIndex] = millis();
      } else if(millis()-SteadyBeamInitMillis[laserIndex] > 1000) {
        currentSensorStatus =  LASER_BEAM_STEADY;
      }
    } else { //not in spec
      SteadyBeamInitMillis[laserIndex] = 0;
      currentSensorStatus = LASER_BEAM_UNSTABLE;
      if(AbsentBeamInitMillis[laserIndex]==0) {
        AbsentBeamInitMillis[laserIndex] = millis();
      }
      else if (millis()-AbsentBeamInitMillis[laserIndex] > 1000){
        currentSensorStatus =  LASER_BEAM_ABSENT;
      } 
    } 
    
    return currentSensorStatus;
}

void Laser_SetStatusLEDs(int laserIndex, int laserBeamStatus){
  
  if(laserIndex==0){
    switch(laserBeamStatus){
       case LASER_BEAM_STEADY:
           digitalWrite(pin_laser_01_red,LOW);
           digitalWrite(pin_laser_01_green,HIGH);
           break;
       case LASER_BEAM_UNSTABLE:
           digitalWrite(pin_laser_01_red,LOW);
           digitalWrite(pin_laser_01_green,LOW);
           break;
       case LASER_BEAM_ABSENT:
           digitalWrite(pin_laser_01_red,HIGH);
           digitalWrite(pin_laser_01_green,LOW);       
    }
  } else {
    switch(laserBeamStatus){
       case LASER_BEAM_STEADY:
           digitalWrite(pin_laser_02_red,LOW);
           digitalWrite(pin_laser_02_green,HIGH);
           break;
       case LASER_BEAM_UNSTABLE:
           digitalWrite(pin_laser_02_red,LOW);
           digitalWrite(pin_laser_02_green,LOW);
           break;
       case LASER_BEAM_ABSENT:
           digitalWrite(pin_laser_02_red,HIGH);
           digitalWrite(pin_laser_02_green,LOW);       
    }    
  }
  
}



long millisToExitPanic = 0;
long millisToExitWarning = 0;
int determineAlarmStatus(){
  long currentMillis = millis();  
  if( (Laser1Status == LASER_BEAM_ABSENT && Laser2Status == LASER_BEAM_ABSENT) || !AbsenseSensor || PresenseSensor) {
    millisToExitPanic = currentMillis + alarmDurationMillis;
    return ALARM_STATUS_PANIC;
  }
  
  if(currentMillis < millisToExitPanic) return ALARM_STATUS_PANIC;
  millisToExitPanic = 0;
  
  if( Laser1Status == LASER_BEAM_UNSTABLE && Laser2Status == LASER_BEAM_UNSTABLE ){
    millisToExitWarning = currentMillis + 0;
    return ALARM_STATUS_WARNING;
  }  
  if(currentMillis < millisToExitWarning) return ALARM_STATUS_WARNING;
  millisToExitWarning = 0;
  
  return ALARM_STATUS_CLEAR;
}

void executeAlarmStatus(int alarmStatus){
  switch(alarmStatus){
     case ALARM_STATUS_PANIC:
       digitalWrite(pin_alarm_buzzer,HIGH);
       digitalWrite(pin_alarm_light,HIGH);
       break;
     case ALARM_STATUS_WARNING:
       for(int i=0; i<=3; i++){
         digitalWrite(pin_alarm_buzzer,HIGH);
         digitalWrite(pin_alarm_light,HIGH);
         delay(50);
         digitalWrite(pin_alarm_buzzer,LOW);
         digitalWrite(pin_alarm_light,LOW);
         delay(50);
       }
       break;
     default :
       digitalWrite(pin_alarm_buzzer,LOW);
       digitalWrite(pin_alarm_light,LOW);
  }
  
}


void setPinModes(){
    pinMode(pin_laser_01, INPUT);
    pinMode(pin_laser_01_green, OUTPUT);
    pinMode(pin_laser_01_red, OUTPUT);
    
    pinMode(pin_laser_02, INPUT);
    pinMode(pin_laser_02_green, OUTPUT);
    pinMode(pin_laser_02_red, OUTPUT);    
    
    pinMode(pin_alarm_light, OUTPUT);    
    pinMode(pin_alarm_buzzer, OUTPUT);    
    
    pinMode(pin_usonic_trigger, OUTPUT);
    pinMode(pin_usonic_echo, INPUT);
    
   digitalWrite(pin_alarm_light,LOW);
   digitalWrite(pin_alarm_buzzer,LOW);       
}

long getDistance(){
  
    long pulseDuration;
    long distanceInCM = -1;
    
    digitalWrite(pin_usonic_trigger, LOW);
    delayMicroseconds(2);
    digitalWrite(pin_usonic_trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(pin_usonic_trigger, LOW);
    
    pulseDuration = pulseIn(pin_usonic_echo, HIGH,1000);
    
    distanceInCM = (pulseDuration/2)/29.1;
    
    return distanceInCM;
}



long getLaserDurationFiveInARow(){
    int laserDurationInSpec_Count = 0;

    long measuredDuration = 0;
    long laserDuration = 0;
    
    for(int i=0; i<10; i++){
       readings_Count ++;
       laserDuration = pulseIn(pin_laser_01, LOW, 3000);
       
       if(isLaserDurationWithinSpec(laserDuration)) {
         laserDurationInSpec_Count ++; 
         readingsInSpec_Count ++;
         measuredDuration = laserDuration;
       }
       
       if(debugMode) {
         Serial.print("Laser Duration # ");
         Serial.print(i);Serial.print(": ");
         Serial.println(laserDuration);
                
       }
    }
    
 
     if(debugMode) {Serial.print("Measured Duration: "); Serial.println(measuredDuration); } 
   
     if(readings_Count==1000) {
         Serial.println("--------------------------------------------------");
         Serial.println("--------------------------------------------------");
         
         Serial.print("----- AVERAGE SAMPLES IN SPEC (A): ");
         Serial.print(readingsInSpec_Count);
         Serial.print(" OF ");
         Serial.println(readings_Count);
         Serial.println("--------------------------------------------------");
         readings_Count=0;
         readingsInSpec_Count=0;
     }
     
     if(laserDurationInSpec_Count >= 3) {
        if(debugMode) Serial.println("Returned MD");
        return measuredDuration;
    } else {
        if(debugMode) Serial.println("Returned 0");
        return 0;
    }
}

long getLaserDurationFiveInARow_b(){
    int laserDurationInSpec_Count = 0;

    long measuredDuration = 0;
    long laserDuration = 0;
    
    for(int i=0; i<10; i++){
       readings_Count_b ++;
       laserDuration = pulseIn(pin_laser_02, LOW, 3000);
       
       if(isLaserDurationWithinSpec(laserDuration)) {
         laserDurationInSpec_Count ++; 
         readingsInSpec_Count_b ++;
         measuredDuration = laserDuration;
       }
       
       if(debugMode) {
         Serial.print("Laser Duration # ");
         Serial.print(i);Serial.print(": ");
         Serial.println(laserDuration);
                
       }
    }
    
 
     if(debugMode) {Serial.print("Measured Duration: "); Serial.println(measuredDuration); } 
   
     if(readings_Count_b==1000) {
         Serial.println("--------------------------------------------------");
         Serial.println("--------------------------------------------------");

         Serial.print("----- AVERAGE SAMPLES IN SPEC (B): ");
         Serial.print(readingsInSpec_Count_b);
         Serial.print(" OF ");
         Serial.println(readings_Count_b);
         Serial.println("--------------------------------------------------");         
         readings_Count_b=0;
         readingsInSpec_Count_b=0;
     }
     
     if(laserDurationInSpec_Count >= 3) {
        if(debugMode) Serial.println("Returned MD");
        return measuredDuration;
    } else {
        if(debugMode) Serial.println("Returned 0");
        return 0;
    }
}



boolean isLaserDurationWithinSpec(long duration){
  return duration > laserDurationSpec_min &&  duration < laserDurationSpec_max;
}

String buildPVCloudValue(String laserStatus, String z1Status, String z2Status, String houseStatus, String distanceFlag, String tempStatus, String lightStatus){
  String pvCloudValue = "{\"per_status\":\""+laserStatus+"\",\"front_door_status\":\""+distanceFlag+"\",\"z1_status\":\""+z1Status+"\",\"z2_status\":\""+z2Status+"\",\"z3_status\":\"H\",\"temp_status\":\""+tempStatus+"\",\"light_status\":\""+lightStatus+"\",\"house_status\":\""+houseStatus+"\"}";
  return pvCloudValue;
}

void sendDataToPVCloud(String label, String value, boolean waitForResponse){
  Serial.println("Sending Data to pvCloud");
  Serial.println(value);
  
  if(waitForResponse){
    String pvcloudCommand = "node /home/root/pvcloud_api.js action='add_value' value='" + value + "' value_type='JSON_TH' value_label='" + label + "' captured_datetime='2015-03-09+21:00' >> pvcloud_log.txt";
    system ( pvcloudCommand.buffer );
  } else {
    String pvcloudCommand = "node /home/root/pvcloud_api.js action='add_value' value='" + value + "' value_type='JSON_TH' value_label='" + label + "' captured_datetime='2015-03-09+21:00' >> pvcloud_log.txt &";
    system ( pvcloudCommand.buffer );
  }
}


String getPin13CommandFromPVCloud(){
  String pvcloudCommand = "node /home/root/pvcloud_api.js action='get_last_value_simple' value_label='PIN_13_STATUS' captured_datetime='2015-03-09+21:00' > pvcloud_pin13_command.txt";
  system ( pvcloudCommand.buffer );  
  String result = readFileValue();
  Serial.println("PIN 13 COMMAND READ!");
  return result;
}


String readFileValue(){
  FILE *filePointer;
  filePointer = fopen("/pvcloud_pin13_command.txt","r");
  char fileContent[100];
  fgets (fileContent , 400 , filePointer);
  Serial.println(fileContent);
  
  return fileContent;
}

int readLightSensor(){
   return analogRead( pin_photocell );
}

double readTemperatureSensor(){
   int result = analogRead( pin_temperature );
   return convertToCM(1024-result);    
}
double convertToCM (int RawADC) {
  double Temp;
  Temp = log (((10240000/RawADC) - 10000));
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
  Temp = Temp - 273.15; // Convert Kelvin to Celcius
  return Temp;
}

double readAbsenceSensor(){
   return analogRead( pin_absence );
}

double readPresenceSensor(){
   return analogRead( pin_presence );
}

void tune(){
  tone(12,NOTE_E6);
  delay(400);
  tone (12,NOTE_A6);
  delay(600);
  noTone(12);
}
