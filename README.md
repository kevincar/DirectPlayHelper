[![Build Status](https://travis-ci.org/kevincar/DPServer.svg?branch=master)](https://travis-ci.org/kevincar/DPServer)

# Direct Play Server

DirectPlay is a deprecated API that was part of Microsoft's DirectX API.
DirectPlay is a network communication library inteded for computer game
development, although its general nature allows it to be used for other
purposes (From Wikipedia).

DirectPlay communication requires a peer-to-peer connection, subsequently
requiring users to open internet protocol (IP) ports for direct connection and
communication. Users needed to know the public IP addresses of the hosting
party to establsih a connection. While DirectPlay has a NAT helper which
includes a component that takes advantage of Universal Plug-N-Play (UPnP),
common on routers, this was not always reliable to establish communcation.

The purpose of this project is to create a server to which which clients can
connect. Once connected, the server then acts as a network holepunch and relay
service to enable connected clients to discover eachother with out the
requirement to forward IP ports nor to know public IP address of other
clients. The hope is that this project will facilitate the ease to connect
across DirectPlay communcations.
