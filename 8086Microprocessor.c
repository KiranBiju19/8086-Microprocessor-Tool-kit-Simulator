//project 8086
//this is a projrct to complete a compiler for 8086 Microprocessor

//memorysize is 100
//stack size is currently 10 


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

 
#define MAX 100
#define max 0x64

char instruction[100],*token;
FILE *fp;
unsigned long int AX = 0, BX = 0, CX = 0, DX = 0;// 16-bit general-purpose registers
unsigned short SP = 0, BP = 0, SI = 0, DI = 0;  // 8-bit Pointers and indexes
unsigned short CS = 0, DS = 0, SS = 0, ES = 0;  // Segment registers
unsigned long int value1, value2, value;
unsigned long int memory[100];
unsigned int OF=0, DF=0, IF=0, TF=0, SF=0, ZF=0, AF=0, PF=0, CF=0;
int lc=0,arith_flag=0;
unsigned long int temp;
int stack[1000],top=-1;
char reg[20]; 
unsigned int result;

unsigned long int AL = 0, AH = 0, BL = 0, BH = 0, CL = 0, CH = 0, DL=0, DH=0;
unsigned short memoryValue = 0; 

// Update 16-bit reg and 8-bit reg
void update_AX(){AX = (AH << 8) | AL; }
void split_AX() {AL = AX & 0x00FF;AH = (AX & 0xFF00) >> 8;}

void update_BX() { BX = (BH << 8) | BL; }
void split_BX() { BL = BX & 0x00FF; BH = (BX & 0xFF00) >> 8; }

void update_CX() { CX = (CH << 8) | CL; }
void split_CX() { CL = CX & 0x00FF; CH = (CX & 0xFF00) >> 8; }

void update_DX() { DX = (DH << 8) | DL; }
void split_DX() { DL = DX & 0x00FF; DH = (DX & 0xFF00) >> 8; }

// To convert into upper case
void to_lower(char *str)     
{
    for (int i = 0; str[i]; i++) 
        str[i] = toupper((unsigned char) str[i]);
}

// Reset Flags
void all_flag_reset()
{
    OF=0, DF=0, IF=0, TF=0, SF=0, ZF=0, AF=0, PF=0, CF=0;
}

// To check if one operand is register
int checkregX(const char *str)   
{
    if(strcmp(str,"AX")==0 || strcmp(str,"BX")==0 || strcmp(str,"CX")==0 || strcmp(str,"DX")==0)
        return 1;
    return 0;
}

// Extract values from []
unsigned long int extract(const char *str)     
{
    char number_str[20];  // Array to hold the extracted number string
    int i = 0, j = 0;
    while (str[i] != '\0') 
    {
        if (str[i] >= '0' && str[i] <= '9') 
            number_str[j++] = str[i];
        i++;
    }
    number_str[j] = '\0';
    unsigned long int hex_value = strtol(number_str, NULL, 10);
    //printf("%ld\n",hex_value);
    if(hex_value<MAX )
        return hex_value;
    else
    {
        printf("\n\t***********Out of Bound EXCEPTION********\n");
        exit(0);
    }    
}

// Convert [registers] to registers
unsigned long int regext(const char *str)
{
    int i = 1, j = 0;
    while (str[i] != ']') 
    {
        reg[j++] = str[i];
        i++;
    }
    reg[j] = '\0';
    
    to_lower(reg);
    if(checkregX(reg))
        return 1;
    else if(strcmp(reg,"SI")==0 || strcmp(reg,"DI")==0)
        return 1;
    return 0;
}

// To get the immediate value
int imd(const char *str)       
{
    int k=0;
    char arr[9];
    for(int i=0;i<strlen(str)-1;i++)
        arr[k++]=str[i];
    arr[k]='\0';
    value=strtol(arr,NULL,16);
    return value;
}

// To exchange values
int xchg(unsigned long int *a, unsigned long int *b)
{
    temp = *a;
    *a = *b;
    *b=temp;
}

// Rotate Right without Carry for 16-bit
unsigned int ror(unsigned int value, unsigned int bits) 
{
    unsigned int numBits = 16;
    return ((value >> bits) | (value << (numBits - bits))) & 0xFFFF; // Mask to 16 bits
}

// Rotate Left without Carry for 16-bit
unsigned int rol(unsigned int value, unsigned int bits)
{
    unsigned int numBits = 16;
    return ((value << bits) | (value >> (numBits - bits))) & 0xFFFF; // Mask to 16 bits
}

// Rotate Right through Carry for 16-bit
unsigned int rcr(unsigned int value, unsigned int bits, unsigned int *carryFlag) 
{
    unsigned int numBits = 16;
    for (unsigned int i = 0; i < bits; i++) {
        unsigned int lsb = value & 1;  // Get LSB of value
        value = (value >> 1) | (*carryFlag << (numBits - 1));
        *carryFlag = lsb;  // Set carry to the old LSB
    }
    return value & 0xFFFF; // Mask to 16 bits
}

// Rotate Left through Carry for 16-bit
unsigned int rcl(unsigned int value, unsigned int bits, unsigned int *carryFlag)
{
    unsigned int numBits = 16;
    for (unsigned int i = 0; i < bits; i++) {
        unsigned int msb = (value >> (numBits - 1)) & 1;  // Get MSB of value
        value = (value << 1) | *carryFlag;
        *carryFlag = msb;  // Set carry to the old MSB
    }
    return value & 0xFFFF; // Mask to 16 bits
}

// Check _L,_H regsters
int checkregHL(char* in)
{
    if(strcmp(in,"AL")==0 || strcmp(in,"BL")==0 || strcmp(in,"CL")==0 || strcmp(in,"DL")==0)
        return 1;
    else if(strcmp(in,"AH")==0 || strcmp(in,"BH")==0 || strcmp(in,"CH")==0 || strcmp(in,"DH")==0 )
        return 1;
    else 
        return 0;
}

// Rotate Right without Carry for 8 bit
unsigned char ror8(unsigned char value, unsigned int bits)
{
    unsigned int numBits = 8; // 8-bit value
    bits %= numBits;          // Handle overflow of bits
    return ((value >> bits) | (value << (numBits - bits))) & 0xFF; // Mask to 8 bits
}

// Rotate Left without Carry for 8 bit
unsigned char rol8(unsigned char value, unsigned int bits) 
{
    unsigned int numBits = 8; // 8-bit value
    bits %= numBits;          // Handle overflow of bits
    return ((value << bits) | (value >> (numBits - bits))) & 0xFF; // Mask to 8 bits
}

// Rotate Right through Carry for 8 bit
unsigned char rcr8(unsigned char value, unsigned int bits, unsigned int *carryFlag) 
{
    unsigned int numBits = 8; // 8-bit value
    bits %= numBits;          // Handle overflow of bits

    // Perform right rotation with carry
    unsigned char result = (value >> bits) | (*carryFlag << (numBits - bits));
    return result & 0xFF; // Mask to 8 bits
}

// Rotate Left through Carry for 8 bit
unsigned char rcl8(unsigned char value, unsigned int bits, unsigned int *carryFlag)
{
    unsigned int numBits = 8; // 8-bit value
    bits %= numBits;          // Handle overflow of bits

    // Perform left rotation with carry
    unsigned char result = (value << bits) | (*carryFlag >> (numBits - bits));
    return result & 0xFF; // Mask to 8 bits
}
    
// Used for 'operation ax,[1234]'' syntax
void setUp(char *arr)
{
    regext(arr);
    char numStr[20]; 
        
    if (strcmp(reg, "SI") == 0) // Handle [SI]
        memoryValue = (memory[SI + 1] << 8) | memory[SI]; // Combine high and low 8-bit values
    else if (strcmp(reg, "DI") == 0) // Handle [DI]
        memoryValue = (memory[DI + 1] << 8) | memory[DI]; // Combine high and low 8-bit values
    else // Handle direct memory address like [1200]
    {
        value = extract(arr); // Extract numeric address from arg2
        memoryValue = (memory[value + 1] << 8) | memory[value]; // Combine high and low 8-bit values
    }
    
}

// Update _H,_L reg
void setHL(char *arg1)
{
    if(strcmp(arg1,"AL")==0 || strcmp(arg1,"AH")==0)    update_AX();
    else if(strcmp(arg1,"BL")==0 || strcmp(arg1,"BH")==0)    update_BX();
    else if(strcmp(arg1,"CL")==0 || strcmp(arg1,"CH")==0)   update_CX();
    else if(strcmp(arg1,"DL")==0 || strcmp(arg1,"DH")==0)   update_DX();
} 

// Update _X reg
void setX(char *arg1)
{
    if(strcmp(arg1,"AX")==0)    split_AX();
    else if(strcmp(arg1,"BX")==0)    split_BX();
    else if(strcmp(arg1,"CX")==0)   split_CX();
    else if(strcmp(arg1,"DX")==0)   split_DX();
}

// To setup SI and DI
int Index(const char *arg2)
{
    char lpg[9];
    int k=0;
    for(int i=1;i<strlen(arg2);i++)
        lpg[k++] = arg2[i];
    lpg[k]='\0';
    return strtol(lpg,NULL,10);
}

// Function for Bitwise OR
unsigned int b_or(unsigned int a, unsigned int b) 
{
    return a | b;
}

// Function for Bitwise NOT
unsigned int b_not(unsigned int a)
{
    return ~a;
}

// Function for Bitwise XOR
unsigned int b_xor(unsigned int a, unsigned int b)
{
    return a ^ b;
}

/**************************  **********************/

