#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define F_CPU 16000000UL
#include <util/delay.h>	
#define CMP_REGISTER (16000/256)		// 16000 kHz

//definicja rejestrów portu do którego podlaczona jest klawiatura
#define KB_PORT PORTA
#define KB_DIR DDRA
#define KB_PIN PINA

// Sterowanie 7-segmentowym wyświetlaczem LED
// ------------------------------------------

//definicja portow sterujacych katodami i segmentami LED
#define SEGMENT_PORT	PORTD	
#define SEGMENT_DIR	    DDRD	
#define CATHODE_PORT	PORTB	//wykorzystywane 4 mlodsze bity
#define CATHODE_DIR		DDRB	//

#define MAX_LED_CODES 	10	//liczba możliwych znakow
#define MAX_LED_DIGITS	4	//liczba wyswietlaczy 7-segmentowych


unsigned int ms_timer;
uint8_t led_i=0;
uint8_t parametr=5; //nasz poczatkowy parametr= 5;

//skan klawaitury
char KbScan(void)
{
	//char result=0;
	KB_DIR =0b0000000;
	KB_PORT=0b11111111;
	if( (KB_PIN & 0x01) ==0 ) return 1;
	if( (KB_PIN & 0x02) ==0 ) return 5;
	if( (KB_PIN & 0x04) ==0 ) mrugaj();

	return 2; // by wyswietlic poczatkowa wartosc parametru
}

// tablica dekodowania cyfr LED
char LedCodeTab[MAX_LED_CODES+1]= {		
 /*'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  ' '  */
  0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F, 0x6F, 0x00  };

char LedsStr[4]={0,0,0,0};	//wyswietlana liczba 

//---------------------------------------------------------------
void LedDisp_Init(void)	//konfiguracja portˇw steruj¦cych wyťwietlaczem
{
	SEGMENT_DIR	=0xFF;	//przypisanie kierunkow portow sterujacych
	CATHODE_DIR =CATHODE_DIR | 0x0F;
}

//---------------------------------------------------------------
//Funkcja aktualizuje wyswietlacz LED 
uint8_t Led_Update(void)
{
 //	static uint8_t led_i=0;	//aktualny indeks wyswietlanej pozycji
	uint8_t tmp;
	//aktualizacja portu C/A wyswietlacza 7-segmentowego
	CATHODE_PORT =(CATHODE_PORT & 0xF0) | ~(1<<led_i);	
		
	tmp=LedsStr[led_i++];
	if(tmp>MAX_LED_CODES)
		tmp=MAX_LED_CODES;

	SEGMENT_PORT= ~LedCodeTab[tmp];
	
	if(led_i >=MAX_LED_DIGITS) led_i=0;
	return tmp;
}
/////////////////////////////////@@@@@/////////////////
///////////////

void InitTimer0(void)
{
	OCR0=CMP_REGISTER;				//aby uzyskać okres 1s
	TCCR0= (1<<WGM01 | 1<< CS02);	//tryb zerowania po zrównaniu, prescaler=256
	TIMSK=0x01 << OCIE0;			//zezwolenie na przerwania
}
ISR(TIMER0_COMP_vect)	// Obsluga przerwania od TIMER0 
{
		
	if(ms_timer ==5)
	{
		Led_Update();
		ms_timer=0;
	}
		ms_timer++;
	
}
//////////////////////////////////////////////////////
void wykonaj(void)
{
	static char liczba=0;
	char tmp;
	tmp=KbScan();//to co zczytalismy wrzucamy do tmp
	_delay_ms(4);
	if(liczba!=KbScan() && KbScan()==tmp)//sprawdzamy to co zczytalismy z klawki
	{
		if(tmp==1) //jesli wcisniety zostal odp. klawisz to zwiekszamy parametr
		{
			parametr++;
		}
		else if(tmp==5)//jesli wcisniety zostal odp. klawisz to zmniejszamy parametr
		{
			parametr--;
		}
		liczba = tmp;
		if(parametr>=20) parametr=20; //ograniczenie do 20
		if(parametr<=1) parametr=1; //ograniczenie od 1
		if(parametr<=20 && parametr>=1) //jesli parametr (0;20) to wyswietlaj
		{
				LedsStr[3]=parametr%10;
				LedsStr[2]=(parametr%100)/10;
				LedsStr[1]=(parametr%1000)/100;
				LedsStr[0]=parametr/1000;
		}
	}
	
}

void mrugaj(void)
{
	uint8_t i;
	
	for(i=0;i<parametr; i++)
	{
		PORTC|=0b00000001;
		_delay_ms(500);
		PORTC&=0b11111110;
		_delay_ms(500);
	}

}
/////////////////////////////@@@@@@@@//////////////////
///////////
//-------------------------------
// test funkcji wyświetlacza LCD
//-------------------------------
int main(void) 
{
	
	LedDisp_Init();
	InitTimer0();
	sei();	
	DDRC|=0b0000001;

	while(1) 
	{	
		wykonaj();
		//mrugaj();
	}
	return 0;
}
