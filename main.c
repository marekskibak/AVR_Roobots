/*
 *  
 * Skiba Krzysztof
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include  "ir_decode.h"
#include "lcd44780.h"
#include "1-Wire/ds18x20.h"

//zmienne globalne dla 1-Wire
uint8_t subzero,cel,cel_fract_bits;
uint8_t czujniki_cnt;

//USARTTT
void uart_putc(char data);
void uart_puts(char *s);
void uart_putint(int value, int radix);

// zmienna dla uarta
char zasoby;


void display_temp(uint8_t x);

/* przydatne definicje pin�w steruj�cych */
#define PWMSIL1 PC4
#define WE_A PC0
#define WE_B PC3
#define PWMSIL2 PD7
#define WE_C PC1
#define WE_D PC2

// Oswietlenie
#define dioda1on PORTA &= ~(1<<PA1) // zalaczenie diody1
#define dioda1off PORTA |= (1<<PA1) // wylaczanie
#define dioda2on PORTA &= ~(1<<PA2) // zalaczanie diody2
#define dioda2off PORTA |= (1<<PA2) // wylaczanie
// koniec definicji ostwietlenia


#define MAX 255;
// Sterowanie Serwami
// Zmienne PWM pwm1, pwm2, pwm3, pwm4 sterowanie serwami i silnikami
volatile uint8_t pwm1, pwm2,pwm3,pwm4;
// Zmienne dla Timer�w programowych
volatile uint16_t Timer1, Timer2, Timer3, Timer4,Timer5;


// Sterowanie Silnika
/* definicje polece� steruj�cych prac� silnika 1*/
#define DC_LEWO PORTC &= ~(1<<WE_A); PORTC |= (1<<WE_B)
#define DC_PRAWO PORTC |= (1<<WE_A); PORTC &= ~(1<<WE_B)
#define DC_STOP PORTC &= ~(1<<WE_A); PORTC &= ~(1<<WE_B)
/* definicje polece� steruj�cych prac� silnika 2*/
#define DC_LEWOO PORTC &= ~(1<<WE_C); PORTC |= (1<<WE_D)
#define DC_PRAWOO PORTC |= (1<<WE_C); PORTC &= ~(1<<WE_D)
#define DC_STOPP PORTC &= ~(1<<WE_C); PORTC &= ~(1<<WE_D)




// Definicje UART RX
#define UART_RX_BUF_SIZE 32// definiujemy bufor o rozmiarze 32 bajt�w
// definiujemy mask� dla naszego bufora
#define UART_RX_BUF_MASK ( UART_RX_BUF_SIZE - 1)
// definiujemy w ko�cu nasz bufor UART_RxBuf
volatile char UART_RxBuf[UART_RX_BUF_SIZE];
// definiujemy indeksy okre�laj�ce ilo�� danych w buforze
volatile uint8_t UART_RxHead; // indeks oznaczaj�cy �g�ow� w�a�
volatile uint8_t UART_RxTail; // indeks oznaczaj�cy �ogon w�a�

// Definicja UART TX
#define UART_TX_BUF_SIZE 16
#define UART_TX_BUF_MASK (UART_TX_BUF_SIZE-1)
volatile char UART_TXBuf[UART_TX_BUF_SIZE];
volatile uint8_t UART_TXHead;
volatile uint8_t UART_TXTail;



// Pomiar napi�cia
uint16_t pomiar(uint8_t kanal);
 #define liczbapom 6


// definiujemy funkcj� pobieraj�c� jeden bajt z bufora cyklicznego
char uart_getc(void);

// deklaracja funkcji nadawczej
void USART_Transmit(unsigned char data);

// deklaracja funkcji odbioreczej
unsigned char USART_Receive(void);

// definicja funkcji inicjalizuj�cej UART
void USART_Init(uint32_t baud);


