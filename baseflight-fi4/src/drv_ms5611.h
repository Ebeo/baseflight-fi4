/*
 * drv_ms5611.h
 *
 *  Created on: 02.07.2012
 *      Author: Alexey Kolokolnikov
 */

#ifndef DRV_MS5611_H_
#define DRV_MS5611_H_

bool  ms5611Init(void);

void ms5611UT_Start(void);
void ms5611UP_Start(void);
void ms5611UP_Read(void);
void ms5611UT_Read(void);
int32_t ms5611Calculate(void);

#endif /* DRV_MS5611_H_ */
