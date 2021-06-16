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

QT_BEGIN_NAMESPACE

ScriptableXz:: ScriptableXz ( QObject * parent )
             : QObject      (           parent )
             , QScriptable  (                  )
             , QtXz         (                  )
{
}

ScriptableXz::~ScriptableXz (void)
{
}

bool ScriptableXz::ToXz(QString file,QString xz,int level)
{
  return FileToXz ( file , xz , level ) ;
}

bool ScriptableXz::ToFile(QString xz,QString file)
{
  return XzToFile ( xz , file ) ;
}

QScriptValue ScriptableXz::Attachment(QScriptContext * context,QScriptEngine * engine)
{
  ScriptableXz * xz = new ScriptableXz ( engine ) ;
  return engine -> newQObject          ( xz     ) ;
}

QT_END_NAMESPACE
