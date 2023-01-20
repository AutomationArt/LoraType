/*This Code applicable to e-ink(1.54/2.13/2.90) driven by HelTec AutoMation WiFi_Kit_Series Development Board.
 *Simple e-ink Display example
 *1.54-BLACK AND WHITE SCREEN(With a BLUE label.)--200 x 200 IMAGE_DATA 1,2
 *2.13-BLACK , WHITE AND RED SCREEN--104 x 212 IMAGE_RED IMAGE_BLACK
 *2.90-BLACK AND WHITE SCREEN--128 x 296 IMAGE 1,2,3,4,5
 *AND SO ON.
 *
 *Please put the modeled array in the "imagedata.h" and "imagedata.cpp" files
 *In the "Display ()" function in the "select.h" file, modify the array name of the modular array of the corresponding size screen
 *Please modify the macro of the screen size  in the "e_ink.h" file in the src folder
 */


#include "select.h"

void setup() 
{
  Serial.begin(115200);
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH);
  epd.Init(lut_full_update);

  /* If you can't refresh normally, please comment out and try again */
  Display_clear(); 

  /* The preprocessing function of screen is defined. */
  Display(); 
}

void loop() 
{
    
}




