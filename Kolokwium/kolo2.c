#include <inttypes.h>
#include <avr/interrupt.h>

#define CMP_REGISTER (16000/256)		// 16000 kHz
#include <stdio.h>
	
#include "Lcd_Fun.h"

//definicja rejestrów portu do którego podlaczona jest klawiatura
#define KB_PORT PORTD
#define KB_DIR DDRD
#define KB_PIN PIND

uint8_t limp=1;
uint8_t okres=2;
uint8_t wartosc;
volatile uint8_t flaga_2linia=1;
volatile uint8_t UST_GEN=0;
//---------------------------------//
//---------------------------------/
int lcd_put(char znak, FILE *s)//znak, wskaznik do strumienia
{	
	if (znak == '\r')
	{ 
		lcd_gotoxy(0, 0);
	} else if (znak == '\n')
		{
			lcd_gotoxy(0, 1);
		} else 
			{
				lcd_putchar(znak);
			}
	return 0;
}
//---------------------------------//

//skan klawaitury
char KbScan(void)
{
	//char result=0;
	KB_DIR =0b0000000;
	KB_PORT=0b11111111;
	if( (KB_PIN & 0x01) ==0 ) return 1;
	if( (KB_PIN & 0x02) ==0 ) return 5;
	if( (KB_PIN & 0x04) ==0 ) return 9;
	if( (KB_PIN & 0x08) ==0 ) return 13;

	return 2; // by wyswietlic poczatkowa wartosc parametru
}
//----------------------------------//
void zmiana(void)
{
	static char liczba=0;
	char tmp;
	//static char flaga=1;
	tmp=KbScan();//to co zczytalismy wrzucamy do tmp
	_delay_ms(4);
	

	if(liczba!=KbScan() && KbScan()==tmp)//sprawdzamy to co zczytalismy z klawki
	{
		if(tmp==1)
		{
			flaga_2linia=(flaga_2linia+1)%2;
		}
		
		if(tmp==5 && flaga_2linia==1) //jesli wcisniety zostal odp. klawisz to zwiekszamy parametr
		{
			limp++;
		}
		else if(tmp==9 && flaga_2linia==1)//jesli wcisniety zostal odp. klawisz to zmniejszamy parametr
		{
			limp--;
		}
		else if(tmp==5 && flaga_2linia==0)
		{
			okres+=2;
		}
		else if(tmp==9 && flaga_2linia==0)
		{
			okres-=2;
		}
		if(tmp==13)
		{
			UST_GEN=1;
		}


//ograniczenia i wyswietlanie
		liczba=tmp;
		if(limp>=20) limp=20; //ograniczenie do 20
		if(limp<=1) limp=1; //ograniczenie od 1
		if(okres<=2) okres=2;
		if(okres>=40) okres=40;

		if(flaga_2linia==1)
		{
			printf("\nLiczba imp: %02d", limp);
		}
		if(flaga_2linia==0)
		{
			printf("\nOkres:   %02d ms", okres);
		}
	}
	
}
///////////////////////////////////
///////////////////////////////////
///////////////////////////////////
//generowanie-mrugania-diody//

void generuj(void)
{
	uint8_t i;
	
	if(UST_GEN==1)
	{
		for(i=0;i<limp; i++)
		{
			PORTA|=0b00000001;
			_delay_ms(okres*10);
			PORTA&=0b11111110;
			_delay_ms(okres*10);
		}
	}
}

////////////////////////////////////
//-------------------------------
static FILE mystdout= FDEV_SETUP_STREAM(lcd_put, NULL,_FDEV_SETUP_WRITE);
//---------------------------------
int main(void)
{

	_delay_ms(1000);
	lcd_init(); //incicjalizacja wyswietlacza
	stdout = &mystdout;
	DDRA=0x01;

	while(1)
	{	
		
		zmiana();
		//lcd_clr();
		if(UST_GEN==0)
		{
		printf("\rUstawienia: ");
		} else
			{
				lcd_clr();
				printf("\rGENERACJA ");
				generuj();
				UST_GEN=0;
			}

	}
	return 0;
}
