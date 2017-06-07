/*
 *  Display Driver
 *
 * Copyright (c) 2017  Catedra Programacion 1 ITBA GEDA
 *
 *  Author(s):
 * 
 *  Nicolas Magliola
 *  Clementina Calvo
 *  Daniel Jacoby
 * 
 * 
 * Compile
 * 
 * gcc  disdrv.c      	// Stand alone (Rename mainTBD function to main)
 * gcc -c disdrv.c    	// As object library (Rename main function to mainTBD)
 * gcc myapp.c disdrv.o // Linking with an applcation
 * 
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <stdbool.h>
#include "termlib.h"
#include "MAX7219.h"
#include "disdrv.h"



#define DEBUG_OFF  0
#define DEBUG_ON   1
#define DEBUG	DEBUG_OFF

// Full debug macro
#define debug_print(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

// A bare minimum debug macro 
#define debug_msg(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, fmt,__VA_ARGS__); } while (0)

#define forever for(;;)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	errno=EINVAL;		// Invalid Argument error message
	perror(s);
	abort();
}
//

#define DISPLAY_ADDRESS     0x00
#define DISPLAY_DATA    	0x01



/***************Driver internal Variables*************************/

// SPI variables

static const char *device0 = "/dev/spidev0.0"; // CS0 GPIO8 Display
static const char *device1 = "/dev/spidev0.1"; // CS1 GPIO7 ADC 
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 100000; //Hz
static uint16_t delay;
static int fd;				// file descriptor

// Driver Variables and definitions



// Buffers 
static uint8_t ram_display_buffer[16][16];
static uint8_t transfer_buffer[4]={0x81,0x42,0x24,0x18};         // 
static dcoord_t disp_origin_coordinates[4]={{8,8},{0,8},{8,0},{0,0}};   // D,C,B,A origin={left=x,upper=y}
static uint8_t* disp_origin_addresses[4];
static daxis_t axis = NORMAL;


// For internal use only

static void transfer(int fd,uint8_t address,uint8_t data);
static void transfer2display(int fd,uint8_t display,uint8_t address,uint8_t data);
static void encode_one_byte(uint8_t *p2byte, uint8_t *p2bit);
static uint8_t *coord2add(uint8_t *pbase,uint8_t x,uint8_t y);
static void configure_spi(void);
static void Show_Display_Buffer (void);


/********************Driver Public Services*********************/



/* ----------------------------------------------
    Write to display Functions 
------------------------------------------------*/ 
/*
 * 
 * This function writes to display buffer  
 * 
 * The display buffer is a square matrix of 16x16 pixels
 * A pixel can be turned on or off 
 * The first parameter of the function is an structure containing 
 * the pixel coordinates 
 * 
 * x: 0 to 15
 * y: 0 to 15
 * 
 * The last parameter is the pixel Value: D_ON or D_OFF
 * 
 * Important Writing to the display buffer does no actually be the same as writing
 * to display. To write to display we need to call display_update() function
 * which transfers the entire display buffer to the display
 * 
 * Example 
 * 
 * 
 * dcoord_t point1={3,2};
 *
 *
 *   Send_2_display((dcoord_t){6,3},D_ON); 	//  Turns on pixel 6,3
 *   Send_2_display(set_coord(7,2),D_ON); 	//  Turns on pixel 6,3
 *   Send_2_display(point1,D_ON); 			//  Turns on pixel at point1
 * 
 * 
 */



void Send_2_display(dcoord_t point, dlevel_t val)
{
	display_write(point.x,point.y,D_ON);

}


/*
 * This function writes to display buffer  
 * 
 * The display buffer is a square matrix of 16x16 pixels
 * A pixel can be turned on or off 
 * The pixel coordinates are the first two parameters of the function
 * x: 0 to 15
 * y: 0 to 15
 * 
 * The last parameter is the pixel Value: D_ON or D_OFF
 * 
 * Important Writing to the display buffer does no actually be the same as writing
 * to display. To write to display we need to call display_update() function
 * which transfers the entire display buffer to the display
 * 
 * Example display_write(2,3,D_ON) Turns on the pixel located at 3rd row and 2nd columm
 * 
 * 
 */
