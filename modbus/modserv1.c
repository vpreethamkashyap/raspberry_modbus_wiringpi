/*
 * Copyright Â© 2008-2010 StÃ©phane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.

 *********************************************************************
 * 2013-2014
 * Heavily modified by Ron Ostafichuk for 'Modbus on The Pi' web article
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <modbus.h>
#include <math.h>

// WAY easier method to access Pi IO functions!  Thanks to WiringPi.com!
#include <wiringPi.h>

enum {
    TCP,
    RTU
};


void CreateSurfaceCard(int nAddress,modbus_mapping_t *mb_mapping)
{
    // this simulates a LUFKIN Rod String Controller MODBUS device
    int nStrokeLenx100_in = 14000;
    int nMinLoad_lbs = 3000;
    int nMaxLoad_lbs=10000;
    int nLoad_lbs = nMinLoad_lbs;
    int nLoadRange = nMaxLoad_lbs - nMinLoad_lbs;

    // write header
    time_t timer;
    time(&timer);
    mb_mapping->tab_input_registers[nAddress++] = timer ;
    mb_mapping->tab_input_registers[nAddress++] = timer >> 16;
    mb_mapping->tab_input_registers[nAddress++] = 5; // high byte= shutdown eventID, low byte=num points, can hold up to 200 pts
    mb_mapping->tab_input_registers[nAddress++] = nMaxLoad_lbs; // max load lbs
    mb_mapping->tab_input_registers[nAddress++] = nMinLoad_lbs; // min load lbs
    mb_mapping->tab_input_registers[nAddress++] = nStrokeLenx100_in; // stroke length x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 1500; // stroke period x 100 (s)
    // Note: card data is in reverse chronological order (bottom stroke, then downstroke then top then upstroke)
    nLoad_lbs = nMinLoad_lbs;
    
    // five points to look like a simple card
    mb_mapping->tab_input_registers[nAddress++] = 23*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 1200; // scaled load (lbs)

    mb_mapping->tab_input_registers[nAddress++] = 120*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 1200; // scaled load (lbs)
    
    mb_mapping->tab_input_registers[nAddress++] = 140*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 2200; // scaled load (lbs)

    mb_mapping->tab_input_registers[nAddress++] = 40*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 2200; // scaled load (lbs)	
    
    mb_mapping->tab_input_registers[nAddress++] = 20*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 1200; // scaled load (lbs)
}

void CreatePumpCard(int nAddress,modbus_mapping_t *mb_mapping)
{
    // this simulates a LUFKIN Rod String Controller MODBUS device
    int nStrokeLenx100_in = 13000;
    int nMinLoad_lbs = 0001;
    int nMaxLoad_lbs=4000;
    int nLoad_lbs = nMinLoad_lbs;
    int nLoadRange = nMaxLoad_lbs - nMinLoad_lbs;
    int i;
    // write header
    time_t timer;
    time(&timer);
    mb_mapping->tab_input_registers[nAddress++] = timer >> 16;
    mb_mapping->tab_input_registers[nAddress++] = timer;
    mb_mapping->tab_input_registers[nAddress++] = nMaxLoad_lbs; // max load lbs
    mb_mapping->tab_input_registers[nAddress++] = nMinLoad_lbs; // min load lbs
    mb_mapping->tab_input_registers[nAddress++] = 5; // high byte= shutdown eventID, low byte=num points , can hold up to 100 pts
    mb_mapping->tab_input_registers[nAddress++] = 100*100; // gross stroke x 100 (in) ??
    mb_mapping->tab_input_registers[nAddress++] = 90*100; // net stroke x 100 (in) ??
    
    mb_mapping->tab_input_registers[nAddress++] = 99; // pump fillage (?)
    mb_mapping->tab_input_registers[nAddress++] = 3000; // fluid load (lbs?)
    // card data is in reverse chronological order (bottom stroke, then downstroke then top then upstroke)

    
    // five points to look like a simple pump card
    mb_mapping->tab_input_registers[nAddress++] = 3*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 10; // scaled load (lbs)
    
    mb_mapping->tab_input_registers[nAddress++] = 100*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 10; // scaled load (lbs)
            
    mb_mapping->tab_input_registers[nAddress++] = 120*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 1000; // scaled load (lbs)
    
    mb_mapping->tab_input_registers[nAddress++] = 20*100; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 1000; // scaled load (lbs)
    
    mb_mapping->tab_input_registers[nAddress++] = 0; // position x 100 (in)
    mb_mapping->tab_input_registers[nAddress++] = 10; // scaled load (lbs)
}

void updateAllSurfaceCards(modbus_mapping_t *mb_mapping)
{
    // this simulates a LUFKIN Rod String Controller MODBUS device
    // update the surface card buffer 5 cards, then the Pump Card buffer 5 cards, then the single surface and pump card, top is most recent (input registers 2669 - cnt=2035)
    int nAddress = 2669; // start of 5 surface card buffer
    int i;
    for( i= 0 ; i < 5; i++ )
    {
      CreateSurfaceCard(nAddress,mb_mapping);
      nAddress += 407;
    }

    nAddress = 4704; // start of 5 Pump Card Buffer
    for( i= 0 ; i < 5; i++ )
    {
      CreatePumpCard(nAddress,mb_mapping);
      nAddress += 209;
    }

    nAddress = 5749; // start of single surface card buffer
    CreateSurfaceCard(nAddress,mb_mapping);

    nAddress = 6156; // start of single pump card buffer
    CreatePumpCard(nAddress,mb_mapping);
}

int main(int argc, char *argv[])
{
    int socket;
    modbus_t *ctx;
    modbus_mapping_t *mb_mapping;
    int rc;
    int use_backend;
    int nPort = 1500; // default for single instance
    int nPin5Value = 0; // for reading Pin5 switch value

    // allow user override of port number, this is done so I can run multiple instances on different ports with a bash script
    if( argc > 1 )
    {
	nPort = atoi(argv[2]);
    }

    wiringPiSetupPhys();
    pinMode(5,INPUT);
    
    // to emulate a large MODBUS device we need at least 10000 input and holding registers
    mb_mapping = modbus_mapping_new(1000, 1000, 10000, 10000);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
    
     /* TCP */
    if (argc > 1) {
        if (strcmp(argv[1], "tcp") == 0) {
            use_backend = TCP;
        } else if (strcmp(argv[1], "rtu") == 0) {
            use_backend = RTU;
        } else {
            printf("Usage:\n  %s [tcp Port|rtu] - Modbus client for testing a server\n\n", argv[0]);
            exit(1);
        }
    } else {
        /* By default */
        use_backend = TCP;
	printf("modserv:\n Running in tcp mode - Modbus client for testing a server\n\n");
    }

    if (use_backend == TCP) {
        printf("Waiting for TCP connection on Port %i \n",nPort);
        ctx = modbus_new_tcp("127.0.0.1", nPort);
        socket = modbus_tcp_listen(ctx, 1);
        modbus_tcp_accept(ctx, &socket);
	printf("TCP connection started!\n");
    } else {
	printf("Waiting for Serial connection on /dev/ttyUSB0\n");
        ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
        modbus_set_slave(ctx, 1);
        modbus_connect(ctx);
	printf("Serial connection started!\n");
    }    

    for(;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

	int nTempPin5 = digitalRead(5);
	if( nTempPin5 != nPin5Value )
	{
	    nPin5Value = nTempPin5;
	    printf("Pin 5 : %d\n",nPin5Value); // show the change in Pin 5 on the console!
	    
	    // now update the register I want to use for Pin 5 (why not use bit 5 for pin 5? that would be easy to remember)
            mb_mapping->tab_bits[5] = nPin5Value;
	}

        rc = modbus_receive(ctx, query);
        if (rc >= 0) {
	    int nToShow = 10;
	    int i=0,nCard=0;
	    
	    updateAllSurfaceCards(mb_mapping); // every request we want a new set of cards

	    printf("Replying to request num bytes=%i (",rc);
	    for(i=0;i<rc;i++)
	      printf("%i, ",query[i]);
	    printf(")\n");
	      
            modbus_reply(ctx, query, rc, mb_mapping);
	    
	    // after each communication, show the first ? ModBus registers so you can see what is happening
	    printf("tab_bits = ");
	    for( i=0;i<nToShow;i++)
		printf("%i, ",mb_mapping->tab_bits[i]);
	    printf("\n");
	    
	    printf("tab_input_bits = ");
	    for( i=0;i<nToShow;i++)
		printf("%i, ",mb_mapping->tab_input_bits[i]);
	    printf("\n");

	    printf("tab_input_registers = ");
	    for( i=0;i<nToShow;i++)
		printf("%i, ",mb_mapping->tab_input_registers[i]);
	    printf("\n");

	    printf("tab_registers = ");
	    for( i=0;i<nToShow;i++)
		printf("%i, ",mb_mapping->tab_registers[i]);
	    printf("\n");
	    
	    // every time we do a communication, update a bunch of the registers so we have something interesting to plot on the graphs
            mb_mapping->tab_registers[0]++; // increment the holding reg 0 for each read
            mb_mapping->tab_registers[1] = rand(); // this register is a full scale random number 0 - 0xffff
   	    mb_mapping->tab_input_registers[0] = 2; // version number
            for( i=1;i<nToShow;i++)
	    {
	        // randomly increase or decrease the register, but do not allow wrapping
		if( rand() > RAND_MAX/2 )
		{		
		    if ( mb_mapping->tab_input_registers[i] < 0xfffe );
			mb_mapping->tab_input_registers[i] += 1;

		    if( mb_mapping->tab_registers[i+1] < 0xfffe )
			mb_mapping->tab_registers[i+1] += 1;
		}
		else
		{
		    if( mb_mapping->tab_input_registers[i] > 0 )
		        mb_mapping->tab_input_registers[i] -= 1;
		    if( mb_mapping->tab_registers[i+1] > 0 )
		        mb_mapping->tab_registers[i+1] -= 1;
		} 
	    }
        } else {
            /* Connection closed by the client or server */
            printf("Con Closed.\n");
	    modbus_close(ctx); // close
	    // immediately start waiting for another request again
            modbus_tcp_accept(ctx, &socket);
        }
    }

    printf("Quit the loop: %s\n", modbus_strerror(errno));

    modbus_mapping_free(mb_mapping);
    close(socket);
    modbus_free(ctx);

    return 0;
}
