#ifndef RYWEBVIEW_H
#define RYWEBVIEW_H

#include <QWebEngineView>

class RyWebView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit RyWebView(QWidget *parent = nullptr);

signals:
    
public slots:
protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dragMoveEvent(QDragMoveEvent *);
    void dropEvent(QDropEvent *);
/*
private:// disable load
    void load(const QNetworkRequest &request, QNetworkAccessManager::Operation operation, const QByteArray &body);
    void load(const QUrl&);
*/
};

#endif // RYWEBVIEW_H
