/*
 * dz.h
 *
 *  Created on: 01.06.2011
 *      Author: kolja
 */

#ifndef DZ_H_
#define DZ_H_

#define FAST_SPEED 600
#define FAST_ACCEL 1000

/**************** DZ *******************/
void helper();
class speedo_dz{
public:
	speedo_dz(void);
	~speedo_dz(void);
	void counter();
	void calc();
	void init();
	void clear_vars();
	bool check_vars();

	unsigned int rounded;                 // to show on display, rounded by 50
	unsigned int exact;                 // real rotation speed
	unsigned int exact_disp;
	unsigned int dz_flat;
	unsigned char dz_faktor_counter;
	unsigned char dz_disp_faktor_counter;
	int blitz_dz;
	bool blitz_en;
private:
	volatile unsigned int peak_count; // max 64k => bei 15krpm sind das 256 sek ... sollte reichen
	unsigned long previous_time;
	unsigned int previous_peaks;		// z�hlerstand vom peak
	unsigned int previous_dz;
	volatile unsigned int peak_helper;
};
/**************** DZ *******************/

#endif /* DZ_H_ */