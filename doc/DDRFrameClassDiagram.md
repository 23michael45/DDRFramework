@startuml'/关联
BaseSocketContainer <-- UdpSocketBase
@enduml
@startuml'/关联
BaseSocketContainer <-- TcpSocketContainer
@enduml






'/关联
@startuml
class MessageSerilizer
class TcpSocketContainer
TcpSocketContainer <--> MessageSerilizer
@enduml
@startuml
class MessageSerilizer
class TcpSocketContainer
MessageSerilizer --> BaseMessageDispatcher
@enduml
'/关联
@startuml
class MessageSerilizer_
class UdpSocketBase
UdpSocketBase <--> MessageSerilizer_
@enduml

@startuml'/继承
TcpSocketContainer <|-- TcpSessionBase
@enduml

@startuml
TcpSocketContainer <--> BaseBehavior
@enduml
@startuml'/继承
BaseBehavior <|-- AndroidClientBehavior
@enduml
@startuml'/继承
BaseBehavior <|-- PCClientBehavior
@enduml


'/聚合
@startuml
TcpSessionBase --o TcpServerBase

@enduml


'/继承
@startuml
BaseMessageDispatcher <|-- ServerMessageDispatcher
@enduml

'/聚合
@startuml
BaseMessageDispatcher o-- BaseProcessor
@enduml




'/实现
@startuml
BaseProcessor <|.. LoginProcessor
BaseProcessor <|.. HeartBeatProcessor
@enduml


'/关联
@startuml
BaseProcessor --> BaseBehavior
@enduml
'/实现
@startuml
TcpServerBase --> GlobalManager
@enduml
