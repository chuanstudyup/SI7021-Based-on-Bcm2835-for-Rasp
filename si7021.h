#ifndef SI7021_H
#define SI7021_H

#include <bcm2835.h>
#include <cstdio>
#include <cstdint>

/*!
 *  I2C ADDRESS/BITS
 */
#define SI7021_DEFAULT_ADDRESS 0x40

#define SI7021_MEASRH_HOLD_CMD                                                 \
  0xE5 /**< Measure Relative Humidity, Hold Master Mode */
#define SI7021_MEASRH_NOHOLD_CMD                                               \
  0xF5 /**< Measure Relative Humidity, No Hold Master Mode */
#define SI7021_MEASTEMP_HOLD_CMD                                               \
  0xE3 /**< Measure Temperature, Hold Master Mode */
#define SI7021_MEASTEMP_NOHOLD_CMD                                             \
  0xF3 /**< Measure Temperature, No Hold Master Mode */
#define SI7021_READPREVTEMP_CMD                                                \
  0xE0 /**< Read Temperature Value from Previous RH Measurement */
#define SI7021_RESET_CMD 0xFE           /**< Reset Command */
#define SI7021_WRITERHT_REG_CMD 0xE6    /**< Write RH/T User Register 1 */
#define SI7021_READRHT_REG_CMD 0xE7     /**< Read RH/T User Register 1 */
#define SI7021_WRITEHEATER_REG_CMD 0x51 /**< Write Heater Control Register */
#define SI7021_READHEATER_REG_CMD 0x11  /**< Read Heater Control Register */
#define SI7021_REG_HTRE_BIT 0x02        /**< Control Register Heater Bit */
#define SI7021_ID1_CMD 0xFA0F           /**< Read Electronic ID 1st Byte */
#define SI7021_ID2_CMD 0xFCC9           /**< Read Electronic ID 2nd Byte */
#define SI7021_FIRMVERS_CMD 0x84B8      /**< Read Firmware Revision */

#define SI7021_REV_1 0xff /**< Sensor revision 1 */
#define SI7021_REV_2 0x20 /**< Sensor revision 2 */

/** An enum to represent sensor types **/
enum si_sensorType {
  SI_Engineering_Samples,
  SI_7013,
  SI_7020,
  SI_7021,
  SI_UNKNOWN,
};

/*!
 *  @brief  Class that stores state and functions for interacting with
 *          Si7021 Sensor
 */

class SI7021{
	public:
		SI7021(uint8_t addr = SI7021_DEFAULT_ADDRESS);
		bool begin();
		
		float readHumi();
		float readTemp();
		void reset();

	  /*!
	   *  @brief  Returns sensor revision established during init
	   *  @return model value
	   */
	  uint8_t getRevision() { return _revision; };
	  
	  /*!
	   *  @brief  Returns sensor sensor types
	   *  @return sensor types
	   */
	  si_sensorType getModel(){ return _model; };

	private:
		uint8_t _addr;
		si_sensorType _model;
		uint8_t _revision;
		uint32_t sernum_a; /**< Serialnum A */
		uint32_t sernum_b; /**< Serialnum B */
		void _readRevision();
		uint8_t _readRegister8(uint8_t reg);
		void _readSerialNumber();

		uint8_t errCode;
		char sendBuf[3];
};
#endif
