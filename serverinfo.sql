use serverdb;

CREATE TABLE IF NOT EXISTS user(
    id int AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    passwd VARCHAR(50) NOT NULL
)

insert into user (id, username, passwd)
                    values(1,'SERVERLOGIN','SERVERLOGIN'),
                       (2,'SERVERSIGNUP','SERVERSIGNUP'),
                       (3,'SERVERECHO','SERVERECHO'),
                       (4,'SERVERPUBLIC','SERVERPUBLIC'),
                       (5,'SERVERPRIVATE','SERVERPRIVATE'),
                       (6,'SERVERMAKEFRIEND','SERVERMAKEFRIEND');

insert into user (username,passwd)
                            values('PRESERVE93','PRESERVE93'),
('PRESERVE92','PRESERVE92'),
('PRESERVE91','PRESERVE91'),
('PRESERVE90','PRESERVE90'),
('PRESERVE89','PRESERVE89'),
('PRESERVE88','PRESERVE88'),
('PRESERVE87','PRESERVE87'),
('PRESERVE86','PRESERVE86'),
('PRESERVE85','PRESERVE85'),
('PRESERVE84','PRESERVE84'),
('PRESERVE83','PRESERVE83'),
('PRESERVE82','PRESERVE82'),
('PRESERVE81','PRESERVE81'),
('PRESERVE80','PRESERVE80'),
('PRESERVE79','PRESERVE79'),
('PRESERVE78','PRESERVE78'),
('PRESERVE77','PRESERVE77'),
('PRESERVE76','PRESERVE76'),
('PRESERVE75','PRESERVE75'),
('PRESERVE74','PRESERVE74'),
('PRESERVE73','PRESERVE73'),
('PRESERVE72','PRESERVE72'),
('PRESERVE71','PRESERVE71'),
('PRESERVE70','PRESERVE70'),
('PRESERVE69','PRESERVE69'),
('PRESERVE68','PRESERVE68'),
('PRESERVE67','PRESERVE67'),
('PRESERVE66','PRESERVE66'),
('PRESERVE65','PRESERVE65'),
('PRESERVE64','PRESERVE64'),
('PRESERVE63','PRESERVE63'),
('PRESERVE62','PRESERVE62'),
('PRESERVE61','PRESERVE61'),
('PRESERVE60','PRESERVE60'),
('PRESERVE59','PRESERVE59'),
('PRESERVE58','PRESERVE58'),
('PRESERVE57','PRESERVE57'),
('PRESERVE56','PRESERVE56'),
('PRESERVE55','PRESERVE55'),
('PRESERVE54','PRESERVE54'),
('PRESERVE53','PRESERVE53'),
('PRESERVE52','PRESERVE52'),
('PRESERVE51','PRESERVE51'),
('PRESERVE50','PRESERVE50'),
('PRESERVE49','PRESERVE49'),
('PRESERVE48','PRESERVE48'),
('PRESERVE47','PRESERVE47'),
('PRESERVE46','PRESERVE46'),
('PRESERVE45','PRESERVE45'),
('PRESERVE44','PRESERVE44'),
('PRESERVE43','PRESERVE43'),
('PRESERVE42','PRESERVE42'),
('PRESERVE41','PRESERVE41'),
('PRESERVE40','PRESERVE40'),
('PRESERVE39','PRESERVE39'),
('PRESERVE38','PRESERVE38'),
('PRESERVE37','PRESERVE37'),
('PRESERVE36','PRESERVE36'),
('PRESERVE35','PRESERVE35'),
('PRESERVE34','PRESERVE34'),
('PRESERVE33','PRESERVE33'),
('PRESERVE32','PRESERVE32'),
('PRESERVE31','PRESERVE31'),
('PRESERVE30','PRESERVE30'),
('PRESERVE29','PRESERVE29'),
('PRESERVE28','PRESERVE28'),
('PRESERVE27','PRESERVE27'),
('PRESERVE26','PRESERVE26'),
('PRESERVE25','PRESERVE25'),
('PRESERVE24','PRESERVE24'),
('PRESERVE23','PRESERVE23'),
('PRESERVE22','PRESERVE22'),
('PRESERVE21','PRESERVE21'),
('PRESERVE20','PRESERVE20'),
('PRESERVE19','PRESERVE19'),
('PRESERVE18','PRESERVE18'),
('PRESERVE17','PRESERVE17'),
('PRESERVE16','PRESERVE16'),
('PRESERVE15','PRESERVE15'),
('PRESERVE14','PRESERVE14'),
('PRESERVE13','PRESERVE13'),
('PRESERVE12','PRESERVE12'),
('PRESERVE11','PRESERVE11'),
('PRESERVE10','PRESERVE10'),
('PRESERVE9','PRESERVE9'),
('PRESERVE8','PRESERVE8'),
('PRESERVE7','PRESERVE7'),
('PRESERVE6','PRESERVE6'),
('PRESERVE5','PRESERVE5'),
('PRESERVE4','PRESERVE4'),
('PRESERVE3','PRESERVE3'),
('PRESERVE2','PRESERVE2'),
('PRESERVE1','PRESERVE1');

                       
DROP TABLE chatrecord;

CREATE TABLE IF NOT EXISTS chatrecord(
    id int AUTO_INCREMENT PRIMARY KEY,
    sourceid int not NULL,
    destinationid int not NULL,
    record text,
    timestamp datetime,
    foreign key(sourceid) REFERENCES user(id),
    foreign key(destinationid) REFERENCES user(id)
)

CREATE TABLE IF NOT EXISTS relation(
    user1_id int,
    user2_id int,
    foreign key(user1_id) REFERENCES user(id),
    foreign key(user2_id) REFERENCES user(id)
)

