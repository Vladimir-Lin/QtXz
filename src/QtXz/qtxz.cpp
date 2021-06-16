/****************************************************************************
 *
 * Copyright (C) 2016 Neutrino International Inc.
 *
 * Author   : Brian Lin ( Vladimir Lin , Vladimir Forest )
 * E-mail   : lin.foxman@gmail.com
 *          : lin.vladimir@gmail.com
 *          : wolfram_lin@yahoo.com
 *          : wolfram_lin@sina.com
 *          : wolfram_lin@163.com
 * Skype    : wolfram_lin
 * WeChat   : 153-0271-7160
 * WhatsApp : 153-0271-7160
 * QQ       : lin.vladimir@gmail.com (2107437784)
 * URL      : http://qtxz.sourceforge.net/
 *
 * QtXz acts as an interface between Qt and XZ Utils library.
 * Please keep QtXz as simple as possible.
 *
 * Copyright 2001 ~ 2016
 *
 ****************************************************************************/

#include <qtxz.h>

extern "C" {
#include <lzma.h>
}

QT_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////////

#define IsNull(item)  ( NULL == (item) )
#define NotNull(item) ( NULL != (item) )
#define NotEqual(a,b) ( (a)  != (b)    )

#define XZ_MAX_UNUSED ( 8 * 1024       )

#pragma pack(push,1)

typedef struct                         {
  char        buffer [ XZ_MAX_UNUSED ] ;
  char        unused [ XZ_MAX_UNUSED ] ;
  int         bufferSize               ;
  bool        Writing                  ;
  lzma_stream Stream                   ;
  lzma_ret    LastError                ;
  bool        InitialisedOk            ;
} XzFile                               ;

#pragma pack(pop)

static unsigned char XzHeader[] = { 0xFD , 0x37 , 0x7A , 0x58 , 0x5A , 0x0 } ;

//////////////////////////////////////////////////////////////////////////////

QtXz:: QtXz     (void)
     : xzPacket (NULL)
{
}

QtXz::~QtXz(void)
{
  CleanUp ( ) ;
}

QString QtXz::Version(void)
{
  return QString::fromUtf8 ( LZMA_VERSION_STRING ) ;
}

bool QtXz::isXz(QByteArray & header)
{
  if ( header . size ( ) < 6                ) return false  ;
  unsigned char * hxz = (unsigned char *) header . data ( ) ;
  if ( 0 == ::memcmp ( XzHeader , hxz , 6 ) ) return true   ;
  return false                                              ;
}

bool QtXz::IsCorrect(int returnCode)
{
  if ( returnCode == LZMA_OK         ) return true ;
  if ( returnCode == LZMA_STREAM_END ) return true ;
  return false                                     ;
}

bool QtXz::IsEnd(int returnCode)
{
  return ( returnCode == LZMA_STREAM_END ) ;
}

bool QtXz::IsFault(int returnCode)
{
  return ( returnCode >= LZMA_MEM_ERROR ) ;
}

void QtXz::CleanUp(void)
{
  if ( NULL == xzPacket ) return ;
  ::free ( xzPacket)             ;
  xzPacket = NULL                ;
}

int QtXz::BeginCompress(QVariantList arguments)
{
  int level = 9                                         ;
  if (arguments.count()>0) level = arguments[0].toInt() ;
  return BeginCompress ( level )                        ;
}

int QtXz::BeginCompress(int level)
{
  XzFile * xzf = NULL                                      ;
  if (level < 1) level = 1                                 ;
  if (level > 9) level = 9                                 ;
  //////////////////////////////////////////////////////////
  xzf = (XzFile *)::malloc(sizeof(XzFile))                 ;
  if (IsNull(xzf)) return LZMA_MEM_ERROR                   ;
  //////////////////////////////////////////////////////////
  lzma_stream stream = LZMA_STREAM_INIT                    ;
  ::memset ( xzf , 0 , sizeof(XzFile) )                    ;
  xzf -> bufferSize      = 0                               ;
  xzf -> Writing         = true                            ;
  xzf -> LastError       = LZMA_OK                         ;
  xzf -> InitialisedOk   = false                           ;
  xzf -> Stream          = stream                          ;
  //////////////////////////////////////////////////////////
  ::lzma_easy_encoder(&xzf->Stream,level,LZMA_CHECK_CRC64) ;
  //////////////////////////////////////////////////////////
  xzf -> Stream.avail_in = 0                               ;
  xzf -> InitialisedOk   = true                            ;
  //////////////////////////////////////////////////////////
  xzPacket               = xzf                             ;
  return LZMA_OK                                           ;
}

