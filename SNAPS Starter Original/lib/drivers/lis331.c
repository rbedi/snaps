#include <string.h>
#include <lis331.h>
#include <spi.h>

#define TIMEOUT 10

// Message formatting bits
#define BIT_READ    0x80
#define BIT_ADDRINC 0x40

// Address bits
#define ADDR_WHO_AM_I           0x0F
#define ADDR_CTRL_REG_1         0x20
#define ADDR_CTRL_REG_2         0x21
#define ADDR_CTRL_REG_3         0x22
#define ADDR_CTRL_REG_4         0x23
#define ADDR_CTRL_REG_5         0x24
#define ADDR_HP_FILTER_RESET	0x25
#define ADDR_REFERENCE          0x26
#define ADDR_STATUS_REG         0x27
#define ADDR_OUT_X_L            0x28
#define ADDR_OUT_X_H            0x29
#define ADDR_OUT_Y_L            0x2A
#define ADDR_OUT_Y_H            0x2B
#define ADDR_OUT_Z_L            0x2C
#define ADDR_OUT_Z_H            0x2D
#define ADDR_INT1_CFG           0x30
#define ADDR_INT1_SRC           0x31
#define ADDR_INT1_THS	        0x32
#define ADDR_INT1_DURATION      0x33
#define ADDR_INT2_CFG           0x34
#define ADDR_INT2_SRC           0x35
#define ADDR_INT2_THS	        0x36
#define ADDR_INT2_DURATION      0x37

// Initialize the peripherals
void LIS331_Init(LIS331* accel)
{
    // Initialize structures
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    // Enable the peripheral clocks
    (*(accel->rccfunc))(accel->clock, ENABLE);
    RCC_AHB1PeriphClockCmd(accel->mosi.clock | accel->miso.clock | \
                           accel->sclk.clock | accel->cs.clock, ENABLE);

    // Connect pins to the SPI peripheral
    GPIO_PinAFConfig(accel->mosi.port, accel->mosi.pinsource, accel->af);
    GPIO_PinAFConfig(accel->miso.port, accel->miso.pinsource, accel->af);
    GPIO_PinAFConfig(accel->sclk.port, accel->sclk.pinsource, accel->af);

    // Set up GPIO for CS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = accel->cs.pin;
    GPIO_Init(accel->cs.port, &GPIO_InitStructure);

    // Set up GPIO for SCLK and MOSI
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = accel->sclk.pin;
    GPIO_Init(accel->sclk.port, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = accel->mosi.pin;
    GPIO_Init(accel->mosi.port, &GPIO_InitStructure);

    // Set up GPIO for MISO
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = accel->miso.pin;
    GPIO_Init(accel->miso.port, &GPIO_InitStructure);

    // Configure SPI peripheral
    SPI_I2S_DeInit(accel->spi);
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_GetDivider(SystemCoreClock, 10000000);
    SPI_Init(accel->spi, &SPI_InitStructure);
    SPI_Cmd(accel->spi, ENABLE);
}

// Quick existence check to verify that a accel can communicate
uint32_t LIS331_WhoAmI(LIS331* accel)
{
    // Create a transfer buffer
    uint8_t bytesIn[] = {BIT_READ | ADDR_WHO_AM_I, 0};
    uint8_t bytesOut[] = {0, 0};

    // Ask the accel to identify itself
    SPI_SetCS(&(accel->cs));
	SPI_TransferMulti(accel->spi, 2, 1, bytesIn, bytesOut);
    SPI_ResetCS(&(accel->cs));

	// Returns true if accelerometer identifies itself properly
    return (bytesOut[1] == 0x32);
}

// Enable the accel
void LIS331_TurnOn(LIS331* accel)
{
    // Create a transfer buffer
    uint8_t bytesOut[] = {ADDR_CTRL_REG_1, 0x3F};
    uint8_t bytesIn[] = {0, 0};

	// Set 'on' mode, maximum data rate, enable all three axes
    SPI_SetCS(&(accel->cs));
	SPI_TransferMulti(accel->spi, 2, 1, bytesIn, bytesOut);
	SPI_ResetCS(&(accel->cs));
}

// Read new X, Y, Z accelerations
void LIS331_UpdateReadings(LIS331* accel)
{
    // Create a transfer buffer
    uint8_t bytesIn[] =
            {BIT_READ | BIT_ADDRINC | ADDR_OUT_X_L, 0, 0, 0, 0, 0, 0};
    uint8_t bytesOut[] = {0, 0, 0, 0, 0, 0, 0};

    // Retrieve x, y, and z accelerations
    SPI_SetCS(&(accel->cs));
    SPI_TransferMulti(accel->spi, 7, 1, bytesIn, bytesOut);
    SPI_ResetCS(&(accel->cs));

    memcpy(&(accel->x), &(bytesOut[1]), 6);

    // Data-munging
    /*uint8_t munged[] = {
        bytesOut[1],
        bytesOut[2],
        bytesOut[3],
        bytesOut[4],
        bytesOut[5],
        bytesOut[6]
    };

    // Assignment
    accel->x = ((int16_t *)munged)[0];
    accel->y = ((int16_t *)munged)[1];
    accel->z = ((int16_t *)munged)[2];*/
}