/** @file handler.h
 *
 * @brief Declares the IRQ and SWI handlers
 *
 * @author Stephen Greco <sgreco@andrew.cmu.edu>
 * @author Ramsey Natour <rnatour@andrew.cmu.edu>
 * @date   2014-12-4 
 */


void irq_handler(void); 

int swi_handler(int);
