#include <QJSEngine>
#include <QJSValue>
#include <qdebug.h>
#include <qnetworkinterface.h>
#include <qhostaddress.h>
#include <qhostinfo.h>
#include <qregexp.h>

#include "proxyautoconfig.h"
#include <QtNetwork>

ProxyAutoConfig* ProxyAutoConfig::_instancePtr = 0;
ProxyAutoConfig* ProxyAutoConfig::instance(){
    if(!_instancePtr){
        // warning: no thread safe here.
        _instancePtr = new ProxyAutoConfig();
    }
    return _instancePtr;
}

ProxyAutoConfig::ProxyAutoConfig():QObject(qApp),_isSettup(false)
{
    engine = new QJSEngine(this);
    QJSValue extObj = engine->newQObject(this);
        QStringList initFunctions = {"dnsResolve","isInNet","shExpMatch","myIpAddress","debug"};
        for(int i=0;i<initFunctions.size();++i){
            qDebug()<<initFunctions[i];
            engine->globalObject().setProperty(initFunctions[i],extObj.property(initFunctions[i]));
        }
}

ProxyAutoConfig::~ProxyAutoConfig()
{
}

void ProxyAutoConfig::setConfigByUrl(const QString &url){
    if(currentPacUrl == url)return;
    currentPacUrl = url;
    QNetworkAccessManager manager;
    //qDebug()<<"url"<<url;
    QTimer timer;
    QEventLoop _loop;
    timer.singleShot(10000,&_loop,SLOT(quit()));//5秒内
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    manager.connect(&manager,SIGNAL(finished(QNetworkReply*)),&_loop,SLOT(quit()));
    QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
    _loop.exec();
    QString script = QString(reply->readAll());
    setConfig(script);
}
void ProxyAutoConfig::setHttpProxy(const QString &http){
    httpProxy = http;
}
void ProxyAutoConfig::setHttpsProxy(const QString &https){
    httpsProxy = https;
}

void ProxyAutoConfig::setConfig( const QString &config )
{
    //qDebug()<<config;
    _isSettup = true;
    connect(engine,SIGNAL(signalHandlerException(QJSValue)),SLOT(onException()));
    engine->evaluate( config );
    engine->disconnect(SIGNAL(signalHandlerException(QJSValue)),this,SLOT(onException()));
}



QString ProxyAutoConfig::findProxyForUrl( const QString &url, const QString &host )
{
    if(!_isSettup){
        if(!httpProxy.isEmpty() && url.startsWith("http://")){
            return httpProxy;
        }else if(!httpsProxy.isEmpty() && url.startsWith("https://")){
            return httpsProxy;
        }
        return "DIRECT";
    }
    QMutexLocker locker(&_queryMutex);
    QJSValue global = engine->globalObject();
    QJSValue fun = global.property("FindProxyForURL");
    if ( !fun.isCallable() ) {
        return QString("DIRECT");
    }

    QJSValueList args;
    args << engine->toScriptValue( url ) << engine->toScriptValue( host );

    QJSValue val = fun.callWithInstance(global,args);
    return val.toString();
}

void ProxyAutoConfig::onException(){
    _isSettup = false;
}
