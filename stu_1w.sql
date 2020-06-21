create table stu(
    id char(10),
    name char(4),
    age int,
    gpa float,
    primary key(id)
);
insert into stu values('3170000001','evxb',22,1.6);
select * from stu where id = '3170000001';
