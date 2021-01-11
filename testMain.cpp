#include "si7021.h"

int main()
{
	bcm2835_init();
	SI7021 si7021;
	if(!si7021.begin())
	{
		printf("SI7021 init failed\n");
		exit(-1);
	}
	float humi = 0;
	float temp = 0;
	while(1)
	{
		humi = si7021.readHumi();
		temp = si7021.readTemp();
		printf("Humi = %f, temp = %f\n",humi,temp);
		bcm2835_delay(500);
	}
	return 0;
}

//g++ si7021.cpp testMain.cpp -lbcm2835 -o testMain
