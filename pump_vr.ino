
#include <SoftwareSerial.h>

//weight sensor libraries
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

SoftwareSerial BTserial(8, 9); // RX | TX

int inByteDir;
int inBytePumpTime;

const long baudRate = 9600;
const int numRelays = 8;

const int PUMP_RELAY_PIN_1 = 7;
const int PUMP_RELAY_PIN_2 = 6;

const int VALVE_RELAY_PIN_1 = 5;
const int VALVE_RELAY_PIN_2 = 4;
const int VALVE_RELAY_PIN_3 = 3;
const int VALVE_RELAY_PIN_4 = 2;

//weight sensor pins:
const int HX711_dout = 10;
const int HX711_sck = 11;

boolean relayStates[numRelays];

enum pumpStates {
  IDLEMODE,
  RIGHT_IN, // pumping into right bottle
  LEFT_IN, // pumping into left bottle
  BOTH_IN,
  RIGHT_OUT, // emptying right bottle
  LEFT_OUT,// etc.
  BOTH_OUT
};

enum pumpStates pumpState;

long int motorStartTime;
const long int maxFillTime = 5200;
const long int maxDrainTime = 6000;
long int drainDur = 5100;
long int fillDur = 3100;
const int numLevels = 5;
const long int fillDurationsMono[numLevels] = {620, 1240, 1860, 2480, 3100};
const long int drainDurationsMono[numLevels] = {1100, 2000 ,3000 , 4100, 5100}; //for draining, the pump runs longer to ensure a complete reset of weight
const long int fillDurationsBi[numLevels] = {1100, 2000,3000, 4100, 5100};
const long int drainDurationsBi[numLevels] = {2000, 3000, 4100, 5100, 5300};

float motorDelay = 100;

//variables for weight test loop with hx711#
const float calibrationValue = 485.96; // calibration value from calibration with HX711_ADC.h
const int maxWeighTests = 30;

long int weightTestStep = fillDur; //1000
long int weightTestPause = 5000; //1000
long int weightTestLogBuffer = 5000; //1000

long int weightTestStartTime;
boolean weightTestRunning = false;
int weightTestCount = 0;
int weightTestIndex = -1;
const int trials = 1; //4
unsigned long steps[trials];
unsigned long pauses[trials];
unsigned long finishTest;
pumpStates testDirection = LEFT_IN;
float lastValidWeight = 0;
float lastValidWeightPrev = 0;
const int outageFlag = 15;

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
//HX711 additional variables
const int serialPrintInterval = 99; 
const int calVal_eepromAdress = 0;
unsigned long t = 0;

void setup() {
  Serial.begin(9600);
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");

  BTserial.begin(baudRate);
  Serial.print("BTserial started at "); Serial.println(baudRate);
  Serial.println(" ");

  setupPumpSystem();
  setupWeightSensor();
}

void loop() {
  // Read from the Bluetooth module and send to the Arduino
  if(BTserial.available()>2){Serial.println("WARNING signal to long.");}

  if (BTserial.available()) {
    inByteDir = BTserial.read();
    Serial.println(inByteDir);
    if (BTserial.available()) {
      inBytePumpTime = BTserial.read();
      Serial.println(inBytePumpTime);
    }
    handleInput();
  }
  //remove this call for weight testing 
  controlWaterFlow();
  
  checkMotorRuntime();

  if (weightTestRunning) {
    String w = trackWeight();
    weightTestLoop(testDirection, w, false);
  }
  //trackWeight();  // uncomment this line for weight testing
  delay(1);
}