int main(void){

//zmienne dla pomiaru napi�cia na akumulatorze
	uint32_t wynik;
    uint16_t pom;
	uint8_t stala;
    uint8_t ulamek;

// zmiena dla pwm1,pwm2
uint8_t dopom=0;
uint8_t a,d;
uint8_t b;
uint8_t i;
i=0;
b=0;
a=0;




DDRD |= (1<<PWMSIL2);
DDRC |= (1<<WE_C)|(1<<WE_D)|(1<<WE_A)|(1<<WE_B)|(1<<PWMSIL1);
DDRD |=(1<<PD5)|(1<<PD3);
// diody DDRA
DDRA |=(1<<PA1)|(1<<PA2);


// USTAWIENIA PRZETWORNICY ADC
  ADMUX |= (1<<REFS1)|(1<<REFS0);
  ADCSRA |=(1<<ADEN)|(1<<ADPS1)|(ADPS2);

// Ustawienia Timer0 dla programowego genrerowania PWM
  TCCR0 |=(1<<WGM01);       // tryb pracy CTC
  TCCR0 |= (1<<CS01)|(1<<CS00); // preskaler = 64
  OCR0 =34;


pwm1=4;  // ustawienia polozenia serw
pwm2=6;  // ustawienia polozenia serw
pwm3  = 200;  // ustawienia pr�dkosci poczatkowe
pwm4 = 200; // ustawienia predkosci poczatkowe

 // Timer2 � inicjalizacja przerwania co 10ms
  	TCCR2 	|= (1<<WGM21);			// tryb pracy CTC
  	TCCR2 	|= (1<<CS22)|(1<<CS21)|(1<<CS20);	// preskaler = 1024
  	OCR2 	= 108;	// przerwanie por�wnania co 10ms (100Hz)


  	TIMSK |= (1<<OCIE2)|(1<<OCIE0);	// Odblokowanie przerwania CompareMatch



  /* inicjalizacja wy�wietlacza */

lcd_init();
lcd_cursor_on();

// INICJALIZACJA funkcji do obs�ugi odbiornika podczerwieni
 ir_init();
// ODBLOKOWANIE PRZERWA�
 sei();

// inicjalizacja modu�u UART
	USART_Init(9600);

// Test uk�adu i czas poswiecony na inicjalizacje bluetotha do pracy
lcd_cls();
lcd_str("Elektronik");
lcd_locate(1,0);
lcd_str("wynikowa");
_delay_ms(100);

czujniki_cnt = search_sensors();
DS18X20_start_meas(DS18X20_POWER_EXTERN, NULL);
_delay_ms(2000);
lcd_cls();
if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel,&cel_fract_bits))
	display_temp(0);
else {
			lcd_locate(1,0);
			lcd_str("error");
					}
_delay_ms(1000);


