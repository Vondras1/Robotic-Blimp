#ifndef SD_card_H
#define SD_card_H

#include "GeneralLib.h"
#include <SD.h>

const int chipSelect = 10;

extern File myFile;

void write_to_SD_card(double a, double b, double c, double dd, double f, double g, double h);

#endif // SD_card_H