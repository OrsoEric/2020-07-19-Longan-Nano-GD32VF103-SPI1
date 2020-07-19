//SPI TEST
//Wire together MISO and MOSI and test Loopback sequence

//TEST1: wait send wait receive check SUCCESS
//TEST2: disable RX wait and receive: SUCCESS (?!?)

#include <gd32vf103.h>

#define EVER (;;)

//#define ENABLE_RXD

extern void delay_us(unsigned int delay);
extern void test1_spi1(  void );

int main( void )
{
	test1_spi1();

	return 0;
}

void test1_spi1(  void )
{
	//----------------------------------------------------------------
	//	VARS
	//----------------------------------------------------------------

	uint32_t blink_delay = 500000;

	//----------------------------------------------------------------
	//	INIT
	//----------------------------------------------------------------

	
	//Clock the GPIO banks
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOC);
	//Setup the R (PC13), G (PA1) and B (PA2) LEDs
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_13);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_1);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,GPIO_PIN_2);
	gpio_bit_set(GPIOC, GPIO_PIN_13);
	gpio_bit_set(GPIOA, GPIO_PIN_1);
	gpio_bit_set(GPIOA, GPIO_PIN_2);
	
	//Initialize SPI0 in master mode
	//Temp SPI initialization structure
	spi_parameter_struct spi_tmp;

	//Clock the GPIOA and GPIOB pin banks
	rcu_periph_clock_enable( RCU_GPIOB );
	//Clock the Alternate Functions
	rcu_periph_clock_enable( RCU_AF );
	//Clock the SPI0
	rcu_periph_clock_enable( RCU_SPI1 );
	
	//PB15	SPI1	MOSI
	//PB14	SPI1	MISO
	//PB13	SPI1	SCK
	//PB12	SPI1	NSS
	//SPI0 GPIO pin configuration
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);	
	gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 );			
	gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14 );			
	gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);			
	
	//SPI0 initialization
	spi_struct_para_init(&spi_tmp);
	spi_tmp.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
	spi_tmp.device_mode          = SPI_MASTER;
	spi_tmp.frame_size           = SPI_FRAMESIZE_8BIT;
	spi_tmp.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
	spi_tmp.nss                  = SPI_NSS_SOFT;
	spi_tmp.prescale             = SPI_PSC_256;
	spi_tmp.endian               = SPI_ENDIAN_MSB;
	spi_init(SPI1, &spi_tmp);

	//Transmit using the DMA
	//spi_dma_enable( SPI0, SPI_DMA_TRANSMIT );
	//Regulat SPI transmission
	spi_dma_disable( SPI1, SPI_DMA_TRANSMIT );
	//spi_crc_polynomial_set(SPI0,7);
	spi_crc_off(SPI1);
	spi_enable(SPI1);
	//If: bad configuration detected
	if (spi_i2s_flag_get(SPI1, SPI_FLAG_CONFERR) == SET)
	{
		blink_delay = 50000;
	}

	//----------------------------------------------------------------
	//	MAIN LOOP
	//----------------------------------------------------------------

	uint8_t rxd, txd;
	txd = 0x5a;
	rxd = 0x00;

	for EVER
	{	
		//----------------------------------------------------------------
		//	MAIN LOOP
		//----------------------------------------------------------------

		//Toggle the RED LED
		gpio_bit_write(GPIOC, GPIO_PIN_13, (bit_status)(1-gpio_input_bit_get(GPIOC, GPIO_PIN_13)));
		//2Hz blink
		delay_us( blink_delay );

		while(spi_i2s_flag_get(SPI1, SPI_FLAG_TBE) == RESET);
		spi_i2s_data_transmit(SPI1, txd);
		
		#ifdef ENABLE_RXD
			while(spi_i2s_flag_get(SPI1, SPI_FLAG_RBNE) == RESET);
			rxd = spi_i2s_data_receive(SPI1);
		#endif
		
		//If: loopback worked: blink green led
		if (txd == rxd)
		{
			gpio_bit_write(GPIOA, GPIO_PIN_1, (bit_status)(1-gpio_input_bit_get(GPIOA, GPIO_PIN_1)));
		}
		//If: loopback didn't work: blink blue led
		else
		{
			gpio_bit_write(GPIOA, GPIO_PIN_2, (bit_status)(1-gpio_input_bit_get(GPIOA, GPIO_PIN_2)));
		}
	}

	//----------------------------------------------------------------
	//	RETURN
	//----------------------------------------------------------------

	return;
}	//End function: main

/***************************************************************************/
//!	@brief public static method
//!	delay_us
/***************************************************************************/
//!	@param delay | unsigned int | how long to wait for in microseconds
//! @return void |
//! @details \n
//!	Use the SysTic timer counter to busy wait for the correct number of microseconds
/***************************************************************************/

void delay_us(unsigned int delay)
{
	//----------------------------------------------------------------
	//	VARS
	//----------------------------------------------------------------

	uint64_t start_mtime, delta_mtime;

	//----------------------------------------------------------------
	//	BODY
	//----------------------------------------------------------------
	//	Wait for the first tick
	//	Wait for the correct number of ticks

	uint64_t tmp = get_timer_value();
	do
	{
		start_mtime = get_timer_value();
	}
	while (start_mtime == tmp);

	do
	{
		delta_mtime = get_timer_value() - start_mtime;
	}
	while(delta_mtime <(SystemCoreClock/4000000.0 *delay ));

	//----------------------------------------------------------------
	//	RETURN
	//----------------------------------------------------------------

	return;
}	//End function: main