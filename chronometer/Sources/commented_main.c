#include <hidef.h>      /* common defines and macros */
#include <mc9s12dg256.h>     /* derivative information */
#include <string.h>

#pragma LINK_INFO DERIVATIVE "mc9s12dg256b"

#define LCD_DATA PORTK
#define LCD_CTRL PORTK
#define RS 0x01
#define EN 0x02

void COMWRT4(unsigned char);
void DATWRT4(unsigned char data) ;//writes on the display one character
void DATWRT(char *);//writes on the displey a string of characters
void MSDelay(unsigned int);
void displayTime(unsigned int, unsigned int, unsigned int);     

int cnt=0,flag=2;
unsigned int minutes=0,seconds=0,hours=0;
char c;

void init(){// the function that is called at the begining of the program 
  
  COMWRT4(0x33);   //reset sequence provided by data sheet
  MSDelay(1);      // delay 1 milisecond
  COMWRT4(0x32);   //reset sequence provided by data sheet
  MSDelay(1);
  COMWRT4(0x28);   //Function set to four bit data length
                 //2 line, 5 x 7 dot format
  MSDelay(1);
  COMWRT4(0x06);  //entry mode set, increment, no shift
  MSDelay(1);
  COMWRT4(0x0C);  //Display set, disp on, cursor on, blink off
  MSDelay(1);
  COMWRT4(0x01);  //Clear display
  MSDelay(1);
  COMWRT4(0x80);  //set start posistion, home position
  MSDelay(1);

  DDRK = 0xFF;   
  TSCR1=0x80;// we use channnel 7 
  TSCR2=0x7;//prescale factor 128
  TC7=0xB71B;//val din reg de numarare
  TIE=0x80;// Timer interrupt enable on channel 7 
  DDRH = 0x00;//all the channels are set as input  
  PTH = 0x0;   // for the buttons (they are active on 0)

}
void stopCounter(){// this function is called when the first pushbutton is pressed
	while(PTH_PTH0 == 0);// for debouncing

	TIE = 0x00;//interrupts are disabled 
  TC7 = 0x00;// counting constant 
}

void resumeCounter(){// this function is called when the second pushbutton is pressed
	while(PTH_PTH1 == 0);

	TIE = 0x80;// Timer interrupt enable on channel 7 
	TC7 = 0xB71B;// TC7 is set to the value 46.875 which is 1/4 seconds; counting constant
}

void resetCounter(){// this function is called when the third pushbutton is pressed
	while(PTH_PTH2 == 0);
//resetting the coounter
	minutes=0;
  seconds=0;
	hours=0;   

	displayTime(hours,minutes,seconds);// display on the LCD the resetted counter
}

void main(void) { //main function 
  __asm CLI;
  
	init();
	
	while(1) {
     
		if(PTH_PTH0 == 0){// stop counter
			stopCounter();
			flag=0;
		}
		
		if(PTH_PTH1 == 0 && flag==0){   // continue if the counter was stopped previously
				resumeCounter();
				flag=1;
			}  
			
		if(PTH_PTH2 == 0) {   //reset counter
			resetCounter();
		}
	} 
}

char *intToString (unsigned int number) 
{    
     
     char *string,*tmp;
     string = (char *)malloc(sizeof(char));

     if(number<10) {
        string[0]='0';
        string[1]=number+48;
     } 
     else{
        string[0]=number/10+48;
        string[1]=number%10+48;
     }
     
     string[2]='\0';
     
     tmp=string; 
     free(string); 
      
     return tmp;
        
}
void displayTime(unsigned int h,unsigned int m, unsigned int s){
    char finalString[10];
    int i;
    COMWRT4(0x80);

    strcpy(finalString, intToString(h));
    strcat(finalString,":");

    strcat(finalString,intToString(m));
    strcat(finalString,":");


    strcat(finalString,intToString(s));
    finalString[8]='\0';
    
    DATWRT(finalString);
}

void COMWRT4(unsigned char command)
  {
        unsigned char x;
        
        x = (command & 0xF0) >> 2;         //shift high nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;          //clear bits Pk5-Pk2
        LCD_DATA = LCD_DATA | x;          //sends high nibble to PORTK
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~RS;         //set RS to command (RS=0)
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //Drop enable to capture command
        MSDelay(15);                       //wait
        x = (command & 0x0F)<< 2;          // shift low nibble to center of byte for Pk5-Pk2
        LCD_DATA =LCD_DATA & ~0x3C;         //clear bits Pk5-Pk2
        LCD_DATA =LCD_DATA | x;             //send low nibble to PORTK
        LCD_CTRL = LCD_CTRL | EN;          //rais enable
        MSDelay(5);
        LCD_CTRL = LCD_CTRL & ~EN;         //drop enable to capture command
        MSDelay(15);
  }

 void DATWRT4(unsigned char data){
  unsigned char x;
       
    x = (data & 0xF0) >> 2;
    LCD_DATA =LCD_DATA & ~0x3C;                     
    LCD_DATA = LCD_DATA | x;
    MSDelay(1);
    LCD_CTRL = LCD_CTRL | RS;
    MSDelay(1);
    LCD_CTRL = LCD_CTRL | EN;
    MSDelay(1);
    LCD_CTRL = LCD_CTRL & ~EN;
    MSDelay(5);
   
    x = (data & 0x0F)<< 2;
    LCD_DATA =LCD_DATA & ~0x3C;                     
    LCD_DATA = LCD_DATA | x;
    LCD_CTRL = LCD_CTRL | EN;
    MSDelay(1);
    LCD_CTRL = LCD_CTRL & ~EN;
    MSDelay(15);
  }

  
 void DATWRT(char * d)
  {
      char data;
      unsigned char x;
      
       data=*d;
        
       while(data!='\0'){
          
        x = (data & 0xF0) >> 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | RS;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(5);
       
        x = (data & 0x0F)<< 2;
        LCD_DATA =LCD_DATA & ~0x3C;                     
        LCD_DATA = LCD_DATA | x;
        LCD_CTRL = LCD_CTRL | EN;
        MSDelay(1);
        LCD_CTRL = LCD_CTRL & ~EN;
        MSDelay(15);
        d++;
        data=*d;
        
       }
  }  
  
 void MSDelay(unsigned int itime){
    unsigned int i; unsigned int j;
    for(i=0;i<itime;i++)
      for(j=0;j<4000;j++);
 } 

interrupt(((0x10000-Vtimch7)/2)-1)void TC7_ISR(void){     
  cnt++ ;  
   
  TC7=TC7+0xB71B;
  TFLG1=TFLG1|(0x80);
  
    if(cnt==4){// if cnt=4 , 1 second passed, so we update the display
       cnt=0;
       ++seconds;
       if(seconds >= 60){
           minutes++;
           seconds=0;
         
       } 
       if(minutes >= 60){
          hours++;
          minutes=0;
       }           
     
     displayTime(hours,minutes,seconds);

    }
  
 }