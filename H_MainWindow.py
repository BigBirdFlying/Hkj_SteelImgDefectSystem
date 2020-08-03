from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
import pyqtgraph as pg
import pyqtgraph.opengl as gl
from PIL import Image
import numpy as np
from Dlg_MainWindow import *
from H_Query import *
from db_oper import *
from file_oper import *
import cv2
import math
from H_CommonFunc import *
from H_ListWidget import UserListWidget


class MyWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, parent=None):
        self.log_oper = Log("Log_WinDefectSystem")
        # 配置信息 #
        self.bIsDebug = True
        self.bOnlyContourMode = True
        self.ip = "192.168.0.104"
        self.topPath = r"\\192.168.0.104\ArNTSteelContourImage"
        self.bottomPath = r"\\192.168.0.104\ArNTSteelContourImage"
        self.xRes = 0.5  # 原始拼接图像分辨率
        self.yRes = 1.0
        self.zoom_scale_total = 2
        self.zoom_scale_3D = 4
        self.contour_single_cut_w = 0
        # 载入配置信息
        self.load_config()
        self.log_oper.AddLog("[Normal] >>成功载入配置信息!")
        # self.bIsDebug = False
        # self.bOnlyContourMode = False
        #print(self.bOnlyContourMode)

        # 界面程序设置 #
        super(MyWindow, self).__init__(parent)
        self.setupUi(self)
        self.setWindowTitle("【钢板全表面缺陷展示界面--Vision1.0】北科工研")
        self.desktop = QApplication.desktop()  # 获取显示器分辨率大小
        self.screenRect = self.desktop.screenGeometry()
        self.winHeight = self.screenRect.height()
        self.winWidth = self.screenRect.width()
        self.resize(self.winWidth, self.winHeight)
        self.showMaximized()
        # 菜单 #
        self.menu_File.triggered[QAction].connect(self.file_click)

        # 在其程序中显示钢板图像处下采样了2倍，所以乘以2
        self.xRes = self.xRes * self.zoom_scale_total
        self.yRes = self.yRes * self.zoom_scale_total

        # R---当前钢板信息 #
        self.progressBar.setFixedSize(300, 20)
        # 缺陷图显示
        self.graphicsView_DefectShow.setFixedSize(300, 300)
        # 钢板信息列表
        self.tableWidget_SteelList.setColumnCount(2)
        self.tableWidget_SteelList.setRowCount(1024)

        # self.tableWidget_SteelList.setHorizontalHeaderLabels(['流水号', '钢板号'])
        self.tableWidget_SteelList.setEditTriggers(QAbstractItemView.NoEditTriggers)
        self.tableWidget_SteelList.horizontalHeader().setStretchLastSection(True)
        # self.tableWidget_SteelList.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        # self.tableWidget_SteelList.verticalHeader().setVisible(False)
        self.tableWidget_SteelList.horizontalHeader().setVisible(False)
        self.tableWidget_SteelList.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.tableWidget_SteelList.clicked.connect(self.set_curr_steel)
        # self.tableWidget_SteelList.setFixedSize(300, self.winHeight - 500)
        self.tableWidget_SteelList.setFixedWidth(300)

        # 模块tab标签 #
        self.tabWidget.setCurrentIndex(3)
        self.tabWidget.currentChanged.connect(self.tab_changed)

        # tab0---钢板图像显示模块 #
        self.win_top = pg.PlotWidget()
        self.verticalLayout_SteelImageTop.addWidget(self.win_top)
        self.win_bottom = pg.PlotWidget()
        self.verticalLayout_SteelImageBottom.addWidget(self.win_bottom)
        # 上下表图像
        self.topImgPath = None
        self.bottomImgPath = None
        self.topImg = None
        self.bottomImg = None
        self.topImg_to3D = None
        self.bottomImg_to3D = None

        #
        self.curr_seqno = None
        self.defects_list = None

        # tab1---分布图显示模块 #
        self.win_distribution_top = pg.GraphicsWindow()
        self.verticalLayout_DistributionTop.addWidget(self.win_distribution_top)
        self.win_distribution_bottom = pg.GraphicsWindow()
        self.verticalLayout_DistributionBottom.addWidget(self.win_distribution_bottom)

        # 缺陷图像列表
        self.listWidget_DefectImg = UserListWidget()
        self.listWidget_DefectImg.setObjectName("listWidget_DefectImg")
        self.horizontalLayout_7.addWidget(self.listWidget_DefectImg)
        self.gridLayout_3.addLayout(self.horizontalLayout_7, 0, 1, 1, 1)
        self.listWidget_DefectImg.setFixedWidth(300)
        # self.listWidget_DefectImg.setFixedSize(280, self.winHeight * 0.8)
        self.listWidget_DefectImg.itemSelectionChanged.connect(self.set_curr_defect)

        # tab2---3D图显示模块 #
        self.win_3D_img = gl.GLViewWidget()
        self.verticalLayout_3DImg.addWidget(self.win_3D_img)

        # tab3---轮廓分析优化剪切模块 #
        self.contour_actual_coord_x = None
        self.contour_actual_coord_y = None
        self.contour_actual_edge_length = None
        self.contour_actual_edge_left = None
        self.contour_actual_edge_right = None
        self.contour_actual_edge_width = None
        self.contour_actual_edge_mark = None
        self.contour_actual_cut_top = None
        self.contour_actual_cut_bottom = None
        self.contour_actual_cut_left = None
        self.contour_actual_cut_right = None
        self.win_Contour = pg.GraphicsWindow()
        self.verticalLayout_ContourImg.addWidget(self.win_Contour)
        self.win_Contour.setFixedHeight(self.winHeight * 0.5)
        self.plot_contour = self.win_Contour.addPlot()
        self.plot_contour.scene().sigMouseMoved.connect(self.mouse_moved)
        self.pushButton_SetPlan.clicked.connect(self.click_contour_set_plan)
        self.spinBox_AddCutHead.valueChanged.connect(self.get_add_cut_head_value)
        self.spinBox_AddCutTail.valueChanged.connect(self.get_add_cut_tail_value)
        self.spinBox_AddCutLeftEdge.valueChanged.connect(self.get_add_cut_left_edge_value)
        self.spinBox_AddCutRightEdge.valueChanged.connect(self.get_add_cut_right_edge_value)

        # 隐藏部分控件
        self.pushButton_SetPlan.setVisible(False)
        self.label_7.setVisible(False)
        self.label_10.setVisible(False)
        self.label_19.setVisible(False)
        self.label_36.setVisible(False)
        self.label_11.setVisible(False)
        self.label_33.setVisible(False)
        self.label_29.setVisible(False)
        self.label_31.setVisible(False)
        self.label_17.setVisible(False)
        self.label_18.setVisible(False)
        self.label_38.setVisible(False)
        self.label_35.setVisible(False)
        self.label_21.setVisible(False)
        self.label_34.setVisible(False)
        self.label_30.setVisible(False)
        self.label_32.setVisible(False)
        self.label_9.setVisible(False)
        self.label_12.setVisible(False)
        self.label_37.setVisible(False)
        self.label_22.setVisible(False)
        self.lineEdit_BaseCutHead.setVisible(False)
        self.lineEdit_BaseCutTail.setVisible(False)
        self.lineEdit_BaseCutLeftEdge.setVisible(False)
        self.lineEdit_BaseCutRightEdge.setVisible(False)
        self.lineEdit_MarginLength.setVisible(False)
        self.lineEdit_MarginWidth.setVisible(False)
        self.lineEdit_PlanLength.setVisible(False)
        self.lineEdit_PlanWidth.setVisible(False)
        self.spinBox_AddCutHead.setVisible(False)
        self.spinBox_AddCutTail.setVisible(False)
        self.spinBox_AddCutLeftEdge.setVisible(False)
        self.spinBox_AddCutRightEdge.setVisible(False)
        self.label_23.setVisible(False)
        self.label_24.setVisible(False)
        self.label_26.setVisible(False)
        self.label_27.setVisible(False)
        self.lineEdit.setVisible(False)
        self.lineEdit_2.setVisible(False)
        self.lineEdit_BaseWidth.setVisible(False)

        # 获得缺陷类别信息
        self.get_defect_class(ip=self.ip)
        #
        self.enable_oper = True

        # 状态栏
        self.progressBar.setValue(100)
        self.statusBar().showMessage("[Normal] >>完成界面初始化!")
        self.log_oper.AddLog("[Normal] >>完成界面初始化!")

    def load_config(self):
        xml_oper = Xml("H_SteelImgDefectSystem.xml")
        iIsDebuge = xml_oper.read_info('./调试模式')
        if int(iIsDebuge) == 0:
            self.bIsDebug = False
        else:
            self.bIsDebug = True
        iOnlyContourMode = xml_oper.read_info('./仅轮廓模式')
        if int(iOnlyContourMode) == 0:
            self.bOnlyContourMode = False
        else:
            self.bOnlyContourMode = True
        self.ip = xml_oper.read_info('./现场配置/数据库IP')
        self.topPath = xml_oper.read_info('./现场配置/上表图像路径')
        self.bottomPath = xml_oper.read_info('./现场配置/下表图像路径')
        self.xRes = float(xml_oper.read_info('./现场配置/横向分辨率'))
        self.yRes = float(xml_oper.read_info('./现场配置/纵向分辨率'))
        self.zoom_scale_total = int(xml_oper.read_info('./现场配置/全图像缩放范围'))
        self.zoom_scale_3D = int(xml_oper.read_info('./现场配置/图像3D缩放范围'))
        self.contour_single_cut_w = int(xml_oper.read_info('./高级配置/轮廓分析优化剪切模式/默认单边剪切量'))

    def tab_changed(self):
        if not self.enable_oper:
            return

        if self.tabWidget.currentIndex() == 0:
            index = self.listWidget_DefectImg.get_curr_index()
            if self.bOnlyContourMode:
                return
            self.draw_steel_img(index)
        elif self.tabWidget.currentIndex() == 1:
            if self.bOnlyContourMode:
                return
            self.draw_distribution()
        elif self.tabWidget.currentIndex() == 2:
            if self.bOnlyContourMode:
                return
            self.draw_3D_img()
        elif self.tabWidget.currentIndex() == 3:
            if self.bOnlyContourMode:
                return
            self.win_Contour.clear()
            self.contour_analysis()

    def get_steel_info(self, value):
        self.statusBar().showMessage("[Normal] >>查询到{}条钢板记录!".format(len(value)))
        self.log_oper.AddLog("[Normal] >>查询到{}条钢板记录!".format(len(value)))
        self.tableWidget_SteelList.clear()
        self.steels_info = []
        for i in range(0, len(value)):
            steel_info = value[i].split('&')
            self.steels_info.append(steel_info)
        for i in range(0, len(self.steels_info)):
            newItem = QTableWidgetItem(self.steels_info[i][0])
            self.tableWidget_SteelList.setItem(i, 0, newItem)
            newItem = QTableWidgetItem(self.steels_info[i][1])
            self.tableWidget_SteelList.setItem(i, 1, newItem)
        QTableWidget.resizeColumnsToContents(self.tableWidget_SteelList)
        QTableWidget.resizeRowsToContents(self.tableWidget_SteelList)

    def set_curr_steel(self, index):
        if not self.enable_oper:
            return
        # 设置当前钢板信息
        self.curr_seqno = self.steels_info[int(index.row())][0]
        self.curr_steelno = self.steels_info[int(index.row())][1]
        curr_detect_time = self.steels_info[int(index.row())][2]
        curr_steel_type = self.steels_info[int(index.row())][6]
        self.label_CurrSeqNo.setText(self.curr_seqno)
        self.label_CurrSteelNo.setText(self.curr_steelno)
        self.label_CurrDetectDateTime.setText(curr_detect_time)
        self.label_CurrSteelType.setText(curr_steel_type)
        self.statusBar().showMessage("[Normal] >>当前显示流水号{}的钢板信息!".format(self.curr_seqno))
        self.log_oper.AddLog("[Normal] >>当前显示流水号{}的钢板信息!".format(self.curr_seqno))
        # 绘制当前钢板数据
        self.draw_curr_steel_thread()

    def draw_curr_steel_thread(self):
        if self.bOnlyContourMode:
            self.win_Contour.clear()
            self.contour_analysis()
            return

        self.progressBar.setValue(0)
        self.get_defect_info(ip=self.ip)
        self.statusBar().showMessage("[Normal] >>查询到{}条缺陷信息!".format(len(self.defects_list)))
        self.log_oper.AddLog("[Normal] >>查询到{}条缺陷信息!".format(len(self.defects_list)))
        self.progressBar.setValue(25)

        self.topImg = None
        self.bottomImg = None
        self.topImg_to3D = None
        self.bottomImg_to3D = None

        if self.bIsDebug:
            self.topImgPath = 'file/contour/Steel.bmp'
            self.bottomImgPath = 'file/contour/Steel.bmp'
        else:
            self.topImgPath = self.topPath + '/' + str(self.curr_seqno) + '/Steel.bmp'
            self.bottomImgPath = self.bottomPath + '/' + str(self.curr_seqno) + '/Steel.bmp'

        if self.topImgPath is None or self.bottomImgPath is None:
            self.topImg = np.zeros((self.winHeight, int(self.WinWidth / 2)), np.uint8)
            self.bottomImg = np.zeros((self.winHeight, int(self.WinWidth / 2)), np.uint8)
        else:
            self.enable_oper = False
            self.load_img_thread = LoadImgThread(parent=None, top_path=self.topImgPath, bottom_path=self.bottomImgPath,
                                                 zoom_scale_total=self.zoom_scale_total,
                                                 zoom_scale_3D=self.zoom_scale_3D)
            self.load_img_thread.start()
            self.load_img_thread.finish_load_image.connect(self.draw_curr_steel)

    def draw_curr_steel(self, img):
        self.enable_oper = True

        self.topImg = img[0]
        self.bottomImg = img[1]
        self.topImg_to3D = img[2]
        self.bottomImg_to3D = img[3]
        self.load_img_thread.stop()

        self.statusBar().showMessage("[Normal] >>成功获取到钢板整体图像信息!")
        self.log_oper.AddLog("[Normal] >>成功获取到钢板整体图像信息!")
        self.progressBar.setValue(50)
        if self.tabWidget.currentIndex() == 0:
            # self.draw_steel_img()
            self.statusBar().showMessage("[Normal] >>成功绘制钢板图像!")
            self.log_oper.AddLog("[Normal] >>成功绘制钢板图像!")
        elif self.tabWidget.currentIndex() == 1:
            # self.draw_distribution()
            self.statusBar().showMessage("[Normal] >>成功绘制缺陷分布图!")
            self.log_oper.AddLog("[Normal] >>成功绘制缺陷分布图!")
        elif self.tabWidget.currentIndex() == 2:
            # self.draw_3D_img()
            self.statusBar().showMessage("[Normal] >>成功绘制3D分布图!")
            self.log_oper.AddLog("[Normal] >>成功绘制3D分布图!")
        elif self.tabWidget.currentIndex() == 3:
            self.win_Contour.clear()
            self.contour_analysis()
            self.statusBar().showMessage("[Normal] >>成功绘制轮廓分析图!")
            self.log_oper.AddLog("[Normal] >>成功绘制轮廓分析图!")
        self.progressBar.setValue(100)

    def get_defect_class(self, ip="192.168.0.100"):
        if self.bIsDebug:
            self.defect_class_list = {0: ["待分类", 's', (255, 255, 255)],
                                      1: ["辊印", 's', (255, 0, 0)],
                                      2: ["划伤", 's', (0, 255, 255)],
                                      3: ["翘皮", 's', (0, 0, 255)]}
        else:
            self.defect_class_list = {}
            config_oper = DB_oper("ConfigCenter", ip)
            config_oper.connect()
            sql = "select * from DefectClass"
            res = config_oper.search(sql)
            for i in range(0, len(res)):
                class_no = res[i][3]
                class_info = []
                class_info.append(res[i][1])
                class_info.append('s')
                class_info.append((res[i][4], res[i][5], res[i][5]))
                self.defect_class_list[int(class_no)] = class_info
            self.statusBar().showMessage("[Normal] >>成功获取钢板缺陷类别信息!")
            self.log_oper.AddLog("[Normal] >>成功获取钢板缺陷类别信息!")

    def get_defect_info(self, ip="192.168.0.100"):
        if self.bIsDebug:
            self.top_defect_num = 3
            self.bottom_defect_num = 3
            self.used_defect_class = [0, 1, 2, 3, 4, 5]
            self.defects_list = [
                [1, 123, 0, 100, 200, 1000, 1100, -1000, -900, 10000, 10100, 100, 2000, 1, 0.9, 0, "file/1.jpg", None],
                [2, 123, 0, 500, 600, 2000, 2100, 100, 200, 15000, 15100, 1000, 1000, 2, 0.9, 0, "file/2.jpg", None],
                [3, 123, 0, 1100, 1200, 3000, 3100, 1000, 1100, 20000, 20100, 2000, 100, 3, 0.9, 0, "file/3.jpg", None],
                [4, 123, 1, 100, 200, 1000, 1100, -1000, -900, 10000, 10100, 100, 2000, 1, 0., 0, "file/1.jpg", None],
                [5, 123, 1, -100, 0, 2000, 2100, 100, 200, 15000, 15100, 1000, 1000, 2, 0.9, 0, "file/2.jpg", None],
                [6, 123, 1, 0, 100, 3000, 3100, 1000, 1100, 20000, 20100, 2000, 100, 3, 0.9, 0, "file/3.jpg", None]]
        else:
            defect_db_oper = DB_oper("SteelRecord", ip)
            defect_db_oper.connect()
            sql = "select * from SteelDefect where SteelNo={}".format(int(self.curr_seqno))
            res = defect_db_oper.search(sql)
            self.defects_list = []
            self.top_defect_num = 0
            self.bottom_defect_num = 0
            self.used_defect_class = []
            for i in range(0, len(res)):
                defect_list = []
                for j in range(1, 19):
                    defect_list.append(res[i][j])
                if res[i][3] == 0:
                    self.top_defect_num = self.top_defect_num + 1
                else:
                    self.bottom_defect_num = self.bottom_defect_num + 1
                self.used_defect_class.append(res[i][14])
                self.defects_list.append(defect_list)
            self.defects_list = sorted(self.defects_list, key=(lambda x: x[2]))
            self.used_defect_class = list(set(self.used_defect_class))

    def draw_distribution(self):
        if self.defects_list is None:
            return
        if self.bIsDebug:
            contour_x_top, contour_y_top = get_curr_steel_contour("file/contour/ContourActualCoord.txt")
            contour_x_bottom, contour_y_bottom = get_curr_steel_contour("file/contour/ContourActualCoord.txt")
            edge_top = get_curr_steel_edge("file/contour/ContourPixelCoord.txt")
            edge_bottom = get_curr_steel_edge("file/contour/ContourPixelCoord.txt")
        else:
            top_contour_path = self.topPath + '/' + self.curr_seqno + "/ContourActualCoord.txt"
            if os.path.exists(top_contour_path):
                contour_x_top, contour_y_top = get_curr_steel_contour(top_contour_path)
                self.log_oper.AddLog("[Normal] >>成功加载上表轮廓信息!")
            else:
                contour_x_top, contour_y_top = [], []

            bottom_contour_path = self.bottomPath + '/' + self.curr_seqno + "/ContourActualCoord.txt"
            if os.path.exists(bottom_contour_path):
                contour_x_bottom, contour_y_bottom = get_curr_steel_contour(bottom_contour_path)
                self.log_oper.AddLog("[Normal] >>成功加载下表轮廓信息!")
            else:
                contour_x_bottom, contour_y_bottom = [], []

            top_edge_path = self.topPath + '/' + self.curr_seqno + "/ContourPixelCoord.txt"
            if os.path.exists(top_edge_path):
                edge_top = get_curr_steel_edge(top_edge_path)
                self.log_oper.AddLog("[Normal] >>成功加载上表边界信息!")
            else:
                edge_top = {}

            bottom_edge_path = self.bottomPath + '/' + self.curr_seqno + "/ContourPixelCoord.txt"
            if os.path.exists(bottom_edge_path):
                edge_bottom = get_curr_steel_edge(bottom_edge_path)
                self.log_oper.AddLog("[Normal] >>成功加载下表边界信息!")
            else:
                edge_bottom = {}

        self.reset_graph(max(len(edge_top), len(edge_bottom)), 4000)
        self.log_oper.AddLog("[Normal] >>成功初始化分布图面板!")
        self.draw_contour(contour_x_top, contour_y_top, contour_x_bottom, contour_y_bottom)
        self.log_oper.AddLog("[Normal] >>成功绘制轮廓曲线!")
        self.draw_defect(self.defect_class_list, self.defects_list)
        self.log_oper.AddLog("[Normal] >>成功绘制缺陷点!")
        self.listWidget_DefectImg.data_update(self.defect_class_list, self.defects_list)
        self.log_oper.AddLog("[Normal] >>成功添加缺陷图像列表显示!")
        self.statusBar().showMessage("[Normal] >>成功绘制分布图面板!")

    def reset_graph(self, steel_length, steel_width):
        self.win_distribution_top.clear()
        self.win_distribution_bottom.clear()

        pg.setConfigOption('background', 'w')

        self.plot_top = self.win_distribution_top.addPlot(title="top face")
        self.plot_bottom = self.win_distribution_bottom.addPlot(title="bottom face")

        self.plot_top.addLegend(size=[80, 20], offset=(-5, 0))
        self.plot_bottom.addLegend(size=[80, 20], offset=(-5, 0))

        self.plot_top.setXRange(0, steel_length + int(steel_length * 0.05))
        self.plot_bottom.setXRange(0, steel_length + int(steel_length * 0.05))

        self.plot_top.setYRange(-int(steel_width) / 2, int(steel_width) / 2)
        self.plot_bottom.setYRange(-int(steel_width) / 2, int(steel_width) / 2)

        self.plot_top.showGrid(x=True, y=True)
        self.plot_bottom.showGrid(x=True, y=True)

    def draw_contour(self, contour_x_top, contour_y_top, contour_x_bottom, contour_y_bottom):
        self.plot_top.plot(contour_y_top, contour_x_top, pen='b', fillLevel=0, fillBrush=(255, 255, 255, 100))
        self.plot_bottom.plot(contour_y_bottom, contour_x_bottom, pen='b', fillLevel=0, fillBrush=(255, 255, 255, 100))

    def draw_defect_class(self, plot, defect_class_list):
        for i in range(0, len(defect_class_list)):
            if i in self.used_defect_class:
                plot.plot([0], [0],
                          pen=defect_class_list[i][2],
                          symbolBrush=defect_class_list[i][2],
                          symbolPen='w',
                          symbol=defect_class_list[i][1],
                          symbolSize=8,
                          name=defect_class_list[i][0])

    def draw_defect(self, defect_class_list, defects_list):
        self.draw_defect_class(self.plot_top, defect_class_list)
        self.draw_defect_class(self.plot_bottom, defect_class_list)

        self.pg_defect_top = pg.ScatterPlotItem(size=8, pen=pg.mkPen(None), pxMode=True)
        self.pg_defect_bottom = pg.ScatterPlotItem(size=8, pen=pg.mkPen(None), pxMode=True)
        spots_top = []
        spots_bottom = []
        for i in range(0, len(defects_list)):
            class_no = defects_list[i][13]
            symbol = defect_class_list[class_no][1]
            color = defect_class_list[class_no][2]
            x = (int(defects_list[i][7]) + int(defects_list[i][8])) / 2
            y = (int(defects_list[i][9]) + int(defects_list[i][10])) / 2
            pos = {'pos': (y, x), 'data': 1, 'brush': color, 'symbol': symbol}
            face = defects_list[i][2]
            if face == 0:
                spots_top.append(pos)
            else:
                spots_bottom.append(pos)

        self.pg_defect_top.addPoints(spots_top)
        self.plot_top.addItem(self.pg_defect_top)
        self.pg_defect_bottom.addPoints(spots_bottom)
        self.plot_bottom.addItem(self.pg_defect_bottom)
        # self.plot_bottom.autoRange()

        self.lastClicked = []
        self.pg_defect_top.sigClicked.connect(self.click_defect)
        self.pg_defect_bottom.sigClicked.connect(self.click_defect)

    def set_curr_defect(self):
        if not self.enable_oper:
            return

        index = self.listWidget_DefectImg.get_curr_index()  # 解决：该模块在清理的时候会执行一次选中，所以由缺陷多到少时chubug
        if index < 0:
            return

        for p in self.pg_defect_top.points():
            p.resetPen()
        for p in self.pg_defect_bottom.points():
            p.resetPen()

        if index < self.top_defect_num:
            self.pg_defect_top.points()[index].setPen('w', width=2)
        else:
            self.pg_defect_bottom.points()[index - self.top_defect_num].setPen('w', width=2)

        self.show_single_defect_img(index)

    def click_defect(self, plot, points):
        if not self.enable_oper:
            return

        for p in self.lastClicked:
            p.resetPen()
        for p in points:
            p.setPen('w', width=2)
            index = 0
            for p_p in self.pg_defect_top.points():
                if p == p_p:
                    self.listWidget_DefectImg.setFocus()
                    self.listWidget_DefectImg.setCurrentRow(index)
                    self.show_single_defect_img(index)
                index = index + 1
            for p_p in self.pg_defect_bottom.points():
                if p == p_p:
                    self.listWidget_DefectImg.setFocus()
                    self.listWidget_DefectImg.setCurrentRow(index)
                    self.show_single_defect_img(index)
                index = index + 1
        self.lastClicked = points

    def show_single_defect_img(self, index):
        face = self.defects_list[index][2]
        left = int(self.defects_list[index][3] / self.zoom_scale_total)  # 图像加载时又下采样了一遍
        right = int(self.defects_list[index][4] / self.zoom_scale_total)
        top = int(self.defects_list[index][5] / self.zoom_scale_total)
        bottom = int(self.defects_list[index][6] / self.zoom_scale_total)
        center_x = (left + right) / 2
        center_y = (top + bottom) / 2
        width = right - left
        height = bottom - top
        show_w = max(256, width)
        show_h = max(256, height)
        show_size = int(max(show_w, show_h))
        show_x = int(center_x - show_size / 2)
        show_y = int(center_y - show_size / 2)
        if face == 0:
            if self.topImg is not None:
                if show_x < 0:
                    show_x = 0
                if show_x >= self.topImg.shape[1] - show_size:
                    show_x = self.topImg.shape[1] - show_size
                if show_y < 0:
                    show_y = 0
                if show_y >= self.topImg.shape[0] - show_size:
                    show_y = self.topImg.shape[0] - show_size
                img = self.topImg[show_y:show_y + show_size, show_x:show_x + show_size].copy()
        else:
            if self.bottomImg is not None:
                if show_x < 0:
                    show_x = 0
                if show_x >= self.bottomImg.shape[1] - show_size:
                    show_x = self.bottomImg.shape[1] - show_size
                if show_y < 0:
                    show_y = 0
                if show_y >= self.bottomImg.shape[0] - show_size:
                    show_y = self.bottomImg.shape[0] - show_size
                img = self.bottomImg[show_y:show_y + show_size, show_x:show_x + show_size].copy()
        img = cv2.rectangle(img, (left - show_x, top - show_y), (right - show_x, bottom - show_y), 255, 1)
        img = bytearray(img)
        Frame = QImage(img, show_size, show_size, QImage.Format_Grayscale8)
        Pix = QPixmap.fromImage(Frame)
        Item = QGraphicsPixmapItem(Pix)
        Scene = QGraphicsScene()
        Scene.addItem(Item)
        self.graphicsView_DefectShow.setScene(Scene)

    def draw_steel_img(self, defect_index=-1):
        self.win_top.clear()
        self.win_bottom.clear()
        if self.topImg is not None and self.bottomImg is not None:
            # top画面
            img_top = pg.ImageItem(border='w')
            img_h, img_w = self.topImg.shape
            size_top_win = max(img_h * self.yRes, img_w * self.xRes)
            self.win_top.setRange(QtCore.QRectF(-int(size_top_win * 0.5), 0, size_top_win, -size_top_win))
            # img_top.translate(-int(img_w * 0.5), -img_h)
            img_top.setImage(self.topImg.T)
            img_top.setRect(QtCore.QRectF(-img_w * self.xRes * 0.5, 0, img_w * self.xRes, -img_h * self.yRes))
            axis_top = pg.PolyLineROI([(-(size_top_win * 0.5), 0), ((size_top_win * 0.5), 0)], pen=(0, 0, 0))
            self.win_top.addItem(img_top)
            self.win_top.addItem(axis_top)

            # bootom画面
            img_bottom = pg.ImageItem(border='w')
            img_h, img_w = self.bottomImg.shape
            size_bottom_win = max(img_h * self.yRes, img_w * self.xRes)
            self.win_bottom.setRange(QtCore.QRectF(-int(size_bottom_win * 0.5), 0, size_bottom_win, -size_bottom_win))
            # img_bottom.translate(-int(img_w * 0.5), 0)
            img_bottom.setImage(self.bottomImg.T)
            img_bottom.setRect(QtCore.QRectF(-img_w * self.xRes * 0.5, 0, img_w * self.xRes, -img_h * self.yRes))
            axis_bottom = pg.PolyLineROI([(-(size_bottom_win * 0.5), 0), ((size_bottom_win * 0.5), 0)], pen=(0, 0, 0))
            self.win_bottom.addItem(img_bottom)
            self.win_bottom.addItem(axis_bottom)

            # 画缺陷，此处由于对拼接图像像素做了实物坐标变换，所以画框和显示区域都可以按实物坐标进行
            for k in range(0, len(self.defects_list)):
                face = self.defects_list[k][2]
                L = int(self.defects_list[k][7])
                R = int(self.defects_list[k][8])
                B = -int(self.defects_list[k][9])
                T = -int(self.defects_list[k][10])
                defect_roi = pg.ROI([L, T], [R - L, B - T], pen='y', movable=False)
                if face == 0:
                    self.win_top.addItem(defect_roi)
                else:
                    self.win_bottom.addItem(defect_roi)

            if defect_index >= 0:
                face = self.defects_list[defect_index][2]
                left = int(self.defects_list[defect_index][7])
                right = int(self.defects_list[defect_index][8])
                top = int(self.defects_list[defect_index][9])
                bottom = int(self.defects_list[defect_index][10])
                center_x = (left + right) / 2
                center_y = (top + bottom) / 2
                show_size = 1024
                show_x = int(center_x - show_size / 2)
                show_y = -int(center_y + show_size / 2)
                if face == 0:
                    self.win_top.setXRange(show_x, show_x + show_size, padding=0)
                    self.win_top.setYRange(show_y, show_y + show_size, padding=0)
                else:
                    self.win_bottom.setXRange(show_x, show_x + show_size, padding=0)
                    self.win_bottom.setYRange(show_y, show_y + show_size, padding=0)
        else:
            self.log_oper.AddLog("[Warning] >>上表或下表图像不存在，请检查!")
            self.statusBar().showMessage("[Warning] >>上表或下表图像不存在，请检查!")

    def draw_3D_img(self):
        if self.topImg_to3D is not None and self.bottomImg_to3D is not None:
            self.win_3D_img.setCameraPosition(distance=5000)

            levels = (0, 255)
            tex_top = pg.makeRGBA(self.topImg_to3D, levels=levels)[0]  # xy plane
            v_top = gl.GLImageItem(tex_top)
            v_top.translate(-self.topImg_to3D.shape[0] / 2, -self.topImg_to3D.shape[1] / 2, 10)
            self.win_3D_img.addItem(v_top)

            tex_bottom = pg.makeRGBA(self.bottomImg_to3D, levels=levels)[0]  # xy plane
            v_bottom = gl.GLImageItem(tex_bottom)
            v_bottom.translate(-self.bottomImg_to3D.shape[0] / 2, -self.bottomImg_to3D.shape[1] / 2, -10)
            self.win_3D_img.addItem(v_bottom)

            for i in range(0, len(self.defects_list)):
                x = self.defects_list[i][9] / (self.zoom_scale_total * self.zoom_scale_3D) - self.topImg_to3D.shape[
                    0] / 2
                y = self.defects_list[i][7] / (self.zoom_scale_total * self.zoom_scale_3D)
                verts = np.array([[x - 16, y - 16, 100],
                                  [x + 16, y - 16, 100],
                                  [x + 16, y + 16, 100],
                                  [x - 16, y + 16, 100],
                                  [x, y, 10], ])
                faces = np.array([[0, 1, 2, 3],
                                  [0, 1, 2, 4],
                                  [0, 1, 3, 4],
                                  [0, 2, 3, 4],
                                  [1, 2, 3, 4]])
                colors = np.array([[1, 0, 0, 0.3],
                                   [0, 1, 0, 0.3],
                                   [0, 0, 1, 0.3],
                                   [0, 1, 1, 0.3],
                                   [1, 1, 0, 0.3]])
                m = gl.GLMeshItem(vertexes=verts, faces=faces, faceColors=colors, smooth=False)
                m.setGLOptions('additive')
                self.win_3D_img.addItem(m)
        else:
            self.log_oper.AddLog("[Warning] >>上表或下表3D图像不存在，请检查!")
            self.statusBar().showMessage("[Warning] >>上表或下表3D图像不存在，请检查!")

    def judge_enable_set_plan(self):
        if self.lineEdit_MeasureLength.text() == '' or self.lineEdit_MeasureWidth.text() == '':
            msgBox = QtGui.QMessageBox.about(self, u'提示', u"\n没有钢板信息，请勿点击确认按钮！！！")
            return False
        if self.lineEdit_PlanLength.text() == '' or self.lineEdit_PlanWidth.text() == '':
            msgBox = QtGui.QMessageBox.about(self, u'提示', u"\n计划宽度和计划长度不能为空，请填写！！！")
            return False
        if not self.lineEdit_PlanLength.text().isdecimal() or not self.lineEdit_PlanWidth.text().isdecimal():
            msgBox = QtGui.QMessageBox.about(self, u'提示', u"\n计划宽度和计划长度存在非法字符，请重新填写！！！")
            return False
        if int(self.lineEdit_PlanLength.text()) > int(self.lineEdit_MeasureLength.text()):
            msgBox = QtGui.QMessageBox.about(self, u'警告', u"\n计划长度超出实际测量长度！！！")
            return False
        if int(self.lineEdit_PlanWidth.text()) > int(self.lineEdit_BaseWidth.text()):
            msgBox = QtGui.QMessageBox.about(self, u'警告', u"\n计划宽度超出测量基准宽度！！！")
            return False
        if int(self.lineEdit_BaseWidth.text()) - int(self.lineEdit_PlanWidth.text()) > 200:
            msgBox = QtGui.QMessageBox.about(self, u'警告', u"\n计划宽度与测量基准宽度超出200mm，请人工确认！！！")
            return False
        return True

    def click_contour_set_plan(self):
        if not self.judge_enable_set_plan():
            return

        self.contour_plan_length = int(self.lineEdit_PlanLength.text())
        self.contour_plan_width = int(self.lineEdit_PlanWidth.text())

        length_start = 0
        length_end = 0
        self.offset = 0
        for i in range(0, len(self.contour_actual_cut_top)):
            l = self.contour_actual_cut_left[i]
            r = self.contour_actual_cut_right[i]
            w = r - l
            if w <= self.contour_plan_width:
                length_start = self.contour_actual_cut_top[i]
                length_end = self.contour_actual_cut_bottom[i]
                self.offset = i
                break

        MarginLength = (length_end - length_start) - self.contour_plan_length
        if MarginLength < 0:
            msgBox = QtGui.QMessageBox.about(self, u'警告', u"\n当前钢板无法针对此计划进行切割！！！")
            self.lineEdit_MarginLength.setText(str(-1))
            self.lineEdit_MarginWidth.setText(str(-1))
            self.lineEdit_BaseCutHead.setText(str(-1))
            self.lineEdit_BaseCutTail.setText(str(-1))
            self.lineEdit_BaseCutLeftEdge.setText(str(-1))
            self.lineEdit_BaseCutRightEdge.setText(str(-1))
            self.spinBox_AddCutHead.setMaximum(0)
            self.spinBox_AddCutTail.setMaximum(0)
            self.spinBox_AddCutLeftEdge.setMaximum(0)
            self.spinBox_AddCutRightEdge.setMaximum(0)
            return

        self.lineEdit_MarginLength.setText(str(MarginLength))
        self.lineEdit_BaseCutHead.setText(str(length_start))
        self.lineEdit_BaseCutTail.setText(str(self.contour_actual_edge_length[-1] - length_end))
        self.lineEdit_BaseCutLeftEdge.setText(str(0))
        self.lineEdit_BaseCutRightEdge.setText(str(0))
        self.spinBox_AddCutHead.setMaximum(MarginLength)
        self.spinBox_AddCutTail.setMaximum(MarginLength)
        self.spinBox_AddCutHead.setValue(int(MarginLength * 0.5))
        self.spinBox_AddCutTail.setValue(MarginLength - int(MarginLength * 0.5))
        self.draw_contour_analysis(self.offset)

    def get_add_cut_head_value(self):
        MarginLength = int(self.lineEdit_MarginLength.text())
        value = self.spinBox_AddCutHead.value()
        self.spinBox_AddCutTail.setValue(MarginLength - value)

        self.plan_top = int(self.lineEdit_BaseCutHead.text()) + self.spinBox_AddCutHead.value()
        self.plan_bottom = self.contour_actual_edge_length[-1] - int(
            self.lineEdit_BaseCutTail.text()) - self.spinBox_AddCutTail.value()
        if self.plan_top < 0 or self.plan_bottom - self.plan_top <= 0:
            return
        # 计算在计划板位置处宽度的余量
        self.plan_max_left = -99999
        self.plan_min_right = 99999
        for i in range(self.plan_top, self.plan_bottom):
            if self.contour_actual_edge_left[i] > self.plan_max_left:
                self.plan_max_left = self.contour_actual_edge_left[i]
            if self.contour_actual_edge_right[i] < self.plan_min_right:
                self.plan_min_right = self.contour_actual_edge_right[i]

        MarginWidth = (self.plan_min_right - self.plan_max_left) - int(self.lineEdit_PlanWidth.text())
        self.lineEdit_MarginWidth.setText(str(MarginWidth))
        self.spinBox_AddCutLeftEdge.setMaximum(MarginWidth)
        self.spinBox_AddCutRightEdge.setMaximum(MarginWidth)
        self.spinBox_AddCutLeftEdge.setValue(int(MarginWidth * 0.5))
        self.spinBox_AddCutRightEdge.setValue(MarginWidth - int(MarginWidth * 0.5))

        self.draw_contour_analysis(self.offset)

    def get_add_cut_tail_value(self):
        MarginLength = int(self.lineEdit_MarginLength.text())
        value = self.spinBox_AddCutTail.value()
        self.spinBox_AddCutHead.setValue(MarginLength - value)

        self.plan_top = int(self.lineEdit_BaseCutHead.text()) + self.spinBox_AddCutHead.value()
        self.plan_bottom = self.contour_actual_edge_length[-1] - int(
            self.lineEdit_BaseCutTail.text()) - self.spinBox_AddCutTail.value()
        if self.plan_top < 0 or self.plan_bottom - self.plan_top <= 0:
            return
        # 计算在计划板位置处宽度的余量
        self.plan_max_left = -99999
        self.plan_min_right = 99999
        for i in range(self.plan_top, self.plan_bottom):
            if self.contour_actual_edge_left[i] > self.plan_max_left:
                self.plan_max_left = self.contour_actual_edge_left[i]
            if self.contour_actual_edge_right[i] < self.plan_min_right:
                self.plan_min_right = self.contour_actual_edge_right[i]

        MarginWidth = (self.plan_min_right - self.plan_max_left) - int(self.lineEdit_PlanWidth.text())
        self.lineEdit_MarginWidth.setText(str(MarginWidth))
        self.spinBox_AddCutLeftEdge.setMaximum(MarginWidth)
        self.spinBox_AddCutRightEdge.setMaximum(MarginWidth)
        self.spinBox_AddCutLeftEdge.setValue(int(MarginWidth * 0.5))
        self.spinBox_AddCutRightEdge.setValue(MarginWidth - int(MarginWidth * 0.5))

        self.draw_contour_analysis(self.offset)

    def get_add_cut_left_edge_value(self):
        MarginWidth = int(self.lineEdit_MarginWidth.text())
        value = self.spinBox_AddCutLeftEdge.value()
        self.spinBox_AddCutRightEdge.setValue(MarginWidth - value)

        self.plan_left = self.plan_max_left + int(
            self.lineEdit_BaseCutLeftEdge.text()) + self.spinBox_AddCutLeftEdge.value()
        self.plan_right = self.plan_min_right - int(
            self.lineEdit_BaseCutRightEdge.text()) - self.spinBox_AddCutRightEdge.value()

        self.draw_contour_analysis(self.offset)

    def get_add_cut_right_edge_value(self):
        MarginWidth = int(self.lineEdit_MarginWidth.text())
        value = self.spinBox_AddCutRightEdge.value()
        self.spinBox_AddCutLeftEdge.setValue(MarginWidth - value)

        self.plan_left = self.plan_max_left + int(
            self.lineEdit_BaseCutLeftEdge.text()) + self.spinBox_AddCutLeftEdge.value()
        self.plan_right = self.plan_min_right - int(
            self.lineEdit_BaseCutRightEdge.text()) - self.spinBox_AddCutRightEdge.value()

        self.draw_contour_analysis(self.offset)

    def contour_analysis(self):
        if self.bIsDebug:
            self.contour_actual_coord_x, self.contour_actual_coord_y = get_steel_contour_actual_coord(
                "file/contour/ContourActualCoord.txt")
            self.contour_actual_edge_length, self.contour_actual_edge_left, self.contour_actual_edge_right, self.contour_actual_edge_width, self.contour_actual_edge_mark = get_steel_contour_actual_edge(
                "file/contour/ContourActualEdge.txt")
            self.contour_actual_cut_top, self.contour_actual_cut_bottom, self.contour_actual_cut_left, self.contour_actual_cut_right = get_steel_contour_actual_cutline(
                "file/contour/ContourActualCutLine.txt")
        else:
            if self.curr_seqno is None:
                return
            contour_actual_coord_path = self.bottomPath + '/' + self.curr_seqno + "/ContourActualCoord.txt"
            if os.path.exists(contour_actual_coord_path):
                self.contour_actual_coord_x, self.contour_actual_coord_y = get_steel_contour_actual_coord(
                    contour_actual_coord_path)
            else:
                self.log_oper.AddLog("[Warning] >>载入钢板轮廓坐标失败!")
                return
            contour_actual_edge_path = self.bottomPath + '/' + self.curr_seqno + "/ContourActualEdge.txt"
            if os.path.exists(contour_actual_edge_path):
                self.contour_actual_edge_length, self.contour_actual_edge_left, self.contour_actual_edge_right, self.contour_actual_edge_width, self.contour_actual_edge_mark = get_steel_contour_actual_edge(
                    contour_actual_edge_path)
            else:
                self.log_oper.AddLog("[Warning] >>载入钢板轮廓边界失败!")
                return
            contour_actual_cut_path = self.bottomPath + '/' + self.curr_seqno + "/ContourActualCutLine.txt"
            if os.path.exists(contour_actual_cut_path):
                self.contour_actual_cut_top, self.contour_actual_cut_bottom, self.contour_actual_cut_left, self.contour_actual_cut_right = get_steel_contour_actual_cutline(
                    contour_actual_cut_path)
            else:
                self.log_oper.AddLog("[Warning] >>载入钢板轮廓优化剪切线失败!")
                return

        # 求角及对角线长度,针对实物钢板的表检坐标
        init_left_top = [-5000, 0]
        init_right_top = [5000, 0]
        init_left_bottom = [-5000, 100000]
        init_right_bottom = [5000, 100000]
        index_left_top = 0
        index_right_top = 0
        index_left_bottom = 0
        index_right_bottom = 0
        mindis_left_top = 1000000
        mindis_right_top = 1000000
        mindis_left_bottom = 1000000
        mindis_right_bottom = 1000000
        for i in range(0, len(self.contour_actual_coord_x)):
            x = self.contour_actual_coord_x[i]
            y = self.contour_actual_coord_y[i]
            if math.sqrt(abs(x - init_left_top[0]) * abs(x - init_left_top[0]) + abs(y - init_left_top[1]) * abs(
                    y - init_left_top[1])) < mindis_left_top:
                mindis_left_top = math.sqrt(
                    abs(x - init_left_top[0]) * abs(x - init_left_top[0]) + abs(y - init_left_top[1]) * abs(
                        y - init_left_top[1]))
                index_left_top = i
            if math.sqrt(abs(x - init_right_top[0]) * abs(x - init_right_top[0]) + abs(y - init_right_top[1]) * abs(
                    y - init_right_top[1])) < mindis_right_top:
                mindis_right_top = math.sqrt(
                    abs(x - init_right_top[0]) * abs(x - init_right_top[0]) + abs(y - init_right_top[1]) * abs(
                        y - init_right_top[1]))
                index_right_top = i
            if math.sqrt(
                    abs(x - init_left_bottom[0]) * abs(x - init_left_bottom[0]) + abs(y - init_left_bottom[1]) * abs(
                        y - init_left_bottom[1])) < mindis_left_bottom:
                mindis_left_bottom = math.sqrt(
                    abs(x - init_left_bottom[0]) * abs(x - init_left_bottom[0]) + abs(y - init_left_bottom[1]) * abs(
                        y - init_left_bottom[1]))
                index_left_bottom = i
            if math.sqrt(
                    abs(x - init_right_bottom[0]) * abs(x - init_right_bottom[0]) + abs(y - init_right_bottom[1]) * abs(
                        y - init_right_bottom[1])) < mindis_right_bottom:
                mindis_right_bottom = math.sqrt(
                    abs(x - init_right_bottom[0]) * abs(x - init_right_bottom[0]) + abs(y - init_right_bottom[1]) * abs(
                        y - init_right_bottom[1]))
                index_right_bottom = i
        # 对角线长度
        main_diag_length = math.sqrt(
            abs(self.contour_actual_coord_x[index_left_top] - self.contour_actual_coord_x[index_right_bottom]) * abs(
                self.contour_actual_coord_x[index_left_top] - self.contour_actual_coord_x[index_right_bottom])
            + abs(self.contour_actual_coord_y[index_left_top] - self.contour_actual_coord_y[index_right_bottom]) * abs(
                self.contour_actual_coord_y[index_left_top] - self.contour_actual_coord_y[index_right_bottom]))
        auxi_diag_length = math.sqrt(
            abs(self.contour_actual_coord_x[index_right_top] - self.contour_actual_coord_x[index_left_bottom]) * abs(
                self.contour_actual_coord_x[index_right_top] - self.contour_actual_coord_x[index_left_bottom])
            + abs(self.contour_actual_coord_y[index_right_top] - self.contour_actual_coord_y[index_left_bottom]) * abs(
                self.contour_actual_coord_y[index_right_top] - self.contour_actual_coord_y[index_left_bottom]))
        self.lineEdit_MainDiagonalLength.setText(str(int(main_diag_length)))
        self.lineEdit_AuxiliaryDiagonalLength.setText(str(int(auxi_diag_length)))
        # 求夹角
        k1 = (self.contour_actual_coord_y[index_right_top] - self.contour_actual_coord_y[index_left_top]) / (
                self.contour_actual_coord_x[index_right_top] - self.contour_actual_coord_x[index_left_top])
        k2 = (self.contour_actual_coord_y[index_left_bottom] - self.contour_actual_coord_y[index_left_top]) / (
                self.contour_actual_coord_x[index_left_bottom] - self.contour_actual_coord_x[index_left_top])
        tan_k = (k1 - k2) / (1 + k2 * k1)
        arctan_a = math.atan(tan_k) * 180.0 / math.pi
        self.lineEdit_CutAngle.setText(str(round(arctan_a, 2)))
        # 上下边线长度(对应表检的左右边线)
        right_edge_length = math.sqrt(
            abs(self.contour_actual_coord_x[index_right_top] - self.contour_actual_coord_x[index_right_bottom]) * abs(
                self.contour_actual_coord_x[index_right_top] - self.contour_actual_coord_x[index_right_bottom])
            + abs(self.contour_actual_coord_y[index_right_top] - self.contour_actual_coord_y[index_right_bottom]) * abs(
                self.contour_actual_coord_y[index_right_top] - self.contour_actual_coord_y[index_right_bottom]))
        left_edge_length = math.sqrt(
            abs(self.contour_actual_coord_x[index_left_top] - self.contour_actual_coord_x[index_left_bottom]) * abs(
                self.contour_actual_coord_x[index_left_top] - self.contour_actual_coord_x[index_left_bottom])
            + abs(self.contour_actual_coord_y[index_left_top] - self.contour_actual_coord_y[index_left_bottom]) * abs(
                self.contour_actual_coord_y[index_left_top] - self.contour_actual_coord_y[index_left_bottom]))
        self.lineEdit_TopEdgeLength.setText(str(int(right_edge_length)))
        self.lineEdit_BottomEdgeLength.setText(str(int(left_edge_length)))
        #
        self.contour_mark_start = 0
        self.contour_mark_end = len(self.contour_actual_edge_mark)
        for i in range(0, len(self.contour_actual_edge_mark)):
            if self.contour_actual_edge_mark[i] > 0:
                self.contour_mark_start = i
                break
        for i in range(len(self.contour_actual_edge_mark) - 1, 0, -1):
            if self.contour_actual_edge_mark[i] > 0:
                self.contour_mark_end = i
                break

        self.lineEdit_MeasureLength.setText(str(self.contour_actual_edge_length[-1]))
        self.lineEdit_MeasureLength.setText(str((int(left_edge_length) + int(right_edge_length)) * 0.5))  # 针对方板
        if len(self.contour_actual_edge_width) > 2000:
            self.lineEdit_MeasureWidth.setText(str(int(np.mean(self.contour_actual_edge_width[1000:-1000]))))
        self.lineEdit_BaseWidth.setText(str(self.contour_actual_cut_right[0] - self.contour_actual_cut_left[0]))


        self.plot_contour = self.win_Contour.addPlot()
        self.draw_contour_analysis()

    def draw_contour_analysis(self, offset=0):
        self.plot_contour.clear()

        self.plot_contour.showGrid(x=True, y=True)
        self.plot_contour.plot(self.contour_actual_coord_y, self.contour_actual_coord_x, pen=(255, 255, 0),
                               fillLevel=0,
                               fillBrush=(255, 255, 255, 100))

        curve_left = self.plot_contour.plot(
            self.contour_actual_edge_length[self.contour_mark_start:self.contour_mark_end],
            self.contour_actual_edge_left[self.contour_mark_start:self.contour_mark_end],
            pen=(255, 255, 0),
            fillLevel=0, fillBrush=(255, 255, 255, 0))
        curve_right = self.plot_contour.plot(
            self.contour_actual_edge_length[self.contour_mark_start:self.contour_mark_end],
            self.contour_actual_edge_right[self.contour_mark_start:self.contour_mark_end],
            pen=(255, 255, 0),
            fillLevel=0, fillBrush=(255, 255, 255, 0))

        self.curvePoint_left = pg.CurvePoint(curve_left)
        self.curvePoint_right = pg.CurvePoint(curve_right)
        arrow_left = pg.ArrowItem(angle=270)
        arrow_left.setParentItem(self.curvePoint_left)
        arrow_right = pg.ArrowItem(angle=90)
        arrow_right.setParentItem(self.curvePoint_right)
        self.text_coord = pg.TextItem("0", anchor=(0.5, 1))
        self.plot_contour.addItem(self.text_coord)

        if offset > 0:
            # 画在计划宽度时头尾可切最长范围
            self.plot_contour.plot([self.contour_actual_cut_top[offset], self.contour_actual_cut_bottom[offset],
                                    self.contour_actual_cut_bottom[offset], self.contour_actual_cut_top[offset],
                                    self.contour_actual_cut_top[offset]],
                                   [self.contour_actual_cut_left[offset], self.contour_actual_cut_left[offset],
                                    self.contour_actual_cut_right[offset], self.contour_actual_cut_right[offset],
                                    self.contour_actual_cut_left[offset]],
                                   pen=(255, 0, 255), fillLevel=0, fillBrush=(255, 255, 0, 0))

            # 画定完切头尾位置后的可切最宽板面
            self.plot_contour.plot([self.plan_top, self.plan_bottom, self.plan_bottom, self.plan_top, self.plan_top],
                                   [self.plan_max_left, self.plan_max_left, self.plan_min_right, self.plan_min_right,
                                    self.plan_max_left],
                                   pen=(0, 255, 0), fillLevel=0, fillBrush=(255, 255, 0, 0))

            # 画整个计划板面
            self.plot_contour.plot([self.plan_top, self.plan_bottom, self.plan_bottom, self.plan_top, self.plan_top],
                                   [self.plan_left, self.plan_left, self.plan_right, self.plan_right, self.plan_left],
                                   pen='r', fillLevel=0, fillBrush=(0, 255, 255, 100))

    def mouse_moved(self, evt):
        pos = [evt.x(), evt.y()]

        if self.plot_contour.sceneBoundingRect().contains(evt):
            mousePoint = self.plot_contour.vb.mapSceneToView(evt)
            if self.contour_actual_edge_length is None:
                return
            if int(mousePoint.x()) in self.contour_actual_edge_length[self.contour_mark_start:self.contour_mark_end] \
                    and int(mousePoint.y()) > np.min(self.contour_actual_edge_left) \
                    and int(mousePoint.y()) < np.max(self.contour_actual_edge_right):
                index = self.contour_actual_edge_length.index(int(mousePoint.x()))
                self.curvePoint_left.setPos(float(index) / (len(self.contour_actual_edge_length) - 1))
                self.curvePoint_right.setPos(float(index) / (len(self.contour_actual_edge_length) - 1))
                self.text_coord.setText('[%d, %d]' % (self.contour_actual_edge_length[index],
                                                      self.contour_actual_edge_width[index]))
                self.text_coord.setPos(int(mousePoint.x()), int(mousePoint.y()))

    def file_click(self, event):
        if not self.enable_oper:
            return

        if event == self.actionOpen:
            imgName, imgType = QFileDialog.getOpenFileName(self, "打开图片", "", "*.jpg;;*.png;;All Files(*)")
            self.topImgPath = imgName
            self.bottomImgPath = imgName
            self.get_steel_img()
            self.draw_steel_img()
        elif event == self.actionQuery:
            if self.bIsDebug:
                self.steels_info = [['10000', 'A1234567890', '2010-11-11 11:11:11', 10000, 100, 10, '345B'],
                                    ['10001', 'A1234567890', '2010-11-11 11:11:11', 10000, 100, 10, '345B'],
                                    ['10002', 'A1234567890', '2010-11-11 11:11:11', 10000, 100, 10, '345B']]
                for i in range(0, len(self.steels_info)):
                    newItem = QTableWidgetItem(self.steels_info[i][0])
                    self.tableWidget_SteelList.setItem(i, 0, newItem)
                    newItem = QTableWidgetItem(self.steels_info[i][1])
                    self.tableWidget_SteelList.setItem(i, 1, newItem)
            else:
                self.child = childWindow(ip=self.ip)
                self.child.show()
                self.child.before_close_signal.connect(self.get_steel_info)

    def closeEvent(self, event):
        reply = QtWidgets.QMessageBox.question(self,
                                               '本程序',
                                               "是否要退出程序？",
                                               QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.No,
                                               QtWidgets.QMessageBox.No)
        if reply == QtWidgets.QMessageBox.Yes:
            event.accept()
        else:
            event.ignore()