void display_write(uint8_t x,uint8_t y, dlevel_t val)
{
	uint8_t xp,yp;
	
	
	if (x<MIN_X || x> MAX_X || y<MIN_X || y> MAX_X)
	
	pabort("display_write Error: Coordinate out of bounds!!"); 
	 
	if (axis==ROTATE)
	{
		xp=15-y;
		yp=x; 
	}
	else
	{
		xp=x;
		yp=y;
   
	}
	 
	ram_display_buffer[xp][yp]=(!!val);  // Ensure 1 or 0 is writen (looks better on memory dump) 
	 

}

/*
 * This function initialize MAX7219 internal registers
 * It must be called once at the very begining of the Aplication
 *  
 * Example: display_init();
 * 
 */
void display_init(void)
{
    configure_spi();
    
	transfer(fd,REG_DECODE_MODE,DecodeMode_NoDecode);	// No predefined charaters just dots!!
	transfer(fd,REG_INTENSITY,Intensity_Level15);       // Intensity level
	transfer(fd,REG_SCAN_LIMIT,ScanLimit_Digit0To7);    // How many digits will be seen
	transfer(fd,REG_SHUTDOWN,ShutdownMode_NormalOperation); // Standby disabled 
	//transfer(fd,REG_DISPLAY_TEST,TestMode);				// All on
	transfer(fd,REG_DISPLAY_TEST,NormalOperation);			// Normal Operation all dots can be changed
	
	// Flush all pending commands!!!! (this is because we changed the transfer function to send data)
	// Change:  transfer() to transfer2display() which sends 4 transfers at once and therefore
	// commands in the chain will never arrive to all displays )
	
	transfer(fd,REG_NO_OP,0x00);  // Flush all
	transfer(fd,REG_NO_OP,0x00);
	transfer(fd,REG_NO_OP,0x00);
	transfer(fd,REG_NO_OP,0x00);

	close(fd);
}
/*
 * This function clears the entire display and the display buffer
 * 
 *  
 * Example: display_clear();
 * 
 */
void display_clear(void)
{
	int i,j;
	
	
	configure_spi();
	
	// Clear Display buffer 
	
	for(i=0;i<16;i++)
		for(j=0;j<16;j++)
			ram_display_buffer[i][j]=0;
	
	
	for (i=0;i<=3;i++)
	{
	transfer(fd,REG_DIGIT_0,0x00);
	transfer(fd,REG_DIGIT_1,0x00);
	transfer(fd,REG_DIGIT_2,0x00);
	transfer(fd,REG_DIGIT_3,0x00);
	
	transfer(fd,REG_DIGIT_4,0x00);
	transfer(fd,REG_DIGIT_5,0x00);
	transfer(fd,REG_DIGIT_6,0x00);
	transfer(fd,REG_DIGIT_7,0x00);
	}
	
	close(fd);
	
}

/*
 * This function updates the the entire display
 * with the display buffer contents
 * 
 *  
 * Example: display_update();
 * 
 */
void display_update(void)
{
	int i,j;
	configure_spi();
	

	// Transfer display buffer to display chain
	
	// Calculate display base addresses on the on the display buffer (this saves time ) 
	
	for(i=0;i<4;i++)	
	disp_origin_addresses[i]=coord2add(&ram_display_buffer[0][0],disp_origin_coordinates[i].x,disp_origin_coordinates[i].y);

	// Encode every 8 bytes in the display buffer in one byte in the transfer buffer 
	// Repeat this for each display (i.e. 4 times)  
	
	for(i=0;i<8;i++)	// Every 8 bytes in the display buffer is a row in the hardware display (DIG 0-7) 
	{
		for(j=0;j<4;j++) // Store that row (for each display) encoded in one byte on the transfer buffer 
		{
			encode_one_byte((disp_origin_addresses[j]+16*i), transfer_buffer+j); // encode one byte and store it on the 4 byte transfer buffer
			debug_msg("T:%.2X ", transfer_buffer[j] );
			transfer2display(fd,j,REG_DIGIT_7-i,transfer_buffer[j]); // 
		//	sleep(1); //for debug 
		}
	}
	
	
	close(fd);  //Close SPI device
	
	
}


