#include "ryproxyserver.h"

RyProxyServer* RyProxyServer::_instance = 0;
RyProxyServer* RyProxyServer::instance(){
    if(!_instance){
        _instance = new RyProxyServer();
    }
    return _instance;
}

RyProxyServer::RyProxyServer() :
    QTcpServer(qApp){
    //qDebug()<<"server initialing";
    _lastPipeId = 0;
    _lastConnectionId = 0;
    isStoping = false;
    setMaxSocket(30);
}
RyProxyServer::~RyProxyServer(){
    //this->blockSignals(true);
    qDebug()<<"~RyProxyServer begin";
    close();
    qDebug()<<"~RyProxyServer done";
}

void RyProxyServer::close(){
    QTcpServer::close();
    QMutexLocker locker(&connectionOpMutex);
    Q_UNUSED(locker)
    isStoping = true;
    // close all sockets
    for(int i=0,l=_connections.length();i<l;++i){
        //qDebug()<<"delete connection"<<l<<i;
        RyConnection *connection = _connections.takeFirst();
        QThread *thread = _threads.take(connection);
        connection->disconnect(this);
        //_cacheConnections.remove(connection);
        QMetaObject::invokeMethod(thread,"quit");
        connection->deleteLater();
    }
    for(int i=0,l=_cachedSockets.values().size();i<l;++i){
        QTcpSocket *socket = _cachedSockets.values().takeFirst();
        socket->deleteLater();
    }
    _connections.clear();
    this->blockSignals(true);
    qDebug()<<"proxy server closed..";
}

int RyProxyServer::maxOfSocket(){
    return _maxOfSocket;
}

void RyProxyServer::setMaxSocket(int max){
    _maxOfSocket = max;
}

quint64 RyProxyServer::nextPipeId(){
    return _lastPipeId++;
}


void RyProxyServer::cacheSocket(QString address,quint16 port,QTcpSocket* socket){
    qDebug()<<"CACHE SOCKET cacheSocket"<<address<<port;
    // TODO the key QString("%1:%2").arg(address.toLower()).arg(port) should be a function or macro
    socket->moveToThread(this->thread());
    QString key = QString("%1:%2").arg(address.toLower()).arg(port);
    QMutexLocker locker(&_socketsOpMutex);
    Q_UNUSED(locker)
    _cachedSockets.insert(key ,socket);
}
QTcpSocket* RyProxyServer::getSocket(QString address,quint16 port,bool* isFromCache,QThread* _desThread){

    //QDateTime time = QDateTime::currentDateTime();
    //qDebug()<<"getSocket "<<time.toMSecsSinceEpoch();
    if(isFromCache) *isFromCache = false;
    QTcpSocket *theSocket = NULL;
    QString theKey = QString("%1:%2").arg(address.toLower()).arg(port);


    QMutexLocker locker(&_socketsOpMutex);
    if(/*_cachedSockets.size() >= _maxOfSocket
            || */_cachedSockets.contains( theKey )){
        //QList<QTcpSocket*> sockets = _cachedSockets.values(theKey);
        theSocket = _cachedSockets.take(theKey);
        locker.unlock();
        if(isFromCache) *isFromCache = true;
        //qDebug()<<"return cached socket";
        //int n = _cachedSockets.remove(theKey,theSocket);

    }else{
        locker.unlock();
        //qDebug()<<"new response socket";
        theSocket = new QTcpSocket();
    }
    theSocket->moveToThread(_desThread);
    //qint64 getSocketDatTime = time.msecsTo(QDateTime::currentDateTime());
    //qDebug()<<"== get socket time(ms)"<<getSocketDatTime<<" "<<time.toMSecsSinceEpoch();
    return theSocket;
}

// connection request comming
//   generate an RyConnection
//   and use handle identify it.
//
//   when new connection come and has the same handle.
//   reuse old RyConnection

#if QT_VERSION >= 0x050000
void RyProxyServer::incomingConnection(qintptr handle){
#else
void RyProxyServer::incomingConnection(qintptr handle){
#endif
    qDebug()<<"incomingConnection";
    if(isStoping){
        qDebug()<<"incommingConnection "<<isStoping;
        return;
    }
    RyConnection* connection = _getConnection(handle);
    Q_UNUSED(connection)
}
// private functions
RyConnection * RyProxyServer::_getConnection(int handle){
    //QMutexLocker locker(&connectionOpMutex);
    //Q_UNUSED(locker)
    //QDateTime time = QDateTime::currentDateTime();
    //qDebug()<<"getConnection:"
    //          <<time.toMSecsSinceEpoch();
    //if(!_cacheConnections.contains(handle)){
    //qDebug()<<"_lastConnectionId"<<_lastConnectionId;
    _lastConnectionId++;

    QMutexLocker locker(&connectionOpMutex);
    QThread *newThread = new QThread();
    RyConnection *connection = new RyConnection(handle,_lastConnectionId);
    _connections.append(connection);
    _threads[connection] = newThread;
    locker.unlock();

    connect(connection,SIGNAL(idleTimeout()),SLOT(onConnectionIdleTimeout()));

    connect(connection,SIGNAL(pipeBegin(RyPipeData_ptr)),
            SLOT(onPipeBegin(RyPipeData_ptr)));
    connect(connection,SIGNAL(pipeComplete(RyPipeData_ptr)),
            SLOT(onPipeComplete(RyPipeData_ptr)));
    connect(connection,SIGNAL(pipeError(RyPipeData_ptr)),
            SLOT(onPipeError(RyPipeData_ptr)));

    connect(connection,SIGNAL(connectionClose()),
            SLOT(onConnectionClosed()));

    connect(connection,SIGNAL(pipeBegin(RyPipeData_ptr)),
            SIGNAL(pipeBegin(RyPipeData_ptr)));
    connect(connection,SIGNAL(pipeComplete(RyPipeData_ptr)),
            SIGNAL(pipeComplete(RyPipeData_ptr)));
    connect(connection,SIGNAL(pipeError(RyPipeData_ptr)),
            SIGNAL(pipeError(RyPipeData_ptr)));


    connection->moveToThread(newThread);
    connect(newThread,SIGNAL(started()),connection,SLOT(run()));
    connect(newThread,SIGNAL(finished()),SLOT(onThreadTerminated()));
    newThread->start();

    /*
    qDebug()<<"=== create connection cost:"
           <<time.msecsTo(QDateTime::currentDateTime())
           <<time.toMSecsSinceEpoch();
    */
    return connection;
}


// private slots:
void RyProxyServer::onConnectionIdleTimeout(){
    RyConnection *connection = qobject_cast<RyConnection*>(sender());
    _connections.removeOne(connection);
    //connection->disconnect(this);
    connection->deleteLater();
}


void RyProxyServer::onPipeBegin(RyPipeData_ptr){
    //qDebug()<<"pipe begin";
}

void RyProxyServer::onPipeComplete(RyPipeData_ptr){
    //qDebug()<<"pipe complete";
}

void RyProxyServer::onPipeError(RyPipeData_ptr){
    //qDebug()<<"pipe error";
}


void RyProxyServer::onThreadTerminated(){
    //qDebug()<<"thread terminated";
    QThread *t = (QThread*)sender();
    if(t){
        t->deleteLater();
    }
}

void RyProxyServer::onConnectionClosed(){
    //qDebug()<<"connection closed";
    RyConnection *connection = qobject_cast<RyConnection*>(sender());
    if(!connection){
        return;
    }

    _connections.removeOne(connection);
    QThread *thread = _threads.take(connection);
    connection->disconnect(this);
    QMetaObject::invokeMethod(thread,"quit");
    connection->deleteLater();
}