int QtXz::doCompress(const QByteArray & Source,QByteArray & Compressed)
{
  if (IsNull(xzPacket)) return LZMA_MEM_ERROR       ;
  int      n, ret                                   ;
  XzFile * xzf = (XzFile *)xzPacket                 ;
  ///////////////////////////////////////////////////
  if (!xzf->Writing) return LZMA_DATA_ERROR         ;
  ///////////////////////////////////////////////////
  Compressed . clear ( )                            ;
  ret = LZMA_OK                                     ;
  if (Source.size()<=0) return LZMA_OK              ;
  ///////////////////////////////////////////////////
  xzf->Stream.avail_in =            Source . size() ;
  xzf->Stream.next_in  = (uint8_t *)Source . data() ;
  ///////////////////////////////////////////////////
  while ( true )                                    {
    xzf->Stream.avail_out = XZ_MAX_UNUSED           ;
    xzf->Stream.next_out  = (uint8_t *)xzf->buffer  ;
    lzma_code ( &(xzf->Stream) , LZMA_RUN )         ;
    n  = XZ_MAX_UNUSED - xzf->Stream.avail_out      ;
    if (n>0)                                        {
      Compressed.append(xzf->buffer,n)              ;
    }                                               ;
    if (xzf->Stream.avail_in == 0) return LZMA_OK   ;
  }                                                 ;
  ///////////////////////////////////////////////////
  return LZMA_DATA_ERROR                            ;
}

int QtXz::doSection(QByteArray & Source,QByteArray & Compressed)
{
  if (IsNull(xzPacket)) return LZMA_MEM_ERROR       ;
  int      n, ret                                   ;
  XzFile * xzf = (XzFile *)xzPacket                 ;
  ///////////////////////////////////////////////////
  if (!xzf->Writing) return LZMA_DATA_ERROR         ;
  ///////////////////////////////////////////////////
  Compressed . clear ( )                            ;
  ret = LZMA_OK                                     ;
  if (Source.size()<=0) return LZMA_OK              ;
  ///////////////////////////////////////////////////
  if (Source.size()>XZ_MAX_UNUSED)                  {
    n                    = XZ_MAX_UNUSED            ;
    xzf->bufferSize      = n                        ;
    xzf->Stream.avail_in = n                        ;
    xzf->Stream.next_in  = (uint8_t *)Source.data() ;
  } else                                            {
    n                    = Source.size()            ;
    xzf->bufferSize      = n                        ;
    xzf->Stream.avail_in = n                        ;
    xzf->Stream.next_in  = (uint8_t *)xzf->unused   ;
    memset(xzf->unused,0,XZ_MAX_UNUSED)             ;
    memcpy(xzf->unused,Source.data(),n)             ;
  }                                                 ;
  ///////////////////////////////////////////////////
  xzf->Stream.avail_out = XZ_MAX_UNUSED             ;
  xzf->Stream.next_out  = (uint8_t *)xzf->buffer    ;
  ::lzma_code ( &(xzf->Stream) , LZMA_RUN )         ;
  ///////////////////////////////////////////////////
  n = ( xzf->bufferSize - xzf->Stream.avail_in )    ;
  if (n>0)                                          {
    Source.remove(0,n)                              ;
  }                                                 ;
  ///////////////////////////////////////////////////
  n  = XZ_MAX_UNUSED - xzf->Stream.avail_out        ;
  if (n>0)                                          {
    Compressed.append(xzf->buffer,n)                ;
    return LZMA_OK                                  ;
  }                                                 ;
  if (xzf->Stream.avail_in == 0) return LZMA_OK     ;
  ///////////////////////////////////////////////////
  return LZMA_DATA_ERROR                            ;
}

