/*
 * sensor_Read.h
 *
 *  Created on: Dec 3, 2016
 *      Author: AaronEwing
 */

#ifndef SENSOR_READ_H_
#define SENSOR_READ_H_

void init_i2c (void);

uint32_t read_Sensor (uint8_t slaveAddress, uint8_t registerAddress);

#endif /* SENSOR_READ_H_ */
