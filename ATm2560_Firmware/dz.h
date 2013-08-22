/*
 * dz.h
 *
 *  Created on: 01.06.2011
 *      Author: kolja
 */

#ifndef DZ_H_
#define DZ_H_

#define FAST_SPEED 500
#define FAST_ACCEL 900


/**************** DZ *******************/
void helper();
class speedo_dz{
public:
	speedo_dz(void);
	~speedo_dz(void);
	void counter();
	bool calc(bool force_calc);
	void init();
	int check_vars();
	unsigned int get_dz(bool exact_dz);
	void shutdown();


	unsigned char dz_faktor_counter;
	int blitz_dz;
	bool blitz_en;
private:
	unsigned int exact;                 // real rotation speed
	unsigned long calc_previous_time;
	unsigned int previous_dz;
	volatile unsigned int peak_count; // max 64k => bei 15krpm sind das 256 sek ... sollte reichen
	volatile unsigned long peak_last_reception_timestamp;
};
/**************** DZ *******************/

#endif /* DZ_H_ */