int QtXz::CompressDone(QByteArray & Compressed)
{
  int      n                                           ;
  XzFile * xzf = (XzFile*)xzPacket                     ;
  if ( IsNull(xzf)   ) return LZMA_OK                  ;
  if ( !xzf->Writing ) return LZMA_DATA_ERROR          ;
  //////////////////////////////////////////////////////
  if (xzf->LastError == LZMA_OK)                       {
    while ( true )                                     {
      xzf -> Stream.avail_out = XZ_MAX_UNUSED          ;
      xzf -> Stream.next_out  = (uint8_t *)xzf->buffer ;
      ::lzma_code ( &(xzf->Stream) , LZMA_FINISH )     ;
      //////////////////////////////////////////////////
      if ( xzf -> Stream.avail_out < XZ_MAX_UNUSED)    {
        n  = XZ_MAX_UNUSED - xzf->Stream.avail_out     ;
        if (n>0)                                       {
          Compressed.append(xzf->buffer,n)             ;
        } else break                                   ;
      } else break                                     ;
    }                                                  ;
  }                                                    ;
  //////////////////////////////////////////////////////
  ::lzma_end ( &(xzf->Stream) )                        ;
  return LZMA_OK                                       ;
}

int QtXz::BeginDecompress(void)
{
  XzFile * xzf     = NULL                         ;
  lzma_ret RetXz   = LZMA_OK                      ;
  /////////////////////////////////////////////////
  xzf = (XzFile *)::malloc(sizeof(XzFile))        ;
  if (IsNull(xzf)) return LZMA_MEM_ERROR          ;
  /////////////////////////////////////////////////
  lzma_stream stream = LZMA_STREAM_INIT           ;
  memset ( xzf , 0 , sizeof(XzFile) )             ;
  xzf->bufferSize    = 0                          ;
  xzf->Writing       = false                      ;
  xzf->LastError     = LZMA_OK                    ;
  xzf->InitialisedOk = false                      ;
  xzf->Stream        = stream                     ;
  /////////////////////////////////////////////////
  RetXz = lzma_stream_decoder                     (
              &(xzf->Stream)                      ,
              UINT64_MAX                          ,
              LZMA_TELL_UNSUPPORTED_CHECK         |
              LZMA_CONCATENATED                 ) ;
  if (RetXz != LZMA_OK)                           {
    ::free(xzf)                                   ;
    return (int)RetXz                             ;
  }                                               ;
  /////////////////////////////////////////////////
  xzf -> Stream.avail_in = xzf->bufferSize        ;
  xzf -> Stream.next_in  = (uint8_t *)xzf->buffer ;
  xzf -> InitialisedOk   = true                   ;
  /////////////////////////////////////////////////
  xzPacket = xzf                                  ;
  return LZMA_OK                                  ;
}