//nieskonczona petla programu
while(1)

 {

//Obluga wyswietlanego naprzemiennie nazwkisk autorow pracy
if(!Timer3){
	b=(b+1);
	Timer3=300;
	if((b%2)==0){
	lcd_cls();
	lcd_str("Roobot");
	lcd_locate(1,1);
	lcd_str("Niszczyciel");}
	if((b%2)==1){
	lcd_cls();
	lcd_str("Wykonal");
	lcd_locate(1,0);
	lcd_str("Skiba Krzysztof");}
	if(b==19){b=0;}
}
/*if(Timer4 == 90) {
					lcd_str("OK");
					DS18X20_start_meas(DS18X20_POWER_EXTERN,NULL);
				}

if(!Timer4)
				{ 	lcd_cls();
					if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel,&cel_fract_bits))
					{	display_temp(0);
				//	lcd_cls();
					//	lcd_int(cel_fract_bits);
					}
					else {
						lcd_locate(1,0);
						lcd_str("error");
					}
				}
                         */

		// pilot sterowanie
		if (Ir_key_press_flag){
			if( !address && vol_up==command ){
				Timer2=20;
				DC_PRAWO;
				DC_PRAWOO;
			}
			if(!address && button_ok==command){

				i=i+1;
				lcd_cls();
				lcd_locate(0,1);
				lcd_int(i);

				if((i%2)==0){
					dioda1on;
					dioda2on;
				}
				if((i%2)!=0){
					dioda1off;
					dioda2off;
				}
		}
			if(!address && cursor_up==command){
				Timer2=20;
				DC_LEWOO;
				DC_PRAWO;
			}
			if(!address && cursor_down==command){
				Timer2=20;
				DC_PRAWOO;
				DC_LEWO;
			}
			if(!address && vol_down==command){
				Timer2=20;
				DC_LEWO;
				DC_LEWOO;
			}
			if(!address && 9==command){
				lcd_cls();
				Timer4=90;
				if(Timer4 == 90) {
								lcd_str("OK");
								DS18X20_start_meas(DS18X20_POWER_EXTERN,NULL);
						while(1){
								if(!Timer4)
							{ 		lcd_cls();
								if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel,&cel_fract_bits))
									{	display_temp(0);
									//	lcd_cls();
									//	lcd_int(cel_fract_bits);
									//if(!Timer4)
										break;
									}
									else {
										lcd_locate(1,0);
										lcd_str("error");
									    break;
									}
									}
						}
				}
			    }

			if (!address && 1==command){

		    	pom = pomiar(4);
		    	wynik = pom * 25 *4,7;
		    	stala = (uint8_t)wynik/10000;
		    	ulamek = (wynik/100) % 100;
		    	lcd_cls();
		    	lcd_int(pom);
		    	lcd_locate(0,5);
		    	lcd_int(stala);
		    	lcd_char('.');
		    	lcd_int(ulamek);
		    	lcd_str("V");
			}
			if (!address && dwa==command){
				d=(d+1);
				if(d>=7)d=7;
				pwm1=d;
				lcd_cls();
				lcd_int(d);

			}
			if(!address && piec==command){
				d=(d-1);
				if(d<4)d=4;
				pwm1=d;
				lcd_cls();
				lcd_int(d);
			}

			if(!address && szesc==command){
				a=7;

				Timer1=15;
				pwm2=a;
				lcd_cls();
				lcd_int(a);
			}
			if(!address && cztery==command){
				a=5;
				Timer1=15;
				pwm2=a;
				lcd_cls();
				lcd_int(a);
			}
			if(!address && teletext_green==command ){


				pwm3=255;
				pwm4=255;
				lcd_cls();
				lcd_str("speed");
				lcd_locate(1,4);
				lcd_str("MAX");

				}
			if(!address && teletext_yellow==command){


				pwm3 = 200;
				pwm4 = 200;
				lcd_cls();
				lcd_str("speed");
				lcd_locate(1,4);
				lcd_str("MIDDLE");
			}
			if (!address && teletext_blue==command){

				pwm3=120;
				pwm4=120;

				lcd_cls();
				lcd_str("speed");
				lcd_locate(1,4);
				lcd_str("SLOW");
			}
			Ir_key_press_flag=0;
						command=0xff;
						address=0xff;
		          }
if(!Timer1) {

        pwm2=6;
	}
if(!Timer2){
	DC_STOPP;
	DC_STOP;
}


//uart sterowanie

		zasoby=uart_getc();


