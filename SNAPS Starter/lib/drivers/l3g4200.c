#include <l3g4200.h>
#include <spi.h>

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
#define ADDR_REFERENCE          0x25
#define ADDR_OUT_TEMP           0x26
#define ADDR_STATUS_REG         0x27
#define ADDR_OUT_X_L            0x28
#define ADDR_OUT_X_H            0x29
#define ADDR_OUT_Y_L            0x2A
#define ADDR_OUT_Y_H            0x2B
#define ADDR_OUT_Z_L            0x2C
#define ADDR_OUT_Z_H            0x2D
#define ADDR_FIFO_CTRL_REG      0x2E
#define ADDR_FIFO_SRC_REG       0x2F
#define ADDR_INT1_CFG           0x30
#define ADDR_INT1_SRC           0x31
#define ADDR_INT1_TSH_XH        0x32
#define ADDR_INT1_TSH_XL        0x33
#define ADDR_INT1_TSH_YH        0x34
#define ADDR_INT1_TSH_YL        0x35
#define ADDR_INT1_TSH_ZH        0x36
#define ADDR_INT1_TSH_ZL        0x37
#define ADDR_INT1_DURATION      0x38

// Initialize the peripherals
void L3G4200_Init(L3G4200* gyro)
{
    // Initialize structures
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStructure;

    // Enable the peripheral clocks
    (*(gyro->rccfunc))(gyro->clock, ENABLE);
    RCC_AHB1PeriphClockCmd(gyro->mosi.clock | gyro->miso.clock | \
                           gyro->sclk.clock | gyro->cs.clock, ENABLE);

    // Connect pins to the SPI peripheral
    GPIO_PinAFConfig(gyro->mosi.port, gyro->mosi.pinsource, gyro->af);
    GPIO_PinAFConfig(gyro->miso.port, gyro->miso.pinsource, gyro->af);
    GPIO_PinAFConfig(gyro->sclk.port, gyro->sclk.pinsource, gyro->af);

    // Set up GPIO for CS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = gyro->cs.pin;
    GPIO_Init(gyro->cs.port, &GPIO_InitStructure);

    // Set up GPIO for SCLK and MOSI
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = gyro->sclk.pin;
    GPIO_Init(gyro->sclk.port, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = gyro->mosi.pin;
    GPIO_Init(gyro->mosi.port, &GPIO_InitStructure);

    // Set up GPIO for MISO
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = gyro->miso.pin;
    GPIO_Init(gyro->miso.port, &GPIO_InitStructure);

    // Configure SPI peripheral
    SPI_I2S_DeInit(gyro->spi);
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_GetDivider(SystemCoreClock, 10000000);
    SPI_Init(gyro->spi, &SPI_InitStructure);
    SPI_Cmd(gyro->spi, ENABLE);
}

// Quick existence check to verify that a gyro can communicate
uint32_t L3G4200_WhoAmI(L3G4200* gyro)
{
    // Ask the gyro to identify itself
	SPI_SetCS(&(gyro->cs));
	SPI_Transfer(gyro->spi, (BIT_READ | ADDR_WHO_AM_I));
	uint32_t readVal = SPI_Transfer(gyro->spi, 0);
	SPI_ResetCS(&(gyro->cs));

	// Returns true if gyroscope identifies itself properly
    return (readVal == 0xD3);
}

// Enable the gyro
void L3G4200_TurnOn(L3G4200* gyro)
{
	SPI_SetCS(&(gyro->cs));
	SPI_Transfer(gyro->spi, ADDR_CTRL_REG_1);
	SPI_Transfer(gyro->spi, 0xFF);
	SPI_ResetCS(&(gyro->cs));
}

// Read new X, Y, Z angular velocities
void L3G4200_UpdateReadings(L3G4200* gyro)
{
    // Update x, y, and z
    SPI_SetCS(&(gyro->cs));
    SPI_Transfer(gyro->spi, (BIT_READ | BIT_ADDRINC | ADDR_OUT_X_L));
	SPI_ReadMulti(gyro->spi, 6, 2, &(gyro->x));
    SPI_ResetCS(&(gyro->cs));
}