// Data transfer instructions
void data_transfer(char *token,char *arg1, char *arg2)
{
    arith_flag=0;
    if(strcmp(token,"MOV")==0)              
    {
        value=imd(arg2);
        if(strcmp(arg1,"SI")==0)  
        {
            value=Index(arg2);
            if(value<MAX)
                SI=value;
            else 
            {
                printf("\n\t**********Check the size*******\n"); 
                exit(0);
                
            }
        }
        else if(strcmp(arg1,"DI")==0) 
        {
            value=Index(arg2);
            if(value<MAX)
                DI=value;
            else 
            {
                printf("\n\t**********Check the size*******\n"); 
                exit(0);
            }
        }
        
        if(checkregHL(arg1))    //mov ah,#12 , mov ah,[0003] , mov ah,al
        {
            if(arg2[0]=='[')      //mov ah,[0003]
            {
                value=extract(arg2);
                if(value>100)
                {
                    printf("\n Large size\n");
                    exit(0);
                }
                
                if(strcmp(arg1,"AH")==0)   AH=memory[value];  
                else if(strcmp(arg1,"AL")==0)   AL=memory[value];  
                else if (strcmp(arg1, "BH") == 0)   BH = memory[value];  
                else if (strcmp(arg1, "BL") == 0)   BL = memory[value];  
                else if (strcmp(arg1, "CH") == 0)     CH = memory[value];  
                else if (strcmp(arg1, "CL") == 0)     CL = memory[value];  
                else if (strcmp(arg1, "DH") == 0)     DH = memory[value];  
                else if (strcmp(arg1, "DL") == 0)     DL = memory[value];  
            }
            else if(arg2[strlen(arg2)-1]=='H')         //mov ah,0012H 
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("Size too much at line %d",lc);
                    exit(0);
                }                   
                
                if(strcmp(arg1,"AH")==0)   AH=value;  
                else if(strcmp(arg1,"AL")==0)   AL=value; 
                else if (strcmp(arg1, "BH") == 0)   BH = value;
                else if (strcmp(arg1, "BL") == 0)       BL = value;
                else if (strcmp(arg1, "CH") == 0)     CH = value;
                else if (strcmp(arg1, "CL") == 0)     CL = value;
                else if (strcmp(arg1, "DH") == 0)     DH = value;
                else if (strcmp(arg1, "DL") == 0)     DL = value;
               
            }
            else if(checkregHL(arg2)) // mov ah, al 
            {
                if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "AL") == 0) AH = AL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BH") == 0) AH = BH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BL") == 0) AH = BL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CH") == 0) AH = CH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CL") == 0) AH = CL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DH") == 0) AH = DH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DL") == 0) AH = DL;
                
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "AH") == 0) AL = AH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BH") == 0) AL = BH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BL") == 0) AL = BL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CH") == 0) AL = CH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CL") == 0) AL = CL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DH") == 0) AL = DH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DL") == 0) AL = DL;
                
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AH") == 0) BH = AH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AL") == 0) BH = AL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "BL") == 0) BH = BL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CH") == 0) BH = CH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CL") == 0) BH = CL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DH") == 0) BH = DH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DL") == 0) BH = DL;
                
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AH") == 0) BL = AH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AL") == 0) BL = AL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "BH") == 0) BL = BH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CH") == 0) BL = CH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CL") == 0) BL = CL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DH") == 0) BL = DH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DL") == 0) BL = DL;
                
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AH") == 0) CH = AH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AL") == 0) CH = AL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BH") == 0) CH = BH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BL") == 0) CH = BL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "CL") == 0) CH = CL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DH") == 0) CH = DH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DL") == 0) CH = DL;
                
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AH") == 0) CL = AH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AL") == 0) CL = AL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BH") == 0) CL = BH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BL") == 0) CL = BL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "CH") == 0) CL = CH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DH") == 0) CL = DH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DL") == 0) CL = DL;
                
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AH") == 0) DH = AH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AL") == 0) DH = AL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BH") == 0) DH = BH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BL") == 0) DH = BL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CH") == 0) DH = CH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CL") == 0) DH = CL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "DL") == 0) DH = DL;
                
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AH") == 0) DL = AH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AL") == 0) DL = AL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BH") == 0) DL = BH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BL") == 0) DL = BL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CH") == 0) DL = CH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CL") == 0) DL = CL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "DH") == 0) DL = DH;
                
                else { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
}

            }
            
            setHL(arg1);
            
        }
        
        else if (checkregX(arg1))    //mov ax,[1200] , mov ax,bx , mov ax,#1234
        {  
           if(checkregX(arg2))             //mov ax,bx
           {
               if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "BX") == 0) AX = BX;
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "CX") == 0) AX = CX;
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "DX") == 0) AX = DX;
                
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "AX") == 0) BX = AX;
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "CX") == 0) BX = CX;
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "DX") == 0) BX = DX;
                
                else if (strcmp(arg1, "CX") == 0 && strcmp(arg2, "AX") == 0) CX = AX;
                else if (strcmp(arg1, "CX") == 0 && strcmp(arg2, "BX") == 0) CX = BX;
                else if (strcmp(arg1, "CX") == 0 && strcmp(arg2, "DX") == 0) CX = DX;
                
                else if (strcmp(arg1, "DX") == 0 && strcmp(arg2, "AX") == 0) DX = AX;
                else if (strcmp(arg1, "DX") == 0 && strcmp(arg2, "BX") == 0) DX = BX;
                else if (strcmp(arg1, "DX") == 0 && strcmp(arg2, "CX") == 0) DX = CX;
                
                else{ printf("Unknown register-to-register operation.\n");
                        exit(0);
                    }
                //printf("\n\t Check ones\n\t");  
           }
           else if(arg2[strlen(arg2)-1]=='H')              //mov ax,#1234
           {
               if(value>0xFFFF)
                {
                    printf("Size too much at line %d",lc);
                    exit(0);
                }
               else 
                {
                value=imd(arg2);
                if (strcmp(arg1, "AX") == 0)        AX = value;
                else if (strcmp(arg1, "BX") == 0)   BX = value;
                else if (strcmp(arg1, "CX") == 0)   CX = value;
                else if (strcmp(arg1, "DX") == 0)   DX = value;
                else if (strcmp(arg1, "SI") == 0)   SI = value;
                else if (strcmp(arg1, "DI") == 0)   DI = value;
                else printf("Unknown operation.\n");
               }
                
           }
           else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']')              // mov ax,[1200]   mov ax,[si]
           {
                setUp(arg2);
                if (strcmp(arg1, "AX") == 0)
                    AX = memoryValue;
                else if (strcmp(arg1, "BX") == 0)
                    BX = memoryValue;
                else if (strcmp(arg1, "CX") == 0)
                    CX = memoryValue;
                else if (strcmp(arg1, "DX") == 0)
                    DX = memoryValue;
                else
                    printf("Wrong input");
            }
       
           setX(arg1);
        }
        
        else if(arg1[0] == '['  &&  arg1[strlen(arg1)-1] == ']')     //mov [0010],#1234 , mov [0010],ax , mov [ax],bx         
        {
            value=extract(arg1);
            if(regext(arg1) && checkregX(arg2))         // mov [ax],bx
            {
                if (strcmp(reg,"AX") == 0) value = (unsigned int)AX;
                else if (strcmp(reg,"BX") == 0) value = BX;
                else if (strcmp(reg,"CX") == 0) value = (unsigned int)CX;
                else if (strcmp(reg,"DX") == 0) value = (unsigned int)DX;
                
                if (value>MAX) 
                {
                    printf("\nMemoryout of bound"); 
                    exit(0);
                }
                
                printf("\t%04lX",AX);
                printf("\t%04lX",BX);
                
                printf("\t%04lX",value);
                printf("\t%ld",value);
                
                if (strcmp(arg2, "BX") == 0){ memory[value]= BX&0xFF; memory[value+1] = (BX >> 8) & 0xFF;}
                else if (strcmp(arg2, "CX") == 0){ memory[value]=CX&0xFF; memory[value+1] = (CX >> 8) & 0xFF;}
                else if (strcmp(arg2, "DX") == 0){ memory[value]=DX&0xFF; memory[value+1] = (DX >> 8) & 0xFF;}
                else if (strcmp(arg2, "AX") == 0){ memory[value]=AX&0xFF; memory[value+1] = (AX >> 8) & 0xFF;}
            }
            else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']' && checkregX(arg2))         //mov [0010],ax
            {
                if (strcmp(arg2, "AX") == 0)       // memory allocation of registers 16 bit 
                {
                    memory[value] = AX & 0xFF;         
                    memory[value+1] = (AX >> 8) & 0xFF;
                } 
                else if (strcmp(arg2, "BX") == 0)  
                {
                    memory[value] = BX & 0xFF;         
                    memory[value+1] = (BX >> 8) & 0xFF;
                }
                else if (strcmp(arg2, "CX") == 0)
                {
                    memory[value] = CX & 0xFF;         
                    memory[value+1] = (CX >> 8) & 0xFF;
                }
                else if (strcmp(arg2, "DX") == 0)  
                {
                    memory[value] = DX & 0xFF;         
                    memory[value+1] = (DX >> 8) & 0xFF;
                }
            }
            else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']' && checkregHL(arg2))    //mov [0020],AH
            {
                if(strcmp(arg2,"AH")==0)   memory[value] = AH & 0xFF;    
                else if(strcmp(arg2,"AL")==0)    memory[value] = AL & 0xFF; 
                else if (strcmp(arg2, "BH") == 0)    memory[value] = BH & 0xFF; 
                else if (strcmp(arg2, "BL") == 0)     memory[value] = BL & 0xFF; 
                else if (strcmp(arg2, "CH") == 0)  memory[value] = CH & 0xFF; 
                else if (strcmp(arg2, "CL") == 0)    memory[value] = CL & 0xFF; 
                else if (strcmp(arg2, "DH") == 0)    memory[value] = DH & 0xFF; 
                else if (strcmp(arg2, "DL") == 0)      memory[value] = DL & 0xFF; 
            }
            else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']' &&  arg2[strlen(arg2)-1]=='H')         //mov [0010],#1234
            {
                value=imd(arg2);
                value1=extract(arg1);
                memory[value1] = value & 0xFF;         
                memory[value1+1] = (value >> 8) & 0xFF;
            }
            
        }
    }
    
    else if(strcmp(token,"PUSH")==0)
    {
        if(checkregX(arg1))
        {
            if(strcmp(arg1,"AX")==0)    stack[++top] = AX;
            else if(strcmp(arg1,"BX")==0)   stack[++top] = BX;
            else if(strcmp(arg1,"CX")==0)   stack[++top] = CX;
            else if(strcmp(arg1,"DX")==0)   stack[++top] = DX;
        }
        else if(arg1[0]=='[' && arg1[strlen(arg1)-1] ==']' && strlen(arg1)==4 )
        {
            if(strcmp(arg1,"[AX]")==0)    stack[++top] = memory[AX];
            else if(strcmp(arg1,"[BX]")==0)   stack[++top] = memory[BX];
            else if(strcmp(arg1,"[CX]")==0)   stack[++top] = memory[CX];
            else if(strcmp(arg1,"[DX]")==0)   stack[++top] = memory[DX];
        }
        else if(checkregHL(arg1))
        {
            if(strcmp(arg1,"AH")==0)   stack[++top] = AH;  
            else if (strcmp(arg1, "AL") == 0)    stack[++top] = AL; 
            else if (strcmp(arg1, "BH") == 0)    stack[++top] = BH;
            else if (strcmp(arg1, "BL") == 0)    stack[++top] = BL;
            else if (strcmp(arg1, "CH") == 0)    stack[++top] = CH;
            else if (strcmp(arg1, "CL") == 0)    stack[++top] = CL;
            else if (strcmp(arg1, "DH") == 0)    stack[++top] = DH;
            else if (strcmp(arg1, "DL") == 0)    stack[++top] = DL;
        }
        
        if(top>=max)
            {
                printf("\n\tStack limit reached.....\nStack overflow!!!!!");
                exit(0);
            }
    }
    
    else if(strcmp(token,"POP")==0)
    {
        if(checkregX(arg1))
        {
            if(strcmp(arg1,"AX")==0)    AX = stack[top--] ;
            else if(strcmp(arg1,"BX")==0)    BX = stack[top--] ;
            else if(strcmp(arg1,"CX")==0)    CX = stack[top--] ;
            else if(strcmp(arg1,"DX")==0)    DX = stack[top--] ;
        }
        else if(arg1[0]=='[' && arg1[strlen(arg1)-1] ==']' && strlen(arg1)==4 )
        {
            if(strcmp(arg1,"[AX]")==0)    memory[AX] = stack[top--] ;
            else if(strcmp(arg1,"[BX]")==0)   memory[BX] = stack[top--] ;
            else if(strcmp(arg1,"[CX]")==0)   memory[CX] = stack[top--] ;
            else if(strcmp(arg1,"[DX]")==0)   memory[DX] = stack[top--] ;
        }
            if(top<=max)
            {
                printf("\n\tStack limit reached.....\nStack Underflow!!!!");
                exit(0);
            }
    }
    
    else if(strcmp(token,"XCHG")==0)
    {
        unsigned long int temp;
        if(checkregX(arg1))
        {
            if(checkregX(arg2))
            {
                if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "BX") == 0 || strcmp(arg1, "BX") == 0 && strcmp(arg2, "AX") == 0) 
                    xchg(&AX, &BX);
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "CX") == 0 || strcmp(arg1, "CX") == 0 && strcmp(arg2, "AX") == 0) 
                    xchg(&AX, &CX);
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "DX") == 0 || strcmp(arg1, "DX") == 0 && strcmp(arg2, "AX") == 0) 
                    xchg(&AX, &DX);
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "CX") == 0 || strcmp(arg1, "CX") == 0 && strcmp(arg2, "BX") == 0)
                    xchg(&BX, &CX);
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "DX") == 0 || strcmp(arg1, "DX") == 0 && strcmp(arg2, "BX") == 0) 
                    xchg(&BX, &DX);
                else if (strcmp(arg1, "CX") == 0 && strcmp(arg2, "DX") == 0 || strcmp(arg1, "DX") == 0 && strcmp(arg2, "CX") == 0)
                    xchg(&CX, &DX);
                else 
                    printf("Invalid register names.\n");
            }
            
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']')
            {
                value=extract(arg2);
                //xchg(arg1)
                if (strcmp(arg1, "AX") == 0) xchg(&AX, &memory[value]);
                else if (strcmp(arg1, "BX") == 0) xchg(&BX, &memory[value]);
                else if (strcmp(arg1, "CX") == 0 ) xchg(&CX, &memory[value]);
                else if (strcmp(arg1, "DX") == 0 ) xchg(&DX, &memory[value]);
                else printf("Invalid register names.\n");
            }
        }
    }
    
    else if(strcmp(token,"CLC")==0)
    {
        CF=0;
    }
    
    else if(strcmp(token,"STC")==0)
    {
        CF=1;
    }
    
    else if(strcmp(token,"CMC")==0)
    {
        CF=~CF;
    }
    
    else if(strcmp(token,"CLD")==0)
    {
        DF=0;
    }
    
    else if(strcmp(token,"STD")==0)
    {
        DF=1;
    }
    
    else if(strcmp(token,"CLI")==0)
    {
        IF=0;
    }
    
    else if(strcmp(token,"STi")==0)
    {
        IF=1;
    }
}

