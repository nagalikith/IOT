
//Pins A0, A1 and 12
int led_red = 26;
int led_green = 25;
int led_yellow = 32;

//The bigger the threshold, the more sensible is the touch
int threshold = 40;
//Touch 1,2,3 were glitching 
bool touch0detected = false;
bool touch6detected = false;

void gotTouch0(){
 touch0detected = true;
}

void gotTouch6(){
 touch6detected = true;
}

void setup() {
  //Initialize led lights
  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_yellow, OUTPUT);
  
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Interrupt Test");
  
  touchAttachInterrupt(T6, gotTouch6, threshold);
  touchAttachInterrupt(T0, gotTouch0, threshold);
}

void loop(){
  if(touch6detected){
    touch6detected = false;
    Serial.println("Touch 6 detected");
    Serial.printf("setting Yellow Light LOW...\n");
    digitalWrite(led_yellow, LOW);        
    
    
    Serial.printf("setting Red Light LOW...\n");
    digitalWrite(led_red, LOW);        

    Serial.printf("setting Green Light HIGH...\n");
    digitalWrite(led_green, HIGH);        
  }
  if(touch0detected){
    touch0detected = false;
    Serial.println("Touch 0 detected");
    Serial.printf("setting Yellow Light LOW...\n");
    digitalWrite(led_yellow, LOW);  
    
    
    Serial.printf("setting 32 HIGH...\n");
    digitalWrite(led_red, HIGH);       

    Serial.printf("setting Green Light Low...\n");
    digitalWrite(led_green, LOW);        
  }

  if((touchRead(T0)< threshold) && (touchRead(T6)< threshold)){
    Serial.println("Touch 6 & 0 simultaneously detected");
    
    Serial.printf("setting Red Light LOW...\n");
    digitalWrite(led_red, LOW);        
    Serial.printf("setting Green Light LOW...\n");
    digitalWrite(led_green, LOW);        

    Serial.printf("setting Yellow Light HIGH...\n");
    digitalWrite(led_yellow, HIGH);
    delay(2000);  
  }
}
