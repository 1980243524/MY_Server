# 环境搭建
## linux开发环境
要求至少c++20标准 

提前安装mysql数据库 

sudo apt install libmysqlclient-dev

# 数据包格式

## 请求包
![alt text](RequireHead.png)

长度 16字节

0-7 表示数据长度 

8-11 表示发送方用户id

12-15 表示接收方用户id 

## 响应包
![alt text](Response.png)

长度 20字节

0-7 表示数据长度 

8-11 表示发送方用户id

12-15 表示接收方用户id 

16-19 表示响应码

# 服务器功能协议
将从1到99 的id保留为服务器使用,用户id从100开始
## Login
### 客户端请求包
SourceId : 1

DestinationId: 1

DataLength: 信息长度

Data: 账号\\密码
### 服务器响应包
#### 登录成功响应包
SourceId : 1

DestinationId: 本用户id

Code: 200

DataLength: 信息长度

Data: 账号\\密码
#### 后续数据响应包
服务器向客户端发送用户数据,包括好友信息等，首先发送好友数量 uint32，之后接受好友id 和name

SourceId : 好友id

DestinationId: 本用户id

Code: 200

DataLength: 信息长度

Data: 账号\\密码
## Signup
### 客户端请求包
SourceId : 2

DestinationId: 2

DataLength: 信息长度

Data: 账号\\密码
### 服务器响应包

SourceId : 2

DestinationId: 2

Code: 200

DataLength: 信息长度

Data: 提示信息
## Echo
### 客户端请求包
SourceId : 本用户id

DestinationId: 3

DataLength: 信息长度

Data: 消息
### 服务器响应包
SourceId : 3

DestinationId: 本用户id

DataLength: 信息长度

Code: 200

Data: 消息

## Public Chat
### 客户端请求包
SourceId : 本用户id

DestinationId: 4

DataLength: 信息长度

Data: 消息
### 服务器响应包
SourceId : 4

DestinationId: 所有用户id

DataLength: 信息长度

Code: 200

Data: 消息

## Private Chat

### 客户端请求包
SourceId : 本用户id

DestinationId: 私聊对象id

DataLength: 信息长度

Data: 消息
### 服务器响应包
SourceId : 本用户id

DestinationId: 私聊对象id

DataLength: 信息长度

Code: 200

Data: 消息


## Make Friend
### 之后使用获取的用户名发送消息
### 客户端请求包
SourceId : 本用户id

DestinationId: 6

DataLength: 信息长度

Data: 消息
### 服务器响应包
SourceId : 6

DestinationId: 本用户id

DataLength: 信息长度

Code: 200

Data: 好友id