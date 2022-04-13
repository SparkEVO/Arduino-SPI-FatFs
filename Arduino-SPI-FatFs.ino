#include "src/ff.h"
#include "src/Adafruit_SPIFlash/src/Adafruit_SPIFlash.h"

#define Serial SerialUSB

#define ERASE_MEMORY
#define FS_TYPE FM_FAT
#define MAX_LENGTH 100

Adafruit_FlashTransport_SPI flashTransport(CHIP_SELECT, SPI);
Adafruit_SPIFlash flash(&flashTransport);

extern "C" void* ff_memalloc (UINT msize) { return malloc( msize ); }
extern "C" void ff_memfree (void* mblock) { free( mblock ); }
FATFS fs;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  
  Serial.println("Flash test begin with attributes:");
  Serial.print  ("CHIP_SELECT ");
  Serial.println(CHIP_SELECT);
  Serial.print  ("MAX_FREQ ");
  Serial.println(MAX_FREQ);

#ifdef ERASE_MEMORY
  Serial.println(F("Erasing memory..."));
  flashTransport.setClockSpeed(MAX_FREQ, MAX_FREQ);
  flash.begin();
  
  flash.eraseChip();
  flash.waitUntilReady();
  Serial.println(F("Erased."));
#endif 

  FRESULT res = f_mount(&fs, "", 1); 
  if (res == FR_NO_FILESYSTEM) {
    Serial.println(F("Init FAT file system..."));
    BYTE work[FF_MAX_SS];    
    MKFS_PARM p = {FS_TYPE, 1, 0, 0, FF_MAX_SS};
    uint8_t mkres = f_mkfs("", &p, work, sizeof work);
    if (mkres != 0) {
      Serial.print(F("File system failure with code "));
      Serial.println(mkres);
      return;
    }

    Serial.println(F("File system ok"));
    res = f_mount(&fs, "", 1);
  }
  
  if (res != FR_OK) {
    Serial.println(F("Cannot mount fs: "));
      Serial.println(res);
    return;
  }
  Serial.println(F("Begin ok!"));

  showStats();

  // test writing speed
  Serial.println(F("Test writing speed..."));
  FIL wtestfile;
  res = f_open(&wtestfile, "WTEST", FA_WRITE | FA_CREATE_ALWAYS);
  if (res != FR_OK) {
    Serial.print(F("Error opening file for WRITE: "));
    Serial.println(res);
    return;
  }
  
  int wres;
  /* // simple write/read test
  char teststr[50] = "this is\n a test!";
  wres = f_puts(teststr, &wtestfile);
  f_close(&wtestfile);
  char buff[50] = "";
  res = f_open(&wtestfile, "WTEST", FA_READ);
  if (res != FR_OK) {
    Serial.print(F("Error opening file for READ: "));
    Serial.println(res);
    return;
  }
  
  f_read(&wtestfile, buff, 50, NULL);
  for (size_t i = 0; i < 50; i++) { (int)Serial.print(teststr[i]); Serial.print(" "); }
  Serial.println();
  for (size_t i = 0; i < 50; i++) { (int)Serial.print(buff[i]); Serial.print(" "); }
  Serial.println();
  f_close(&wtestfile);

  res = f_open(&wtestfile, "WTEST", FA_WRITE);
  if (res != FR_OK) {
    Serial.print(F("Error opening file for WRITE: "));
    Serial.println(res);
    return;
  }
  */
 
  unsigned int means[MAX_LENGTH];
  for (size_t l = 1; l <= MAX_LENGTH; l++)
  {
    char* str = (char*)malloc(sizeof(char) * l + 1);
    for (size_t i = 0; i < l; i++) str[i] = 'A';
    str[l] = 0;

    means[l-1] = 0;
    unsigned int c = 0;
    while(c < 5)
    {
      unsigned long t = millis();
      wres = f_puts(str, &wtestfile);
      f_sync(&wtestfile);
      means[l-1] += (millis() - t);
      if (wres < 0) {
        Serial.print(F("Error writing to test file: "));
        Serial.println(wres);
        showStats();
        return;
      }

      c++;
    }

    Serial.println();
    Serial.print(MAX_FREQ);
    Serial.print(",");
    Serial.print(l);
    Serial.print(",");
    Serial.print(means[l-1] / c);
    
    free(str);
  }
/*
  char buff[32];
  UINT rd = 0;
  Serial.println("Previous file:");
  f_lseek(&wtestfile, 0);
  while (rd > 0) {
    wres = f_read(&wtestfile, buff, 32, &rd);
    Serial.print(rd);
    Serial.print(" ");
    Serial.println(buff);
  }

  while(1)
  {
    delay(100);
    t = millis();
    wres = f_printf(&wtestfile, "%d", t);
    f_sync(&wtestfile);
    if (wres < 0) {
      Serial.print(F("Error writing to test file: "));
      Serial.println(wres);
      return;
    }
    
    Serial.println(t);
  }*/
}

void showStats() {
  // Show capacity and free space of SD card
  FATFS* fsp = &fs;
  DWORD fre_clust, fre_sect, tot_sect;
  f_getfree("", &fre_clust, &fsp);
  Serial.print(F("Sectors per cluster: "));
  Serial.println(fsp->csize);
  tot_sect = (fsp->n_fatent - 2) * fsp->csize;
  Serial.print(F("Total sectors: "));
  Serial.println(tot_sect);
  fre_sect = fre_clust * fsp->csize;
  Serial.print(F("Free sectors: "));
  Serial.println(fre_sect);
  Serial.print(F("Total drive space: "));
  Serial.println(tot_sect * SECTOR_SIZE);
  Serial.print(F("Available drive space: "));
  Serial.println(fre_sect * SECTOR_SIZE);
}

void loop() {
  
}
