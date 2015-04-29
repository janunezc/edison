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

#define alarmDurationMillis 2000

boolean  debugMode          = false; /*Use true for DEBUG MODE ACTIVE, and false for DEBUG MODE INACTIVE*/

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

boolean isFirstLoop = true;
int alarmStatus_PREV = -1;
void loop(){
  if(isFirstLoop) Serial.println("INITIAL LOOP BEGIN");
   
  readSensors(false); 
  
  
  int alarmStatus = Alarm_DetermineStatus();
   
  if (alarmStatus != alarmStatus_PREV && alarmStatus_PREV!=-1){
    Serial.println("ALARM STATUS CHANGED!");
    alarmStatus_PREV = alarmStatus;
    switch(alarmStatus){
      case ALARM_STATUS_CLEAR:
        Serial.println("ALARM STATUS: CLEAR");
        sendDataToPVCloud("HOUSE_STATUS","{\"per_status\":\"S\",\"front_door_status\":\"H\",\"z1_status\":\"H\",\"z2_status\":\"H\",\"z3_status\":\"H\",\"temp_status\":\"H\",\"light_status\":\"H\",\"house_status\":\"C\"}", false);
        break;
      case ALARM_STATUS_WARNING:
        Serial.println("ALARM STATUS: WARNING!");
        break;
      case ALARM_STATUS_PANIC:
        Serial.println("ALARM STATUS: PANIC!!!!!");
        sendDataToPVCloud("HOUSE_STATUS","{\"per_status\":\"A\",\"front_door_status\":\"H\",\"z1_status\":\"H\",\"z2_status\":\"H\",\"z3_status\":\"H\",\"temp_status\":\"H\",\"light_status\":\"H\",\"house_status\":\"U\"}", false);
        break;
    }  
  } else if ( alarmStatus_PREV == -1 ) {
    alarmStatus_PREV = alarmStatus;
  }
  
  Alarm_ExecuteStatus(alarmStatus);  
  
  

  
 
  
  if(isFirstLoop){
    Serial.println("INITIAL LOOP END");
    isFirstLoop = false;
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

int prev_DistanceInCM = -1;
int prev_LightSensor = -1;
int prev_TemperatureInCelsius = -1;
boolean prev_AbsenseSensor = true;
boolean prev_PresenseSensor = false;
int prev_Laser1DurationInMilliseconds = 0;
int prev_Laser2DurationInMilliseconds = 0;
int prev_Laser1Status = 1;
int prev_Laser2Status = 1;

int DistanceInCM = -1;
int LightSensor = -1;
int TemperatureInCelsius = -1;
boolean AbsenseSensor = true;
boolean PresenseSensor = false;
int Laser1DurationInMilliseconds = 0;
int Laser2DurationInMilliseconds = 0;
int Laser1Status = 1;
int Laser2Status = 1;


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
int Alarm_DetermineStatus(){
  long currentMillis = millis();  
  if( Laser1Status == LASER_BEAM_ABSENT && Laser2Status == LASER_BEAM_ABSENT){
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

void Alarm_ExecuteStatus(int alarmStatus){
  switch(alarmStatus){
     case ALARM_STATUS_PANIC:
       digitalWrite(pin_alarm_buzzer,HIGH);
       break;
     case ALARM_STATUS_WARNING:
       for(int i=0; i<=3; i++){
         digitalWrite(pin_alarm_buzzer,HIGH);
         delay(50);
         digitalWrite(pin_alarm_buzzer,LOW);
         delay(50);
       }
       break;
     default :
       digitalWrite(pin_alarm_buzzer,LOW);
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
    
    pulseDuration = pulseIn(pin_usonic_echo, HIGH);
    
    distanceInCM = (pulseDuration/2)/29.1;
    
    return distanceInCM;
}



long getLaserDurationFiveInARow(){
    int laserDurationInSpec_Count = 0;

    long measuredDuration = 0;
    long laserDuration = 0;
    
    for(int i=0; i<10; i++){
       readings_Count ++;
       laserDuration = pulseIn(pin_laser_01, LOW, 5000);
       
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
       laserDuration = pulseIn(pin_laser_02, LOW, 5000);
       
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


void sendDataToPVCloud(String label, String value, boolean waitForResponse){
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
