//------------------------------------------------
// Funkcje obslugi wyswietlacza tekstowego LCD
// dotyczy ukladów zgodnych z opisem KS0066U
//------------------------------------------------

#include <avr/io.h>
#include <avr/pgmspace.h>		//obsluga pamieci FLASH programu-kody specjalne

#define F_CPU 16000000UL

#include <util/delay.h>				//opoznienia


//------------------------------------------
// Funkcje pomocnicze wyswietlacza LCD
//------------------------------------------

void lcd_init(void);	//inicjowanie wyświetlacza 


//czyszczenie LCD
void lcd_clr(void); 	
void lcd_gotoxy(char col, char row);//ustawienie kursora	
void lcd_puttxt(char*msg);	//wyświetlanie napisu
void lcd_putchar(char x); 	// wyświetlanie znaku z formatowaniem

void lcd_cursor_on(void);	// sterowanie kursorem
void lcd_cursor_off(void);	// sterowanie kursorem
void lcd_pol_character(void);	//definicja znaków polskich
void DispleyFromFlash( PGM_P  str );