// uart sterowanie
		if(zasoby == 'V')
		{
		uart_putc('a');
		while(1)
			{
		//uart sterowanie

		zasoby=uart_getc();

		if(zasoby == 'C')
		{
			DC_STOPP;
			DC_STOP;
		}
		if(zasoby=='W')
		{
			//Timer2=20;
			DC_LEWOO;
		    DC_PRAWO;
		}

		if(zasoby=='S'){
			//Timer2=20;
			DC_PRAWOO;
			DC_LEWO;
		}
		if(zasoby=='A'){
			//Timer2=20;
			DC_LEWO;
			DC_LEWOO;
		}
		if(zasoby=='D'){
			// Timer2=20;
			DC_PRAWO;
			DC_PRAWOO;
		}
		if(zasoby == 'p'){
			Timer5=90;

				if(Timer5 == 90) {
					DS18X20_start_meas(DS18X20_POWER_EXTERN,NULL);
									while(1){
											if(!Timer5)
										    {
				if(DS18X20_OK == DS18X20_read_meas(gSensorIDs[0], &subzero, &cel,&cel_fract_bits))
												{
					 uart_putint(subzero,  10);
					 uart_putint( cel,  10);
					 uart_putint(cel_fract_bits, 2);
													//uart_putc('p');
											/*		uart_putc(subzero);
													uart_putc(cel);
													uart_puts(cel_fract_bits);  */
													break;
												}
												else {
												    break;
												}
												}

									}
							}
		}



		if(zasoby=='I'){
			d=(d+1);
			if(d>5)d=5;
			pwm1=d;
			lcd_cls();
			lcd_int(d);
				}
		if(zasoby=='K'){
			d=(d-1);
			if(d<3)d=3;
			pwm1=d;
			lcd_cls();
			lcd_int(d);
				}
		if(zasoby=='J'){
			a=5;
			Timer1=15;
			pwm2=a;
			lcd_cls();
			lcd_int(a);
			}
		if(zasoby=='l'){
			a=7;

		    Timer1=15;
		    pwm2=a;
			lcd_cls();
			lcd_int(a);
			}
		if(zasoby=='B'){

			pwm3=255;
			pwm4=255;
			lcd_cls();
			lcd_str("speed");
			lcd_locate(1,4);
			lcd_str("MAX");
		}
		if(zasoby=='N'){

			pwm3=200;
			pwm4=200;
			lcd_cls();
			lcd_str("speed");
			lcd_locate(1,4);
			lcd_str("MIDDLE");
		}
		if(zasoby=='M'){

			pwm3=120;
			pwm4=120;
            lcd_cls();
			lcd_str("speed");
			lcd_locate(1,4);
			lcd_str("SLOW");
		}
 if(zasoby == 'E')  // wyj�cie z p�tli bletooth
 {
	 break;
 }
 if (Ir_key_press_flag){ // wyj�cie z p�tli bluetooth
	 break;
 }
 if(!Timer1) {
 		a=7;
         pwm2=6;
 	}

			} // koniec petli bluetooth

	} // koniec ifa bluetooth

 }// koniec g��wnej p�tli niesko�czonej





}//koniec funckji main




// definicja funkcji odbiorczej
unsigned char USART_Receive(void){
	// czeka a� dane zostan� odberane z bufora
	while (!(UCSRA&(1<<RXC)));
	return UDR;
}

// definicja funkcji nadawczej
void USART_Transmit(unsigned char moc)
	{
	// czekaj a� bufor nadawczy b�dzie pusty
	while(!(UCSRA&(1<<UDRE)));
	// wrzu� dane do bufora nadawczego, start transmisji
	UDR=moc;
	}

// definicja funkcji inicjalizuj�cej UART
void USART_Init(uint32_t baud)
	{
	// wyliczenie UBRR dla trybu asynchronicznego (U2X=0)
	uint16_t _ubr = (F_CPU/16/baud-1);
	// ustawienie pr�dko�ci
	UBRRH = (uint8_t)(_ubr>>8);
	UBRRL = (uint8_t) _ubr;
	// za��czenie nadajnika i odbiornika
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	// ustawienie formatu ramki: 8 bit�w danych, 1 bit stopu
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
	}

// definiujemy funkcj� pobieraj�c� jeden bajt z bufora cyklicznego
char uart_getc(void) {
    // sprawdzamy czy indeksy s� r�wne
    if ( UART_RxHead == UART_RxTail ) return 0;

    // obliczamy i zapami�tujemy nowy indeks �ogona w�a� (mo�e si� zr�wna� z g�ow�)
    UART_RxTail = (UART_RxTail + 1) & UART_RX_BUF_MASK;
    // zwracamy bajt pobrany z bufora  jako rezultat funkcji
    return UART_RxBuf[UART_RxTail];
}

