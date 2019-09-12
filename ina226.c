#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <wiringPiI2C.h>
#include "ina226.h"

#define INA226_ADDRESS 0x40

int fd;
float current_lsb;

uint16_t read16(int fd, uint8_t ad){
	uint16_t result = wiringPiI2CReadReg16(fd,ad);
	// Chip uses different endian
	return  (result<<8) | (result>>8);
}

void write16(int fd, uint8_t ad, uint16_t value){
	// Chip uses different endian
	wiringPiI2CWriteReg16(fd,ad,(value<<8)|(value>>8));
}

// R of shunt resistor in ohm. Max current in Amp
void ina226_calibrate(float r_shunt, float max_current)
{
	current_lsb = max_current / (1 << 15);
	float calib = 0.00512 / (current_lsb * r_shunt);
	uint16_t calib_reg = (uint16_t) floorf(calib);
	current_lsb = 0.00512 / (r_shunt * calib_reg);

	//printf("LSB %f\n",current_lsb);
	//printf("Calib %f\n",calib);
	//printf("Calib R%#06x / %d\n",calib_reg,calib_reg);

	write16(fd,INA226_REG_CALIBRATION, calib_reg);
}

void ina226_configure(uint8_t bus, uint8_t shunt, uint8_t average, uint8_t mode)
{
    write16(fd,INA226_REG_CONFIGURATION, (average<<9) | (bus<<6) | (shunt<<3) | mode);
}

uint16_t ina226_conversion_ready()
{
    return read16(fd,INA226_REG_MASK_ENABLE) & INA226_MASK_ENABLE_CVRF;
}

void ina226_read(float *voltage, float *current, float *power, float* shunt_voltage)
{
    uint16_t voltage_reg;
    int16_t current_reg;
    int16_t power_reg;
    int16_t shunt_voltage_reg;

    voltage_reg = read16(fd,INA226_REG_BUS_VOLTAGE);
    current_reg = (int16_t) read16(fd,INA226_REG_CURRENT);
    power_reg = (int16_t) read16(fd,INA226_REG_POWER);
    shunt_voltage_reg = (int16_t) read16(fd,INA226_REG_SHUNT_VOLTAGE);

    //printf("%u, %u, %u, %u, ",voltage_reg,current_reg,power_reg,shunt_voltage_reg);

    if (voltage)
        *voltage = (float) voltage_reg * 1.25e-3;

    if (current)
        *current = (float) current_reg * 1000.0 * current_lsb;

    if (power)
        *power = (float) power_reg * 25000.0 * current_lsb;

    if (shunt_voltage)
        *shunt_voltage = (float) shunt_voltage_reg * 2.5e-3;
}

inline void ina226_reset()
{
    write16(fd, INA226_REG_CONFIGURATION, INA226_RESET);
}

inline void ina226_disable()
{
    write16(fd, INA226_REG_CONFIGURATION, INA226_MODE_OFF);
}

int main() {
	fd = wiringPiI2CSetup(INA226_ADDRESS);
	if(fd < 0) {
		printf("Device not found");
		return -1;
	}

	//uint16_t manufacturer = read16(fd,INA226_REG_MANUFACTURER);
	//uint16_t chip = read16(fd,INA226_REG_DIE_ID);
	//printf("Manufacturer %X Chip %X\n",manufacturer,chip);

	// BUS / SHUNT / Averages / Mode
	ina226_configure(INA226_TIME_8MS, INA226_TIME_8MS, INA226_AVERAGES_16, INA226_MODE_SHUNT_BUS_CONTINUOUS);

	// Shunt resistor (Ohm), Max Current (Amp)
	ina226_calibrate(0.1, 1.0);

	printf("date,time,timestamp,bus voltage(V),current (mA),power (mW),shunt voltage (mV),energy (kWh year)\n");

	for(;;){
		float voltage,current,power,shunt,energy;

		ina226_read(&voltage, &current, &power, &shunt);

		energy = voltage*current*24*365.25/1000000;

                // Timestamp / Date
                time_t rawtime;
                time(&rawtime);
                struct tm *info = localtime( &rawtime );
                char buffer[80];
                strftime(buffer,80,"%Y-%m-%d,%H:%M:%S", info);

		printf("%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f\n",buffer,(int)rawtime,voltage,current,voltage*current,shunt,energy);
		fflush(NULL);

		//usleep(100000);
		sleep(1);
	}

	ina226_disable();

	return 0;
}