// Arithmetic instructions
void arithmetic(char *token,char *arg1, char *arg2)
{
    arith_flag=1;    
    if(strcmp(token,"ADD")==0 || strcmp(token,"ADC")==0)
    {
        if(CF==1 && strcmp(token,"ADC")==0 )
        {
            value=CF;
            if(strcmp(arg1,"AX")==0) AX += value;
            else if (strcmp(arg1, "BX") == 0)   BX += value;
            else if (strcmp(arg1, "CX") == 0)   CX += value;
            else if (strcmp(arg1, "DX") == 0)   DX += value;
            
            else if (strcmp(arg1, "AH") == 0)   AH += value;  
            else if (strcmp(arg1, "AL") == 0)   AL += value; 
            else if (strcmp(arg1, "BH") == 0)   BH += value;
            else if (strcmp(arg1, "BL") == 0)   BL += value;
            else if (strcmp(arg1, "CH") == 0)   CH += value;
            else if (strcmp(arg1, "CL") == 0)   CL += value;
            else if (strcmp(arg1, "DH") == 0)   DH += value;
            else if (strcmp(arg1, "DL") == 0)   DL += value;
            CF=0;
        }
        //printf("\t%d",CF);
        unsigned int result;
        
        if(checkregHL(arg1))      //add ah,al  add ah,#23  add ah,[12340]
        {
            if(checkregHL(arg2))          //add ah,al
            {
                if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "AL") == 0) AH += AL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BH") == 0) AH += BH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BL") == 0) AH += BL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CH") == 0) AH += CH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CL") == 0) AH += CL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DH") == 0) AH += DH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DL") == 0) AH += DL;
                
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "AH") == 0) AL += AH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BH") == 0) AL += BH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BL") == 0) AL += BL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CH") == 0) AL += CH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CL") == 0) AL += CL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DH") == 0) AL += DH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DL") == 0) AL += DL;
                
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AH") == 0) BH += AH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AL") == 0) BH += AL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "BL") == 0) BH += BL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CH") == 0) BH += CH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CL") == 0) BH += CL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DH") == 0) BH += DH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DL") == 0) BH += DL;
                
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AH") == 0) BL += AH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AL") == 0) BL += AL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "BH") == 0) BL += BH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CH") == 0) BL += CH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CL") == 0) BL += CL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DH") == 0) BL += DH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DL") == 0) BL += DL;
                
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AH") == 0) CH += AH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AL") == 0) CH += AL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BH") == 0) CH += BH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BL") == 0) CH += BL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "CL") == 0) CH += CL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DH") == 0) CH += DH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DL") == 0) CH += DL;
                
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AH") == 0) CL += AH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AL") == 0) CL += AL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BH") == 0) CL += BH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BL") == 0) CL += BL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "CH") == 0) CL += CH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DH") == 0) CL += DH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DL") == 0) CL += DL;
                
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AH") == 0) DH += AH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AL") == 0) DH += AL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BH") == 0) DH += BH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BL") == 0) DH += BL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CH") == 0) DH += CH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CL") == 0) DH += CL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "DL") == 0) DH += DL;
                
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AH") == 0) DL += AH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AL") == 0) DL += AL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BH") == 0) DL += BH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BL") == 0) DL += BL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CH") == 0) DL += CH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CL") == 0) DL += CL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "DH") == 0) DL += DH;
                
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }
            }
            else if(arg2[strlen(arg2)-1]=='H')       // add ah,#12
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH += value;  
                else if (strcmp(arg1, "AL") == 0)   AL += value; 
                else if (strcmp(arg1, "BH") == 0)   BH += value;
                else if (strcmp(arg1, "BL") == 0)   BL += value;
                else if (strcmp(arg1, "CH") == 0)   CH += value;
                else if (strcmp(arg1, "CL") == 0)   CL += value;
                else if (strcmp(arg1, "DH") == 0)   DH += value;
                else if (strcmp(arg1, "DL") == 0)   DL += value;
            }
            else if(arg2[0]=='[' )             // add ah,[1234]
            {
                value=extract(arg2);
                if(value>max)
                {
                    printf("memory out of bound");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH+=memory[value];  
                else if(strcmp(arg1,"AL")==0)   AL+=memory[value];  
                else if (strcmp(arg1, "BH") == 0)   BH += memory[value];  
                else if (strcmp(arg1, "BL") == 0)   BL += memory[value]; 
                else if (strcmp(arg1, "CH") == 0)   CH += memory[value];  
                else if (strcmp(arg1, "CL") == 0)   CL += memory[value];  
                else if (strcmp(arg1, "DH") == 0)   DH += memory[value];  
                else if (strcmp(arg1, "DL") == 0)   DL += memory[value];  
            }
            
            setHL(arg1);
            
            if(strcmp(arg1,"AH")==0)   result = AH;  
            else if(strcmp(arg1,"AL")==0)   result = AL; 
            else if (strcmp(arg1, "BH") == 0)   result = BH;
            else if (strcmp(arg1, "BL") == 0)   result = BL;
            else if (strcmp(arg1, "CH") == 0)   result = CH;
            else if (strcmp(arg1, "CL") == 0)   result = CL;
            else if (strcmp(arg1, "DH") == 0)  result = DH;
            else if (strcmp(arg1, "DL") == 0)   result = DL;
            if (result > 0xFF) 
                AF=1;
            else if(result == 0x000)
                ZF=1;
            
        }
        //here below its a problem with adding from memory it will be also there for sub be careful abt that 
        else if(checkregX(arg1))          //add ax,#1234     add ax,bx   add ax,[si]  add ax,[1200]  
        {
            if(arg2[strlen(arg2)-1]=='H')     //add ax,#1234
            {
                value=imd(arg2);
                if(strcmp(arg1,"AX")==0) AX+=value;
                else if(strcmp(arg1,"BX")==0) BX+=value;
                else if(strcmp(arg1,"CX")==0) CX+=value;
                else if(strcmp(arg1,"DX")==0) DX+=value;
                else
                    printf("Wrong input");
            }
            else if(checkregX(arg2))     //add ax,bx 
            {   
                if(strcmp(arg1,"AX")==0 && strcmp(arg2,"BX")==0) AX+=BX;
                else if(strcmp(arg1,"AX")==0 && strcmp(arg2,"CX")==0) AX+=CX;
                else if(strcmp(arg1,"AX")==0 && strcmp(arg2,"DX")==0) AX+=DX;
                
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"AX")==0) BX+=AX;
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"CX")==0) BX+=CX;
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"DX")==0) BX+=DX;
                
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"BX")==0) CX+=BX;
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"AX")==0) CX+=AX;
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"DX")==0) CX+=DX;
                
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"BX")==0) DX+=BX;
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"CX")==0) DX+=CX;
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"AX")==0) DX+=AX;
                else
                    printf("Wrong input");
        }
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']') //add ax,[si] add ax,[1234]
            {
                setUp(arg2);
                if (strcmp(arg1, "AX") == 0)
                    AX += memoryValue;
                else if (strcmp(arg1, "BX") == 0)
                    BX += memoryValue;
                else if (strcmp(arg1, "CX") == 0)
                    CX += memoryValue;
                else if (strcmp(arg1, "DX") == 0)
                    DX += memoryValue;
                else
                    printf("Wrong input");
            }  
            setX(arg1);
           
           if(strcmp(arg1,"AX")==0)    result = AX;
           else if(strcmp(arg1,"BX")==0)   result = BX;
           else if(strcmp(arg1,"CX")==0)   result = CX;
           else if(strcmp(arg1,"DX")==0)   result = DX;
            
    
            // Detect carry
            if (result > 0xFFFF) 
            {
                CF=1;
                OF=1;
                result &= 0xFFFF;// Mask to keep it within 16 bits
                printf("\t%d",CF);
            } 
            else if(result == 0x0000)
                ZF=1;
        }
    }
    
    else if(strcmp(token,"SUB")==0 || strcmp(token,"SBB")==0)
    {
        if(CF==1 && strcmp(token,"SBB")==0)
        {
                value=CF;
                if(strcmp(arg1,"AX")==0) AX-=value;
                else if (strcmp(arg1, "BX") == 0) BX-=value;
                else if (strcmp(arg1, "CX") == 0) CX-=value;
                else if (strcmp(arg1, "DX") == 0) DX-=value;
                
                else if (strcmp(arg1, "AH") == 0)   AH-=value;  
                else if (strcmp(arg1, "AL") == 0)   AL-=value; 
                else if (strcmp(arg1, "BH") == 0)   BH -= value;
                else if (strcmp(arg1, "BL") == 0)   BL -= value;
                else if (strcmp(arg1, "CH") == 0)   CH -= value;
                else if (strcmp(arg1, "CL") == 0)   CL -= value;
                else if (strcmp(arg1, "DH") == 0)   DH -= value;
                else if (strcmp(arg1, "DL") == 0)   DL -= value;
            }
        unsigned int result;
        if(checkregHL(arg1))      //sub ah,al  sub ah,23H  sub ah,[1234]
        {
            if(checkregHL(arg2))          //sub ah,al
            {
                if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "AL") == 0) AH -= AL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BH") == 0) AH -= BH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BL") == 0) AH -= BL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CH") == 0) AH -= CH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CL") == 0) AH -= CL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DH") == 0) AH -= DH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DL") == 0) AH -= DL;
                
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "AH") == 0) AL -= AH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BH") == 0) AL -= BH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BL") == 0) AL -= BL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CH") == 0) AL -= CH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CL") == 0) AL -= CL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DH") == 0) AL -= DH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DL") == 0) AL -= DL;
                
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AH") == 0) BH -= AH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AL") == 0) BH -= AL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "BL") == 0) BH -= BL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CH") == 0) BH -= CH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CL") == 0) BH -= CL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DH") == 0) BH -= DH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DL") == 0) BH -= DL;
                
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AH") == 0) BL -= AH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AL") == 0) BL -= AL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "BH") == 0) BL -= BH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CH") == 0) BL -= CH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CL") == 0) BL -= CL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DH") == 0) BL -= DH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DL") == 0) BL -= DL;
                
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AH") == 0) CH -= AH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AL") == 0) CH -= AL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BH") == 0) CH -= BH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BL") == 0) CH -= BL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "CL") == 0) CH -= CL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DH") == 0) CH -= DH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DL") == 0) CH -= DL;
                
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AH") == 0) CL -= AH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AL") == 0) CL -= AL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BH") == 0) CL -= BH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BL") == 0) CL -= BL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "CH") == 0) CL -= CH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DH") == 0) CL -= DH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DL") == 0) CL -= DL;
                
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AH") == 0) DH -= AH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AL") == 0) DH -= AL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BH") == 0) DH -= BH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BL") == 0) DH -= BL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CH") == 0) DH -= CH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CL") == 0) DH -= CL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "DL") == 0) DH -= DL;
                
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AH") == 0) DL -= AH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AL") == 0) DL -= AL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BH") == 0) DL -= BH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BL") == 0) DL -= BL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CH") == 0) DL -= CH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CL") == 0) DL -= CL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "DH") == 0) DL -= DH;
                
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }
            }
            else if(arg2[strlen(arg2)-1]=='H')    // sub ah, 12H
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                //printf("%04lX\n",AL);
                
                if(strcmp(arg1,"AH")==0)    AH -=value;  
                else if(strcmp(arg1,"AL")==0)   AL -=value; 
                else if (strcmp(arg1, "BH") == 0)   BH -= value;
                else if (strcmp(arg1, "BL") == 0)   BL -= value;
                else if (strcmp(arg1, "CH") == 0)   CH -= value;
                else if (strcmp(arg1, "CL") == 0)   CL -= value;
                else if (strcmp(arg1, "DH") == 0)   DH -= value;
                else if (strcmp(arg1, "DL") == 0)   DL -= value;
                
            }
            else if(arg2[0]=='[' )             // sub ah,[1234]
            {
                value=extract(arg2);
                if(value>max)
                {
                    printf("memory out of bound");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH -=memory[value];  
                else if(strcmp(arg1,"AL")==0)   AL -=memory[value];  
                else if (strcmp(arg1, "BH") == 0)   BH -= memory[value];  
                else if (strcmp(arg1, "BL") == 0)   BL -= memory[value];  
                else if (strcmp(arg1, "CH") == 0)   CH -= memory[value];  
                else if (strcmp(arg1, "CL") == 0)   CL -= memory[value];  
                else if (strcmp(arg1, "DH") == 0)   DH -= memory[value];  
                else if (strcmp(arg1, "DL") == 0)   DL -= memory[value];  
            }
            
            if(strcmp(arg1,"AL")==0 || strcmp(arg1,"AH")==0)    update_AX();
            else if(strcmp(arg1,"BL")==0 || strcmp(arg1,"BH")==0)    update_BX();
            else if(strcmp(arg1,"CL")==0 || strcmp(arg1,"CH")==0)   update_CX();
            else if(strcmp(arg1,"DL")==0 || strcmp(arg1,"DH")==0)   update_DX();
            
            if(strcmp(arg1,"AH")==0)   result = AH;  
            else if(strcmp(arg1,"AL")==0)   result = AL; 
            else if (strcmp(arg1, "BH") == 0)   result = BH;
            else if (strcmp(arg1, "BL") == 0)   result = BL;
            else if (strcmp(arg1, "CH") == 0)   result = CH;
            else if (strcmp(arg1, "CL") == 0)   result = CL;
            else if (strcmp(arg1, "DH") == 0)  result = DH;
            else if (strcmp(arg1, "DL") == 0)   result = DL;
            
             if(result > 0xFF)
            {
                CF = 1;
                SF = 1;
                OF = 1;
            }
            else 
            {
                CF = 0;
                SF = 0;
                OF = 0;
            }
             
        }
        else if(checkregX(arg1))   // sub ax,1020H , sub ax, bx , sub ax,[1200]
        {
            if(arg2[strlen(arg2)-1]=='H')      //sub ax, 1020h
            {
                value=imd(arg2);
                if(strcmp(arg1,"AX")==0) AX-=value;
                else if(strcmp(arg1,"BX")==0) BX-=value;
                else if(strcmp(arg1,"CX")==0) CX-=value;
                else if(strcmp(arg1,"DX")==0) DX-=value;
                else
                    printf("Wrong input");
        }
            else if(checkregX(arg2))    //sub ax,bx 
            {
                if(strcmp(arg1,"AX")==0 && strcmp(arg2,"BX")==0) AX-=BX;
                else if(strcmp(arg1,"AX")==0 && strcmp(arg2,"CX")==0) AX-=CX;
                else if(strcmp(arg1,"AX")==0 && strcmp(arg2,"DX")==0) AX-=DX;
                
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"AX")==0) BX-=AX;
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"CX")==0) BX-=CX;
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"DX")==0) BX-=DX;
                
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"BX")==0) CX-=BX;
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"AX")==0) CX-=AX;
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"DX")==0) CX-=DX;
                
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"BX")==0) DX-=BX;
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"CX")==0) DX-=CX;
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"AX")==0) DX-=AX;
                else
                    printf("Wrong input");
        }
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']' ) //sub ax,[1200]
            {
                setUp(arg2);
                if (strcmp(arg1, "AX") == 0)
                    AX -= memoryValue;
                else if (strcmp(arg1, "BX") == 0)
                    BX -= memoryValue;
                else if (strcmp(arg1, "CX") == 0)
                    CX -= memoryValue;
                else if (strcmp(arg1, "DX") == 0)
                    DX -= memoryValue;
                else
                    printf("Wrong input");
            }  
            setX(arg1);
            
            if(strcmp(arg1,"AX")==0)    result = AX;
            else if(strcmp(arg1,"BX")==0)   result = BX;
            else if(strcmp(arg1,"CX")==0)   result = CX;
            else if(strcmp(arg1,"DX")==0)   result = DX;
            
            if(result > 0xFFFF)
            {
                CF = 1;
                SF = 1;
                OF = 1;
            }
        }
        
    }
    
    else if(strcmp(token,"INC")==0)
    {
        if(checkregX(arg1))
        {
            if(strcmp(arg1,"AX")==0)        AX++;
            else if(strcmp(arg1,"BX")==0)   BX++;
            else if(strcmp(arg1,"CX")==0)   CX++;
            else if(strcmp(arg1,"DX")==0)   DX++;
            
            setX(arg1);
            
            if(strcmp(arg1,"AX")==0)    result = AX;
           else if(strcmp(arg1,"BX")==0)   result = BX;
           else if(strcmp(arg1,"CX")==0)   result = CX;
           else if(strcmp(arg1,"DX")==0)   result = DX;
            
    
            // Detect carry
            if (result > 0xFFFF) 
            {
                CF=1;
                OF=1;
                result &= 0xFFFF;// Mask to keep it within 16 bits
            } 
            else if(result == 0x0000)
                ZF=1;
        }
        else if(checkregHL(arg1))
        {
            if (strcmp(arg1, "AH") == 0)   AH++;
            else if (strcmp(arg1, "AL") == 0)   AL++;
            else if (strcmp(arg1, "BH") == 0)   BH++;
            else if (strcmp(arg1, "BL") == 0)   BL++;
            else if (strcmp(arg1, "CH") == 0)   CH++;
            else if (strcmp(arg1, "CL") == 0)   CL++;
            else if (strcmp(arg1, "DH") == 0)   DH++;
            else if (strcmp(arg1, "DL") == 0)   DL++;
            
            setHL(arg1);
            
            if(strcmp(arg1,"AH")==0)   result = AH;  
            else if(strcmp(arg1,"AL")==0)   result = AL; 
            else if (strcmp(arg1, "BH") == 0)   result = BH;
            else if (strcmp(arg1, "BL") == 0)   result = BL;
            else if (strcmp(arg1, "CH") == 0)   result = CH;
            else if (strcmp(arg1, "CL") == 0)   result = CL;
            else if (strcmp(arg1, "DH") == 0)  result = DH;
            else if (strcmp(arg1, "DL") == 0)   result = DL;
            if (result > 0xFF) 
                AF=1;
            else if(result == 0x000)
                ZF=1;
        }
        else if(strcmp(arg1,"SI")==0)   SI++;
        else if(strcmp(arg1,"DI")==0)   DI++;
        
        //printf("%04X",SI);
    }
    
    else if(strcmp(token,"DEC")==0)
    {
        if(checkregX(arg1))
        {
            if(strcmp(arg1,"AX")==0)        AX--;
            else if(strcmp(arg1,"BX")==0)   BX--;
            else if(strcmp(arg1,"CX")==0)   CX--;
            else if(strcmp(arg1,"DX")==0)   DX--;
            
            setX(arg1);
            
            if(strcmp(arg1,"AX")==0)    result = AX;
            else if(strcmp(arg1,"BX")==0)   result = BX;
            else if(strcmp(arg1,"CX")==0)   result = CX;
            else if(strcmp(arg1,"DX")==0)   result = DX;
            
    
            // Detect carry
            if (result > 0xFFFF) 
            {
                CF=1;
                OF=1;
                result &= 0xFFFF;// Mask to keep it within 16 bits
            } 
            else if(result == 0x0000)
                ZF=1;
        }
        else if(checkregHL(arg1))
        {
            if (strcmp(arg1, "AH") == 0)   AH--;
            else if (strcmp(arg1, "AL") == 0)   AL--;
            else if (strcmp(arg1, "BH") == 0)   BH--;
            else if (strcmp(arg1, "BL") == 0)   BL--;
            else if (strcmp(arg1, "CH") == 0)   CH--;
            else if (strcmp(arg1, "CL") == 0)   CL--;
            else if (strcmp(arg1, "DH") == 0)   DH--;
            else if (strcmp(arg1, "DL") == 0)   DL--;
            
            setHL(arg1);
            
            if(strcmp(arg1,"AH")==0)   result = AH;  
            else if(strcmp(arg1,"AL")==0)   result = AL; 
            else if (strcmp(arg1, "BH") == 0)   result = BH;
            else if (strcmp(arg1, "BL") == 0)   result = BL;
            else if (strcmp(arg1, "CH") == 0)   result = CH;
            else if (strcmp(arg1, "CL") == 0)   result = CL;
            else if (strcmp(arg1, "DH") == 0)  result = DH;
            else if (strcmp(arg1, "DL") == 0)   result = DL;
            if (result > 0xFF) 
                AF=1;
            else if(result == 0x000)
                ZF=1;
        }
        else if(strcmp(arg1,"SI")==0)   SI--;
        else if(strcmp(arg1,"DI")==0)   DI--;
        
    }
    
    else if(strcmp(token,"MUL")==0)
    {
        if(checkregX(arg1))
        {
            unsigned int result = AX * (strcmp(arg1, "BX") == 0 ? BX : CX);
            
            AX = result & 0xFFFF;         // Lower 16 bits
            DX = (result >> 16) & 0xFFFF; // Upper 16 bits
            
            if(strcmp(arg1,"AX")==0)
            {
                split_AX();
                split_DX();
            }
        }
        else if(checkregHL(arg1))
        {
            if (strcmp(arg1, "AH") == 0)  AX=AL*AH;
            else if (strcmp(arg1, "AL") == 0)   AX=AL*AL;
            else if (strcmp(arg1, "BH") == 0)   AX=AL*BH;
            else if (strcmp(arg1, "BL") == 0)   AX=AL*BL;
            else if (strcmp(arg1, "CH") == 0)   AX=AL*CH;
            else if (strcmp(arg1, "CL") == 0)   AX=AL*CL;
            else if (strcmp(arg1, "DH") == 0)   AX=AL*DH;
            else if (strcmp(arg1, "DL") == 0)   AX=AL*DL;
            
            split_AX();
        }
    }
    
    else if(strcmp(token,"DIV")==0)
    {
        if(checkregX(arg1))
        {
            unsigned int result = AX / (strcmp(arg1, "BX") == 0 ? BX : CX);
            DX = AX % (strcmp(arg1, "BX") == 0 ? BX : CX);
            
            AX = result & 0xFFFF;  
            if(strcmp(arg1,"AX")==0)
            {
                split_AX();
                split_DX();
            }
        }
        else if(checkregHL(arg1))
        {
            unsigned long int reg;
            if (strcmp(arg1, "BH") == 0)   reg=BH;
            else if (strcmp(arg1, "BL") == 0)   reg=BL;
            else if (strcmp(arg1, "CH") == 0)   reg=CH;
            else if (strcmp(arg1, "CL") == 0)   reg=CL;
            else if (strcmp(arg1, "DH") == 0)   reg=DH;
            else if (strcmp(arg1, "DL") == 0)   reg=DL;
            if (reg == 0)
            {
                printf("\n\tDivision by zero error in register !!!!\n");
                exit(0);
            }
            AL = AX / reg;      // Quotient (lower 8 bits)
            AH = AX % reg;      // Remainder (upper 8 bits)
            update_AX();
        }
    }
    
    else if(strcmp(token,"CMP")==0)
    {
        //d,s
        //if s>d   cf=1,sf=1    else cf=0
        //if d-s==0    zf=1   else zf=0
        result=0;
        if(checkregX(arg1))
        {
            if(checkregX(arg2))
            {   
                if(strcmp(arg1,"AX")==0 && strcmp(arg2,"BX")==0) result=AX-BX;
                else if(strcmp(arg1,"AX")==0 && strcmp(arg2,"CX")==0) result=AX-CX;
                else if(strcmp(arg1,"AX")==0 && strcmp(arg2,"DX")==0) result=AX-DX;
                
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"AX")==0) result=BX-AX;
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"CX")==0) result=BX-CX;
                else if(strcmp(arg1,"BX")==0 && strcmp(arg2,"DX")==0) result=BX-DX;
                
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"BX")==0) result=CX-BX;
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"AX")==0) result=CX-AX;
                else if(strcmp(arg1,"CX")==0 && strcmp(arg2,"DX")==0) result=CX-DX;
                
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"BX")==0) result=DX-BX;
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"CX")==0) result=DX-CX;
                else if(strcmp(arg1,"DX")==0 && strcmp(arg2,"AX")==0) result=DX-AX;
            }
            else if(arg2[strlen(arg2)-1]=='H')   
            {
                value=imd(arg2);
                if(strcmp(arg1,"AX")==0)        result=AX-value;
                else if(strcmp(arg1,"BX")==0)   result=BX-value;
                else if(strcmp(arg1,"CX")==0)   result=CX-value;
                else if(strcmp(arg1,"DX")==0)   result=DX-value;
            }
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']')
            {
                setUp(arg2);
                if (strcmp(arg1, "AX") == 0)
                    result = AX - memoryValue;
                else if (strcmp(arg1, "BX") == 0)
                    result = BX - memoryValue;
                else if (strcmp(arg1, "CX") == 0)
                    result = CX - memoryValue;
                else if (strcmp(arg1, "DX") == 0)
                    result = DX - memoryValue;
                else
                    printf("Wrong input");
            }
            //printf("%04X",result);
            if(result>0xFFFF)
            {
                CF=1;
                SF=1;
            }
            else if(result==0x0000)
                ZF=1;
        }
        else if(checkregHL(arg1))
        {
            if(checkregHL(arg2))          
            {
                
                if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "AL") == 0) result = AH - AL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BH") == 0) result = AH - BH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BL") == 0) result = AH - BL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CH") == 0) result = AH - CH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CL") == 0) result = AH - CL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DH") == 0) result = AH - DH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DL") == 0) result = AH - DL;
            
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "AH") == 0) result = AL - AH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BH") == 0) result = AL - BH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BL") == 0) result = AL - BL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CH") == 0) result = AL - CH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CL") == 0) result = AL - CL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DH") == 0) result = AL - DH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DL") == 0) result = AL - DL;
            
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AH") == 0) result = BH - AH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AL") == 0) result = BH - AL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "BL") == 0) result = BH - BL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CH") == 0) result = BH - CH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CL") == 0) result = BH - CL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DH") == 0) result = BH - DH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DL") == 0) result = BH - DL;
            
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AH") == 0) result = BL - AH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AL") == 0) result = BL - AL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "BH") == 0) result = BL - BH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CH") == 0) result = BL - CH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CL") == 0) result = BL - CL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DH") == 0) result = BL - DH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DL") == 0) result = BL - DL;
            
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AH") == 0) result = CH - AH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AL") == 0) result = CH - AL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BH") == 0) result = CH - BH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BL") == 0) result = CH - BL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "CL") == 0) result = CH - CL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DH") == 0) result = CH - DH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DL") == 0) result = CH - DL;
            
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AH") == 0) result = CL - AH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AL") == 0) result = CL - AL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BH") == 0) result = CL - BH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BL") == 0) result = CL - BL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "CH") == 0) result = CL - CH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DH") == 0) result = CL - DH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DL") == 0) result = CL - DL;
            
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AH") == 0) result = DH - AH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AL") == 0) result = DH - AL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BH") == 0) result = DH - BH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BL") == 0) result = DH - BL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CH") == 0) result = DH - CH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CL") == 0) result = DH - CL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "DL") == 0) result = DH - DL;
            
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AH") == 0) result = DL - AH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AL") == 0) result = DL - AL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BH") == 0) result = DL - BH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BL") == 0) result = DL - BL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CH") == 0) result = DL - CH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CL") == 0) result = DL - CL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "DH") == 0) result = DL - DH;
            
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }
            }
            else if(arg2[strlen(arg2)-1]=='H')           
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   result = AH - value;
                else if (strcmp(arg1, "AL") == 0)   result = AL - value; 
                else if (strcmp(arg1, "BH") == 0)   result = BH - value;
                else if (strcmp(arg1, "BL") == 0)   result = BL - value;
                else if (strcmp(arg1, "CH") == 0)   result = CH - value;
                else if (strcmp(arg1, "CL") == 0)   result = CL - value;
                else if (strcmp(arg1, "DH") == 0)   result = DH - value;
                else if (strcmp(arg1, "DL") == 0)   result = DL - value;
            }
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']')         
            {
                value=memory[extract(arg2)];
                if(value>max)
                {
                    printf("memory out of bound");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   result = AH - value;
                else if (strcmp(arg1, "AL") == 0)   result = AL - value; 
                else if (strcmp(arg1, "BH") == 0)   result = BH - value;
                else if (strcmp(arg1, "BL") == 0)   result = BL - value;
                else if (strcmp(arg1, "CH") == 0)   result = CH - value;
                else if (strcmp(arg1, "CL") == 0)   result = CL - value;
                else if (strcmp(arg1, "DH") == 0)   result = DH - value;
                else if (strcmp(arg1, "DL") == 0)   result = DL - value;
            }
            
            if(result>0xFF)
            {
                CF=1;
                SF=1;
            }
            else if(result==0x00)
                ZF=1;
            
        }
        else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']')
        {
            
        }
    }
    
}

