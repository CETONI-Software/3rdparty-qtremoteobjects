/****************************************************************************
**
** Copyright (C) 2017 Ford Motor Company
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtRemoteObjects module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qconnection_local_backend_p.h"

QT_BEGIN_NAMESPACE

LocalClientIo::LocalClientIo(QObject *parent)
    : ClientIoDevice(parent)
    , m_socket(new QLocalSocket(this))
{
    connect(m_socket, &QLocalSocket::readyRead, this, &ClientIoDevice::readyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &LocalClientIo::onError);
    connect(m_socket, &QLocalSocket::stateChanged, this, &LocalClientIo::onStateChanged);
}

LocalClientIo::~LocalClientIo()
{
    close();
}

QIODevice *LocalClientIo::connection() const
{
    return m_socket;
}

void LocalClientIo::doClose()
{
    if (m_socket->isOpen()) {
        connect(m_socket, &QLocalSocket::disconnected, this, &QObject::deleteLater);
        m_socket->disconnectFromServer();
    } else {
        this->deleteLater();
    }
}

void LocalClientIo::doDisconnectFromServer()
{
    m_socket->disconnectFromServer();
}

void LocalClientIo::connectToServer()
{
    if (!isOpen())
        m_socket->connectToServer(url().path());
}

bool LocalClientIo::isOpen() const
{
    return !isClosing() && (m_socket->state() == QLocalSocket::ConnectedState
                            || m_socket->state() == QLocalSocket::ConnectingState);
}

void LocalClientIo::onError(QLocalSocket::LocalSocketError error)
{
    qCDebug(QT_REMOTEOBJECT) << "onError" << error << m_socket->serverName();

    switch (error) {
    case QLocalSocket::ServerNotFoundError:
    case QLocalSocket::UnknownSocketError:
    case QLocalSocket::PeerClosedError:
        //Host not there, wait and try again
        emit shouldReconnect(this);
        break;
    case QLocalSocket::ConnectionError:
    case QLocalSocket::ConnectionRefusedError:
        //... TODO error reporting
#ifdef Q_OS_UNIX
        emit shouldReconnect(this);
#endif
        break;
    default:
        break;
    }
}

void LocalClientIo::onStateChanged(QLocalSocket::LocalSocketState state)
{
    if (state == QLocalSocket::ClosingState && !isClosing()) {
        m_socket->abort();
        emit shouldReconnect(this);
    }
    if (state == QLocalSocket::ConnectedState)
        initializeDataStream();
}

LocalServerIo::LocalServerIo(QLocalSocket *conn, QObject *parent)
    : ServerIoDevice(parent), m_connection(conn)
{
    m_connection->setParent(this);
    connect(conn, &QIODevice::readyRead, this, &ServerIoDevice::readyRead);
    connect(conn, &QLocalSocket::disconnected, this, &ServerIoDevice::disconnected);
}

QIODevice *LocalServerIo::connection() const
{
    return m_connection;
}

void LocalServerIo::doClose()
{
    m_connection->disconnectFromServer();
}

LocalServerImpl::LocalServerImpl(QObject *parent)
    : QConnectionAbstractServer(parent)
{
    connect(&m_server, &QLocalServer::newConnection, this, &QConnectionAbstractServer::newConnection);
}

LocalServerImpl::~LocalServerImpl()
{
    m_server.close();
}

ServerIoDevice *LocalServerImpl::configureNewConnection()
{
    if (!m_server.isListening())
        return nullptr;

    return new LocalServerIo(m_server.nextPendingConnection(), this);
}

bool LocalServerImpl::hasPendingConnections() const
{
    return m_server.hasPendingConnections();
}

QUrl LocalServerImpl::address() const
{
    QUrl result;
    result.setPath(m_server.serverName());
    result.setScheme(QRemoteObjectStringLiterals::local());

    return result;
}

bool LocalServerImpl::listen(const QUrl &address)
{
#ifdef Q_OS_UNIX
    bool res = m_server.listen(address.path());
    if (!res) {
        QLocalServer::removeServer(address.path());
        res = m_server.listen(address.path());
    }
    return res;
#else
    return m_server.listen(address.path());
#endif
}

QAbstractSocket::SocketError LocalServerImpl::serverError() const
{
    return m_server.serverError();
}

void LocalServerImpl::close()
{
    m_server.close();
}

QT_END_NAMESPACE
