# 作业09A——TCP聊天室

胡益铭

### 主要内容

1. 单线程的I/O多路复用
   - server端采用了select完成了单线程的I/O多路复用，通过维护包含多个socket 文件描述符的fd_set实现对多客户端连接的同时监听。
   - client端采用了epoll进行类似的功能实现(写client端的时候才知道除了select还有epoll。。)，event的概念更清晰一些，从实现上好像也比select要快一些。
2. 用户上下线与昵称设置
  - 对于client端，在启动后连接前会要求用户输入昵称，随后在连接成功后的第一次消息中将昵称发送给server并由server记录保存，client下线后昵称不再保留。
  - 当server端收到来自client的连接以及该client的昵称后会将该client记为在线状态并存入一个在线client的列表当中，之后每次收到消息时会根据该消息的socket文件描述符到该表中检索对应的client信息。
3. 消息发送
   - **Chatroom状态信息**，每次有新的用户上线或下线，server将通知所有在线的用户。
   - **聊天信息**。当server收到来自某用户的新消息时，会根据对应的socket文件描述符来寻找对应的client编号及昵称。并将昵称作为消息头加到该消息的前面，随后向所有在线用户进行broadcast。