int QtXz::doDecompress(const QByteArray & Source,QByteArray & Decompressed)
{
  int      n                                               ;
  lzma_ret RetXz = LZMA_OK                                 ;
  XzFile * xzf   = (XzFile*)xzPacket                       ;
  if ( IsNull(xzf)  ) return LZMA_OK                       ;
  if ( xzf->Writing ) return LZMA_DATA_ERROR               ;
  //////////////////////////////////////////////////////////
  if ( xzf->LastError == LZMA_STREAM_END)                  {
    Decompressed . clear ( )                               ;
    return LZMA_STREAM_END                                 ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  xzf->LastError = LZMA_OK                                 ;
  RetXz          = LZMA_OK                                 ;
  if (Source.size()<=0)                                    {
    Decompressed . clear ( )                               ;
    return LZMA_STREAM_END                                 ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  xzf -> Stream.avail_out = Decompressed.size()            ;
  xzf -> Stream.next_out  = (uint8_t *)Decompressed.data() ;
  //////////////////////////////////////////////////////////
  char * src = (char *)Source.data()                       ;
  int    idx = 0                                           ;
  while ( true )                                           {
    if ( xzf->Stream.avail_in == 0 )                       {
      if ( ( Source.size() - idx ) > XZ_MAX_UNUSED )       {
        n = XZ_MAX_UNUSED                                  ;
      } else                                               {
        n = Source.size() - idx                            ;
      }                                                    ;
      memcpy(xzf->buffer,src,n)                            ;
      src += n                                             ;
      idx += n                                             ;
      //////////////////////////////////////////////////////
      xzf->bufferSize      = n                             ;
      xzf->Stream.avail_in =            xzf->bufferSize    ;
      xzf->Stream.next_in  = (uint8_t *)xzf->buffer        ;
    }                                                      ;
    ////////////////////////////////////////////////////////
    RetXz = lzma_code ( &(xzf->Stream) , LZMA_RUN )        ;
    if ( (RetXz!=LZMA_OK) && (RetXz!=LZMA_STREAM_END) )    {
      return (int)RetXz                                    ;
    }                                                      ;
    ////////////////////////////////////////////////////////
    if (RetXz == LZMA_OK                                  &&
        xzf -> Stream.avail_in == 0                       &&
        xzf -> Stream.avail_out > 0 )                      {
      n = Decompressed.size() - xzf->Stream.avail_out      ;
      if (n==0) Decompressed . clear  (   )                ;
           else Decompressed . resize ( n )                ;
      xzf->LastError = LZMA_OK                             ;
      return LZMA_OK                                       ;
    }                                                      ;
    if ( RetXz == LZMA_STREAM_END )                        {
      n = Decompressed.size() - xzf->Stream.avail_out      ;
      if (n==0) Decompressed . clear  (   )                ;
           else Decompressed . resize ( n )                ;
      xzf->LastError = LZMA_STREAM_END                     ;
      return LZMA_STREAM_END                               ;
    }                                                      ;
    if (xzf->Stream.avail_out == 0)                        {
      Decompressed . clear ( )                             ;
      xzf->LastError = LZMA_OK                             ;
      return LZMA_OK                                       ;
    }                                                      ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  return LZMA_OK                                           ;
}

int QtXz::undoSection(QByteArray & Source,QByteArray & Decompressed)
{
  int      n                                               ;
  lzma_ret RetXz = LZMA_OK                                 ;
  XzFile * xzf   = (XzFile*)xzPacket                       ;
  if ( IsNull(xzf)  ) return LZMA_OK                       ;
  if ( xzf->Writing ) return LZMA_DATA_ERROR               ;
  //////////////////////////////////////////////////////////
  if ( xzf->LastError == LZMA_STREAM_END)                  {
    Decompressed . clear ( )                               ;
    return LZMA_STREAM_END                                 ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  xzf->LastError = LZMA_OK                                 ;
  RetXz          = LZMA_OK                                 ;
  if (Source.size()<=0)                                    {
    Decompressed . clear ( )                               ;
    return LZMA_STREAM_END                                 ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  xzf -> Stream.avail_out = XZ_MAX_UNUSED                  ;
  xzf -> Stream.next_out  = (uint8_t *)xzf->unused         ;
  //////////////////////////////////////////////////////////
  char * src = (char *)Source.data()                       ;
  if ( Source.size() > XZ_MAX_UNUSED )                     {
    n                    = XZ_MAX_UNUSED                   ;
    xzf->bufferSize      = n                               ;
    xzf->Stream.avail_in = n                               ;
    xzf->Stream.next_in  = (uint8_t *)src                  ;
  } else                                                   {
    n                    = Source.size()                   ;
    memcpy(xzf->buffer,src,n)                              ;
    xzf->bufferSize      = n                               ;
    xzf->Stream.avail_in = n                               ;
    xzf->Stream.next_in  = (uint8_t *)xzf->buffer          ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  RetXz = lzma_code ( &(xzf->Stream) , LZMA_RUN )          ;
  if ( (RetXz!=LZMA_OK) && (RetXz!=LZMA_STREAM_END) )      {
    return (int)RetXz                                      ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  n = ( xzf->bufferSize - xzf->Stream.avail_in )           ;
  if (n>0) Source.remove(0,n)                              ;
  //////////////////////////////////////////////////////////
  if (RetXz == LZMA_OK                                    &&
      xzf   -> Stream.avail_in == 0                       &&
      xzf   -> Stream.avail_out > 0 )                      {
    n = XZ_MAX_UNUSED - xzf->Stream.avail_out              ;
    if (n==0) Decompressed . clear  (   ) ; else           {
      Decompressed .append(xzf->unused,n)                  ;
    }                                                      ;
    xzf->LastError = LZMA_OK                               ;
    return LZMA_OK                                         ;
  }                                                        ;
  if ( RetXz == LZMA_STREAM_END )                          {
    n = XZ_MAX_UNUSED - xzf->Stream.avail_out              ;
    if (n==0) Decompressed . clear  (   ) ; else           {
      Decompressed .append(xzf->unused,n)                  ;
    }                                                      ;
    xzf->LastError = LZMA_STREAM_END                       ;
    return LZMA_STREAM_END                                 ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  if (xzf->Stream.avail_out == 0)                          {
    if (RetXz==LZMA_OK)                                    {
      n = XZ_MAX_UNUSED - xzf->Stream.avail_out            ;
      if (n==0) Decompressed . clear  (   ) ; else         {
        Decompressed .append(xzf->unused,n)                ;
      }                                                    ;
      xzf->LastError = LZMA_OK                             ;
    } else                                                 {
      Decompressed . clear ( )                             ;
      xzf->LastError = LZMA_OK                             ;
      return LZMA_OK                                       ;
    }                                                      ;
  }                                                        ;
  //////////////////////////////////////////////////////////
  return LZMA_OK                                           ;
}

int QtXz::DecompressDone(void)
{
  XzFile * xzf = (XzFile*)xzPacket           ;
  if ( IsNull(xzf)  ) return LZMA_OK         ;
  if ( xzf->Writing ) return LZMA_DATA_ERROR ;
  ////////////////////////////////////////////
  if ( xzf->InitialisedOk)                   {
    lzma_end ( &(xzf->Stream) )              ;
  }                                          ;
  return LZMA_OK                             ;
}

bool QtXz::IsTail(QByteArray & header)
{
  if (header.size()<12) return false    ;
  char * footer = (char *)header.data() ;
  bool correct = true                   ;
  if (footer[10]!='Y') correct = false  ;
  if (footer[11]!='Z') correct = false  ;
  return correct                        ;
}

//////////////////////////////////////////////////////////////////////////////

QByteArray CompressXz(const QByteArray & data,int level)
{
  QByteArray    Body                                 ;
  if (data.size()<=0) return Body                    ;
  ////////////////////////////////////////////////////
  lzma_stream strm     = LZMA_STREAM_INIT            ;
  bool        done     = false                       ;
  int         Total    = data.size()                 ;
  int         Out      = 0                           ;
  int         Index    = 0                           ;
  int         Compress = 0                           ;
  uint8_t   * Data     = (uint8_t *)data.data()      ;
  int         Size     = 256 * 1024                  ;
  uint8_t     Buffer   [ 256 * 1024 ]                ;
  ////////////////////////////////////////////////////
  lzma_easy_encoder(&strm,level,LZMA_CHECK_CRC64)    ;
  do                                                 {
    strm . next_in   = &Data[Index]                  ;
    strm . avail_in  = Size                          ;
    if ((Total-Index)<=Size)                         {
      strm . avail_in  = Total-Index                 ;
      done = true                                    ;
    }                                                ;
    strm . next_out  = Buffer                        ;
    strm . avail_out = Size                          ;
    Compress         =  strm.avail_in                ;
    lzma_code ( &strm , LZMA_RUN )                   ;
    Out      = Size - strm.avail_out                 ;
    Compress = Compress - strm.avail_in              ;
    Index   += Compress                              ;
    if (Out>0)                                       {
      Body.append((const char *)Buffer,Out)          ;
    }                                                ;
    if (Index>=Total) done = true                    ;
  } while ( !done )                                  ;
  strm . avail_in  = 0                               ;
  strm . next_out  = Buffer                          ;
  strm . avail_out = Size                            ;
  lzma_code ( &strm , LZMA_FINISH )                  ;
  Out      = Size - strm.avail_out                   ;
  if (Out>0)                                         {
    Body.append((const char *)Buffer,Out)            ;
  }                                                  ;
  lzma_end  ( &strm               )                  ;
  ////////////////////////////////////////////////////
  return Body                                        ;
}

//////////////////////////////////////////////////////////////////////////////

QByteArray UncompressXz(const QByteArray & data)
{
  QByteArray    Body                            ;
  if (data.size()<=0) return Body               ;
  ///////////////////////////////////////////////
  lzma_stream strm = LZMA_STREAM_INIT           ;
  bool        done     = false                  ;
  int         Total    = data.size()            ;
  int         Out      = 0                      ;
  int         Index    = 0                      ;
  int         Compress = 0                      ;
  uint8_t   * Data     = (uint8_t *)data.data() ;
  int         Size     = 256 * 1024             ;
  int         Ssize    = Size / 64              ;
  uint8_t     Buffer   [ 256 * 1024 ]           ;
  lzma_ret    RetXz                             ;
  ///////////////////////////////////////////////
  RetXz = ::lzma_stream_decoder                 (
              &strm                             ,
              UINT64_MAX                        ,
              LZMA_TELL_UNSUPPORTED_CHECK       |
              LZMA_CONCATENATED               ) ;
  if (RetXz != LZMA_OK) return Body             ;
  ///////////////////////////////////////////////
  do                                            {
    strm . next_in   = &Data[Index]             ;
    strm . avail_in  = Ssize                    ;
    if ((Total-Index)<=Ssize)                   {
      strm . avail_in  = Total-Index            ;
      done = true                               ;
    }                                           ;
    strm . next_out  = Buffer                   ;
    strm . avail_out = Size                     ;
    Compress         =  strm.avail_in           ;
    lzma_code ( &strm , LZMA_RUN )              ;
    Out      = Size - strm.avail_out            ;
    Compress = Compress - strm.avail_in         ;
    Index   += Compress                         ;
    if (Out>0)                                  {
      Body.append((const char *)Buffer,Out)     ;
    }                                           ;
    if (Index>=Total) done = true               ;
  } while ( !done )                             ;
  strm . avail_in  = 0                          ;
  strm . next_out  = Buffer                     ;
  strm . avail_out = Size                       ;
  lzma_code ( &strm , LZMA_FINISH )             ;
  Out      = Size - strm.avail_out              ;
  if (Out>0)                                    {
    Body.append((const char *)Buffer,Out)       ;
  }                                             ;
  ::lzma_end ( &strm )                          ;
  return Body                                   ;
}

//////////////////////////////////////////////////////////////////////////////

bool ToXz(const QByteArray & data,QByteArray & xz,int level)
{
  if ( data . size ( ) <= 0 ) return false ;
  //////////////////////////////////////////
  QtXz         L                           ;
  int          r                           ;
  QVariantList v                           ;
  v << level                               ;
  r = L . BeginCompress ( v )              ;
  if ( L . IsCorrect ( r ) )               {
    L . doCompress   ( data , xz )         ;
    L . CompressDone (        xz )         ;
  }                                        ;
  //////////////////////////////////////////
  return ( xz . size ( ) > 0 )             ;
}

//////////////////////////////////////////////////////////////////////////////

bool FromXz(const QByteArray & xz,QByteArray & data)
{
  if ( xz . size ( ) <= 0 ) return false ;
  ////////////////////////////////////////
  QtXz L                                 ;
  int  r                                 ;
  r = L . BeginDecompress ( )            ;
  if ( L . IsCorrect ( r ) )             {
    L . doDecompress ( xz , data )       ;
  }                                      ;
  ////////////////////////////////////////
  return ( data . size ( ) > 0 )         ;
}

//////////////////////////////////////////////////////////////////////////////

bool SaveXz (QString filename,QByteArray & data,int level)
{
  if ( data . size ( ) <= 0         ) return false                 ;
  QByteArray xz                                                    ;
  if ( level < 0 ) level = 9                                       ;
  if ( ! ToXz ( data , xz , level ) ) return false                 ;
  if ( xz . size ( ) <= 0           ) return false                 ;
  QFile F ( filename )                                             ;
  if ( ! F . open ( QIODevice::WriteOnly | QIODevice::Truncate ) ) {
    return false                                                   ;
  }                                                                ;
  F . write ( xz )                                                 ;
  F . close (    )                                                 ;
  return true                                                      ;
}

//////////////////////////////////////////////////////////////////////////////

bool LoadXz (QString filename,QByteArray & data)
{
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::ReadOnly ) ) return false ;
  QByteArray xz                                          ;
  xz = F . readAll ( )                                   ;
  F . close         ( )                                  ;
  if ( xz . size ( ) <= 0                 ) return false ;
  if ( ! FromXz ( xz , data )             ) return false ;
  return ( data . size ( ) > 0 )                         ;
}

//////////////////////////////////////////////////////////////////////////////

bool FileToXz (QString filename,QString xz,int level)
{
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::ReadOnly ) ) return false ;
  QByteArray data                                        ;
  data = F . readAll ( )                                 ;
  F . close ( )                                          ;
  if ( data . size ( ) <= 0               ) return false ;
  return SaveXz ( xz , data , level )                    ;
}

//////////////////////////////////////////////////////////////////////////////

bool XzToFile (QString xz,QString filename)
{
  QByteArray data                                        ;
  if ( ! LoadXz ( xz , data )             ) return false ;
  if ( data . size ( ) <=0                ) return false ;
  QFile F ( filename )                                   ;
  if ( ! F . open ( QIODevice::WriteOnly                 |
                    QIODevice::Truncate ) ) return false ;
  F . write ( data )                                     ;
  F . close (      )                                     ;
  return true                                            ;
}

//////////////////////////////////////////////////////////////////////////////

qint64 PhysicalMemory(void)
{
  return (qint64)::lzma_physmem ( ) ;
}

//////////////////////////////////////////////////////////////////////////////

QT_END_NAMESPACE