void setupPumpSystem() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PUMP_RELAY_PIN_1, OUTPUT);
  pinMode(PUMP_RELAY_PIN_2, OUTPUT);

  pinMode(VALVE_RELAY_PIN_1, OUTPUT);
  pinMode(VALVE_RELAY_PIN_2, OUTPUT);
  pinMode(VALVE_RELAY_PIN_3, OUTPUT);
  pinMode(VALVE_RELAY_PIN_4, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PUMP_RELAY_PIN_1, HIGH);
  digitalWrite(PUMP_RELAY_PIN_2, HIGH);

  digitalWrite(VALVE_RELAY_PIN_1, HIGH);
  digitalWrite(VALVE_RELAY_PIN_2, HIGH);
  digitalWrite(VALVE_RELAY_PIN_3, HIGH);
  digitalWrite(VALVE_RELAY_PIN_4, HIGH);

  for ( int i = 0; i < numRelays; i++)
  {
    relayStates[i] = false;
  }

  pumpState = IDLEMODE;

  motorStartTime = -1 * maxFillTime;
  Serial.println(motorStartTime);
}


void setupWeightSensor() {
  //Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");

  LoadCell.begin();
#if defined(ESP8266)|| defined(ESP32)
  //EEPROM.begin(512);
#endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue);
  unsigned long stabilizingtime = 2000;
  boolean _tare = true;
  LoadCell.start(stabilizingtime, _tare);

  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
}

void controlWaterFlow(){
    if (((pumpState == LEFT_OUT)||(pumpState == RIGHT_OUT)||(pumpState == BOTH_OUT)) && (millis() > (motorStartTime + drainDur + motorDelay))) {
    stopMotor();
    resetValves();
    }

    if (((pumpState == LEFT_IN)||(pumpState == RIGHT_IN)||(pumpState == BOTH_IN)) && (millis() > (motorStartTime + fillDur + motorDelay))) {
    stopMotor();
    resetValves();
    }
  
  }

void checkMotorRuntime(){
    if (((relayStates[0] == true)) && (millis() > (motorStartTime + maxDrainTime))) {
    stopMotor();
    }
  
    if (((relayStates[1] == true)) && (millis() > (motorStartTime + maxFillTime))) {
    stopMotor();
    }
  }

void stopMotor() {
  digitalWrite(PUMP_RELAY_PIN_1, HIGH);
  relayStates[0] = false;

  digitalWrite(PUMP_RELAY_PIN_2, HIGH);
  relayStates[1] = false;

  pumpState = IDLEMODE;
  Serial.print("Motor stopped at: ");Serial.println(millis());
}

void setTimeOut(pumpStates newState){
  int level = inBytePumpTime-48;
  Serial.print("Level ");Serial.println(level);
  if (1 <= level && level <=numLevels){
    
    if ((newState == LEFT_OUT)||(newState == RIGHT_OUT)){
      drainDur = drainDurationsMono[level-1];Serial.print("Drain for ");Serial.println(drainDur);
    }
    else if ((newState == LEFT_IN)||(newState == RIGHT_IN)){
      fillDur = fillDurationsMono[level-1];Serial.print("Fill for ");Serial.println(fillDur);
    }
    else if (newState == BOTH_OUT){
      drainDur = drainDurationsBi[level-1];Serial.print("Drain for ");Serial.println(drainDur);
    }
    else if (newState == BOTH_IN){
      fillDur = fillDurationsBi[level-1];Serial.print("Fill for ");Serial.println(fillDur);
    }
  }
}

void handleInput() {

  switch (inByteDir) {
    case (int)'R':
      stopMotor();
      setTimeOut(RIGHT_IN);
      startPumping(RIGHT_IN);
      break;
    case (int)'r':
      stopMotor();
      setTimeOut(RIGHT_OUT);
      startPumping(RIGHT_OUT);
      break;
    case (int)'L':
      stopMotor();
      setTimeOut(LEFT_IN);
      startPumping(LEFT_IN);
      break;
    case (int)'l':
      stopMotor();
      setTimeOut(LEFT_OUT);
      startPumping(LEFT_OUT);
      break;
    case (int)'B':
      stopMotor();
      setTimeOut(BOTH_IN);
      startPumping(BOTH_IN);
      break;
    case (int)'b':
      stopMotor();
      setTimeOut(BOTH_OUT);
      startPumping(BOTH_OUT);
      break;
    case (int)'h':
      stopMotor();
      resetValves();
      break;
    case (int)'H':
      stopMotor();
      resetValves();
      break;
    case (int)'W':
      startWeightTest();
      break;
    case (int)'w':
      weightTestRunning == false;
      break;
    default:
      break;
  }

}


