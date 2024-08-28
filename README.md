# personal-study-projects

个人学习途中产生的复数代码项目。通过代码托管，获取多端同步、查看修改、追溯历史等便利。

## Study

学习到知识点时，在这测试代码实例。

### 数据库SQLite使用

#### 安装

windows版本，直接解压、导入到环境变量。就可以用了

#### 开始使用

在`cmd`或`powershell`，使用`sqlite3`命令进入SQlite命令提示符模式。

- 点命令：  
  不以分号结束的命令。利用`.help`可获取可用的点命令清单。  
  例如：  
  `.quit`: 退出SQList命令提示符。
  `.dump`: 导出数据库到文本文档

- 注释：  
  以两个连续的 `-`开始，到换行或结束符结束。

#### 数据类型

| 类型     | 描述                                  |
| -------- | ------------------------------------- |
| NULL     | ...                                   |
| INTERGER | 整形，根据数据大小存在1\2\4\8字节中。 |
| REAL     | 实型(浮点型)，8字节IEEE。             |
| TEXT     | 文本类型，UTF-8存储                   |
| BOLB     | ?                                     |

>boolen类型存储为整形

#### 创建数据库

两种方法创建数据库：

1. cmd\powershell下: `$ sqlite3 DatabaseName.db`
2. SQLite命令提示符下: `.open DatabaseName.db`

>open命令会在没有对应数据库的情况下先创建

#### 附加数据库、分离数据库

用`ATTACH DATABASE`附加数据库。  
用`DETACH DATABASE`分离数据库。

```sql
ATTACH DATABASE file_name AS database_name; --附加数据库：
DETACH DATABASE 'Alias-Name';               --删除数据库：

.databases  --查看数据库：
```

#### 创建表、删除表

`CREATE TABLE`语句用以创建表。  
`DROP TABLE`语句用以删除表。

```sql
-- 创建表 基本语法
CREATE TABLE database_name.table_name
(
  column1 datatype  PRIMARY KEY(one or more columns),
  column2 datatype,
  column3 datatype,
  .....
  columnN datatype,
);
-- 删除表 基本语法
DROP TABLE database_name.table_name;

-- 创建表 实例
CREATE TABLE COMPANY
(
  ID INT PRIMARY KEY     NOT NULL,
  NAME           TEXT    NOT NULL,
  AGE            INT     NOT NULL,
  ADDRESS        CHAR(50),
  SALARY         REAL
);

-- 查看表
.tables -- 列出表名
.schema table_name -- 查看特定表信息

```

#### 添加

`INSERT INTO`语句用于向数据库的某个表中添加新的数据行。

```sql
-- 语法 (两种)
INSERT INTO TABLE_NAME [(column1, column2, column3,...columnN)]  
VALUES (value1, value2, value3,...valueN);
-- 实例
INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)
VALUES (1, 'Paul', 32, 'California', 20000.00 );

INSERT INTO COMPANY VALUES (7, 'James', 24, 'Houston', 10000.00 );
```
