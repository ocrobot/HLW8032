#include "HLW8032.h"

HLW8032::HLW8032()
{
}

void HLW8032::begin(HardwareSerial& SerialData)
{
	 SerialID = &SerialData; 
	 SerialID->begin(4800,SERIAL_8E1);   //指定4800波特率，偶校验  符号为->指针调用
}

void HLW8032::SerialReadLoop()
{
	if (SerialID->available()>0)   //检查串口是否有数据，并且缓冲区是否可用
	{
		
		SeriaDataLen = SerialID->available();
		if(SeriaDataLen < 24)   //缓存数据不足则跳出
		{
			return;
		}
		for (int a = 0; a < SeriaDataLen; a++)  //获取所有字节数
		{
			SerialTemps[a] =  SerialID->read();
		}
		
		/*处理字节*/
		
		if(SerialTemps[2] != 0x5A)  //标记识别,如果不是就抛弃
		{
			return;
		}
		if(Checksum() == false)   // 校验测试，如果错误就抛弃
		{
			return;
		}
		
		//如果通过了以上测试，则说明数据包应该没问题，获取其中的数据
		
		VolPar = ((uint32_t)SerialTemps[3]  <<16) + ((uint32_t)SerialTemps[4] <<8) + SerialTemps[5]; //获取电压参数寄存器
		if(bitRead(SerialTemps[20], 6) == 1)  //如果电压寄存器刷新，则取数据
		{
			VolData = ((uint32_t)SerialTemps[6]  <<16) + ((uint32_t)SerialTemps[7] <<8) + SerialTemps[8]; //获取电压寄存器
		}
		CurrentPar = ((uint32_t)SerialTemps[9]  <<16) + ((uint32_t)SerialTemps[10] <<8) + SerialTemps[11];  //电流参数 
		if(bitRead(SerialTemps[20], 5) == 1)   //如果电流寄存器更新，则取数据
		{
			CurrentData = ((uint32_t)SerialTemps[12]  <<16) + ((uint32_t)SerialTemps[13] <<8) + SerialTemps[14];  //电流
		}
		PowerPar = ((uint32_t)SerialTemps[15]  <<16) + ((uint32_t)SerialTemps[16] <<8) + SerialTemps[17];   // 功率参数
		if(bitRead(SerialTemps[20], 4) == 1)   // 如果功率寄存器数据更新，则取数据
		{
			PowerData = ((uint32_t)SerialTemps[18]  <<16) + ((uint32_t)SerialTemps[19] <<8) + SerialTemps[20];    //功率数据
		}
		PF = ((uint32_t)SerialTemps[22] <<8) + SerialTemps[23];   //脉冲数量寄存器       
		
		// 确认 PF进位寄存器是否进位，进位则添加1
		if(bitRead(SerialTemps[20], 7) == 1)
		{
			PFData++;
		}		
	}
}
// 获取电压
float HLW8032::GetVol()
{
	VF = VolR1 / VolR2 ;   //求电压系数
	float Vol = (VolPar / VolData) * VF;   //求电压有效值
	return Vol;
} 

//获取有效电流
float HLW8032::GetCurrent()
{
	CF = 1.0 / (CurrentRF *1000.0);    //计算电流系数
	float Current = (CurrentPar / CurrentData) * CF;    //计算有效电流
	return Current;
}
//计算有功功率
float HLW8032::GetActivePower()
{
	float Power = (PowerPar/PowerData) * VF * CF;  // 求有功功率
	return Power;
}

//计算视在功率
float HLW8032::GetInspectingPower()
{
	float vol = GetVol();
	float current = GetCurrent();
	return vol* current;
}

//计算功率因数
float HLW8032::GetPowerFactor()
{
	float ActivePower = GetActivePower();   //获取有功功率
	float InspectingPower = GetInspectingPower(); //视在功率
	return ActivePower / InspectingPower ;  
}


//获取脉冲计数器值
uint16_t HLW8032::GetPF()
{
	return PF;
}


//获取总脉冲数
uint32_t HLW8032::GetPFAll()
{
	return PFData * PF;
}


//获取累积电量
float HLW8032::GetKWh()
{
	float InspectingPower = GetInspectingPower(); //视在功率
	uint32_t PFcnt = (1/PowerPar) *(1/InspectingPower) * 1000000000 * 3600   //一度电的脉冲数量
	float KWh = (PFData * PF) / PFcnt;  //总脉冲除以1度电的脉冲量
	return KWh;

}


//校验测试
bool HLW8032::Checksum()
{
	byte check = 0;
	for(byte a = 2;a<=23;a++)
	{
		check = check + SerialTemps[a];
	}
	if (check  == SerialTemps[24])
	{
		//校验通过
		return true;
	}
	else
	{
		return false;  //校验不通过
	}
}