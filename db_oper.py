import pymssql

class DB_oper:
    def __init__(self,db_name,db_ip="192.168.0.100",db_user="ARNTUSER",db_pwd="ARNTUSER"):
        self.db_ip=db_ip
        self.db_user=db_user
        self.db_pwd=db_pwd
        self.db_name=db_name
    def connect(self):
        self.conn=pymssql.connect(self.db_ip,self.db_user,self.db_pwd,self.db_name)
        self.cursor=self.conn.cursor()
    def search(self,sql):
        rows=[]
        self.cursor.execute(sql)
        for row in self.cursor:
            rows.append(row)
        return rows
    def close(self):
        self.conn.close()
