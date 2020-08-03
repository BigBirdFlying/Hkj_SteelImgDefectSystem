from PyQt5.QtCore import *
from db_oper import *
from Dlg_Query import *


class childWindow(QtWidgets.QWidget, Ui_Form):
    before_close_signal = pyqtSignal(list)

    def __init__(self, ip="192.168.0.100"):
        super(childWindow, self).__init__()
        self.setupUi(self)

        self.ip = ip

        self.dateTimeEdit_begin.setDisplayFormat('yyyy-MM-dd HH:mm:ss')
        self.dateTimeEdit_end.setDisplayFormat('yyyy-MM-dd HH:mm:ss')
        self.dateTimeEdit_begin.setDateTime(QDateTime.currentDateTime().addDays(-1))
        self.dateTimeEdit_end.setDateTime(QDateTime.currentDateTime())

        self.pushButton_querytime.clicked.connect(self.click_querytime)
        self.pushButton_querysteelno.clicked.connect(self.click_querysteelno)
        self.pushButton_querysquence.clicked.connect(self.click_querysquence)

    def click_querytime(self):
        dateTime_begin = self.dateTimeEdit_begin.dateTime()
        dateTime_end = self.dateTimeEdit_end.dateTime()

        data_begin, time_begin = dateTime_begin.toString(Qt.ISODate).split('T')
        year_begin, month_begin, day_begin = data_begin.split('-')
        hour_begin, min_begin, second_begin = time_begin.split(":")

        data_end, time_end = dateTime_end.toString(Qt.ISODate).split('T')
        year_end, month_end, day_end = data_end.split('-')
        hour_end, min_end, second_end = time_end.split(":")

        db_oper = DB_oper("SteelRecord", self.ip)
        db_oper.connect()
        sql = "select top 512 * from steel where TopDetectTime between '{} {}' and '{} {}' order by TopDetectTime desc".format(
            data_begin, time_begin, data_end, time_end)
        res = db_oper.search(sql)

        steelinfos = []
        for i in range(0, len(res)):
            self.squence = res[i][1]
            steelno = res[i][3]
            steellength = res[i][5]
            steeldetecttime = res[i][8]
            steelwidth = res[i][11]
            steelthick = res[i][16]
            steeltype = res[i][17]
            if int(steellength) <= 0:
                continue

            # sql = "select * from SteelID where ID='{}'".format(str(steelno))
            # res_ex = db_oper.search(sql)
            # if len(res_ex) > 0:
            #     steelwidth = res_ex[0][2]
            #     steelthick = res_ex[0][3]
            #     steeltype = res_ex[0][8]

            steelinfo = str(self.squence) + '&' + str(steelno) + '&' + str(steeldetecttime) + '&' + str(
                steellength) + '&' + str(steelwidth) + '&' + str(steelthick) + '&' + str(steeltype)
            steelinfos.append(steelinfo)
        db_oper.close()
        self.before_close_signal.emit(steelinfos)

    def click_querysteelno(self):
        self.steelno = self.lineEdit_steelno.text()  # 9408214000
        if len(self.steelno) < 6:
            print("钢板号查询信息不规范！")
            return
        db_oper = DB_oper("SteelRecord", self.ip)
        db_oper.connect()
        sql = "select * from steel where SteelID='{}'".format(str(self.steelno))
        res = db_oper.search(sql)
        steelinfos = []
        for i in range(0, len(res)):
            self.squence = res[i][1]
            steelno = res[i][3]
            steellength = res[i][5]
            steeldetecttime = res[i][8]
            steelwidth = -1
            steelthick = -1
            steeltype = "******"
            sql = "select * from SteelID where ID='{}'".format(str(steelno))
            res_ex = db_oper.search(sql)
            if len(res_ex) > 0:
                steelwidth = res_ex[0][2]
                steelthick = res_ex[0][3]
                steeltype = res_ex[0][8]
            steelinfo = str(self.squence) + '&' + str(steelno) + '&' + str(steeldetecttime) + '&' + str(
                steellength) + '&' + str(steelwidth) + '&' + str(steelthick) + '&' + str(steeltype)
            steelinfos.append(steelinfo)
        db_oper.close()
        self.before_close_signal.emit(steelinfos)

    def click_querysquence(self):
        self.squence = self.lineEdit_squence.text()
        if self.squence.isdigit() == False:
            print("流水号查询信息不规范！")
            return

        db_oper = DB_oper("SteelRecord", self.ip)
        db_oper.connect()
        sql = "select * from steel where SequeceNo={}".format(int(self.squence))
        res = db_oper.search(sql)
        steelinfos = []
        for i in range(0, len(res)):
            steelno = res[i][3]
            steellength = res[i][5]
            steeldetecttime = res[i][8]
            steelwidth = -1
            steelthick = -1
            steeltype = "******"
            sql = "select * from SteelID where ID='{}'".format(str(steelno))
            res_ex = db_oper.search(sql)
            if len(res_ex) > 0:
                steelwidth = res_ex[0][2]
                steelthick = res_ex[0][3]
                steeltype = res_ex[0][8]
            steelinfo = str(self.squence) + '&' + str(steelno) + '&' + str(steeldetecttime) + '&' + str(
                steellength) + '&' + str(steelwidth) + '&' + str(steelthick) + '&' + str(steeltype)
            steelinfos.append(steelinfo)
        db_oper.close()
        self.before_close_signal.emit(steelinfos)
