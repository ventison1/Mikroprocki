//***********************************************
// Funkcje obslugi tekstowego wyswietlacza LCD 
// Dotyczy ukladów zgodnych ze sterownikiem: 
//             S 6 A 0 0 6 9  !!!!
// 2015 A.W. 
//***********************************************
//#include "defs.h"

#include "Lcd_Fun.h"

//--------------------------------------------------
// Definicja portow procesora i trybu pracy LCD
// -dla interfejsu 4-bitowego zakladamy: starsza czesc LCD_DATA_PORT
//  jet podlaczona do D4-D7 LCD 

#define INTERFACE_8_BITS	0	//1	-dla interfejsu 8-bitowego
								//0	-dla interfejsu 4-bitowego

#define LCD_DATA_PORT	PORTC	//port danych
#define LCD_DATA_DDR	DDRC	//rejestr kierunku portu danych
#define LCD_CTR_PORT	PORTC	//port sterowania
#define LCD_CTR_DDR		DDRC	//rejestr kierunku portu sterowania

#define LCD_PIN_E		PC2		//piny sterujace
#define LCD_PIN_RS		PC0

//--------------------------------------------------
//Pomocnicze definicje ustawiajace sygnaly 	sterowania					
#define LCD_SET_E 	(LCD_CTR_PORT |=  (1<<LCD_PIN_E))
#define LCD_CLR_E   (LCD_CTR_PORT &= ~(1<<LCD_PIN_E))

#define LCD_SET_RS  (LCD_CTR_PORT |=  (1<<LCD_PIN_RS))
#define LCD_CLR_RS  (LCD_CTR_PORT &= ~(1<<LCD_PIN_RS))

//------------------------------------------------
// Generacja impulsu strobującego E
// Uwagi czasowe:
//   -min czas pomiedzy kolejnymi cyklami E - 500ns
//   -min dł. impulsu E - 230 ns
//   -min czas od ustalenia RS do zbocza narastającego E - 40ns
//	 -min czas od danych do zbocza opadajacego  E - 80ns
// Dla CPU 16 MHz  - czas wykonania instrukcji -62 ns

static void lcd_wr(char dd)	// cykl zapisu do LCD 
{
	asm volatile ("nop ");
	LCD_SET_E;				// E stan wysoki

#if INTERFACE_8_BITS
	LCD_DATA_PORT = dd;	// ustawienie danej
	asm volatile ("nop ");	//_delay_us(1);
	asm volatile ("nop ");	//_delay_us(1);
#else
	// ustawienie starszej częsci portu danych przy zachowaniu 
	// stanu mlodszej czesci w asemblerze ok 6 instrukcji
	LCD_DATA_PORT= (LCD_DATA_PORT & 0x0F) | dd;	
#endif	
	asm volatile ("nop ");	//_delay_us(1);
	LCD_CLR_E;				// opadajace zbocze na E -> zapis do wyswietlacza
	asm volatile ("nop ");  //_delay_us(1);
}

//------------------------------------------------
static void lcd_wr_data(unsigned char data)//cykl zapisu danej do LCD
{
	LCD_SET_RS;				//operacja RAM
	
#if INTERFACE_8_BITS
	lcd_wr(data);
#else
	lcd_wr( data &0xF0);		// zapis MSB 
	lcd_wr( (data<<4)&0xF0);	// zapis LSB 
#endif
	_delay_us(60);
}

//------------------------------------------------
/*static */ void lcd_wr_command(char data) //cykl zapisu komendy do LCD
{
	LCD_CLR_RS;				//komenda
#if INTERFACE_8_BITS
	lcd_wr(data);	
#else
	lcd_wr( data &0xF0);			// zapis MSB
	lcd_wr( (data<<4)&0xF0);		// zapis LSB
#endif
	_delay_us(60);
}

//------------------------------------------------
void lcd_clr(void) 		//czyszczenie LCD 
 {
	lcd_wr_command(0x01); 
	_delay_ms(2);		//wymagane dodatkowe opoznienie
 }

//------------------------------------------------
void lcd_init(void)	//inicjowanie LCD
 {

 	//ustawienie na wyjscie portu dla lini sterujacych 
	LCD_CTR_DDR |= ((1<<LCD_PIN_E)|(1<<LCD_PIN_RS));		//piny sterujące dla ZL10AVR
	//ustawienie na wyjscie portu linii danych 
	LCD_DATA_DDR =0xFF;
	LCD_DATA_PORT=0x00;

	//LCD_CLR_RS;			//tryb komend
	//LCD_CLR_E;
	_delay_ms(30);

#if INTERFACE_8_BITS
	lcd_wr(0x3C);		//ustaw interfejs 8-bitów,2-linie
	_delay_us(50);
#else
	//uwaga wpisywana tylko starsza część bajtow
	lcd_wr(0x20);		//tryb 4- bitowy
	lcd_wr(0x20);
	lcd_wr(0xC0);		//2-linie mode, display On
	_delay_us(60);		// !!b.wazne

#endif
	lcd_wr_command(0x0E); //Display OnOff control:display ON,cursor on,blink off
	lcd_clr();			  //Display clear
	lcd_wr_command(0x06); //Entry mode: decrement, entire shift off
 }


//------------------------------------------
void lcd_putchar(char x)	// wyświetlanie znaku z formatowaniem
{

		lcd_wr_data(x);
	
}
//------------------------------------------
void lcd_puttxt(char*msg)// wyświetlanie stringu 'msg'
{
	while( *msg!=0)  lcd_putchar(*msg++);
}

//-----------------------------------------
void lcd_gotoxy(char col, char row)//ustawienie kursora
{
	char adres=0x00;
	if (row  == 0)
	{
		adres=adres+col;
	}
	else
		adres=adres+col+0x40; //2gi wiersz

	adres=(adres|0x80);
	lcd_wr_command(adres);
}
//------------------------------
