#include"../head/chatserver.h"


void ChatServer::Init(const std::string& Severip,const int& Serverport,const int& threadNum,
    const std::string DBUser, const std::string DBPassWord,const std::string DBName,const int DBPort, const unsigned int DBConnNum){
    m_ip=Severip;
    m_port=Serverport;
    m_threadPool=std::make_shared<ThreadPool>(threadNum);
    m_dbPool=DbPool::getinstance();
    m_dbPool->init(Severip,DBUser,DBPassWord,DBName,DBPort,DBConnNum);
}

void ChatServer::addfd(int fd, bool oneshot ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN|EPOLLRDHUP;
    if ( oneshot ) {
        event.events |= EPOLLONESHOT;
    }
    int ret=epoll_ctl( m_eopllfd, EPOLL_CTL_ADD, fd, &event );
}

void ChatServer::reSetfd(int fd, bool oneshot ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN|EPOLLRDHUP;
    if ( oneshot ) {
        event.events |= EPOLLONESHOT;
    }
    int ret=epoll_ctl( m_eopllfd, EPOLL_CTL_MOD, fd, &event );
}

void ChatServer::Start(){
    Listen();

    //定义事件

    epoll_event tep;
    tep.events = EPOLLIN;
    tep.data.fd = m_listenfd;

    epoll_event events[MAX_EVENT_NUMBER];
    m_eopllfd = epoll_create(10);
    if(m_eopllfd<0) throw std::logic_error("failed to create epoll");

    int ret = epoll_ctl(m_eopllfd, EPOLL_CTL_ADD, m_listenfd, &tep);
    if(ret<0) throw std::logic_error("failed to add socket to epoll");

    while (1) {
        int nready = epoll_wait(m_eopllfd, events, MAX_EVENT_NUMBER, -1);
        //assert(nready >= 0);
        if(nready<0) std::cout<<errno<<std::endl;
        for (int i = 0; i < nready; i++) {
            int sockfd=events[i].data.fd;
            if (sockfd == m_listenfd) {
                UserOnline();
            }else if (events[i].events & EPOLLRDHUP) {
                UserDownline(sockfd);
            } else if(events[i].events & EPOLLIN){
                m_threadPool->addTask(&ChatServer::ProcessRequire,this,sockfd);
            }

        }
    }
}

void ChatServer::Listen(){
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(m_listenfd<0) throw std::logic_error("failed to get socket");

    int ret = 0;
    sockaddr_in address;

    //定义套接字
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr *) &address, sizeof(address));
    if(ret<0) throw std::logic_error(std::format("failed to bind socket port:{}",m_port));
    ret = listen(m_listenfd, 5);
    if(ret<0) throw std::logic_error("failed to listen");
}


//处理客户端发出的请求
void ChatServer::ProcessRequire(void* arg,int connfd){
    ChatServer* server=(ChatServer*)arg;
    {
    std::lock_guard lock(server->m_mutex);
    server->m_users[connfd].m_timer->Reset(600);
    }
    Require require;
    int ret=server->ParseRequire(connfd,require);
    if(ret<0) return;
    ret=server->Service(connfd,require);
    server->reSetfd(connfd,true);
}

int ChatServer::ParseHead(int connfd,Require::Head& head){
    int ret=recv(connfd,&head,Require::HEADLEN,0);
    if(ret<0) {
        std::cout<<errno<<std::endl;
        return -1;
    }

    return 0;
}

//解析请求对应的操作
int ChatServer::ParseRequire(int connfd,Require& require) {

    int ret=ParseHead(connfd,require.m_head);
    require.m_data.resize(require.m_head.Length);
    if(ret<0) return -1;
    std::vector<std::byte> buffer(require.m_head.Length);

    size_t received = 0;
    while (received < require.m_head.Length) {
        ssize_t ret = recv(connfd, buffer.data()+received, require.m_head.Length - received, 0);
        if (ret <= 0) {
            // 处理错误（ret == 0：连接关闭；ret < 0：错误）
            return -1;
        }
        received += ret;
    }
    // 反序列化 std::string
    std::memcpy(require.m_data.data(),buffer.data(),require.m_head.Length);
    //require.m_data.assign(reinterpret_cast<const char*>(buffer.data()), require.m_head.Length);
    if(received<0) return -1;


    return 0;
}