// Logical Instructions
void logical(char *token,char *arg1, char *arg2)
{
    arith_flag=0;
    if(strcmp(token,"AND")==0)
    {
        if(checkregX(arg1))        //and ax,bx    and ax, 1234H    and ax,[1234]
        {
            if(checkregX(arg2))       //reg-> reg
            {
                if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "BX") == 0)         AX=AX & BX;
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "CX") == 0)    AX=AX & CX;
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "DX") == 0)    AX=AX & DX;
                
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "AX") == 0)    BX= BX & AX;
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "CX") == 0)    BX= BX & CX;
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "DX") == 0)    BX= BX & DX;
                
                else if(strcmp(arg1, "CX") == 0 && strcmp(arg2, "AX") == 0)     CX= CX & AX;
                else if(strcmp(arg1, "CX") == 0 && strcmp(arg2, "BX") == 0)     CX= CX & BX;
                else if (strcmp(arg1, "CX") == 0 && strcmp(arg2, "DX") == 0)    CX= CX & DX;
                
                else if(strcmp(arg1, "DX") == 0 && strcmp(arg2, "AX") == 0)     DX=DX & AX;
                else if( strcmp(arg1, "DX") == 0 && strcmp(arg2, "BX") == 0)    DX= DX & BX;
                else if(strcmp(arg1, "DX") == 0 && strcmp(arg2, "CX") == 0)     DX=DX & AX;
                else 
                    printf("Invalid register names.\n");
            } 
            else if(arg2[strlen(arg2)-1]=='H')       //reg-> immediate
            {
                value=imd(arg2);
                //printf("%04lX\n",value);
                if (strcmp(arg1, "AX") == 0)  AX=AX & value;
                else if (strcmp(arg1, "BX") == 0)    BX= BX & value;
                else if(strcmp(arg1, "CX")==0) CX= CX & value;
                else if(strcmp(arg1, "DX") == 0)    DX=DX & value;
                else 
                    printf("Invalid register names.\n");
            }
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']')//reg-> memory
            {
                setUp(arg2);
                if (strcmp(arg1, "AX") == 0)
                    AX &= memoryValue;
                else if (strcmp(arg1, "BX") == 0)
                    BX &= memoryValue;
                else if (strcmp(arg1, "CX") == 0)
                    CX &= memoryValue;
                else if (strcmp(arg1, "DX") == 0)
                    DX &= memoryValue;
                else
                    printf("Wrong input");
            }  
            setX(arg1);
            
        }
        else if(arg1[0]=='[' && strlen(arg1)==6)   //and [1234],ax     and [1234],#1234 
        {
            if(checkregX(arg2))          //memory-> reg
            {
                value=memory[extract(arg1)];
                printf("%04lX\n",value);
                if (strcmp(arg2, "AX") == 0)        value=AX & value;
                else if (strcmp(arg2, "BX") == 0)   value= BX & value;
                else if(strcmp(arg2, "CX")==0)      value= CX & value;
                else if(strcmp(arg2, "DX") == 0)    value=DX & value;
                else 
                    printf("Invalid register names.\n");
            }
            else if(arg2[0]=='#')        //memory-> immediate
            {
                value = imd(arg2);
                value1 = memory[extract(arg1)];
                memory[extract(arg1)] = value1 & value;
            }
        }
        else if(checkregHL(arg1))
        {
            if(checkregHL(arg2))         // and al, ah 
            {
                if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "AL") == 0) AH &= AL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BH") == 0) AH &= BH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "BL") == 0) AH &= BL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CH") == 0) AH &= CH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "CL") == 0) AH &= CL;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DH") == 0) AH &= DH;
                else if (strcmp(arg1, "AH") == 0 && strcmp(arg2, "DL") == 0) AH &= DL;
                
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "AH") == 0) AL &= AH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BH") == 0) AL &= BH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "BL") == 0) AL &= BL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CH") == 0) AL &= CH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "CL") == 0) AL &= CL;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DH") == 0) AL &= DH;
                else if (strcmp(arg1, "AL") == 0 && strcmp(arg2, "DL") == 0) AL &= DL;
                
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AH") == 0) BH &= AH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "AL") == 0) BH &= AL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "BL") == 0) BH &= BL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CH") == 0) BH &= CH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "CL") == 0) BH &= CL;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DH") == 0) BH &= DH;
                else if (strcmp(arg1, "BH") == 0 && strcmp(arg2, "DL") == 0) BH &= DL;
                
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AH") == 0) BL &= AH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "AL") == 0) BL &= AL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "BH") == 0) BL &= BH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CH") == 0) BL &= CH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "CL") == 0) BL &= CL;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DH") == 0) BL &= DH;
                else if (strcmp(arg1, "BL") == 0 && strcmp(arg2, "DL") == 0) BL &= DL;
                
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AH") == 0) CH &= AH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "AL") == 0) CH &= AL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BH") == 0) CH &= BH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "BL") == 0) CH &= BL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "CL") == 0) CH &= CL;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DH") == 0) CH &= DH;
                else if (strcmp(arg1, "CH") == 0 && strcmp(arg2, "DL") == 0) CH &= DL;
                
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AH") == 0) CL &= AH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "AL") == 0) CL &= AL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BH") == 0) CL &= BH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "BL") == 0) CL &= BL;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "CH") == 0) CL &= CH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DH") == 0) CL &= DH;
                else if (strcmp(arg1, "CL") == 0 && strcmp(arg2, "DL") == 0) CL &= DL;
                
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AH") == 0) DH &= AH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "AL") == 0) DH &= AL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BH") == 0) DH &= BH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "BL") == 0) DH &= BL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CH") == 0) DH &= CH;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "CL") == 0) DH &= CL;
                else if (strcmp(arg1, "DH") == 0 && strcmp(arg2, "DL") == 0) DH &= DL;
                
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AH") == 0) DL &= AH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "AL") == 0) DL &= AL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BH") == 0) DL &= BH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "BL") == 0) DL &= BL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CH") == 0) DL &= CH;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "CL") == 0) DL &= CL;
                else if (strcmp(arg1, "DL") == 0 && strcmp(arg2, "DH") == 0) DL &= DH;
                
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }

            }
            else if(arg2[strlen(arg2)-1]=='H')           // and al, 12 H
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH &= value;  
                else if (strcmp(arg1, "AL") == 0)   AL &= value; 
                else if (strcmp(arg1, "BH") == 0)   BH &= value;
                else if (strcmp(arg1, "BL") == 0)   BL &= value;
                else if (strcmp(arg1, "CH") == 0)   CH &= value;
                else if (strcmp(arg1, "CL") == 0)   CL &= value;
                else if (strcmp(arg1, "DH") == 0)   DH &= value;
                else if (strcmp(arg1, "DL") == 0)   DL &= value;
            }
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']')//reg-> memory
            {
               value=extract(arg2);
                if(value>max)
                {
                    printf("memory out of bound");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH &=memory[value];  
                else if (strcmp(arg1, "AL") == 0)   AL &=memory[value];  
                else if (strcmp(arg1, "BH") == 0)   BH &= memory[value];  
                else if (strcmp(arg1, "BL") == 0)   BL &= memory[value]; 
                else if (strcmp(arg1, "CH") == 0)   CH &= memory[value];  
                else if (strcmp(arg1, "CL") == 0)   CL &= memory[value];  
                else if (strcmp(arg1, "DH") == 0)   DH &= memory[value];  
                else if (strcmp(arg1, "DL") == 0)   DL &= memory[value];   
            }
            
            setHL(arg1);     
        }
    }
    
    else if(strcmp(token,"ROR")==0)
    {
        //Registers or memory:
        /*Count operand: count can be 1 or a value in the CL register, which allows for variable shifts.
Effect on Flags:
CF (Carry Flag) is affected for all rotate instructions.
OF (Overflow Flag) is affected in specific cases, such as ROR and ROL, when the count is 1.*/
        
        if(checkregX(arg1))
        {
            if(strcmp(arg2,"CL")==0)
            {   
                if (strcmp(arg1, "AX") == 0)        AX=ror(AX,CL);
                else if (strcmp(arg1, "BX") == 0)   BX=ror(BX,CL);
                else if(strcmp(arg1, "DX") == 0)    DX=ror(DX,CL);
                else if(strcmp(arg1, "CX")==0)   
                {   
                    printf("\nNot Possible\n"); 
                    exit(0);
                }
                else 
                    printf("Invalid register names.\n");
            }
            else if(arg2[strlen(arg2)-1]=='H')   
            {
                value=imd(arg2); 
                if (strcmp(arg1, "AX") == 0)        AX=ror(AX,value);
                else if (strcmp(arg1, "BX") == 0)   BX=ror(BX,value);
                else if(strcmp(arg1, "DX") == 0)    DX=ror(DX,value);
                else if(strcmp(arg1, "CX")==0)   CX=ror(CX,value);
                else 
                    printf("Invalid register names.\n");
            }
            
            setHL(arg1);
            
        }
        else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']')
        {
            value1=memory[extract(arg1)];
            memory[extract(arg1)] = ror(value1,CL);
        }
        else if(checkregHL(arg1))
        {
            if(checkregHL(arg2))         // ror al, ah 
            {
                if (strcmp(arg1, "AH") == 0 ) AH = ror8(AH,CL);
                else if (strcmp(arg1, "AL") == 0 ) AL = ror8(AL,CL);
                else if (strcmp(arg1, "BH") == 0 ) BH = ror8(BH,CL);
                else if (strcmp(arg1, "BL") == 0 ) BL = ror8(BL,CL);
                else if (strcmp(arg1, "DH") == 0 ) DH = ror8(DH,CL);
                else if (strcmp(arg1, "DL") == 0 ) DL = ror8(DL,CL);
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }

            }
            else if(arg2[strlen(arg2)-1]=='H')           // ror al, 12 H
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH = ror8(AH,value); 
                else if(strcmp(arg1,"AL")==0)   AL = ror8(AL,value); 
                else if (strcmp(arg1, "BH") == 0)   BH = ror8(BH,value);
                else if (strcmp(arg1, "BL") == 0)   BL = ror8(BL,value);
                else if (strcmp(arg1, "DH") == 0)   DH = ror8(DH,value);
                else if (strcmp(arg1, "DL") == 0)   DL = ror8(DL,value);
            }
            
            setX(arg1);   
        }
    }
    
    else if(strcmp(token,"ROL")==0)
    {
        if(checkregX(arg1))
        {
            if(strcmp(arg2,"CL")==0)
            {   
                if (strcmp(arg1, "AX") == 0)        AX=rol(AX,CL);
                else if (strcmp(arg1, "BX") == 0)   BX=rol(BX,CL);
                else if(strcmp(arg1, "DX") == 0)    DX=rol(DX,CL);
                else if(strcmp(arg1, "CX")==0)   
                {   
                    printf("\nNot Possible\n"); 
                    exit(0);
                }
                else 
                    printf("Invalid register names.\n");
            }
            else if(arg2[strlen(arg2)-1]=='H')   
            {
                value=imd(arg2); 
                if (strcmp(arg1, "AX") == 0)        AX=rol(AX,value);
                else if (strcmp(arg1, "BX") == 0)   BX=rol(BX,value);
                else if(strcmp(arg1, "DX") == 0)    DX=rol(DX,value);
                else if(strcmp(arg1, "CX")==0)   CX=rol(CX,value);
                else 
                    printf("Invalid register names.\n");
            }
            
            setX(arg1);
            
        }
        else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']')
        {
            value1=memory[extract(arg1)];
            memory[extract(arg1)] = rol(value1,CL);
        }
        else if(checkregHL(arg1))
        {
            if(checkregHL(arg2))         // rol al, ah 
            {
                if (strcmp(arg1, "AH") == 0 ) AH = rol8(AH,CL);
                else if (strcmp(arg1, "AL") == 0 ) AL = rol8(AL,CL);
                else if (strcmp(arg1, "BH") == 0 ) BH = rol8(BH,CL);
                else if (strcmp(arg1, "BL") == 0 ) BL = rol8(BL,CL);
                else if (strcmp(arg1, "DH") == 0 ) DH = rol8(DH,CL);
                else if (strcmp(arg1, "DL") == 0 ) DL = rol8(DL,CL);
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }

            }
            else if(arg2[strlen(arg2)-1]=='H')           // rol al, 12H 
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH = rol8(AH,value); 
                else if(strcmp(arg1,"AL")==0)   AL = rol8(AL,value); 
                else if (strcmp(arg1, "BH") == 0)   BH = rol8(BH,value);
                else if (strcmp(arg1, "BL") == 0)   BL = rol8(BL,value);
                else if (strcmp(arg1, "DH") == 0)   DH = rol8(DH,value);
                else if (strcmp(arg1, "DL") == 0)   DL = rol8(DL,value);
            }
            
            setHL(arg1);    
        }
    }
    
    else if(strcmp(token,"RCL")==0)
    {
        if(checkregX(arg1))
        {
            if (strcmp(arg1, "AX") == 0)        AX=rcl(AX,CL,&CF);
            else if (strcmp(arg1, "BX") == 0)   BX=rcl(BX,CL,&CF);
            else if(strcmp(arg1, "DX") == 0)    DX=rcl(DX,CL,&CF);
            else if(strcmp(arg1, "CX")==0)   
            {   
                printf("\nNot Possible\n"); 
                exit(0);
            }
            else if(arg2[strlen(arg2)-1]=='H') 
            {
                value=imd(arg2); 
                if (strcmp(arg1, "AX") == 0)        AX=rcl(AX,value,&CF);
                else if (strcmp(arg1, "BX") == 0)   BX=rcl(BX,value,&CF);
                else if(strcmp(arg1, "DX") == 0)    DX=rcl(DX,value,&CF);
                else if(strcmp(arg1, "CX")==0)   CX=rcl(CX,value,&CF);
                else 
                    printf("Invalid register names.\n");
            }
            else 
            {
                printf("Invalid format.\n");
                exit(0);
            }
            
            setX(arg1);
            
        }
        else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']')
        {
            value1=memory[extract(arg1)];
            memory[extract(arg1)] = rcl(value1,CL,&CF);
        }
        else if(checkregHL(arg1))
        {
            if(checkregHL(arg2))         // rcl al, ah 
            {
                if (strcmp(arg1, "AH") == 0 ) AH = rcl8(AH,CL,&CF);
                else if (strcmp(arg1, "AL") == 0 ) AL = rcl8(AL,CL,&CF);
                else if (strcmp(arg1, "BH") == 0 ) BH = rcl8(BH,CL,&CF);
                else if (strcmp(arg1, "BL") == 0 ) BL = rcl8(BL,CL,&CF);
                else if (strcmp(arg1, "DH") == 0 ) DH = rcl8(DH,CL,&CF);
                else if (strcmp(arg1, "DL") == 0 ) DL = rcl8(DL,CL,&CF);
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }

            }
            else if(arg2[strlen(arg2)-1]=='H')           // rcl al, 12H 
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH = rcl8(AH,value,&CF); 
                else if(strcmp(arg1,"AL")==0)   AL = rcl8(AL,value,&CF); 
                else if (strcmp(arg1, "BH") == 0)   BH = rcl8(BH,value,&CF);
                else if (strcmp(arg1, "BL") == 0)   BL = rcl8(BL,value,&CF);
                else if (strcmp(arg1, "DH") == 0)   DH = rcl8(DH,value,&CF);
                else if (strcmp(arg1, "DL") == 0)   DL = rcl8(DL,value,&CF);
            }
            
            setHL(arg1);   
        }
    }
    
    else if(strcmp(token,"RCR")==0)
    {
        if(checkregX(arg1))
        {
            if (strcmp(arg1, "AX") == 0)        AX=rcr(AX,CL,&CF);
            else if (strcmp(arg1, "BX") == 0)   BX=rcr(BX,CL,&CF);
            else if(strcmp(arg1, "DX") == 0)    DX=rcr(DX,CL,&CF);
            else if(strcmp(arg1, "CX")==0)   
            {   
                printf("\nNot Possible\n"); 
                exit(0);
            }
            else if(arg2[strlen(arg2)-1]=='H')   
            {
                value=imd(arg2); 
                if (strcmp(arg1, "AX") == 0)        AX=rcr(AX,value,&CF);
                else if (strcmp(arg1, "BX") == 0)   BX=rcr(BX,value,&CF);
                else if(strcmp(arg1, "DX") == 0)    DX=rcr(DX,value,&CF);
                else if(strcmp(arg1, "CX")==0)   CX=rcr(CX,value,&CF);
                else 
                    printf("Invalid register names.\n");
            }
            else 
            {
                printf("Invalid format.\n");
                exit(0);
            }
            setX(arg1);
            
        }
        else if(arg1[0]=='[' && arg1[strlen(arg1)-1]==']')
        {
            value1=memory[extract(arg1)];
            memory[extract(arg1)] = rcr(value1,CL,&CF);
        }
        else if(checkregHL(arg1))
        {
            if(checkregHL(arg2))         // rcr al, ah 
            {
                if (strcmp(arg1, "AH") == 0 ) AH = rcr8(AH,CL,&CF);
                else if (strcmp(arg1, "AL") == 0 ) AL = rcr8(AL,CL,&CF);
                else if (strcmp(arg1, "BH") == 0 ) BH = rcr8(BH,CL,&CF);
                else if (strcmp(arg1, "BL") == 0 ) BL = rcr8(BL,CL,&CF);
                else if (strcmp(arg1, "DH") == 0 ) DH = rcr8(DH,CL,&CF);
                else if (strcmp(arg1, "DL") == 0 ) DL = rcr8(DL,CL,&CF);
                else 
                { 
                    printf("Unknown register-to-register operation.\n");
                    exit(0);
                }

            }
            else if(arg2[strlen(arg2)-1]=='H')           // rcr al, 12H 
            {
                value=imd(arg2);
                if(value>0xFF)
                {
                    printf("large size");
                    exit(0);
                }
                if(strcmp(arg1,"AH")==0)   AH = rcr8(AH,value,&CF); 
                else if(strcmp(arg1,"AL")==0)   AL = rcr8(AL,value,&CF); 
                else if (strcmp(arg1, "BH") == 0)   BH = rcr8(BH,value,&CF);
                else if (strcmp(arg1, "BL") == 0)   BL = rcr8(BL,value,&CF);
                else if (strcmp(arg1, "DH") == 0)   DH = rcr8(DH,value,&CF);
                else if (strcmp(arg1, "DL") == 0)   DL = rcr8(DL,value,&CF);
            }
            
            setHL(arg1);  
        }
    }
    
    else if(strcmp(token,"OR")==0)
    {
        if(checkregX(arg1)) 
        {
            if(checkregX(arg2))       //reg-> reg
            {
                if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "BX") == 0)         AX=b_or(AX, BX);
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "CX") == 0)    AX=b_or(AX, CX);
                else if (strcmp(arg1, "AX") == 0 && strcmp(arg2, "DX") == 0)    AX=b_or(AX, DX);
                
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "AX") == 0)    BX=b_or( BX , AX);
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "CX") == 0)    BX=b_or( BX , CX);
                else if (strcmp(arg1, "BX") == 0 && strcmp(arg2, "DX") == 0)    BX=b_or( BX , DX);
                
                else if(strcmp(arg1, "CX") == 0 && strcmp(arg2, "AX") == 0)     CX=b_or( CX , AX);
                else if(strcmp(arg1, "CX") == 0 && strcmp(arg2, "BX") == 0)     CX=b_or( CX , BX);
                else if (strcmp(arg1, "CX") == 0 && strcmp(arg2, "DX") == 0)    CX=b_or( CX , DX);
                
                else if(strcmp(arg1, "DX") == 0 && strcmp(arg2, "AX") == 0)     DX=b_or( DX , AX);
                else if( strcmp(arg1, "DX") == 0 && strcmp(arg2, "BX") == 0)    DX=b_or( DX , BX);
                else if(strcmp(arg1, "DX") == 0 && strcmp(arg2, "CX") == 0)     DX=b_or( DX , AX);
                else 
                    printf("Invalid register names.\n");
            }
            else if(arg2[strlen(arg2)-1]=='H')      //reg-> immediate
            {
                value=imd(arg2);
                
                if (strcmp(arg1, "AX") == 0)  AX=b_or(AX, value);
                else if (strcmp(arg1, "BX") == 0)    BX=b_or( BX , value);
                else if (strcmp(arg1, "CX") == 0)    CX=b_or( CX , value);
                else if (strcmp(arg1, "DX") == 0)    DX=b_or( DX , value);
                else 
                    printf("Invalid register names.\n");
            }
            else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']') // OR AX, [1234H]
            {
                setUp(arg2);
                if (strcmp(arg1, "AX") == 0)
                    AX=b_or(AX, memoryValue);
                else if (strcmp(arg1, "BX") == 0)
                    BX=b_or( BX , memoryValue);
                else if (strcmp(arg1, "CX") == 0)
                    CX=b_or( CX , memoryValue);
                else if (strcmp(arg1, "DX") == 0)
                    DX=b_or( DX , memoryValue);
                else
                    printf("Wrong input");
            }  
            else
            {
                printf("Invalid second argument for OR with register %s.\n", arg1);
                exit(0);
            }
            
            setX(arg1);
        }
        else if(checkregHL(arg1))
        {
            if(checkregHL(arg2))         
            { }
            setHL(arg1);
        }
        else if(arg2[0]=='[' && arg2[strlen(arg2)-1]==']')
        {
            if(checkregX(arg2)) 
            {
               
            }
            else if(arg2[0]=='#')
            {
               
            }
            else
            {
                printf("Invalid second argument for OR with memory at %s.\n", arg1);
            }
        }
        else
        {
            printf("Invalid first argument for OR instruction: %s.\n", arg1);
        }

    }
    
    else if(strcmp(token,"NOT")==0)
    {
        if(checkregX(arg1)) 
        {
            if (strcmp(arg1, "AX") == 0)  AX=b_not(AX);
            else if (strcmp(arg1, "BX") == 0)    BX=b_not(BX);
            else if (strcmp(arg1, "CX") == 0)    CX=b_not(CX);
            else if (strcmp(arg1, "DX") == 0)    DX=b_not(DX);
            else 
                printf("Invalid register names.\n");
                
            setX(arg1);
        }
        else if(checkregHL(arg1))
        {
            if(strcmp(arg1,"AH")==0)   AH =b_not(AH); 
            else if (strcmp(arg1, "AL") == 0)   AL =b_not(AL); 
            else if (strcmp(arg1, "BH") == 0)   BH =b_not(BH); 
            else if (strcmp(arg1, "BL") == 0)   BL =b_not(BL); 
            else if (strcmp(arg1, "CH") == 0)   CH =b_not(CH); 
            else if (strcmp(arg1, "CL") == 0)   CL =b_not(CL); 
            else if (strcmp(arg1, "DH") == 0)   DH =b_not(DH);  
            else if (strcmp(arg1, "DL") == 0)   DL =b_not(DL);  
            
            setHL(arg1);
        }
    }
    
    else if(strcmp(token,"XOR")==0)
    {
        
    }
    
}

