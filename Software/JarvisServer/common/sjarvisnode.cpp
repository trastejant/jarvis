#include "sjarvisnode.h"

//hack para eliminar el F() que hay en el protocolo para que se salve en la rom del arduino
#define F(v) v

sJarvisNode::sJarvisNode(QObject* parent) : QObject(parent)
{
    connect(&m_tcpClient,SIGNAL(tx()),this,SIGNAL(tx()));
    connect(&m_tcpClient,SIGNAL(rx()),this,SIGNAL(rx()));
    connect(&m_pingTimer,SIGNAL(timeout()),this,SLOT(ping()));
    connect(&m_initTimer,SIGNAL(timeout()),this,SLOT(initDone()));
    connect(&m_initTimeout,SIGNAL(timeout()),this,SLOT(initTimeout()));
//Slots del nodo tcp
    //connect(&m_tcpClient,SIGNAL(socket_rx(QByteArray)),this,SLOT(data_rx(QByteArray)));
    connect(this,SIGNAL(writeData(QByteArray)),&m_tcpClient,SLOT(socket_tx(QByteArray)));
    connect(&m_tcpClient,SIGNAL(connected()),this,SLOT(initNode()));
    connect(&m_tcpClient,SIGNAL(disconnected()),this,SLOT(socketDisconected()));

    m_pingTimer.setInterval(10000);
}


void sJarvisNode::connectTCP(QString host, quint16 port)
{
    m_tcpClient.connectToHost(host,port);
}

void sJarvisNode::closeTCP()
{
    m_tcpClient.close();
    m_pingTimer.stop();
    deleteComponents();
}

//protected:

void sJarvisNode::parseBuffer(QString& buf)
{
    //qDebug() << "BufferStart:" << buf;
    //qDebug() << "-PacketSeparators:" << P_PACKETSTART << P_PACKETSEPARATOR << P_PACKETTERMINATOR;
    if(buf.length() == 0) return;
    int s_index = buf.indexOf(P_PACKETSTART);
    int e_index = buf.indexOf(P_PACKETTERMINATOR);
    //saneado del buffer
    //qDebug() << "indexs:" << s_index << "indexe" << e_index;
    if(s_index < 0)
    {// si no hay inicio de paquete lo que hay en el buffer tiene que ser basura.
        //qDebug() << "Limpiando Buffer";
        buf.clear();
        return;
    }
    //extraccion de comandos
    while ((s_index >= 0) && (e_index >= 0)) //Si hay inicio y fin de paquete se extrae el comando.
    {// lo que haya en el buffer hasta el inicio de paquete se descarta(basura)

        QString packet = buf.mid(s_index+1,e_index-s_index-1);
        parsePacket(packet);
        buf = buf.mid(e_index+1);
        s_index = buf.indexOf(P_PACKETSTART);
        e_index = buf.indexOf(P_PACKETTERMINATOR);
    }
    //qDebug() << "Buffer:End" << m_rxBuffer;
}

void sJarvisNode::parsePacket(QString& packet)
{
//    qDebug() << "Packet:" << packet;
    QStringList args;
    args = packet.split(P_PACKETSEPARATOR);
    if(args.count() < 2) return;
//    qDebug() << "Packet:" << args;
    if (args[0] != M_JARVISMSG) return;
    args.removeFirst();
    QString arg = args[0];
    args.removeFirst();

    if      (arg == C_ID)
    {
        if(args.count() == 1)
            m_id = args[0];

    }else if(arg == C_PONG)
    {
        pong();
    }else if(arg == C_COMPONENT)
    {
        parseComponent(args);
//    }else if(arg == C_SENSOR)
//    {
//        parseSensor(args);

    }else if(arg == C_SENSORS)
    {
        parseSensors(args);

    }else if(arg == E_EVENT)
    {
        parseEvent(args);
    }else
    {
        qDebug() << "[sJarvisNode::parsePacket]daFuckIsThis:"<< args;
    }
}

void sJarvisNode::parseComponent(QStringList args)
{
    if(m_initDone) return;
    m_initTimer.start(200);
    m_components.append(new sJarvisNodeComponent(this,args));
    connect(this,SIGNAL(incomingEvent(QString,jarvisEvents,QStringList)),m_components.last(),SLOT(parseEvent(QString,jarvisEvents,QStringList)));
    emit newComponent(m_components.last());
}

//void sJarvisNode::parseSensor(QStringList args)
//{
//
//}

void sJarvisNode::parseSensors(QStringList args)
{
    if(args.count() < 2) return;
    QVector<QString>  fields;
    QVector<double>    data;
    for(int i = 0 ; i < args.count()-1 ; i++)
    {
        fields.append(args[i]);
        i++;
        data.append(args[i].toDouble());
    }
    emit sensorReads(fields,data);
}

void sJarvisNode::parseEvent(QStringList args)
{
    if(!args.count())return;
    QString component = args.first();
    args.removeFirst();
    if(!args.count())return;
    jarvisEvents event = jarvisEvents(args.first().toInt());
    args.removeFirst();
    emit incomingEvent(component,event,args);
}


//repuestas para el protocolo:

QByteArray sJarvisNode::encodeEspMsg(QStringList args)
{
    QByteArray result;
    if(args.isEmpty()) return result;
    result += P_PACKETSTART;
    result += M_ESPMSG;
    result += P_PACKETSEPARATOR;
    result += args[0];
    for (int i = 1 ; i < args.size() ; i++)
    {
        result += P_PACKETSEPARATOR;
        result += args[i];
    }
    result += P_PACKETTERMINATOR;
    return result;
}