class LoadImgThread(QtCore.QThread):
    finish_load_image = QtCore.pyqtSignal(list)

    def __init__(self, parent=None, top_path=None, bottom_path=None, zoom_scale_total=1, zoom_scale_3D=1):
        super(LoadImgThread, self).__init__(parent)
        self.top_path = top_path
        self.bottom_path = bottom_path
        self.zoom_scale_total = zoom_scale_total
        self.zoom_scale_3D = zoom_scale_3D
        self.is_running = True

    def run(self):
        self.top_img = cv2.imread(self.top_path, 0)
        self.bottom_img = cv2.imread(self.bottom_path, 0)

        if self.top_img is not None:
            self.top_img = cv2.resize(self.top_img, (int(self.top_img.shape[1] / self.zoom_scale_total),
                                                     int(self.top_img.shape[0] / self.zoom_scale_total)))
            self.top_img_to_3D = cv2.resize(self.top_img, (int(self.top_img.shape[1] / self.zoom_scale_3D),
                                                           int(self.top_img.shape[0] / self.zoom_scale_3D)))
        if self.bottom_img is not None:
            self.bottom_img = cv2.resize(self.bottom_img, (int(self.bottom_img.shape[1] / self.zoom_scale_total),
                                                           int(self.bottom_img.shape[0] / self.zoom_scale_total)))
            self.bottom_img_to_3D = cv2.resize(self.bottom_img, (int(self.bottom_img.shape[1] / self.zoom_scale_3D),
                                                                 int(self.bottom_img.shape[0] / self.zoom_scale_3D)))

        if self.top_img is not None and self.bottom_img is not None:
            res_img = []
            res_img.append(self.top_img)
            res_img.append(self.bottom_img)
            res_img.append(self.top_img_to_3D)
            res_img.append(self.bottom_img_to_3D)
            self.finish_load_image.emit(res_img)

    def stop(self):
        self.is_running = False
        self.terminate()
