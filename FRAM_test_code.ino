#include <EEPROM.h>

#include "Adafruit_FRAM_I2C.h"
#include <Wire.h>



/* Example code for the Adafruit I2C FRAM breakout */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 5.0V DC
   Connect GROUND to common ground */
   
Adafruit_FRAM_I2C fram     = Adafruit_FRAM_I2C();
uint32_t memory_size();
int memory_write();
int memory_read();
int validate_memory();
int error_gen();

uint8_t data = 13;
long randNumber;
uint32_t max_memory;
int count=0;
uint8_t readData;

void setup() {
  Serial.begin(115200);
   if (fram.begin()) {  // you can stick the new i2c addr in here, e.g. begin(0x51);
    Serial.println("Found I2C FRAM");
  } else {
    Serial.println("I2C FRAM not identified ... check your connections?\r\n");
    Serial.println("Will continue in case this processor doesn't support repeated start\r\n");
    while (1);
  }
  max_memory=memory_size();
  //Serial.println(max_memory);
  validate_memory(max_memory);
  randomSeed(analogRead(A0));
}

void loop() {
 
  memory_write();
  
  uint8_t number_errors = error_gen();
  Serial.print("Number of errors introduced: ");
  Serial.println(number_errors);
  
  uint8_t number_errors_count = memory_read();

  Serial.print("Number of errors found: ");
  Serial.println(number_errors_count);
   
  data++;
  if(data==5000)
  {
    data=13; //to keep the data and error distinct 
  }
  
  delay(60000);

}

int memory_write()
{
  // Write data to FRAM
  for(uint16_t i=0; i<max_memory; i++)
  {
    fram.write(i, data); //Write data in memory address i
  }

  Serial.print("Written Data: ");
  Serial.println(data);

}


int memory_read()
{
  count=0;
  for(uint16_t j=0; j<max_memory; j++)
  {
    readData = fram.read(j); //Read data from memory address j
    if(readData!=data)
    {
      Serial.print("Corrupted Data: ");
      Serial.println(readData);
      Serial.print("at Address: ");
      Serial.println(j);
      count++;
    }
  }
  Serial.print("Read Data: ");
  Serial.println(readData);
  return count;
  
}


uint32_t memory_size()
{
  // Write the first byte to 0xAF
  uint8_t test = 0xAF;
  fram.write(0x0, test);


  uint32_t max_addr;
  for (max_addr = 1; max_addr < 0xFFFF; max_addr++) {
    if (fram.read(max_addr) != test)
      continue; // def didnt wrap around yet

    // maybe wraped? try writing the inverse
    if (! fram.write(max_addr, (byte)~test)) {
        Serial.print("Failed to write address 0x");
        Serial.println(max_addr, HEX);
    }

    // read address 0x0 again
    uint8_t val0 = fram.read(0);

    // re-write the old value
    if (! fram.write(max_addr, test)) {
        Serial.print("Failed to re-write address 0x");
        Serial.println(max_addr, HEX);
    }    

    // check if addr 0 was changed
    if (val0 == (byte)~test) {
      Serial.println("Found max address");
      break;
    }
  }
  Serial.print("This FRAM can store ");
  Serial.print(max_addr);
  Serial.println(" bytes");
  return max_addr;

}

int error_gen()
{
  randNumber = random(1000);
  
  
  for(uint8_t a=0; a<randNumber; a++)
  {
    uint16_t randaddr = random(32767);
    uint8_t randerror = random(5000,6000);
    fram.write(randaddr, randerror);
  }
  
  return randNumber;
}

int validate_memory(uint32_t max_add)
{
  // validate the memory
  Serial.println("Validating every address in memory");
  uint8_t val;
  for (uint16_t add = 0; add < max_add; add++) {
    if (add % 2048 == 0) {
      Serial.print("Testing address ");
      Serial.print(add);
      Serial.print(" to ");
      Serial.println(add+2047);
    }
    val = fram.read(add);
    
    fram.write(add, 0x55);
    if (fram.read(add) != 0x55) {
      Serial.print("Failed to write 0x55 to address 0x");
      Serial.println(add, HEX);
    }
    fram.write(add, 0xAA);
    if (fram.read(add) != 0xAA) {
      Serial.print("Failed to write 0xAA to address 0x");
      Serial.println(add, HEX);
    }
    
    fram.write(add, val);
    if (fram.read(add) != val) {
      Serial.print("Failed to write original value to address 0x");
      Serial.println(add, HEX);
    }
  }
  Serial.println("Validated!");
}
