#pragma once

#include "error.h"
#include "types.h"
#include "utils.h"

namespace dsp56k
{
	class AGU
	{
	public:
		static TWord calcModuloMask(TWord m)
		{
//			const auto zeroBits = countZeroBitsReversed(m);
//			const int testRes = (1<<((sizeof(m)*CHAR_BIT)-zeroBits))-1;

			m |= m >> 1;
			m |= m >> 2;
			m |= m >> 4;
			m |= m >> 8;

//			assert(m == testRes);
			return m;
		}

		static void updateAddressRegister( TWord& r, int n, TWord m )
		{
			// linear addressing
			if( m == 0xffffff )
			{
				r += n;
			}
			else
			{
				const TWord moduloTest = (m & 0x00ffff);

				// bit-reverse mode
				if( moduloTest == 0x0000 )
				{
					assert( 0 && "bitreverse mode is used" );

					r = bitreverse24(r);
					n = bitreverse24(n);
					r += n;
					r &= 0x00ffffff;
					r = bitreverse24(r);
				}

				// modulo
				else if( moduloTest <= 0x007fff )
				{
					if( abs(signextend<int,24>(n)) >= static_cast<int>(m + 1) )
					{
						// the doc says it's only valid for N = P x (2 pow k), but assume the assembly is okay
//						LOG( "r " << std::hex << r << " + n " << std::hex << n << " = " << std::hex << ((r+n)&0x00ffffff) );
						r += n;
					}
					else
					{
						const TWord moduloMask		= calcModuloMask(m);
						const TWord baseAddrMask	= ~moduloMask;
						const int32_t modulo		= moduloTest + 1;

						const TWord rBase			= r & baseAddrMask;
						const int32_t rOffset		= (r & moduloMask);
						
						// If (rOffset<0) then bit 31 of rOffset is set. Shift it down 31 places to get -1. Otherwise you have 0.
						// and this value with modulo to get either modulo or zero. Add to rOffset.
						// If (moduloTest-rOffset<0) (rOffset>moduloTest) (i.e. rOffset exceeds moduloTest), do the same trick.
						const int32_t p				= rOffset + n;
						const int32_t mt			= moduloTest;
						const int32_t rOffsetNew	= p + ((p>>31) & modulo) - (((mt-p)>>31) & modulo);

						const TWord rNew 			= rBase | rOffsetNew;

//						LOG( "r " << std::hex << r << " + n " << std::hex << n << "(m=" << std::hex << m << ") = " << std::hex << rNew << " mask=" << std::hex << moduloMask << " baseAddrMask=" << std::hex << baseAddrMask << " baseAddr=" << std::hex << baseAddr );

						r = rNew;
					}
				}
				else
				{
					LOG_ERR_NOTIMPLEMENTED( "AGU multiple Wrap-Around Modulo Modifier: r=" << HEX(r) <<", n=" << HEX(n) << ", m=" << HEX(m) );					
				}
			}
			r &= 0x00ffffff;
		}
	};
};
