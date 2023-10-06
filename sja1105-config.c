/*
© [2018] Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software and 
any derivatives exclusively with Microchip products. It is your responsibility 
to comply with third party license terms applicable to your use of third party 
software (including open source software) that may accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER EXPRESS, 
IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES 
OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER 
RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF 
THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY 
LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS 
SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY 
TO MICROCHIP FOR THIS SOFTWARE. 
*/
#include "mcp2210_api.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define SPI_BUF_LEN							1024

void fill_pattern(uint8_t *pdata, int len);

int main(int argc, char **argv)
{
	int ictr, iRes;
	int fd;
	unsigned char txdata[SPI_BUF_LEN], rxdata[SPI_BUF_LEN];
    int spimode         = 0;
    int speed           = 100000;
    int actcsval        = 0xFFFE;
    int idlecsval       = 0xFFFF;
    int gpcsmask        = 0x0001;
    int cs2datadly      = 0;
    int data2datadly    = 0;
    int data2csdly      = 0;
    int xferlength      = 4;
	printf("\nMCP2210 Evaluation Board Tests");
	printf("\nParameters: %d\n", argc);
	for (ictr = 0; ictr < argc; ictr++)
	{
        switch(ictr)
        {
            case 0:
                break; //ignore this one
            case 1:
                printf("\nHID Raw index(%d) -> %s", ictr, argv[ictr]);
                break;
            case 2:
                printf("\nData to send -> %s", argv[ictr]);
                char *tok;
                tok = strtok(argv[2], ",");
                int cnt = 0;
                while (tok != NULL)
                {
                    txdata[cnt] = atoi(tok);
                    cnt++;
                    tok = strtok(NULL, ",");
                }
                xferlength = cnt;
                break;
            case 3:
                speed = atoi(argv[ictr]);
                printf("\nSpeed -> %s", argv[ictr]);
                break;
            default:
                printf("\nUnused argument -> %s", argv[ictr]);
                break;
        }
	}
	
	if (argc > 1)
	{
		printf("\nTry to open:\"%s\"", argv[1]);
		fd = open_device(argv[1]);
		if (fd > 0)
		{
			printf("\nDevice successfully opened\n\n");
			
			/* make an SPI transfer */
            iRes = spi_data_xfer(fd, txdata, rxdata, xferlength,
								spimode, speed, actcsval, idlecsval,
								gpcsmask, cs2datadly, data2datadly,
								data2csdly);
			printf("\nSPI Xfer() returned -> %d", iRes);
            printf("\nData received: ");
            for(ictr = 0; ictr < xferlength - 1; ictr++)
            {
               printf("%d,", rxdata[ictr]);
            }
            printf("%d", rxdata[xferlength - 1]);
                        
			iRes = close_device(fd);
			printf("\n\n\nDevice closed -> %d", iRes);
		}
		else
		{
			printf("\nCouldn't open the device! ... Exiting");
			return -1;
		}
	}
        else
        {
            printf("\nA path to the \"/dev/hidrawX\" is required\n");
        }
	
	printf("\nMCP2210 demo ended\n");
	return 0;
}