/*
 * This function sets display orientation 
 * 
 * It recieves one parameter (orientation) wich defines the display axis orientation
 * Posible values NORMAL or ROTATE
 * 
 * NORMAL Joystick on the bottom of display ( Right Handed) 
 * ROTATE Joystick on the LEFT of display ( Left Handed) 
 *   
 * 
 * Example: set_display_axis(ROTATE);
 *       	 					   
 * 
 */
void set_display_axis(uint8_t orientation)
{
	axis=orientation;
}

int mainTBD(int argc, char *argv[])  //Sample main Test Bench 
{
	int ret=0;

	display_init();				// Initialize Display Hardware 

	set_display_axis(NORMAL);	// Set axis mode 

	display_clear();			// Clears Display buffer and Hardware Display 



// *********** Write Data to display buffer ********** /


	dcoord_t point1={3,2};

    Send_2_display((dcoord_t){6,3},D_ON); 	//  Turns on pixel 6,3
    Send_2_display(set_coord(7,2),D_ON); 	//  Turns on pixel 6,3
    Send_2_display(point1,D_ON); 			//  Turns on pixel at point1
    
    display_write(0,0,D_ON);
    display_write(0,1,D_ON);
    display_write(0,2,D_ON);
    display_write(0,3,D_OFF);
    display_write(0,4,D_ON);
    display_write(0,5,D_ON);
    display_write(0,6,D_ON);
    display_write(0,7,D_OFF);


    display_write(0,0+8,D_ON);
    display_write(0,1+8,D_ON);
    display_write(0,2+8,D_ON);
    display_write(0,3+8,D_OFF);
    display_write(0,4+8,D_ON);
    display_write(0,5+8,D_ON);
    display_write(0,6+8,D_ON);
    display_write(0,7+8,D_OFF);


    
    display_write(8,0,D_ON);
    display_write(8,1,D_ON);
    display_write(8,2,D_ON);
    display_write(8,3,D_OFF);
    display_write(8,4,D_ON);
    display_write(8,5,D_ON);
    display_write(8,6,D_ON);
    display_write(8,7,D_OFF);


    
    display_write(8,0+8,D_ON);
    display_write(8,1+8,D_ON);
    display_write(8,2+8,D_ON);
    display_write(8,3+8,D_OFF);
    display_write(8,4+8,D_ON);
    display_write(8,5+8,D_ON);
    display_write(8,6+8,D_ON);
    display_write(8,7+8,D_OFF);
    
    display_write(15,0+8,D_ON);
    display_write(15,1+8,D_ON);
    display_write(15,2+8,D_ON);
    display_write(15,3+8,D_OFF);
    display_write(15,4+8,D_ON);
    display_write(15,5+8,D_ON);
    display_write(15,6+8,D_ON);
    display_write(15,7+8,D_OFF);


	Show_Display_Buffer ();      // just for debug (not a service)

	display_update();
	
	return ret;
}


/********************* Driver internal services ***********************/

static void Show_Display_Buffer (void)
{
	int i,j;
	
	for(i=0;i<16;i++)      // Show display buffer (debug)
	{
		debug_msg("%s\n","");
		for(j=0;j<16;j++)
			debug_msg("%d ",ram_display_buffer[i][j]);	
			
	}
	
	debug_msg("%s\n","");	

	
} 





/*
 * This function converts 8 bytes in a row  
 * into one byte. Each bit of the byte is set
 * to one when the associated byte is <> 0
 * The function starts with the lsb bit of the byte
 * So the resulting byte will be flipped i.e lsb must be sent last 
 * (see schematics 8x8-matrix-module-3.jpg)	
 * 
 * Example  {0x33,0x00,0x00,0x33,0x33,0x33,0x00,0x33} is converted to 1011 1001
 * 
 * p2byte must point to the row of 8 bytes while p2bit must point to the byte to be encoded
 */
static void encode_one_byte(uint8_t *p2byte, uint8_t *p2bit)
{
	
uint8_t mask = 0x01; 
int i;		

		*p2bit=0;	//clear all
		
		for(i=0;i<8;i++)
		{
			debug_msg("P%.2X ",*(p2byte+i));
			if(*(p2byte+i))
				*p2bit|=mask;
			mask<<=1;
		}	
			debug_msg("B%.2X ",*p2bit);
}