// definiujemy procedur� obs�ugi przerwania odbiorczego, zapisuj�c� dane do bufora cyklicznego
ISR( USART_RXC_vect ) {
    uint8_t tmp_head;
    char data;


    data = UDR; //pobieramy natychmiast bajt danych z bufora sprz�towego

    // obliczamy nowy indeks �g�owy w�a�
    tmp_head = ( UART_RxHead + 1) & UART_RX_BUF_MASK;

    // sprawdzamy, czy w�� nie zacznie zjada� w�asnego ogona
    if ( tmp_head == UART_RxTail ) {

    } else {
	UART_RxHead = tmp_head; 		// zapami�tujemy nowy indeks
	UART_RxBuf[tmp_head] = data; 	// wpisujemy odebrany bajt do bufora
    }
}
// definicja przerwan nadawczych od TX USARTA
ISR(USART_UDRE_vect) {
	if(UART_TXHead != UART_TXTail)
	{
		UART_TXTail = (UART_TXTail+1) & UART_TX_BUF_MASK;
		UDR = UART_TXBuf[UART_TXTail];
	}else {
		UCSRB &=~(1<<UDRIE);
	}

}
//definicja funkcji nadawczej
void uart_putc(char data){
	uint8_t tmp_head;
	tmp_head=(UART_TXHead+1) & UART_TX_BUF_MASK;

	while(tmp_head == UART_TXTail)
	{}
	UART_TXBuf[tmp_head] = data;
	UART_TXHead = tmp_head;

	UCSRB |= (1<<UDRIE);
}

void uart_puts(char *s)
{
  register char c;
  while ((c = *s++)) uart_putc(c);
}

void uart_putint(int value, int radix)
{
	char string[17];
	itoa(value, string, radix);
	uart_puts(string);
}


// definicja funkcji pomiar

uint16_t pomiar(uint8_t kanal)
{
	ADMUX |= (ADMUX & 0xF0)| kanal;
	ADCSRA |= (1<<ADSC);

	while (ADCSRA & (1<<ADSC));
	return ADCW;

}

// Przerwania od Timera0 dla PWM programowych

ISR( TIMER0_COMP_vect )
 {
	static uint8_t cnt;  // definicja naszego licznika PWM

	// bezpo�rednio sterowanie wyj�ciami kana��w PWM
	if (cnt<=pwm3) PORTC |=(1<<PWMSIL1); else PORTC &=~(1<<PWMSIL1);
	if (cnt<=pwm4) PORTD |=(1<<PWMSIL2); else PORTD &=~(1<<PWMSIL2);
	if (cnt<=pwm1) PORTD |=(1<<PD3); else PORTD &=~(1<<PD3);
	if (cnt<=pwm2) PORTD |=(1<<PD5); else PORTD &=~(1<<PD5);

	cnt++;

 }

// Timer programowe
ISR(TIMER2_COMP_vect)
{
	uint16_t n;

	n = Timer1;		/* 100Hz Timer1 */
	if (n) Timer1 = --n;
	n = Timer2;		/* 100Hz Timer2 */
	if (n) Timer2 = --n;
	n = Timer3;		/* 100Hz Timer3 */
	if (n) Timer3 = --n;
	n = Timer4;		/* 100Hz Timer4 */
	if (n) Timer4 = --n;
	n = Timer5;
	if (n) Timer5 = --n;

}
/* wy�wietlanie temperatury na pozycji X w drugiej linii LCD */
void display_temp(uint8_t x) {
	lcd_locate(1,x);
	if(subzero) lcd_str("-");	/* je�li subzero==1 wy�wietla znak minus (temp. ujemna) */
	else lcd_str(" ");	/* je�li subzero==0 wy�wietl spacj� zamiast znaku minus (temp. dodatnia) */
	lcd_int(cel);	/* wy�wietl dziesi�tne cz�ci temperatury  */
	lcd_str(".");	/* wy�wietl kropk� */
	lcd_int(cel_fract_bits); /* wy�wietl dziesi�tne cz�ci stopnia */
	lcd_str("\x80""C "); /* wy�wietl znak jednostek (C - stopnie Celsiusza) */
}
