#include "qnetworkproxyfactoryexendforpac.h"
#include <QJSEngine>
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QRegExp>


QNetworkProxyFactoryExendForPAC::QNetworkProxyFactoryExendForPAC()
{
    engine = new QJSEngine(this);
}

QNetworkProxyFactoryExendForPAC::~QNetworkProxyFactoryExendForPAC()
{
}


void QNetworkProxyFactoryExendForPAC::setConfig(QString backPACUrl)
{
    QJSValue extObj = engine->newQObject(this);
        QStringList initFunctions = {"dnsResolve","isInNet","shExpMatch","myIpAddress","debug"};
        for(int i=0;i<initFunctions.size();++i){
            qDebug()<<initFunctions[i];
            engine->globalObject().setProperty(initFunctions[i],extObj.property(initFunctions[i]));
        }

        QString backScript = getContentByUrl(backPACUrl);
        engine->evaluate(backScript);
}