/*
 * This function converts the cartesian coordinates of an element of an bidimentional array 
 * of uint8_t to an absolute address.  
 * 
 * pbase is the start address of the array
 * x (row) and y (column) are the coordinates of an array element
 *
 * The function returns the absolute address
 * 
 * Example  uint8_t myarray[3][3]={0x01,0x05,0x08,
 *       	 					   0x03,0x33,0x12,
 *           					   0x22,0x42,0x50};
 *
 * ptr=coord2add(myarray,1,2); if myarray is at 0x1000 the function returns 0x1005
 *
 */
	
static uint8_t *coord2add(uint8_t *pbase,uint8_t x,uint8_t y)
{
	
	return (pbase+x+16*y);
	
	
}

/*
 * This function sends data to the MAX7219 display controller connected to the
 * Raspberry PI SPI bus 
 *   
 * Warning!!!! Use this function to send SAME data to ALL displays chained on the SPI bus
 * It is assumed that after data was sent the folowing transfers will shift data to the 
 * rest of the displays.
 * 
 * This function is usefull to initial configuration of all displays on the chain
 * 
 * Function parameters: 
 * fd: Is the file descriptor asociated to the recently opened SPI device
 * Address: Is the register to be written (see MAX7219 data sheet)
 * Data:    Is the data to be written 
 * 
 * Example: transfer(fd,REG_DIGIT_0,0x22);
 *       	 					   
 * Data 0x22 is sent to first display on the chain subsequents writes of any kind
 * will transfer this data to the rest of the displays on the chain, provided that 
 * the number of writes exceed the numbrer of displays on the chain. 
 */
static void transfer(int fd,uint8_t address,uint8_t data)
{
	int ret;
	uint8_t tx0[] = {
		0x00, 0x00
			
	};
	uint8_t rx0[ARRAY_SIZE(tx0)] = {0, };
	struct spi_ioc_transfer tr0 = {
		.tx_buf = (unsigned long)tx0,
		.rx_buf = (unsigned long)rx0,
		.len = ARRAY_SIZE(tx0),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};



	tx0[DISPLAY_ADDRESS]=address;
	tx0[DISPLAY_DATA]=data; 
	
	
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr0);
	if (ret < 1)
		pabort("can't send spi message");

		debug_msg("A:%.2X ", tx0[0]);
		debug_msg("D:%.2X ", tx0[1]);
	    debug_msg("%s\n","  ");
	
}

/*
 * This function sends data to the MAX7219 display controller connected to the
 * Raspberry PI SPI bus 
 *   
 *  
 * This function sends data to a specific display on the chain Not disturbing the rest
 * 
 * Function parameters: 
 * fd: Is the file descriptor asociated to the recently opened SPI device
 * Display : Is the number of display on the chain
 * Address: Is the register to be written (see MAX7219 data sheet)
 * Data:    Is the data to be written 
 * 
 * Example: transfer2display(fd,1,REG_DIGIT_0,0x22);
 *       	 					   
 * Data 0x22 is sent to second display on the chain. 
 */


static void transfer2display(int fd,uint8_t display,uint8_t address,uint8_t data)
{
	int ret;
	uint8_t tx0[] = {
		// ADDRESS,DATA
		REG_NO_OP, 0x00,
		REG_NO_OP, 0x00,
		REG_NO_OP, 0x00,
		REG_NO_OP, 0x00,
			
	};
	uint8_t rx0[ARRAY_SIZE(tx0)] = {0, };
	struct spi_ioc_transfer tr0 = {
		.tx_buf = (unsigned long)tx0,
		.rx_buf = (unsigned long)rx0,
		.len = ARRAY_SIZE(tx0),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};



	tx0[2*display]=address;
	tx0[2*display+1]=data; 
	
	
	
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr0);
	if (ret < 1)
		pabort("can't send spi message");

		debug_msg("A:%.2X ", tx0[2*display]);
		debug_msg("D:%.2X ",  tx0[2*display+1]);
		debug_msg("%s\n","  ");
	                
	
}




static void configure_spi(void)
{	
	
	int ret = 0;
	
	fd = open(device0, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	debug_msg("spi mode: %d\n", mode);
	debug_msg("bits per word: %d\n", bits);
	debug_msg("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
}
