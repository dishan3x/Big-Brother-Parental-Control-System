#include "circularBuffer.h"

void InitBuffer(commBuffer_t* comm)
{
	comm->head = 0;
	comm->tail = 0;
	comm->size = 0;
	
}


void putChar(commBuffer_t* comm, char ch)
{
	
  
	if(comm->size == (MAXCOMMBUFFER - 1))
	{
		comm->buffer[comm->head] = '\n';
	}
		
	else
	{
		comm->buffer[comm->head] = ch;
		comm->head = (comm->head + 1) % MAXCOMMBUFFER;
		comm->size++;
	}
	
}


char getChar(commBuffer_t* comm)
{
	char temp = comm->buffer[comm->tail];
	if(comm->tail != comm->head)
	{
	   comm->tail = (comm->tail + 1) % MAXCOMMBUFFER; 
		 
	}
	comm->size--;
	
	return temp;
}


void putStr(commBuffer_t* comm, char* str, uint8_t length)
{
	int i;
	for(i = 0; i < length; i++)
	{
		putChar(comm,str[i]);
	}
}


void getStr(commBuffer_t* comm, char* str)
{
	int i = 0;
	do
	{
		str[i] = getChar(comm);
		i++;
	}while(str[i-1] != '\n' && str[i-1] != '\0');
	
}


uint8_t haveStr(commBuffer_t* comm)
{
	
	
  int i=comm->tail;
	
	if(comm->tail != comm->head)
	{
    do
	{
		if(comm->buffer[i] == '\n'||comm->buffer[i] == '\0')
		{
			return 1;
		}
		i = (i + 1)%MAXCOMMBUFFER;
	} while(i != (comm->head+1) % MAXCOMMBUFFER);
 }
	return 0;
}
