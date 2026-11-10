#pragma once

/**
 * 
 */
typedef unsigned long long sample_t;

/**
 * 
 */
#define S8_TRANSLATION 0x7F
#define S16_TRANSLATION 0x7FFF
#define S32_TRANSLATION 0x7FFFFFFF

#define U8_TRANSLATION 0xDF
#define U16_TRANSLATION 0xDFFF
#define U32_TRANSLATION 0xDFFFFFFF

/**
 * 
 */
#define TRANSLATE_FROM_FLOAT(src_, dst_, dstType_, translator_) dst_ = static_cast<dstType_>(src_ * translator_)
#define TRANSLATE_TO_FLOAT(src_, dst_, translator_) dst_ = static_cast<float>(src_) / translator_

#define FLOAT_TO_S8(src_, dst_) TRANSLATE_FROM_FLOAT(src_, dst_, int8_t, S8_TRANSLATION)
#define FLOAT_TO_S16(src_, dst_) TRANSLATE_FROM_FLOAT(src_, dst_, int16_t, S16_TRANSLATION)
#define FLOAT_TO_S32(src_, dst_) TRANSLATE_FROM_FLOAT(src_, dst_, int32_t, S32_TRANSLATION)

#define FLOAT_TO_U8(src_, dst_) TRANSLATE_FROM_FLOAT(src_, dst_, int8_t, U8_TRANSLATION)
#define FLOAT_TO_U16(src_, dst_) TRANSLATE_FROM_FLOAT(src_, dst_, int16_t, U16_TRANSLATION)
#define FLOAT_TO_U32(src_, dst_) TRANSLATE_FROM_FLOAT(src_, dst_, int32_t, U32_TRANSLATION)

#define S8_TO_FLOAT(src_, dst_) TRANSLATE_TO_FLOAT(src_, dst_, S8_TRANSLATION)
#define S16_TO_FLOAT(src_, dst_) TRANSLATE_TO_FLOAT(src_, dst_, S16_TRANSLATION)
#define S32_TO_FLOAT(src_, dst_) TRANSLATE_TO_FLOAT(src_, dst_, S32_TRANSLATION)

#define U8_TO_FLOAT(src_, dst_) TRANSLATE_TO_FLOAT(src_, dst_, U8_TRANSLATION)
#define U16_TO_FLOAT(src_, dst_) TRANSLATE_TO_FLOAT(src_, dst_, U16_TRANSLATION)
#define U32_TO_FLOAT(src_, dst_) TRANSLATE_TO_FLOAT(src_, dst_, U32_TRANSLATION)
