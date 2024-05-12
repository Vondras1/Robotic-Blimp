#include "SD_card.h"

File myFile;

void write_to_SD_card(double a, double b, double c, double dd, double f, double g, double h){
  myFile = SD.open("data1.txt", FILE_WRITE);
  if (myFile) {
    myFile.println();
    myFile.println("Ultrasonic - Humidity - temp_DHC - temp_MPL - pressure - altitude - current");
    myFile.print(a);
    myFile.print(" - ");
    myFile.print(b);
    myFile.print(" - ");
    myFile.print(c);
    myFile.print(" - ");
    myFile.print(dd);
    myFile.print(" - ");
    myFile.print(f);
    myFile.print(" - ");
    myFile.print(g);
    myFile.print(" - ");
    myFile.println(h);
	  // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening file");
  }
}