int ChatServer::Service(int connfd,const Require& require){
    if(require.m_head.DestinationId<100){
        switch (require.m_head.DestinationId){
            case 1:
                LogIn(connfd,require);
                break;
            case 2:         
                SignUp(connfd,require);
                break;
            case 3:
                Echo(connfd,require);
                break;
            case 4:
                PublicChat(connfd,require);
                break;
            case 5:
                PrivateChat(connfd,require);
                break;
            case 6:
                MakeFriend(connfd,require);
                break;
        }
        return 0;
    }
    //私聊处理
    PrivateChat(connfd,require);
    return 0;
}



int ChatServer::UserOnline(){
    struct sockaddr_in client;
    socklen_t client_length = sizeof( client );
    int conn_fd = accept( m_listenfd, ( struct sockaddr * )&client,&client_length );
    if(conn_fd<0){
        std::cout<<errno<<std::endl;
        return -1;
    }

    {    
        std::lock_guard lock(m_mutex);
        m_users[conn_fd]=User(std::format("user{}",conn_fd),conn_fd);
        std::cout<<std::string("user")+std::to_string(conn_fd)+"online"<<std::endl;
        m_users[conn_fd].m_timer->SetEvent(&ChatServer::UserDownline,this,conn_fd);
        m_users[conn_fd].m_timer->Start(300);
    }
    //对每个非监听文件描述符都注册 EPOLLONESHOT 事件
    addfd(conn_fd, true );
    return 0;
}

int ChatServer::UserDownline(int connfd){

    int ret=close(connfd);
    if(ret<0) {
        std::cout<<errno<<std::endl;
    }
    ret=epoll_ctl(m_eopllfd,EPOLL_CTL_DEL,connfd,nullptr);

 
    std::lock_guard lock(m_mutex);
    std::cout<<m_users[connfd].m_name+"downline"<<std::endl;
    m_id_fdMap.erase(m_users[connfd].id);
    m_users.erase(connfd);

    return 0;
}



