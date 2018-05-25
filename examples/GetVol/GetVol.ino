#include "HLW8032.h"
//mega2560 examples
HLW8032 HL;

void setup()
{
	HL.begin(Serial1);   //使用硬件串口1链接HLW8032
	Serial.begin(9600);
}

void loop()
{
	HL.SerialReadLoop();    //串口循环业务，尽可能快的执行，不应小于50ms/周期
	Serial.println(HL.GetVol());
}