/* -*- C++ -*- vim:set syntax=cpp: 
 *
 * Byte Order Mark (BOM) Handling.
 *
 * The byte order mark (BOM) is a Unicode character used to signal 
 * the endianness (byte order) of a text file or stream. Its code 
 * point is U+FEFF. 
 * [Source: <http://en.wikipedia.org/wiki/Byte_order_mark>]
 *
 * This file implements a function to cut the BOM and tell about 
 * the encoding of the data stream.
 *
 * (C) 2010 Frank-Rene Schaefer    
 * ABSOLUTELY NO WARRANTY                    */
#ifndef __QUEX_INCLUDE_GUARD__BOM_I
#define __QUEX_INCLUDE_GUARD__BOM_I

#include <quex/code_base/bom>

#include <quex/code_base/temporary_macros_on>

#if ! defined(__QUEX_OPTION_PLAIN_C)
namespace quex {
#endif

QUEX_INLINE QUEX_TYPE_BOM
#if ! defined(__QUEX_OPTION_PLAIN_C)
bom_snap(__QUEX_STD_FILE* InputHandle)
#else
quex_bom_snap(__QUEX_STD_FILE* InputHandle)
#endif
/* This function can **only** be used with **normally** behaving streams
 * where the position increases by one with every character being read. If
 * this is not the case then use the **binary** option of your stream.     */
{
    uint8_t        buffer[4] = { 0, 0, 0, 0};
    QUEX_TYPE_BOM  result    = QUEX_BOM_NONE;
    size_t         byte_n    = 0;
    size_t         read_n    = 0;
    long           p0        = __QUEX_STD_ftell(InputHandle);

    read_n = (size_t)__QUEX_STD_fread((uint8_t*)buffer, 1, 4, InputHandle);
#   if ! defined(__QUEX_OPTION_PLAIN_C)
    result = __bom_snap_core(buffer, read_n, &byte_n);
#   else
    result = __quex_bom_snap_core(buffer, read_n, &byte_n);
#   endif

    /* Avoid temporary function argument. Store sum in p0. */
    p0 += (long)byte_n;
    __QUEX_STD_fseek(InputHandle, p0, SEEK_SET);

    return result;
}

#if ! defined(__QUEX_OPTION_PLAIN_C)
template <class InputStream> QUEX_INLINE QUEX_TYPE_BOM
QUEX_NAME(bom_snap)(InputStream* p_input_stream)
/* This function can **only** be used with **normally** behaving streams
 * where the position increases by one with every character being read. If
 * this is not the case then use the **binary** option of your stream.     */
{
    uint8_t               buffer[4] = { 0, 0, 0, 0};
    QUEX_TYPE_BOM         result    = QUEX_BOM_NONE;
    size_t                byte_n    = 0;
    size_t                read_n    = 0;
    /**/
    const size_t                   CharSize  = sizeof(typename InputStream::char_type);
    typename InputStream::pos_type p0 = p_input_stream->tellg() * CharSize;

    p_input_stream->read((typename InputStream::char_type*)buffer, 4 / CharSize);
    read_n = (size_t)(p_input_stream->gcount());
    result = quex::__bom_snap_core(buffer, read_n, &byte_n);

    /* Avoid temporary function argument. Store sum in p0. */
    p0 += typename InputStream::pos_type(byte_n);
    p_input_stream->seekg(p0 / CharSize); 

    return result;
}
#endif

QUEX_INLINE QUEX_TYPE_BOM
#if ! defined (__QUEX_OPTION_PLAIN_C)
__bom_snap_core(uint8_t buffer[4], size_t read_n, size_t* byte_n)
#else
__quex_bom_snap_core(uint8_t buffer[4], size_t read_n, size_t* byte_n)
#endif
{
    /* For non-existing bytes fill 0x77, because it does not occur
     * anywhere as a criteria, see 'switch' after that.             */
    switch( read_n ) {
        case 0: return QUEX_BOM_NONE;
        case 1: buffer[1] = 0x77; buffer[2] = 0x77; buffer[3] = 0x77; break; 
        case 2:                   buffer[2] = 0x77; buffer[3] = 0x77; break;
        case 3:                                     buffer[3] = 0x77; break;
    }

#   if ! defined(__QUEX_OPTION_PLAIN_C)
    return bom_identify(buffer, byte_n);
#   else
    return quex_bom_identify(buffer, byte_n);
#   endif
}

QUEX_INLINE QUEX_TYPE_BOM
#if ! defined (__QUEX_OPTION_PLAIN_C)
bom_identify(const uint8_t* const Buffer, size_t* n)
#else
quex_bom_identify(const uint8_t* const Buffer, size_t* n)
#endif
    /* Assume, that the buffer contains at least 4 elements! */
{
    /* Table of byte order marks (BOMs), see file 'quex/code_base/bom' */
    const uint8_t B0 = Buffer[0];
    const uint8_t B1 = Buffer[1];
    const uint8_t B2 = Buffer[2];
    const uint8_t B3 = Buffer[3];
    QUEX_TYPE_BOM  x = QUEX_BOM_NONE;

    switch( B0 ) {
    case 0x00: if( B1 == 0x00 && B2 == 0xFE && B3 == 0xFF ) { *n = 4; x = QUEX_BOM_UTF_32_BE;        } break; 
    case 0x0E: if( B1 == 0xFE && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU;            } break;
    case 0x0F: if( B1 == 0xFE && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_TO_UCS;     } break; 
    case 0x18: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W0_TO_FE80; } break; 
    case 0x19: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W1_TO_FE80; } break; 
    case 0x1A: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W2_TO_FE80; } break; 
    case 0x1B: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W3_TO_FE80; } break; 
    case 0x1C: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W4_TO_FE80; } break; 
    case 0x1D: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W5_TO_FE80; } break; 
    case 0x1E: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W6_TO_FE80; } break; 
    case 0x1F: if( B1 == 0xA5 && B2 == 0xFF )               { *n = 3; x = QUEX_BOM_SCSU_W7_TO_FE80; } break; 
    case 0x2B: 
           /* In any case, the UTF7 BOM is not eaten. 
            * This is too complicated, since it uses a base64 code. It would require
            * to re-order the whole stream. This shall do the converter (if he wants). */
           *n = 0;
           if( B1 == 0x2F && B2 == 0x76 ) {
               switch( B3 ) 
               { case 0x2B: case 0x2F: case 0x38: case 0x39: x = QUEX_BOM_UTF_7; } 
           }
           break;
    case 0x84: if( B1 == 0x31 && B2 == 0x95 && B3 == 0x33 ) { *n = 4; x = QUEX_BOM_GB_18030;   } break;
    case 0xDD: if( B1 == 0x73 && B2 == 0x66 && B3 == 0x73 ) { *n = 4; x = QUEX_BOM_UTF_EBCDIC; } break;
    case 0xEF: if( B1 == 0xBB && B2 == 0xBF )               { *n = 3; x = QUEX_BOM_UTF_8;      } break;
    case 0xF7: if( B1 == 0x64 && B2 == 0x4C )               { *n = 3; x = QUEX_BOM_UTF_1;      } break;
    case 0xFB: 
           if( B1 == 0xEE && B2 == 0x28 ) {
               if( B3 == 0xFF )  { *n = 4; x = QUEX_BOM_BOCU_1; } 
               else              { *n = 3; x = QUEX_BOM_BOCU_1; }
           }
           break;
    case 0xFE: 
           if( B1 == 0xFF ) { *n = 2; x = QUEX_BOM_UTF_16_BE; } break;
    case 0xFF: 
           if( B1 == 0xFE ) {
               if( B2 == 0x00 && B3 == 0x00 ) { *n = 4; x = QUEX_BOM_UTF_32_LE; }
               else                           { *n = 2; x = QUEX_BOM_UTF_16_LE; } 
           }
           break;
    default: 
           *n = 0;
    }

    return x;
}           

