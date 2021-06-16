/****************************************************************************
 *
 * Copyright (C) 2001~2016 Neutrino International Inc.
 *
 * Author   : Brian Lin ( Foxman , Vladimir Lin , Vladimir Forest )
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

#ifndef QT_XZ_H
#define QT_XZ_H

#include <QtCore>
#include <QtScript>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#    if defined(QT_BUILD_QTXZ_LIB)
#      define Q_XZ_EXPORT Q_DECL_EXPORT
#    else
#      define Q_XZ_EXPORT Q_DECL_IMPORT
#    endif
#else
#      define Q_XZ_EXPORT
#endif

#define QT_XZ_LIB 1

class Q_XZ_EXPORT QtXz         ;
class Q_XZ_EXPORT ScriptableXz ;

class Q_XZ_EXPORT QtXz
{
  public:

    explicit     QtXz               (void) ;
    virtual     ~QtXz               (void) ;

    static  QString Version         (void) ;

    virtual bool isXz               (QByteArray & header) ;

    virtual void CleanUp            (void) ;

    virtual bool IsCorrect          (int returnCode) ;
    virtual bool IsEnd              (int returnCode) ;
    virtual bool IsFault            (int returnCode) ;

    // Compression functions

    virtual int  BeginCompress      (int level = 9) ;
    virtual int  BeginCompress      (QVariantList arguments = QVariantList() ) ;
    virtual int  doCompress         (const QByteArray & Source      ,
                                           QByteArray & Compressed) ;
    virtual int  doSection          (      QByteArray & Source      ,
                                           QByteArray & Compressed) ;
    virtual int  CompressDone       (QByteArray & Compressed) ;

    // Decompression functions

    virtual int  BeginDecompress    (void) ;
    virtual int  doDecompress       (const QByteArray & Source        ,
                                           QByteArray & Decompressed) ;
    virtual int  undoSection        (      QByteArray & Source        ,
                                           QByteArray & Decompressed) ;
    virtual int  DecompressDone     (void) ;

    virtual bool IsTail             (QByteArray & header) ;

  protected:

    void * xzPacket ;

  private:

} ;

class Q_XZ_EXPORT ScriptableXz : public QObject
                               , public QScriptable
                               , public QtXz
{
  Q_OBJECT
  public:

    static QScriptValue Attachment   (QScriptContext * context,QScriptEngine * engine) ;

    explicit            ScriptableXz (QObject * parent) ;
    virtual            ~ScriptableXz (void) ;

  protected:

  private:

  public slots:

    virtual bool        ToXz         (QString file,QString ucl,int level = 9) ;
    virtual bool        ToFile       (QString ucl,QString file) ;

  protected slots:

  private slots:

  signals:

} ;

Q_XZ_EXPORT QByteArray CompressXz     (const QByteArray & data        ,
                                       int                level     ) ;
Q_XZ_EXPORT QByteArray UncompressXz   (const QByteArray & data      ) ;
Q_XZ_EXPORT bool       ToXz           (const QByteArray & data        ,
                                             QByteArray & xz          ,
                                       int                level = 9 ) ;
Q_XZ_EXPORT bool       FromXz         (const QByteArray & xz          ,
                                             QByteArray & data      ) ;
Q_XZ_EXPORT bool       SaveXz         (QString            filename    ,
                                       QByteArray       & data        ,
                                       int                level = 9 ) ;
Q_XZ_EXPORT bool       LoadXz         (QString            filename    ,
                                       QByteArray       & data      ) ;
Q_XZ_EXPORT bool       FileToXz       (QString            file        ,
                                       QString            xz          ,
                                       int                level = 9 ) ;
Q_XZ_EXPORT bool       XzToFile       (QString            xz          ,
                                       QString            file      ) ;
Q_XZ_EXPORT qint64     PhysicalMemory (void) ;

QT_END_NAMESPACE

#endif