// Control Transfer Instruction
void control_transfer(char *token,char *arg1, char *arg2)
{
    //int cc=5;   
    if(strcmp(token,"JNC")==0 || strcmp(token,"JNB")==0)
    {
        value=strtol(arg1,NULL,10);
        if(CF!=1)
        {
            // while(cc!=0)
            // {
            //     if (fgets(instruction, sizeof(instruction), fp) == NULL) 
            //     {
            //         rewind(fp);
            //     }
            //     else if(strcmp(instruction,arg1)==0)
            //         break;
            // }
            if(lc<value)
            {   
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            else if(lc>value)
            {
                rewind(fp);
                lc=0;
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            CF=0;
            printf("\n");
        }
    }
    
    else if( strcmp(token,"JC")==0 || strcmp(token,"JB")==0)
    {
        value=strtol(arg1,NULL,10);
        if(CF==1)
        {
            if(lc<value)
            {
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            else if(lc>value)
            {
                rewind(fp);
                lc=0;
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            all_flag_reset();
            printf("\n");
        }
    }
    
    else if(strcmp(token,"LOOP")==0)
    {
        value=strtol(arg1,NULL,10);
        if(CX!=0)
        {
            if(lc<value)
            {
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++; 
                }
            }
            else if(lc>value)
            {
                rewind(fp);
                lc=0;
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            CX--;
            printf("\n");
        }
    }
    
    else if(strcmp(token,"JMP")==0)
    {
        value=strtol(arg1,NULL,10);
        if(lc<value)
        {   
            while(lc<value-1)
            {
                if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                {
                    printf("Reached end of file.\n");
                    printf("MISTAKE IN LOOP VARIABLE COUNT");
                    exit(0);
                }
                lc++;  
            }
        }
        else if(lc>value)
        {
            rewind(fp);
            lc=0;
            while(lc<value-1)
            {
                if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                {
                    printf("Reached end of file.\n");
                    printf("MISTAKE IN LOOP VARIABLE COUNT");
                    exit(0);
                }
                lc++;  
            }
        }
        printf("\n");
    }
    
    else if(strcmp(token,"JZ")==0 || strcmp(token,"JE")==0)
    {
        if(ZF==1)
        {
            value=strtol(arg1,NULL,10);
            if(lc<value)
            {
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            else if(lc>value)
            {
                rewind(fp);
                lc=0;
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("Reached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            all_flag_reset();
            printf("\n");
        }
    }
    
    else if(strcmp(token,"JNZ")==0 || strcmp(token,"JNE")==0)
    {
        if(ZF!=1)
        {
            value=strtol(arg1,NULL,10);
            if(lc<value)
            {
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("\nReached end of file.\n");
                        printf("MISTAKE IN LOOP VARIABLE COUNT");
                        exit(0);
                    }
                    lc++;  
                }
            }
            else if(lc>value)
            {
                rewind(fp);
                lc=0;
                while(lc<value-1)
                {
                    if (fgets(instruction, sizeof(instruction), fp) == NULL) 
                    {
                        printf("\n\t****Reached end of file.*****\n");
                        printf("\t****MISTAKE IN LOOP VARIABLE COUNT****");
                        exit(0);
                    }
                    lc++;  
                }
            }
            ZF=0;
            printf("\n");
        }
    }
    

}

// Assembler Directive 
void assembler_dir(char *token,char *arg1, char *arg2)
{
    //db,dw,assume,end,endp,ends,equ,extern & pblic,segment,proc,offset
}

/**************************   ***********************/

// Starting function
void check(char *instruct)
{
    lc++;
    printf("\nline: %d : ",lc);
    char *arg1, *arg2;
    char *comment = strchr(instruct, ';');
    if (comment != NULL) 
        *comment = '\0';
    token=strtok(instruct, " ,\t\n");
    arg1=strtok(NULL, " ,\t\n;");
    arg2=strtok(NULL, " ,\t\n;");
    printf("%s ",token);
    if(arg1!=NULL)
        printf(" %s",arg1);
    if(arg2!=NULL)
        printf(", %s",arg2);

    data_transfer(token,arg1,arg2);
    
    arithmetic(token,arg1,arg2);
    
    logical(token,arg1,arg2);
    
    control_transfer(token,arg1,arg2);
    
     //mov,pop,xchg,add,sub,adc,sbb,inc,dec, and, ror, rol, rcl,rcr, jnc, jnb, jc, jb
}

// Main function
int main()
{
    char fname[50];
    printf("Enter the file name: \n");
    scanf("%s",fname);
    
    for(int i=0;i<100;i++)
        memory[i]=0;
 
    fp=fopen(fname,"r");

    if (fp)  
    {
           while(fgets(instruction, sizeof(instruction), fp) != NULL)
        {
            to_lower(instruction);
            
            if(instruction[0]=='H'&&instruction[1]=='L'&&instruction[2]=='T')
            {    
                printf("\nline: %d : %s",++lc,instruction);
                break;
            }
            check(instruction);
        }
    
        // while (1) 
        // {
        //     printf("Enter the instruction: ");
        //     fgets(instruction, sizeof(instruction), stdin);
            
        //     // Remove the newline character from the end of the string
        //     instruction[strcspn(instruction, "\n")] = 0;
            
        //     // Case changer
        //     to_lower(instruction);
            
        //     // Exit condition: if instruction is "HLT", break the loop
        //     if (strcmp(instruction, "HLT") == 0)
        //     {
        //         printf("HLT entered, exiting...\n");
        //         break;
        //     }
            
        //     // Pass the instruction to the tokenizer function
        //     check(instruction);
        // }
    }
    else 
    {
        perror("Error opening file\n Exiting the code.......");
        exit(0);
    }
    char label[100], mn[100], arg1[100], arg2[100];
    
    
    AX &=0xFFFF,AL&=0x00FF, AH&=0xFF00;
    BX &=0xFFFF,BL&=0x00FF, BH&=0xFF00;
    CX &=0xFFFF,CL&=0x00FF, BH&=0xFF00;
    DX &=0xFFFF,DL&=0x00FF, DH&=0xFF00;;
    int k=0;
    
    
    printf("\nSI:%04X\tDI:%04X",SI,DI);
    printf("\nAL:%02lX\tBL:%02lX\tCL:%02lX\tDL:%02lX",AL,BL,CL,DL);
    printf("\nAH:%02lX\tBH:%02lX\tCH:%02lX\tDH:%02lX",AH,BH,CH,DH);
    printf("\nAX:%04lX\tBX:%04lX\tCX:%04lX\tDX:%04lX",AX,BX,CX,DX);
    
    
    int col;
    printf("\n\n\tEnter the memory size you want to see:\t");
    scanf("%d",&col);
    if(col>MAX)
    {
      printf("\tSize too much\tAdjusting it to 25");
      col=25;
    }
    printf("\n\t\t1\t|\t2\t|");
    
    printf("\n-----------------------------------------\n");
    for(int j=0;j<col;j++) // rows
    {
        printf(" %d\t|\t",j*2);
        for(int i=1;i<=2;i++)    // colums
            if(memory[k]!=0x0000)
                printf("\033[4m%02lX\t|\t\033[0m", memory[k++] & 0xFF);  //underline while printing
            else    
                printf("%02lX\t|\t",memory[k++]&0xFFFF);

        printf("\n");
    }
    printf("------------------------------------------\n");
 
    //printf("\nOF\tDF\tIF\tTF\tSF\tZF\tAF\tPF\tCF");
    //printf("\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",OF,DF,IF,TF,SF,ZF,AF,PF,CF);
    
    //printf("Stack: \n");
    //for(int i=0;i<10;i++)
     //   printf("%d | ",stack[i]);
    
    fclose(fp); 
    
    return 0;
}