QUEX_INLINE const char*
#if ! defined (__QUEX_OPTION_PLAIN_C)
bom_name(QUEX_TYPE_BOM BOM)
#else
quex_bom_name(QUEX_TYPE_BOM BOM)
#endif
{
    switch( BOM ) {
    case QUEX_BOM_UTF_8:           return "UTF_8";                      
    case QUEX_BOM_UTF_1:           return "UTF_1";                      
    case QUEX_BOM_UTF_EBCDIC:      return "UTF_EBCDIC";            
    case QUEX_BOM_BOCU_1:          return "BOCU_1";                    
    case QUEX_BOM_GB_18030:        return "GB_18030";                
    case QUEX_BOM_UTF_7:           return "UTF_7";                      
    case QUEX_BOM_UTF_16:          return "UTF_16";                                  
    case QUEX_BOM_UTF_16_LE:       return "UTF_16_LE";              
    case QUEX_BOM_UTF_16_BE:       return "UTF_16_BE";              
    case QUEX_BOM_UTF_32:          return "UTF_32";                    
    case QUEX_BOM_UTF_32_LE:       return "UTF_32_LE";              
    case QUEX_BOM_UTF_32_BE:       return "UTF_32_BE";              
    case QUEX_BOM_SCSU:            return "SCSU";                        
    case QUEX_BOM_SCSU_TO_UCS:     return "SCSU_TO_UCS";          
    case QUEX_BOM_SCSU_W0_TO_FE80: return "SCSU_W0_TO_FE80";  
    case QUEX_BOM_SCSU_W1_TO_FE80: return "SCSU_W1_TO_FE80";  
    case QUEX_BOM_SCSU_W2_TO_FE80: return "SCSU_W2_TO_FE80";  
    case QUEX_BOM_SCSU_W3_TO_FE80: return "SCSU_W3_TO_FE80";  
    case QUEX_BOM_SCSU_W4_TO_FE80: return "SCSU_W4_TO_FE80";  
    case QUEX_BOM_SCSU_W5_TO_FE80: return "SCSU_W5_TO_FE80";  
    case QUEX_BOM_SCSU_W6_TO_FE80: return "SCSU_W6_TO_FE80";  
    case QUEX_BOM_SCSU_W7_TO_FE80: return "SCSU_W7_TO_FE80";  
    default:
    case QUEX_BOM_NONE:            return "NONE";                        
    }
}

#if ! defined(__QUEX_OPTION_PLAIN_C)
} /* namespace quex */
#endif


#include <quex/code_base/temporary_macros_off>

#endif /* __QUEX_INCLUDE_GUARD__BOM_I */

