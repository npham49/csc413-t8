// Example sketch to read the ID from an Addicore 13.56MHz RFID tag
// as found in the RFID AddiKit found at: 
// http://www.addicore.com/RFID-AddiKit-with-RC522-MIFARE-Module-RFID-Cards-p/126.htm

#include <AddicoreRFID.h>
#include <SPI.h>
#include <Servo.h>

//Includes the Arduino Stepper Library
#include <Stepper.h>

#define	uchar	unsigned char
#define	uint	unsigned int

uchar fifobytes;
uchar fifoValue;

// Defines the number of steps per rotation
const int stepsPerRevolution = 2048;

AddicoreRFID myRFID; // create AddicoreRFID object to control the RFID module

/////////////////////////////////////////////////////////////////////
//set the pins
/////////////////////////////////////////////////////////////////////
const int chipSelectPin = 10;
const int NRSTPD = 5;

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, 6, 7, 8, 9);

const int LED_RED = 4;
const int LED_GREEN = 2;
Servo lockServo;    // Servo for locking mechanism
int lockPos = 15;   // Locked position limit
int unlockPos = 75; // Unlocked position limit
boolean locked = true;

//Maximum length of the array
#define MAX_LEN 16

void setup() {                
   Serial.begin(9600);                        // RFID reader SOUT pin connected to Serial RX pin at 9600bps 
 
  // start the SPI library:
  SPI.begin();
  
  pinMode(chipSelectPin,OUTPUT);              // Set digital pin 10 as OUTPUT to connect it to the RFID /ENABLE pin 
    digitalWrite(chipSelectPin, LOW);         // Activate the RFID reader
  pinMode(NRSTPD,OUTPUT);                     // Set digital pin 10 , Not Reset and Power-down
    digitalWrite(NRSTPD, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  myRFID.AddicoreRFID_Init();
  myStepper.setSpeed(5);

  lockServo.attach(3); // Servo connected to pin 3
  lockServo.write(lockPos);
}

void loop()
{
  	uchar i, tmp, checksum1;
	uchar status;
        uchar str[MAX_LEN];
        uchar RC_size;
        uchar blockAddr;	//Selection operation block address 0 to 63
        String mynum = "";

        str[1] = 0x4400;
	//Find tags, return tag type
	status = myRFID.AddicoreRFID_Request(PICC_REQIDL, str);	
	if (status == MI_OK)
	{
          Serial.println("RFID tag detected");
          Serial.print("Tag Type:\t\t");
          uint tagType = str[0] << 8;
          tagType = tagType + str[1];
          switch (tagType) {
            case 0x4400:
              Serial.println("Mifare UltraLight");
              break;
            case 0x400:
              Serial.println("Mifare One (S50)");
              break;
            case 0x200:
              Serial.println("Mifare One (S70)");
              break;
            case 0x800:
              Serial.println("Mifare Pro (X)");
              break;
            case 0x4403:
              Serial.println("Mifare DESFire");
              break;
            default:
              Serial.println("Unknown");
              break;
          }
	}

	//Anti-collision, return tag serial number 4 bytes
	status = myRFID.AddicoreRFID_Anticoll(str);
	if (status == MI_OK)
	{
          checksum1 = str[0] ^ str[1] ^ str[2] ^ str[3];
          Serial.print("The tag's number is:\t");
    	    Serial.print(str[0]);
            Serial.print(" , ");
    	    Serial.print(str[1]);
            Serial.print(" , ");
    	    Serial.print(str[2]);
            Serial.print(" , ");
    	    Serial.println(str[3]);

          Serial.print("Read Checksum:\t\t");
    	      Serial.println(str[4]);
          Serial.print("Calculated Checksum:\t");
            Serial.println(checksum1);
            Serial.println(str[0]);
            
            // Should really check all pairs, but for now we'll just use the first
            if(str[0] == 114)                      //You can change this to the first byte of your tag by finding the card's ID through the Serial Monitor
            {
                Serial.println("\nWelcome!!\n");
                digitalWrite(LED_GREEN, HIGH);  // turn the LED on (HIGH is the voltage level)
                delay(2000);                      // wait for a second
                digitalWrite(LED_GREEN, LOW);   // turn the LED off by making the voltage LOW
                delay(1000);
                if (locked == true) // If the lock is closed then open it
                {
                  Serial.println("Opening...\n");
                  lockServo.write(unlockPos);
                  delay(1000);
                  // Rotate CW slowly at 5 RPM
                  myStepper.step(stepsPerRevolution);
                  locked = false;
                  Serial.println("Finish opening...\n");
                } 
            } else {             //You can change this to the first byte of your tag by finding the card's ID through the Serial Monitor
                Serial.println("\nNo entry!\n");
                digitalWrite(LED_RED, HIGH);  // turn the LED on (HIGH is the voltage level)
                delay(2000);                      // wait for a second
                digitalWrite(LED_RED, LOW);   // turn the LED off by making the voltage LOW
                delay(1000);
            }
            Serial.println();
            delay(1000);
	}
		
        myRFID.AddicoreRFID_Halt();		   //Command tag into hibernation              

}

