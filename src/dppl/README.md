# DirectPlay Proxy Library

This library of classes is responsible for providing the tools and
functionality to interface with the local DirectPlay application. The
libraries main tools are the `interceptor` and the `proxy`. The `interceptor`,
as the name might suggest, intercepts messages that are released by the
DirectPlay application and assigns `proxy` peer and host connections to handle
communications on behalf of the specific destination target.
