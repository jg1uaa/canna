/* Copyright 1994 NEC Corporation, Tokyo, Japan.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of NEC
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  NEC Corporation makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * NEC CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN 
 * NO EVENT SHALL NEC CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
 * OTHER TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE. 
 */

/* LINTLIBRARY */

#include "RKintern.h"

/*
 * PackBits
 */
#define	BIT_UNIT	8

/*
  学習ファイル用ビット操作処理

  学習オフセットの情報を、できるだけ小さいビットの配列として保持する。

    候補数     1 2 3 4 5 6 7 8 9 ...    n
    ビット幅   2 3 3 4 4 4 4 5 5     log(n) + 1

  RkPackBits は unsigned の配列の数を dst_bits の dst_offset(ビットで
  カウント)の先から格納する。端数が出た場合は下位ビットから使われる。

  引数
    dst_bits   -- ビット配列へのポインタ
    dst_offset -- 実際にアクセスするところまでのオフセット(ビットでカウント)
    bit_size   -- ビット配列の１つの要素のビット幅
    src_ints   -- 格納したい数値の配列
    count      -- 格納したい数

  戻り値

 */

long
_RkPackBits(dst_bits, dst_offset, bit_size, src_ints, count)
unsigned char	*dst_bits;
long		dst_offset;
int		bit_size;
unsigned	*src_ints;
int		count;
{
  unsigned char	*dstB;
  unsigned		dstQ;
  unsigned		dstCount;
  unsigned		bitMask;
  
  dstB = dst_bits + dst_offset / BIT_UNIT;
  dstCount = (dst_offset % BIT_UNIT);

  /* 途中なので、手を付けない部分があることに注意 */
  dstQ  = *dstB & ((1 << dstCount) - 1);
  bitMask = (1 << bit_size) - 1;
  while (count-- > 0) {
    dstQ |= (*src_ints++ & bitMask) << dstCount;
    dstCount += bit_size;
    dst_offset += bit_size;
    while (dstCount >= BIT_UNIT) {
      *dstB++ = dstQ & ((1 << BIT_UNIT) - 1);
      dstQ >>= BIT_UNIT;
      dstCount -= BIT_UNIT;
    }
  }
  if (dstCount) {
    *dstB = (*dstB & ~((1 << dstCount) - 1)) | (dstQ & ((1 << dstCount) - 1));
  }
  return dst_offset;
}

/*
  UnpackBits

  RkUnpackBits は dst_bits の dst_offset(ビットでカウント)に格納されて
  いるビットの配列を unsigned の配列に取り出す。offset に端数が出た場
  合は下位ビットから使われる。

  引数
    dst_ints   -- 取り出した数値を格納する配列へのポインタ
    src_bits   -- ビット配列へのポインタ
    src_offset -- 実際に格納するところまでのオフセット(ビットでカウント)
    bit_size   -- ビット配列の１つの要素のビット幅
    count      -- 取り出したい数

  戻り値

 */

long
_RkUnpackBits(dst_ints, src_bits, src_offset, bit_size, count)
unsigned	*dst_ints;
unsigned char	*src_bits;
long		src_offset;
int		bit_size;
int		count;
{
  unsigned char	*srcB;
  unsigned		srcQ;
  unsigned		srcCount;
  unsigned		bitMask;
  
  srcB = src_bits + src_offset / BIT_UNIT;
  srcCount = BIT_UNIT - (src_offset % BIT_UNIT);
  srcQ  = *srcB++ >> (src_offset % BIT_UNIT);
  bitMask = (1 << bit_size) - 1;
  while (count-- > 0) {
    while (srcCount < (unsigned)bit_size) {
      srcQ |= (*srcB++ << srcCount);
      srcCount += BIT_UNIT;
    }
    *dst_ints++ = srcQ & bitMask;
    srcQ >>= bit_size;
    srcCount -= bit_size;
    src_offset += bit_size;
  }
  return src_offset;
}

/*
  CopyBits

  RkCopyBits は src_bits の src_offset に格納されているビット配列を 
  count 個だけ dst_bits の dst_offsetに移動させる。

  引数
    dst_bits   -- 移動先ビット配列へのポインタ
    dst_offset -- 実際に格納するところまでのオフセット(ビットでカウント)
    bit_size   -- ビット配列の１つの要素のビット幅
    src_bits   -- 移動元ビット配列へのポインタ
    src_offset -- 取り出すところまでのオフセット(ビットでカウント)
    count      -- 移動したい数

  戻り値

 */

