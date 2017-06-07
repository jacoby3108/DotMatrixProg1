#ifndef MCP3008_h
#define MCP3008_h


#define SD_MASK			7		// single & differential mode bit position
#define CH_MASK			4	    // channel select lsb bit position

#define SINGLE 			(1 << SD_MASK)
#define DIFFERENTIAL 	(0 << SD_MASK)

#define CH0				(0 << CH_MASK)
#define CH1				(1 << CH_MASK)
#define CH2				(2 << CH_MASK)
#define CH3				(3 << CH_MASK)
#define CH4				(4 << CH_MASK)
#define CH5				(5 << CH_MASK)
#define CH6				(6 << CH_MASK)
#define CH7				(7 << CH_MASK)

#endif // MCP3008 definition end
