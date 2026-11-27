#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>

#include "net.h"

enum {
    PROTOCOL_VERSION = 2, // Exige o mesmo campo de 1280x720 nos dois computadores.
    HEADER_SIZE = 5,
    INPUT_SIZE = HEADER_SIZE + 1,
    STATE_SIZE = HEADER_SIZE + 13,
    MOVE_UP = 1,
    MOVE_DOWN = 2
};

static SOCKET udpSocket = INVALID_SOCKET;
static struct sockaddr_in peer;
static bool isHost;
static bool peerActive;
static double lastReceive;
static uint16_t sendSequence;
static uint16_t receiveSequence;
static uint8_t remoteInput;

bool NetOpen(bool host, const char *hostIp)
{
    WSADATA winsock;
    struct sockaddr_in address = {0};
    u_long nonblocking = 1;

    NetClose();
    if (WSAStartup(MAKEWORD(2, 2), &winsock) != 0)
        return false;

    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (udpSocket == INVALID_SOCKET)
    {
        WSACleanup();
        return false;
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(NET_PORT);
    if (ioctlsocket(udpSocket, FIONBIO, &nonblocking) == SOCKET_ERROR ||
        (host && bind(udpSocket, (const struct sockaddr *)&address,
                      sizeof(address)) == SOCKET_ERROR) ||
        (!host && (hostIp == NULL ||
                   inet_pton(AF_INET, hostIp, &address.sin_addr) != 1)))
    {
        NetClose();
        return false;
    }

    isHost = host;
    peer = address;
    return true;
}

void NetClose(void)
{
    if (udpSocket != INVALID_SOCKET)
    {
        closesocket(udpSocket);
        WSACleanup();
        udpSocket = INVALID_SOCKET;
    }

    peerActive = false;
    sendSequence = 0;
}

static int ReadNumber(const uint8_t *bytes)
{
    return (bytes[0] << 8) | bytes[1];
}

static void WriteNumber(uint8_t *bytes, int value)
{
    bytes[0] = (uint8_t)(value >> 8);
    bytes[1] = (uint8_t)value;
}

static void SendPacket(uint8_t *packet, int size, uint8_t kind)
{
    packet[0] = 'P';
    packet[1] = PROTOCOL_VERSION;
    packet[2] = kind;
    WriteNumber(packet + 3, sendSequence++);

    sendto(udpSocket, (const char *)packet, size, 0,
           (const struct sockaddr *)&peer, sizeof(peer));
}

void NetSendInput(bool up, bool down)
{
    uint8_t packet[INPUT_SIZE];

    if (udpSocket == INVALID_SOCKET || isHost)
        return;

    packet[HEADER_SIZE] = (up ? MOVE_UP : 0) | (down ? MOVE_DOWN : 0);
    SendPacket(packet, sizeof(packet), 'I');
}

void NetSendState(NetState state)
{
    uint8_t packet[STATE_SIZE];
    uint8_t *data = packet + HEADER_SIZE;

    if (!isHost || !peerActive)
        return;

    WriteNumber(data, state.player1Y);
    WriteNumber(data + 2, state.player2Y);
    WriteNumber(data + 4, state.ballX + BALL_RADIUS);
    WriteNumber(data + 6, state.ballY);
    WriteNumber(data + 8, state.score1);
    WriteNumber(data + 10, state.score2);
    data[12] = state.gameOver;
    SendPacket(packet, sizeof(packet), 'S');
}

static bool DecodeState(const uint8_t *data, NetState *state)
{
    const int maxPaddleY = SCREEN_HEIGHT - PADDLE_HEIGHT;
    NetState received = {
        .player1Y = ReadNumber(data),
        .player2Y = ReadNumber(data + 2),
        .ballX = ReadNumber(data + 4) - BALL_RADIUS,
        .ballY = ReadNumber(data + 6),
        .score1 = ReadNumber(data + 8),
        .score2 = ReadNumber(data + 10),
        .gameOver = data[12] != 0
    };

    if (received.player1Y > maxPaddleY || received.player2Y > maxPaddleY ||
        received.ballX < -BALL_RADIUS || received.ballX > SCREEN_WIDTH + BALL_RADIUS ||
        received.ballY < BALL_RADIUS || received.ballY > SCREEN_HEIGHT - BALL_RADIUS ||
        received.score1 > 5 || received.score2 > 5 || data[12] > 1)
        return false;

    *state = received;
    return true;
}

static void ReceivePackets(double now, NetState *state)
{
    int expectedSize = isHost ? INPUT_SIZE : STATE_SIZE;
    uint8_t expectedKind = isHost ? 'I' : 'S';

    if (peerActive && now - lastReceive >= 2.0)
        peerActive = false;

    for (int i = 0; i < 64; ++i)
    {
        uint8_t packet[STATE_SIZE + 1];
        struct sockaddr_in source;
        int sourceSize = sizeof(source);
        int size = recvfrom(udpSocket, (char *)packet, sizeof(packet), 0,
                            (struct sockaddr *)&source, &sourceSize);

        if (size == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error == WSAEMSGSIZE || error == WSAECONNRESET)
                continue;
            break;
        }

        if (size != expectedSize || packet[0] != 'P' ||
            packet[1] != PROTOCOL_VERSION || packet[2] != expectedKind)
            continue;

        if ((!isHost || peerActive) &&
            (source.sin_addr.s_addr != peer.sin_addr.s_addr ||
             source.sin_port != peer.sin_port))
            continue;

        uint16_t sequence = (uint16_t)ReadNumber(packet + 3);
        uint16_t distance = (uint16_t)(sequence - receiveSequence);
        if (peerActive && (distance == 0 || distance >= 32768))
            continue;

        if (isHost)
        {
            if (packet[HEADER_SIZE] > (MOVE_UP | MOVE_DOWN))
                continue;
            remoteInput = packet[HEADER_SIZE];
        }
        else if (!DecodeState(packet + HEADER_SIZE, state))
            continue;

        receiveSequence = sequence;
        peer = source;
        lastReceive = now;
        peerActive = true;
    }
}

bool NetReceiveInput(double now, bool *up, bool *down)
{
    *up = false;
    *down = false;
    if (udpSocket == INVALID_SOCKET || !isHost)
        return false;

    ReceivePackets(now, NULL);
    if (peerActive && now - lastReceive < 0.25)
    {
        *up = (remoteInput & MOVE_UP) != 0;
        *down = (remoteInput & MOVE_DOWN) != 0;
    }
    return peerActive;
}

bool NetReceiveState(double now, NetState *state)
{
    if (udpSocket == INVALID_SOCKET || isHost)
        return false;

    ReceivePackets(now, state);
    return peerActive;
}
