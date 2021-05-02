# COM3505 Lab 1

Constructed a breadboard based IoT device which is capable of sensing touch and incidicating different tocuhes with blinking different LED'sensing

# Functions
1. Touch the Red wire to switch on the red LED light
2. Touch the Green wire to switch on the Green LED light
3. Mixing Red and Green we get Yellow. Therefore touching both the wires switches on Yellow led light

# Code Review
Used two inbulit methods from the arduino IDE. touchRead() and touchAttachInterrupt()

```C++
  touchAttachInterrupt(T6, gotTouch6, threshold);
  touchAttachInterrupt(T0, gotTouch0, threshold);
```

```C++
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
```

# Hardware Layout

![Image 1](/images/img_1.jpg)