long
_RkCopyBits(dst_bits, dst_offset, bit_size, src_bits, src_offset, count)
unsigned char	*dst_bits;
long		dst_offset;
int		bit_size;
unsigned char	*src_bits;
long		src_offset;
int		count;
{
  unsigned char	*dstB;
  unsigned		dstQ;
  unsigned		dstCount;
  unsigned char	*srcB;
  unsigned		srcQ;
  unsigned		srcCount;
  unsigned		bitMask;
  unsigned		bits;
  
  dstB = dst_bits + dst_offset / BIT_UNIT;
  dstCount = (dst_offset % BIT_UNIT);
  dstQ  = *dstB & ((1 << dstCount) - 1);
  srcB = src_bits + src_offset / BIT_UNIT;
  srcCount = BIT_UNIT - (src_offset % BIT_UNIT);
  srcQ  = *srcB++ >> (src_offset % BIT_UNIT);
  bitMask = (1 << bit_size) - 1;
  while (count-- > 0) {
    /* unpack */
    while (srcCount < (unsigned)bit_size) {
      srcQ |= (*srcB++ << srcCount);
      srcCount += BIT_UNIT;
    }
    bits = srcQ & bitMask;
    srcQ >>= bit_size;
    srcCount -= bit_size;
    src_offset += bit_size;
    /* pack */
    dstQ |= bits << dstCount;
    dstCount += bit_size;
    dst_offset += bit_size;
    while (dstCount >= BIT_UNIT) {
      *dstB++ = dstQ & ((1 << BIT_UNIT) - 1);
      dstQ >>= BIT_UNIT;
      dstCount -= BIT_UNIT;
    }
  }
  if (dstCount) {
    *dstB = (*dstB & ~((1 << dstCount) - 1)) | (dstQ & ((1 << dstCount) - 1));
  }
  return dst_offset;
}

/*
  _RkSetBitNum

  _RkSetBitNum は bit 配列の offset 位置から n 番目の値として val を格
  納する関数である。

  引数
    dst_bits   -- ビット配列へのポインタ
    dst_offset -- 実際に格納するところまでのオフセット(ビットでカウント)
    bit_size   -- ビット配列の１つの要素のビット幅
    n          -- 先頭から何番目の要素かを与える。
    val        -- 格納する値を与える。

  戻り値

 */

int
_RkSetBitNum(dst_bits, dst_offset, bit_size, n, val)
unsigned char	*dst_bits;
unsigned long	dst_offset;
int		bit_size, n, val;
{
  unsigned char	*dstB;
  unsigned dstQ, dstCount, bitMask;

  dst_offset += bit_size * n;

  dstB = dst_bits + dst_offset / BIT_UNIT;
  dstCount = (dst_offset % BIT_UNIT);

  /* 途中なので、手を付けない部分があることに注意 */
  dstQ  = *dstB & ((1 << dstCount) - 1);
  bitMask = (1 << bit_size) - 1;

  dstQ |= (val & bitMask) << dstCount;
  dstCount += bit_size;
  dst_offset += bit_size;
  while (dstCount >= BIT_UNIT) {
    *dstB++ = dstQ & ((1 << BIT_UNIT) - 1);
    dstQ >>= BIT_UNIT;
    dstCount -= BIT_UNIT;
  }
  if (dstCount) {
    *dstB = (*dstB & ~((1 << dstCount) - 1)) | (dstQ & ((1 << dstCount) - 1));
  }
  return dst_offset;
}

int
_RkCalcFqSize(n)
int	n;
{
  return n*(_RkCalcLog2(n) + 1);
}

#ifdef __BITS_DEBUG__
#include <stdio.h>
_RkPrintPackedBits(bits, offset, bit_size, count)
unsigned char	*bits;
int		offset;
int		bit_size;
int		count;
{
    fprintf(stderr, "%d <", count);
    while ( count-- > 0 ) {
	unsigned w;

        offset = _RkUnpackBits(&w, bits, offset, bit_size, 1);
        fprintf(stderr, " %d", w/2);
    };
    fprintf(stderr, ">\n");
}

int 
_RkCalcLog2(n)
     int n;
{
  int	lg2;
  
  n--;
  for (lg2 = 0; n > 0; lg2++)
    n >>= 1;
  return(lg2);
}

main()
{
  int		 offset;
  int		 bit_size;
  int		 size;
  unsigned char bits[1024*8];
  unsigned char Bits[1024*8];
  int	c, i;
  int		ec;
  int	o;
  
  /* create test run */
  for ( size = 1; size <= 32; size++ ) {
    bit_size = _RkCalcLog2(size) + 1;
    printf("#%4d/%2d\t", size, bit_size);
    /* pack 'em all */
    o = 0;
    for ( i = 0; i < size; i++ )
      o = _RkPackBits(Bits, o, bit_size, &i, 1);
    printf("PK ");
    for ( i = 0; i < (bit_size*size+7)/8; i++ )
      printf(" %02x", Bits[i]);
    printf("\n");
    
    
    for ( offset = 0; offset < 16; offset++ ) {
      /* copybits */
      o = _RkCopyBits(bits, offset, bit_size, Bits, 0, size);
      printf("%d ", offset);
      for ( i = 0; i < (o + 7)/8; i++ )
	printf(" %02x", bits[i]);
      printf("\n");
      
      /* unpack 'em again */
      ec = 0;
      o = offset;
      for ( i = 0; i < size; i++ ) {
	unsigned w;
	  
	o = _RkUnpackBits(&w, bits, o, bit_size, 1);
	if ( w != i )
	  ec++;
      };
      if ( ec )
	printf(" %d", offset);
      else
	printf(".");
    };
    printf("\n");
  };
}
#endif