String trackWeight() {
  static boolean newDataReady = 0;
  String weightLog = String("") + t + String("; ") + getStateName(pumpState) + String("; ; ");
  // check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float weight = LoadCell.getData();
      lastValidWeight = weight;
      t = millis();
    

      weightLog = String("") + t + String("; ") + getStateName(pumpState) + String("; ") + weight + String("; ");
      Serial.println(weightLog);
      newDataReady = 0;

    }
  }
  // receive command from serial terminal, send 't' to initiate tare operation:
  /*if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }*/

  // check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }

  return weightLog;
}


void startPumping(pumpStates newState) {
  resetValves();
  
  switch (newState) {
    case LEFT_IN:
      digitalWrite(VALVE_RELAY_PIN_1, LOW);
      digitalWrite(VALVE_RELAY_PIN_3, LOW);
      delay(motorDelay);
      digitalWrite(PUMP_RELAY_PIN_2, LOW);
      relayStates[1] = true;
      break;
    case RIGHT_IN:
      digitalWrite(VALVE_RELAY_PIN_2, LOW);
      digitalWrite(VALVE_RELAY_PIN_4, LOW);
      delay(motorDelay);
      digitalWrite(PUMP_RELAY_PIN_2, LOW);
      relayStates[1] = true;
      break;
    case BOTH_IN:
      digitalWrite(VALVE_RELAY_PIN_1, LOW);
      digitalWrite(VALVE_RELAY_PIN_4, LOW);
      delay(motorDelay);
      digitalWrite(PUMP_RELAY_PIN_2, LOW);
      relayStates[1] = true;
      break;
    case LEFT_OUT:
      digitalWrite(VALVE_RELAY_PIN_2, LOW);
      digitalWrite(VALVE_RELAY_PIN_4, LOW);
      delay(motorDelay);
      digitalWrite(PUMP_RELAY_PIN_1, LOW);
      relayStates[0] = true;
      break;
    case RIGHT_OUT:
      digitalWrite(VALVE_RELAY_PIN_3, LOW);
      digitalWrite(VALVE_RELAY_PIN_1, LOW);
      delay(motorDelay);
      digitalWrite(PUMP_RELAY_PIN_1, LOW);
      relayStates[0] = true;
      break;
    case BOTH_OUT:
      digitalWrite(VALVE_RELAY_PIN_2, LOW);
      digitalWrite(VALVE_RELAY_PIN_3, LOW);
      delay(motorDelay);
      digitalWrite(PUMP_RELAY_PIN_1, LOW);
      relayStates[0] = true;
      break;

    default:
      break;
  }
  pumpState = newState;
  motorStartTime = millis();
  Serial.print("New Pump State: ");Serial.print(getStateName(pumpState));Serial.print("; Motor started at: ");Serial.println(motorStartTime);
}

void resetValves() {
  digitalWrite(VALVE_RELAY_PIN_1, HIGH);
  digitalWrite(VALVE_RELAY_PIN_2, HIGH);
  digitalWrite(VALVE_RELAY_PIN_3, HIGH);
  digitalWrite(VALVE_RELAY_PIN_4, HIGH);
}


void startWeightTest() {
  weightTestRunning = true;
  weightTestStartTime = millis();
  weightTestCount++;
  lastValidWeightPrev=lastValidWeight;
  Serial.print(" ; ; ; ; Start Weight test; ;");
  Serial.println(weightTestCount);
  weightTestIndex = 0;


  for (int i = 0; i < trials; i++) {
    if (i == 0) {
      steps[i] = weightTestLogBuffer;
      pauses[i] = steps[i] + weightTestStep;
    } else {
      steps[i] = pauses[i - 1] + weightTestPause;
    }
    pauses[i] = steps[i] + weightTestStep;
  }
  finishTest = pauses[trials - 1] + weightTestLogBuffer;

}


