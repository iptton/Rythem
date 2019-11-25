#ifndef QNETWORKPROXYFACTORYEXENDFORPAC_H
#define QNETWORKPROXYFACTORYEXENDFORPAC_H

#include <QNetworkProxyFactory>
#include <QJSValue>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QEventLoop>

class QNetworkProxyFactoryExendForPAC : public QObject
{


    Q_OBJECT

public:
    QNetworkProxyFactoryExendForPAC();
    ~QNetworkProxyFactoryExendForPAC();

    /**
     * Call this to set the script to be executed. Note that the argument should be
     * the content of the .pac file to be used, not the URL where it is located.
     */
    void setConfig(QString config );

    /**
     * Returns the result
     */
    Q_INVOKABLE QString findProxyForUrl( const QString &url, const QString &host ){}

private:

    QString getContentByUrl(QString &url) {
        qDebug()<<"url"<<url;
        if(!QUrl(url).isValid()){
            return "";
        }

        QNetworkAccessManager manager;
        QTimer timer;
        QEventLoop _loop;
        timer.singleShot(10000,&_loop,SLOT(quit()));//5秒内
        QNetworkProxyFactory::setUseSystemConfiguration(true);
        manager.connect(&manager,SIGNAL(finished(QNetworkReply*)),&_loop,SLOT(quit()));
        QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
        _loop.exec();
        QString script = QString(reply->readAll());
        return script;
    }

    Q_INVOKABLE QJSValue dnsResolve(QString name){
        QHostInfo info = QHostInfo::fromName( name );
        QList<QHostAddress> addresses = info.addresses();
        if ( addresses.isEmpty() )
            return QJSValue::NullValue; // TODO: Should this be undefined or an exception? check other implementations

        return addresses.first().toString();
    }

    Q_INVOKABLE QJSValue debug(QString str){
        qDebug()<<str;
        return QJSValue::UndefinedValue;
    }

    Q_INVOKABLE QJSValue myIpAddress(){// myIpAddress  有多个值 ，如何取？
        QString result="";
        foreach( QHostAddress address, QNetworkInterface::allAddresses() ) {
                if ( address != QHostAddress::LocalHost
                     && address != QHostAddress::LocalHostIPv6 )
                    // how to remove fe80::1%lo0 ?
//                    qDebug()<<address;
                    result =  address.toString();
        }
        return result;
//        return QJSValue::UndefinedValue;
    }

    Q_INVOKABLE QJSValue isInNet(QString _addr,QString _netAddress,QString _netMask){

        QHostAddress addr( _addr );
        QHostAddress netaddr( _netAddress );
        QHostAddress netmask( _netMask);

        if ( (netaddr.toIPv4Address() & netmask.toIPv4Address()) == (addr.toIPv4Address() & netmask.toIPv4Address()) )
            return true;

        return false;
    }

    Q_INVOKABLE QJSValue shExpMatch(QString src,QString pattern){
//        qDebug()<<src<<pattern;
        QRegExp re( pattern, Qt::CaseInsensitive, QRegExp::Wildcard );
        return re.exactMatch( src);
    }

private:
    QJSEngine *engine;

};


#endif // QNETWORKPROXYFACTORYEXENDFORPAC_H
