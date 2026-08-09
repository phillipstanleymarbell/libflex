//TODO: bring this in line with stdint uint8_t etc.

//typedef	char			bool;
typedef	unsigned char		uchar;
typedef unsigned short		ushort;
typedef	unsigned int		uint;
typedef	unsigned long		ulong;
typedef	unsigned long long	uvlong;
typedef	int			integer;
typedef	float			real;

#if defined FLEX64
#	define	FlexAddr	unsigned long long
#elif defined FLEX32
#	define	FlexAddr	unsigned long
#elif defined FLEX16
#	define FlexAddr		unsigned int
#elif defined  FLEX8
#	define FlexAddr		unsigned char
#else
#	error "You must define one of FLEX64/FLEX32/FLEX16/FLEX8"
#endif