void weightTestLoop(pumpStates testType, String wLog, boolean continuous) {

  unsigned long duration = millis() - weightTestStartTime;

  if (weightTestIndex > trials * 2) {
    if (duration >= finishTest) {
      
      weightTestIndex = -1;
      weightTestRunning = false;
      Serial.print(wLog);
      Serial.print("Finish; "); Serial.print(duration);
      Serial.print("; ;"); Serial.println(weightTestStep);
      onWeightEpochFinish(testType);
    }
  } else if (((weightTestIndex % 2) == 0) && (duration >= pauses[(weightTestIndex / 2) - 1])) {

    weightTestIndex++;
    stopMotor();
    Serial.print(wLog); Serial.print((weightTestIndex / 2));
    Serial.print(". Pause; "); Serial.print(duration);
    Serial.print("; ;"); Serial.println(weightTestStep);

  } else if (((weightTestIndex % 2) != 0) && (duration >= steps[((weightTestIndex + 1) / 2) - 1])) {
    
    Serial.print(wLog);Serial.print(((weightTestIndex + 1) / 2));
    Serial.print(". Step; "); Serial.print(millis() - weightTestStartTime);
    Serial.print("; ; "); Serial.println(weightTestStep);
    weightTestIndex++;
    startPumping(testType);
    Serial.print(wLog); Serial.print(((weightTestIndex + 1) / 2));
    Serial.print(". Step; "); Serial.print(millis() - weightTestStartTime);
    Serial.print("; ; "); Serial.println(weightTestStep);

  } else if ((duration >= 0) && (weightTestIndex == 0)) {

    weightTestIndex++;
    Serial.print(wLog);
    Serial.print("Pre loop; "); Serial.print(duration);
    Serial.print("; ; "); Serial.println(weightTestStep);

  }
}

void onWeightEpochFinish(pumpStates prevState) {
  pumpStates newState;

  switch (prevState)
  {
    case LEFT_IN:
      newState = LEFT_OUT;
      weightTestStep = drainDur;
      break;
    case RIGHT_IN:
      newState = RIGHT_OUT;
      weightTestStep = drainDur;
      break;
    case BOTH_IN:
      newState = BOTH_OUT;
      weightTestStep = drainDur;
      break;
    case LEFT_OUT:
      newState = LEFT_IN;
      weightTestStep = fillDur;
      //setupWeightSensor();
      break;
    case RIGHT_OUT:
      newState = RIGHT_IN;
      weightTestStep = fillDur;
      //setupWeightSensor();
      break;
    case BOTH_OUT:
      newState = BOTH_IN;
      weightTestStep = fillDur;
      //setupWeightSensor();
      break;
    default:
      break;

  }

  testDirection = newState;
  Serial.print(";;;;;;;;Weight prev: ");Serial.print(lastValidWeightPrev);Serial.print(". Weight now: ");Serial.println(lastValidWeight);
  
  if (checkForOutage()==true){
    Serial.print(";;;;OUTAGE (after ");Serial.print(weightTestCount);Serial.println(" epochs)");
    }
  else if (weightTestCount<maxWeighTests){
    startWeightTest();
  } else {Serial.print(";;;;Weight test successful (");Serial.print(weightTestCount);Serial.println(" epochs)");}

}

boolean checkForOutage(){
  if (abs(lastValidWeight - lastValidWeightPrev)<outageFlag){
    return true;
  }
  return false;
  }

const char* getStateName(pumpStates state)
{
  switch (state)
  {
    case IDLEMODE: return "Idle; Idle";
    case RIGHT_IN: return "Right; In";
    case LEFT_IN: return "Left; In";
    case BOTH_IN: return "Both; In";
    case RIGHT_OUT: return "Right; Out";
    case LEFT_OUT: return "Left; Out";
    case BOTH_OUT: return "Both; Out";
  }
}