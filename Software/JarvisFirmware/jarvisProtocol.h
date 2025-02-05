#ifndef JPROTOCOL
#define JPROTOCOL

#define PROTOCOL_VERSION 02

//Separadores de paquete
#define P_PACKETSTART       "["
#define P_PACKETSEPARATOR   "|"
#define P_PACKETTERMINATOR  "]"

//Etiquetas de protocolo
#define M_NODEMSG         F("N")
#define M_JARVISMSG       F("J")
#define M_ESPMSG          F("E")

#define M_NODE_GREETING     F("your wish is my command")
#define M_JARVIS_GREETING   F("obey the overmind")

#define C_GET_PROTOCOL_VERSION  F("gpv")
#define C_PROTOCOL_VERSION      F("pv")

#define M_ESPMSG          F("E")

#define C_GET_FREEM     F("gfm")
#define C_FREEMEM       F("fm")

#define C_SETAP         F("sap")
#define C_SETCLIENT     F("scl")
#define C_WSTATUS       F("ws")
#define C_LOCALIP       F("lip")
#define C_BRIDGEMODE    F("bm")
#define C_RESET         F("rst")
#define C_ENABLE        F("en")
#define C_DISABLE       F("dis")
#define C_PING          F("ping")
#define C_PONG          F("pong")

#define C_GETID         F("gid")
#define C_ID            F("id")

#define C_GETCOMPONENTS F("gcomps")
#define C_COMPONENT     F("comps")

#define C_POLLSENSOR    F("psen")//pregunta
#define C_SENSOR        F("sen")//respuesta

#define C_POLLSENSORS   F("psens")//pregunta
#define C_SENSORS       F("sens")//respuesta

#define C_STOP_POLLING  F("spsens")

#define C_DOACTION      F("da")

#define C_SET_UPDATE_INTERVAL F("sui")

#define E_EVENT       F("ev")
#define E_ACTION      F("act")

//Acciones y eventos
enum jarvisEvents
{
    E_ENABLED,
    E_DISABLED,
    E_ACTIVATED,
    E_DEACTIVATED,
    E_RAW_READ,
    E_DATA_READ,
    E_GLOBAL_POWERON,
    E_GLOBAL_SHUTDOWN,
    E_COFFEE_MAKING,
    E_COFFEE_MADE
};

enum jarvisActions
{
    A_ENABLE,
    A_DISABLE,
    A_ACTIVATE,
    A_DEACTIVATE,
    A_TOGGLE,
    A_READ_RAW,
    A_READ_DATA,
    A_DIMM,
    A_BLINK,
    A_GLOW,
    A_FADE,
    A_SET_COLOR,
    A_CYLON,
    A_SET_LEDS,
    A_SET_LED,
    A_BEEP,
    A_MAKE_COFFEE
};

#endif
