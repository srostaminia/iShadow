#include <plib.h>					// Peripheral Library
#include "../sys_config.h"
#include "../fatfs/ff.h"
#include "../fatfs/diskio.h"

// *****************************************************************************
// *****************************************************************************
// Section: Configuration bits
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 40 MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
// WDT OFF
// Other options are don't care
// *****************************************************************************
// *****************************************************************************
//#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
//#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_1

// *****************************************************************************
// *****************************************************************************
// Section: System Macros
// *****************************************************************************
// *****************************************************************************
#define DESIRED_BAUDRATE    	(230400)      //The desired BaudRate
//#define DESIRED_BAUDRATE    	(115200)


void WriteString(const char *string);

int UART_init(void)
{
	// Explorer-16 uses UART1 to connect to the PC.
	// This initialization assumes 36MHz Fpb clock. If it changes,
	// you will have to modify baud rate initializer.
    UARTConfigure(UART1, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(UART1, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(UART1, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(UART1, GetPeripheralClock(), DESIRED_BAUDRATE);
    UARTEnable(UART1, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

	// Configure UART1 RX Interrupt
	INTEnable(INT_SOURCE_UART_RX(UART1), INT_ENABLED);
    INTSetVectorPriority(INT_VECTOR_UART(UART1), INT_PRIORITY_LEVEL_2);
    INTSetVectorSubPriority(INT_VECTOR_UART(UART1), INT_SUB_PRIORITY_LEVEL_0);

    // configure for multi-vectored mode
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

    // enable interrupts
    //INTEnableInterrupts();


   WriteString("*** UART Interrupt-driven Comunication Established ***\r\n");

   return 0;
}


// helper functions
void WriteString(const char *string)
{
    while(*string != '\0')
    {
        while(!UARTTransmitterIsReady(UART1))
            ;

        UARTSendDataByte(UART1, *string);

        string++;

        while(!UARTTransmissionHasCompleted(UART1))
            ;
    }
}

void PutCharacter(const char character)
{
        while(!UARTTransmitterIsReady(UART1))
            ;

        UARTSendDataByte(UART1, character);


        while(!UARTTransmissionHasCompleted(UART1))
            ;
}

// *****************************************************************************
// UINT32 GetDataBuffer(char *buffer, UINT32 max_size)
// *****************************************************************************
UINT32 GetDataBuffer(char *buffer, UINT32 max_size)
{
    UINT32 num_char;

    num_char = 0;

    while(num_char < max_size)
    {
        UINT8 character;

        while(!UARTReceivedDataIsAvailable(UART2))
            ;

        character = UARTGetDataByte(UART2);

        if(character == '\r')
            break;

        *buffer = character;

        buffer++;
        num_char++;
    }

    return num_char;
}


//
int string_compare(char * string1, char * string2)
{
    int i=0;
    int result=0;
    while(1)
    {
        if(string1[i]!='\r' && string2[i]!='\r' && string1[i]!='\n' && string2[i]!='\n')
        {
            if(string1[i]==string2[i])
            {
                i++;
                continue;
            }
            else
            {
                result=0;//two strings are diffrent
                break;
            }
        }
           

        if((string1[i]=='\r' && string2[i]=='\r') || (string1[i]=='\n' && string2[i]=='\n'))
        {
            result=1;//two strings are same
            break;
        }

        else if((string1[i]=='\r' || string1[i]=='\n') && (string2[i]!='\r' && string2[i]!='\n'))
        {
            result=2;//string1 is a part of string2
            break;
        }
        
        else if((string2[i]=='\r' || string2[i]=='\n') && (string1[i]!='\r' && string1[i]!='\n'))
        {
            result=3;// string2 is a part of string1
            break;
        }

        else
        {
            result=0;
            break;
        }
    }
    return result;
}


//get all the number in the string(e.g:fe35e3=353)
int str2int(const char *str)
{
	int temp = 0;
	const char *ptr = str;  //ptr??str?????
	while((*str != '\r') && (*str != '\n'))
	{
		if((*str < '0') || (*str > '9'))  //??????????
		{                       //?????
			str++;
			continue;
		}

		temp = temp * 10 + (*str - '0'); //??????????????
		str++;      //???????
	}

	return temp;
}


// *****************************************************************************
// UINT32 GetDataBuffer(char *buffer, UINT32 max_size)
//Note: To check wether the message in the buffer is a command
// *****************************************************************************
extern int start_signal;
extern int num_frames;
extern int file_set;
int command_check(char * buf)
{
    char * str_start="/start\r";
    char * str_mask="/mask\r";
    char * str_frame_num="/frame+\r";
    char * str_dir_set="/dir_set+\r";
    char * str_serial_out="/serial_out\r";
    char * str_stop="/stop\r";
    char * str_single_transmit="/single_transmit\r";
    char * str_run_predict="/run_predict\r";

    int result=0;
    char buffer[100];
    if(string_compare(str_start, buf)==1)
    {
        result=1;
        start_signal=1;
    }
    else if(string_compare(str_mask, buf)==1)
    {
        get_mask();
        result=2;
    }
    else if(string_compare(str_frame_num, buf)==2)//"/frame+ \r" is the part of "/frame+129\r". So we can get the frames number
    {
        result=3;
        num_frames=str2int(buf);
        sprintf(buffer, "Number of frames for Stonyman is setted as %d.\r\n", num_frames);
        WriteString(buffer);
    }
    else if(string_compare(str_dir_set, buf)==2)
    {
        result=4;
        file_set=str2int(buf);
        sprintf(buffer, "The directory number(name) for Webcam and image data file name for Stonyman are set as %d.\r\n", file_set);
        WriteString(buffer);
    }
    else if(string_compare(str_serial_out, buf)==1)
    {
        serial_out();
        result=5;
    }
    else if(string_compare(str_stop, buf)==1)
    {
        start_signal = 0;
        result=6;
    }
    else if(string_compare(str_single_transmit, buf)==1)
    {
        result=7;
        single_transmit();
    }
    else if(string_compare(str_run_predict, buf)==1)
    {
        result=8;
        read_and_predict();
    }
    else result=0;
    return result;
}