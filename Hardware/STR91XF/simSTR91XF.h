/**********************************************************************
   Mark Butcher    Bsc (Hons) MPhil MIET

   M.J.Butcher Consulting
   Birchstrasse 20f,    CH-5406, Rütihof
   Switzerland

   www.uTasker.com    Skype: M_J_Butcher

   ---------------------------------------------------------------------
   File:   	    simSTR91XF.h
   Project: 	Single Chip Embedded Internet
   ---------------------------------------------------------------------
   Copyright (C) M.J.Butcher Consulting 2004..2011
   *********************************************************************/  


typedef struct stSTR91XF_FMI
{
unsigned long FMI_BBSR;       
unsigned long FMI_NBBSR; 
unsigned long ulRes1; 
unsigned long FMI_BBADR;  
unsigned long FMI_NBBADR;  
unsigned long ulRes2;
unsigned long FMI_CR;  
unsigned long FMI_SR;  
unsigned long FMI_BCE5ADDR;          
} STR91XF_FMI;


typedef struct stSTR91XF_WIU
{
unsigned long WIU_CTRL;       
unsigned long WIU_MR;  
unsigned long WIU_TR;  
unsigned long WIU_PR;  
unsigned long WIU_INTR;            
} STR91XF_WIU;


typedef struct stSTR91XF_TIMER
{
unsigned long TIM_IC1R;       
unsigned long TIM_IC2R;       
unsigned long TIM_OC1R; 
unsigned long TIM_OC2R;       
const unsigned long TIM_CNTR;
unsigned long TIM_CR1; 
unsigned long TIM_CR2;        
unsigned long TIM_SR;        
} STR91XF_TIMER;


typedef struct stSTR91XF_GPIO
{
unsigned long GPIO_DATA[256];
unsigned long GPIO_DIR;  
unsigned long GPIO_SEL;       
} STR91XF_GPIO;

typedef struct stSTR91XF_SCU
{
unsigned long SCU_CLKCNTR;
unsigned long SCU_PLLCONF;
unsigned long SCU_SYSSTATUS;
unsigned long SCU_PWRMNG;
unsigned long SCU_ITCSK;
unsigned long SCU_PCGR0;
unsigned long SCU_PCGR1;
unsigned long SCU_PRR0;
unsigned long SCU_PRR1;
unsigned long SCU_MGR0;              
unsigned long SCU_MGR1;              
unsigned long SCU_PECGR0;      
unsigned long SCU_PECGR1;      
unsigned long SCU_SCR0;              
unsigned long SCU_SCR1;              
unsigned long SCU_SCR2;              
unsigned long ulRes0;
unsigned long SCU_GPIOOUT0;
unsigned long SCU_GPIOOUT1;
unsigned long SCU_GPIOOUT2;
unsigned long SCU_GPIOOUT3;
unsigned long SCU_GPIOOUT4;
unsigned long SCU_GPIOOUT5;
unsigned long SCU_GPIOOUT6;
unsigned long SCU_GPIOOUT7;
unsigned long SCU_GPIOIN0;
unsigned long SCU_GPIOIN1;
unsigned long SCU_GPIOIN2;
unsigned long SCU_GPIOIN3;
unsigned long SCU_GPIOIN4;
unsigned long SCU_GPIOIN5;
unsigned long SCU_GPIOIN6;
unsigned long SCU_GPIOIN7;
unsigned long SCU_GPIOTYPE0;
unsigned long SCU_GPIOTYPE1;
unsigned long SCU_GPIOTYPE2;
unsigned long SCU_GPIOTYPE3;
unsigned long SCU_GPIOTYPE4;
unsigned long SCU_GPIOTYPE5;
unsigned long SCU_GPIOTYPE6;
unsigned long SCU_GPIOTYPE7;  
unsigned long SCU_GPIOTYPE8; 
unsigned long SCU_GPIOTYPE9; 
unsigned long SCU_GPIOEMI;   
unsigned long SCU_WKUPSEL;   
unsigned long ulRes2[2];
unsigned long SCU_GPIOANA;   
} STR91XF_SCU;

typedef struct stSTR91XF_UART
{
unsigned long UART_DR;  
unsigned long UART_RSECR;
unsigned long ulRes1[4];
unsigned long UART_FR; 
unsigned long ulRes2;
unsigned long UART_ILPR;
unsigned long UART_IBRD; 
unsigned long UART_FBRD; 
unsigned long UART_LCR;  
unsigned long UART_CR;   
unsigned long UART_IFLS;
unsigned long UART_IMSC; 
unsigned long UART_RIS;  
unsigned long UART_MIS;  
unsigned long UART_ICR;  
unsigned long UART_DMACR;
} STR91XF_UART;


typedef struct stSTR91XF_SSP
{
unsigned long SSP_CR_0; 
unsigned long SSP_CR_1; 
unsigned long SSP_DR;   
unsigned long SSP_SR;   
unsigned long SSP_PR;   
unsigned long SSP_IMSCR;
unsigned long SSP_RISR; 
unsigned long SSP_MISR; 
unsigned long SSP_ICR;  
unsigned long SSP_DMACR;
} STR91XF_SSP;


