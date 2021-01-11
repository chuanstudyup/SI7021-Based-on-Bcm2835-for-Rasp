#include "si7021.h"

SI7021::SI7021(uint8_t addr)
{
	_addr = addr;
	sernum_a = sernum_b = 0;
	_model = SI_7021;
	_revision = 0;
}

/**
 * @brief Before SI7021::init(), please bcm2835_init() in main.c!
 **/
bool SI7021::begin()
{
	printf("SI7021 Init...\n");
	if(!bcm2835_i2c_begin())
	{
		printf("bcm2835_i2c_begin failed at %s:%d\n",__FILE__ ,__LINE__);
		return false;
	}
	bcm2835_i2c_setSlaveAddress(_addr);
	bcm2835_i2c_set_baudrate(100000);
	
	if (_readRegister8(SI7021_READRHT_REG_CMD) != 0x3A)
		return false;
	_readSerialNumber();
	_readRevision();
	return true;
}

uint8_t SI7021::_readRegister8(uint8_t reg)
{
	char value;
	sendBuf[0] = reg;
	if((errCode = bcm2835_i2c_write_read_rs(sendBuf,1,&value,1)))
	{
		printf("bcm2835_i2c_write_read_rs failed at%s:%d, errCode = 0x%x\n",__FILE__ ,__LINE__, errCode);
		return 0;
	}
	//printf("value = %d\n", value);
	return static_cast<uint8_t>(value);
}

void SI7021::_readSerialNumber()
{
	sendBuf[0] = static_cast<uint8_t>(SI7021_ID1_CMD >> 8);
	sendBuf[1] = static_cast<uint8_t>(SI7021_ID1_CMD & 0xFF);
	char recvBuf[8];
	if((errCode = bcm2835_i2c_write_read_rs(sendBuf,2,recvBuf,8)))
	{
		printf("bcm2835_i2c_write_read_rs failed at %s:%d, errCode = 0x%x\n", __FILE__, __LINE__, errCode);
		return;
	}
	sernum_a = (recvBuf[0]<<24)|(recvBuf[2]<<16)|(recvBuf[4]<<8)|(recvBuf[6]);
	//printf("sernum_a = %d\n", sernum_a);
	
	sendBuf[0] = static_cast<uint8_t>(SI7021_ID2_CMD >> 8);
	sendBuf[1] = static_cast<uint8_t>(SI7021_ID2_CMD & 0xFF);
	if((errCode = bcm2835_i2c_write_read_rs(sendBuf,2,recvBuf,8)))
	{
		printf("bcm2835_i2c_write_read_rs failed at %s:%d, errCode = 0x%x\n", __FILE__, __LINE__, errCode);
		return;
	}
	sernum_b = (recvBuf[0]<<24)|(recvBuf[2]<<16)|(recvBuf[4]<<8)|(recvBuf[6]);
	//printf("sernum_b = %d\n", sernum_b);
	
	switch (sernum_b >> 24) {
		case 0:
		case 0xff:
			_model = SI_Engineering_Samples;
		break;
		case 0x0D:
			_model = SI_7013;
		break;
		case 0x14:
			_model = SI_7020;
		break;
		case 0x15:
			_model = SI_7021;
		break;
		default:
			_model = SI_UNKNOWN;
		break;
	}
	//printf("_model = %d\n",_model);
}

void SI7021::_readRevision()
{
	sendBuf[0] = static_cast<uint8_t>(SI7021_FIRMVERS_CMD >> 8);
	sendBuf[1] = static_cast<uint8_t>(SI7021_FIRMVERS_CMD & 0xFF);
	char recvBuf[2];
	if((errCode = bcm2835_i2c_write_read_rs(sendBuf,2,recvBuf,2)))
	{
		printf("bcm2835_i2c_write_read_rs failed at %s:%d, errCode = 0x%x\n",__FILE__ ,__LINE__, errCode);
		return;
	}

	if(recvBuf[0] == SI7021_REV_1)
		_revision = 1;
	else if(recvBuf[0] == SI7021_REV_2)
		_revision = 2;
	else
		_revision = 0;
	//printf("_revision = %d\n",_revision);
}
/**
 * @brief Reset the SI7021 device, it takes almost 50 ms
 */
void SI7021::reset()
{
	bcm2835_i2c_setSlaveAddress(_addr);
	sendBuf[0] = SI7021_RESET_CMD;
	if((errCode = bcm2835_i2c_write(sendBuf,1)))
	{
		printf("bcm2835_i2c_write failed at %s:%d, errCode = 0x%x\n", __FILE__, __LINE__ ,errCode);
		return;
	}
	bcm2835_delay(50);
}

/**
 * @brief  Reads the humidity value from Si7021 (No Master hold), the function takes almost 20ms.
 * @return Returns humidity as float value or -1 when error
 */
float SI7021::readHumi()
{
	bcm2835_i2c_setSlaveAddress(_addr);
	sendBuf[0] = SI7021_MEASRH_NOHOLD_CMD;
	if((errCode = bcm2835_i2c_write(sendBuf,1)))
	{
		printf("bcm2835_i2c_write failed at %s:%d, errCode = 0x%x\n", __FILE__, __LINE__ ,errCode);
		return -1;
	}
	
	bcm2835_delay(20);
	
	char recvBuf[3];
	if((errCode = bcm2835_i2c_read(recvBuf,3)))
	{
		printf("bcm2835_i2c_read failed at %s:%d, errCode = 0x%x\n", __FILE__, __LINE__ ,errCode);
		return -1;
	}
	float humi = recvBuf[0]<<8 | recvBuf[1];
	humi *= 125;
	humi /= 65536;
	humi -= 6;
	return humi > 100.0 ? 100.0 : humi;
}

/**
 * @brief  Reads the temperature value from Si7021 (Read temperature value from previous RH measurement)
 * @return Returns temperature as float value or -1 when there is error
 */
float SI7021::readTemp()
{
	bcm2835_i2c_setSlaveAddress(_addr);
	sendBuf[0] = SI7021_READPREVTEMP_CMD;
	char recvBuf[2];
	if((errCode = bcm2835_i2c_write_read_rs(sendBuf,1,recvBuf,2)))
	{
		printf("bcm2835_i2c_write_read_rs failed at %s:%d, errCode = 0x%x\n", __FILE__, __LINE__ ,errCode);
		return -1;
	}
	float temp = recvBuf[0]<<8 | recvBuf[1];
	temp *= 175.72;
	temp /= 65536;
	temp -= 46.85;
	return temp;
}