int ChatServer::LogIn(int connfd,const Require& require){
    auto mid =require.m_data.find('\\');
    std::string account=require.m_data.substr(0,mid);
    std::string passwd=require.m_data.substr(mid+1);

    MysqlRAII mysql_conn(m_dbPool);

    std::shared_ptr<SqlResult> result;

    std::string sql=std::format("select * from user where username='{}'",account);
    int ret=mysql_conn.getConn().Select(std::move(sql),result);

    if(ret<0){
        std::cout<<"查询语句执行失败"<<std::endl;
        Response response(1,1,500,"unknow error\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return -1;
    }
    if(result->getNum()<1){
        Response response(1,1,500,"user not exist\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return 0;
    }

    auto userinfo=result->getRows(0,1)[0];

    if(userinfo["passwd"]!=passwd){
        Response response(1,1,500,"password is uncorrect\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return 0;
    }
    uint32_t curid=std::stoi(userinfo["id"]);


    Response response(1,curid ,200,"successfully login\n");
    send(connfd,Serialize(response).data(),response.size(),0);

    sql=std::format("select relation.user2_id as id, user.username as name \
        from relation inner join user  on user.id=relation.user2_id where user1_id='{}';",curid);
    mysql_conn.getConn().Select(std::move(sql),result);
    auto users=result->getRows();
    uint32_t n=result->getNum();
    send(connfd,&n,sizeof(uint32_t),0);
    for(auto user:users){
        Response response(std::stoi(user["id"]),curid ,200,user["name"]);
        send(connfd,Serialize(response).data(),response.size(),0);
    }

    {
        std::lock_guard lock(m_mutex);
        m_users[connfd].id=curid;
        m_users[connfd].m_name=userinfo["username"];
        m_id_fdMap[m_users[connfd].id]=connfd;
    }
    return 0;
}

int ChatServer::SignUp(int connfd,const Require& require){

    auto mid =require.m_data.find('\\');
    std::string account=require.m_data.substr(0,mid);
    std::string passwd=require.m_data.substr(mid+1);

    MysqlRAII mysql_conn(m_dbPool);

    std::shared_ptr<SqlResult> result;

    std::string sql=std::format("select * from user where username='{}';",account);
    int ret=mysql_conn.getConn().Select(std::move(sql),result);

    if(ret<0){
        std::cout<<"查询语句执行失败"<<std::endl;
        Response response(2,2,500,"unknow error\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return -1;
    }
    if(result->getNum()>0){
        Response response(2,2,500,"user exists\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return 0;
    }
    sql=std::format("insert into user (username,passwd) values('{}','{}');",account,passwd);
    ret=mysql_conn.getConn().Insert(std::move(sql));
    if(ret<0){
        std::cout<<"插入用户信息失败"<<std::endl;
        Response response(2,2,500,"unknow error\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return -1;
    }
    //Response response(1,mysql_insert_id(mysql_conn.getConn().get()),200,"user successfully sign up\n");
    Response response(2,2,200,"user successfully sign up\n");
    send(connfd,Serialize(response).data(),response.size(),0);
    return 0;
}


int ChatServer::Echo(int connfd,const Require& require){
    Response response(3,require.m_head.SourceId,200,std::format("回声| {}: {}\n",m_users[connfd].m_name,require.m_data));
    int ret=write(connfd,Serialize(response).data(),response.size());
    if(ret<0) return -1;
    return 0;
}

int ChatServer::MakeFriend(int connfd,const Require& require){
    std::string name=require.m_data;
    MysqlRAII mysql_conn(m_dbPool);

    std::shared_ptr<SqlResult> result;
    int curId=require.m_head.SourceId;
    std::string sql=std::format("select * from user where username='{}';",name);
    int ret=mysql_conn.getConn().Select(std::move(sql),result);

    if(ret<0){
        std::cout<<"查询语句执行失败"<<std::endl;
        Response response(6,curId,500,"unknow error\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return -1;
    }
    if(result->getNum()<1){
        Response response(6,curId,500,"user not exist\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return 0;
    }
    
    auto friendId=result->getRows(0,1)[0]["id"];

    sql=std::format("select * from relation where user1_id={} and user2_id='{}';",curId,friendId);

    ret=mysql_conn.getConn().Select(std::move(sql),result);

    if(ret<0){
        std::cout<<"查询语句执行失败"<<std::endl;
        Response response(6,curId,500,"unknow error\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return -1;
    }
    if(result->getNum()>0){
        Response response(6,curId,500,"already existed friend\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return 0;
    }

    sql=std::format("insert into relation values('{}','{}'),('{}','{}');",curId,friendId,friendId,curId);
    ret=mysql_conn.getConn().Insert(std::move(sql));

    Response response(6,curId,200,friendId);
    send(connfd,Serialize(response).data(),response.size(),0);
    return 0;
}
int ChatServer::PublicChat(int connfd,const Require& require){
    std::lock_guard lock(m_mutex);
    for(auto user:m_id_fdMap){
        Response response(4,user.first,200,std::format("公共频道| {}: {}\n",m_users[connfd].m_name,require.m_data));
        send(user.second,Serialize(response).data(),response.size(),0);
    }
    return 0;
}

int ChatServer::PrivateChat(int connfd,const Require& require){
    std::string name=require.m_data;
    MysqlRAII mysql_conn(m_dbPool);

    auto user1=require.m_head.SourceId,user2=require.m_head.DestinationId;
    std::shared_ptr<SqlResult> result;

    std::string sql=std::format("select * from relation where user1_id='{}' and user2_id='{}' ;",user1,user2);

    int ret=mysql_conn.getConn().Select(std::move(sql),result);

    if(ret<0){
        std::cout<<"查询语句执行失败"<<std::endl;
        Response response(5,user1,500,"unknow error\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return -1;
    }
    if(result->getNum()<1){
        Response response(5,user1,500,"the user is not your friend\n");
        send(connfd,Serialize(response).data(),response.size(),0);
        return 0;
    }

    std::lock_guard lock(m_mutex);
    Response response(user1,user2,200,std::format("私聊| {}: {}",m_users[connfd].m_name,require.m_data));
    send(m_id_fdMap[user2],Serialize(response).data(),response.size(),0);
    send(connfd,Serialize(response).data(),response.size(),0);

    return 0;
}