typedef struct stSTR91XF_ADC
{
unsigned long ADC_CR;
unsigned long ADC_CCR;
unsigned long ADC_HTR;
unsigned long ADC_LTR;
unsigned long ADC_CRR;
unsigned long ADC_DR0;
unsigned long ADC_DR1;
unsigned long ADC_DR2;
unsigned long ADC_DR3;
unsigned long ADC_DR4;
unsigned long ADC_DR5;
unsigned long ADC_DR6;
unsigned long ADC_DR7;
unsigned long ADC_PRS;
} STR91XF_ADC;


typedef struct stSTR91XF_WDT
{
unsigned long WDG_CR; 
unsigned long WDG_PR; 
unsigned long WDG_VR; 
unsigned long WDG_CNT;
unsigned long WDG_SR; 
unsigned long WDG_MR; 
unsigned long WDG_KR; 
} STR91XF_WDT;



typedef struct stSTR91XF_IIC
{
unsigned long I2C_CR;  
unsigned long I2C_SR1;
unsigned long I2C_SR2;
unsigned long I2C_CCR; 
unsigned long I2C_OAR1;
unsigned long I2C_OAR2;
unsigned long I2C_DR; 
unsigned long I2C_ECCR; 
} STR91XF_IIC;





typedef struct stSTR91XF_ENET
{
unsigned long ENET_SCR;
unsigned long ENET_IER;
unsigned long ENET_ISR;            
unsigned long ENET_CCR;            
unsigned long ENET_RXSTR;          
unsigned long ENET_RXCR;           
unsigned long ENET_RXSAR;          
unsigned long ENET_RXNDAR;         
unsigned long ENET_RXCAR;          
unsigned long ENET_RXCTCR;               
unsigned long ENET_RXTOR;                
unsigned long ENET_RXSR;           
unsigned long ENET_TXSTR;          
unsigned long ENET_TXCR;                 
unsigned long ENET_TXSAR;                
unsigned long ENET_TXNDAR;              
unsigned long ENET_TXCAR;
unsigned long ENET_TXCTCR;         
unsigned long ENET_TXTOR;          
unsigned long ENET_TXSR;  
unsigned long ulRes1[44];         
unsigned long ENET_RX_FIFO[32];
unsigned long ENET_TX_FIFO[32];
unsigned long ulRes2[32];
unsigned long ENET_MCR;           
unsigned long ENET_MAH;           
unsigned long ENET_MAL;           
unsigned long ENET_MCHA;           
unsigned long ENET_MCLA;          
unsigned long ENET_MIIA;          
unsigned long ENET_MIID;          
unsigned long ENET_MCF;          
unsigned long ENET_VL1;           
unsigned long ENET_VL2;           
unsigned long ENET_MTS;           
unsigned long ENET_MRS; 
} STR91XF_ENET;


typedef struct stSTR91XF_VIC
{
unsigned long VIC_ISR;    
unsigned long VIC_FSR;    
unsigned long VIC_RINTSR; 
unsigned long VIC_INTSR;  
unsigned long VIC_INTER;  
unsigned long VIC_INTECR; 
unsigned long VIC_SWINTR; 
unsigned long VIC_SWINTCR;
unsigned long VIC_PER;    
unsigned long VIC_res1[3];
unsigned long VIC_VAR;
unsigned long VIC_DVAR;   
unsigned long VIC_res2[50];
unsigned long VIC_VAXR[16];
unsigned long VIC_res3[48];
unsigned long VIC_VCXR[16];
} STR91XF_VIC;




// A copy of the STR91XF peripheral registers for simulation purposes
// (each peripheral block is identical to reality but the ordering and position of blocks are generally not important)

typedef struct stSTR91XF_PERIPH
{
    STR91XF_FMI         ucFMI;

    STR91XF_WIU         ucWIU;
    STR91XF_TIMER       ucTimer0;
    STR91XF_TIMER       ucTimer1;
    STR91XF_TIMER       ucTimer2;
    STR91XF_TIMER       ucTimer3;

    STR91XF_GPIO        ucPorts[10];

    STR91XF_SCU         ucSCU;

    STR91XF_UART        ucUart[3];
    STR91XF_SSP         ucSSP[2];

    STR91XF_ADC         ucADC;
    STR91XF_WDT         ucWDT;
    STR91XF_IIC         ucIIC[2];

    STR91XF_ENET        ucENET;
    STR91XF_VIC         ucVIC0;
    STR91XF_VIC         ucVIC1;
} STR91XF_PERIPH;




extern STR91XF_PERIPH ucSTR91XF;

#define _GPIO_0                         0                                // References for display in simulator
#define _GPIO_1                         1
#define _GPIO_2                         2
#define _GPIO_3                         3
#define _GPIO_4                         4
#define _GPIO_5                         5
#define _GPIO_6                         6
#define _GPIO_7                         7
#define _GPIO_8                         8
#define _GPIO_9                         9








////////////////////////////////////////////////////////








