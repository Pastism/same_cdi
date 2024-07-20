/* 7zStream.c -- 7z Stream functions
2013-11-12 : Igor Pavlov : Public domain */

#include "Precomp.h"

#include <string.h>

#include "7zTypes.h"

static SRes LookToRead_Look_Lookahead(void *pp, const void **buf, size_t *size)
{
  SRes res = SZ_OK;
  CLookToRead *p = (CLookToRead *)pp;
  size_t size2 = p->size - p->pos;
  if (size2 == 0 && *size > 0)
  {
    p->pos = 0;
    size2 = LookToRead_BUF_SIZE;
    res = p->realStream->Read(p->realStream, p->buf, &size2);
    p->size = size2;
  }
  if (size2 < *size)
    *size = size2;
  *buf = p->buf + p->pos;
  return res;
}

static SRes LookToRead_Look_Exact(void *pp, const void **buf, size_t *size)
{
  SRes res = SZ_OK;
  CLookToRead *p = (CLookToRead *)pp;
  size_t size2 = p->size - p->pos;
  if (size2 == 0 && *size > 0)
  {
    p->pos = 0;
    if (*size > LookToRead_BUF_SIZE)
      *size = LookToRead_BUF_SIZE;
    res = p->realStream->Read(p->realStream, p->buf, size);
    size2 = p->size = *size;
  }
  if (size2 < *size)
    *size = size2;
  *buf = p->buf + p->pos;
  return res;
}

static SRes LookToRead_Skip(void *pp, size_t offset)
{
  CLookToRead *p = (CLookToRead *)pp;
  p->pos += offset;
  return SZ_OK;
}

static SRes LookToRead_Read(void *pp, void *buf, size_t *size)
{
  CLookToRead *p = (CLookToRead *)pp;
  size_t rem = p->size - p->pos;
  if (rem == 0)
    return p->realStream->Read(p->realStream, buf, size);
  if (rem > *size)
    rem = *size;
  memcpy(buf, p->buf + p->pos, rem);
  p->pos += rem;
  *size = rem;
  return SZ_OK;
}

static SRes LookToRead_Seek(void *pp, Int64 *pos, ESzSeek origin)
{
  CLookToRead *p = (CLookToRead *)pp;
  p->pos = p->size = 0;
  return p->realStream->Seek(p->realStream, pos, origin);
}

void LookToRead_CreateVTable(CLookToRead *p, int lookahead)
{
  p->s.Look = lookahead ?
      LookToRead_Look_Lookahead :
      LookToRead_Look_Exact;
  p->s.Skip = LookToRead_Skip;
  p->s.Read = LookToRead_Read;
  p->s.Seek = LookToRead_Seek;
}

void LookToRead_Init(CLookToRead *p)
{
  p->pos = p->size = 0;
}

static SRes SecToLook_Read(void *pp, void *buf, size_t *size)
{
  CSecToLook *p = (CSecToLook *)pp;
  return LookInStream_LookRead(p->realStream, buf, size);
}

static SRes SecToRead_Read(void *pp, void *buf, size_t *size)
{
  CSecToRead *p = (CSecToRead *)pp;
  return p->realStream->Read(p->realStream, buf, size);
}