QByteArray sJarvisNode::encodeNodeMsg(QStringList args)
{
    QByteArray result;
    if(args.isEmpty()) return result;
    result += P_PACKETSTART;
    result += M_NODEMSG;
    result += P_PACKETSEPARATOR;
    result += args[0];
    for (int i = 1 ; i < args.size() ; i++)
    {
        result += P_PACKETSEPARATOR;
        result += args[i];
    }
    result += P_PACKETTERMINATOR;
    return result;
}
void sJarvisNode::sendPing()
{
    send(encodeNodeMsg(QStringList(QString(C_PING))));//formula abreviada para crear una lista de argumntos con un unico elemento y mandarlo
}

void sJarvisNode::sendGetID()
{
    send(encodeNodeMsg(QStringList(QString(C_GETID))));
}

void sJarvisNode::sendGetComponents()
{
    deleteComponents();
    m_initDone = false;
    send(encodeNodeMsg(QStringList(QString(C_GETCOMPONENTS))));
}

void sJarvisNode::sendPollSensor(QString sen, int interval)
{
    QStringList args;
    args.append(C_POLLSENSOR);
    args.append(sen);
    if(interval)
        args.append(QString(interval));
    send(encodeNodeMsg(args));
}

void sJarvisNode::sendPollSensors(int interval)
{
    QStringList args;
    args.append(C_POLLSENSORS);
    if(interval >= 0)
        args.append(QString::number(interval,'f',0));
    send(encodeNodeMsg(args));
}

void sJarvisNode::sendStopPolling()
{
    send(encodeNodeMsg(QStringList(QString(C_STOP_POLLING))));
}

void sJarvisNode::sendDoAction(QString componentId,jarvisActions action, QStringList arguments)
{
    QStringList args;
    args.append(C_DOACTION);
    args.append(componentId);
    args.append(QString::number(action));
    args.append(arguments);
    send(encodeNodeMsg(args));
}


//protected Slots:
void sJarvisNode::data_rx(QByteArray data)
{
    m_rxBuffer.append(data);
    m_commLog.append(data);
    parseBuffer(m_rxBuffer);
    emit rawInput(data);
}

void sJarvisNode::initNode()
{
    m_initDone =false;
    connect(&m_tcpClient,SIGNAL(socket_rx(QByteArray)),this,SLOT(validateClient(QByteArray)));
    send(encodeNodeMsg(QStringList(QString(M_JARVIS_GREETING))));
    // le damos un segundo para que reciba de vuelta la informacion, al cumplir el segundo se llama a la funcion initDone()
    m_initTimeout.start(5000);
}


void sJarvisNode::validateClient(QByteArray data)
{
    QString greet;
    greet += P_PACKETSTART ;
    greet += M_JARVISMSG;
    greet += P_PACKETSEPARATOR;
    greet += M_NODE_GREETING;
    greet += P_PACKETTERMINATOR;
    qDebug() << data << "/" << greet  ;
    if(data == greet)
    {
        disconnect(&m_tcpClient,SIGNAL(socket_rx(QByteArray)),this,SLOT(validateClient(QByteArray)));
        connect(&m_tcpClient,SIGNAL(socket_rx(QByteArray)),this,SLOT(data_rx(QByteArray)));
    }
    else
        closeTCP();
}

void sJarvisNode::ping()
{
    sendPing();
    qDebug() << "KeepATimer:" <<m_keepAliveTimer.elapsed();
    if(m_keepAliveTimer.elapsed() > 25000)
    {
        emit rawInput("Timeout, Disconnected!");
        this->closeTCP();
    }
}

void sJarvisNode::pong()
{
    m_keepAliveTimer.restart();
    qDebug() << ":" <<m_keepAliveTimer.elapsed();

}

void sJarvisNode::initDone()
{

    m_initTimer.stop();
    m_initTimeout.stop();

    if(m_id.isEmpty() || m_components.isEmpty())
    {
        qDebug() << "Incompatible client or some problem on the init stage!";
        m_valid = false;
    }
    else
    {
        m_valid = true;
    }

    m_keepAliveTimer.start();
    ping();
    m_pingTimer.start();
    m_initDone = true;
    emit ready();
}

void sJarvisNode::initTimeout()
{
    closeTCP();
    m_initTimeout.stop();
}

void sJarvisNode::socketDisconected()
{
    disconnect(&m_tcpClient,SIGNAL(socket_rx(QByteArray)),this,SLOT(data_rx(QByteArray)));
    m_pingTimer.stop();
    m_initTimer.stop();
    deleteComponents();
    emit disconnected();
}


void sJarvisNode::deleteComponents()
{
    for(int i = 0 ; i < m_components.count() ; i++)
    {
        m_components[i]->deleteLater();
    }
    m_components.clear();
}

void sJarvisNode::setUpdateInterval(int interval)
{
    QStringList args;
    args.append(C_SET_UPDATE_INTERVAL);
    args.append(QString::number(interval));
    send(encodeNodeMsg(args));
}

//public Slots

void sJarvisNode::doAction(QString Component, jarvisActions action, QStringList args)
{
    sendDoAction(Component,action,args);
}

void sJarvisNode::pollSensor(QString sen, int interval)
{
    if(sen == "ALL")
        sendPollSensors(interval);
    else
        sendPollSensor(sen,interval);
}

void sJarvisNode::stopPollingSensors()
{
    sendStopPolling();
}

void sJarvisNode::resetNode()
{
    QStringList args;
    args.append(C_RESET);
    send(encodeNodeMsg(args));
}
