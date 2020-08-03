from PyQt5 import QtGui
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
import numpy as np
import cv2
from H_CommonFunc import *


class UserListWidget(QListWidget):
    map_listwidget = []

    def __init__(self):
        super().__init__()
        self.data_init()
        self.ui_init()

    def data_init(self):
        item = QListWidgetItem()
        font = QFont()
        font.setPointSize(8)
        item.setFont(font)
        item.setText("【缺陷类别】：{}\n【纵向位置】：{}\n【距传动测】：{}\n【距操作测】：{}\n"
                     "【缺陷长度】：{}\n【缺陷宽度】：{}\n【缺陷面积】：{}\n【置信程度】：{}".format(
            '无', 0, 0, 0, 0, 0, 0, 0.0))
        item.setForeground(QBrush(Qt.green))
        item.setTextAlignment(Qt.AlignLeft | Qt.AlignVCenter)
        zero = np.zeros((128, 128), np.uint8)
        qimage = QtGui.QImage(zero, 128, 128, QtGui.QImage.Format_Grayscale8)
        pixmap = QtGui.QPixmap.fromImage(qimage)
        item.setIcon(QIcon(pixmap))
        self.addItem(item)

    def ui_init(self):
        self.setIconSize(QSize(128, 128))
        self.setStyleSheet("QListWidget{border:1px solid gray; color:red; }"
                           "QListWidget::Item{padding-top:2px; padding-bottom:4px; }"
                           "QListWidget::Item:hover{background:skyblue; }"
                           "QListWidget::item:selected:!active{border-width:0px; background:lightgreen; }"
                           )
        self.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.itemSelectionChanged.connect(self.get_list_items)

    def data_update(self, defect_class_list, defects_list):
        self.setCurrentRow(-1)
        self.clear()
        for i in range(0, len(defects_list)):
            item = QListWidgetItem()

            font = QFont()
            font.setPointSize(8)
            item.setFont(font)
            item.setText("【缺陷类别】：{}\n【纵向位置】：{}\n【距传动测】：{}\n【距操作测】：{}\n"
                         "【缺陷长度】：{}\n【缺陷宽度】：{}\n【缺陷面积】：{}\n【置信程度】：{}".format(
                defect_class_list[int(defects_list[i][13])][0],
                (int(defects_list[i][9]) + int(defects_list[i][10])) / 2,
                defects_list[i][11],
                defects_list[i][12],
                int(defects_list[i][10]) - int(defects_list[i][9]),
                int(defects_list[i][8]) - int(defects_list[i][7]),
                (int(defects_list[i][10]) - int(defects_list[i][9])) * (
                        int(defects_list[i][8]) - int(defects_list[i][7])),
                defects_list[i][14]))
            item.setTextAlignment(Qt.AlignLeft | Qt.AlignVCenter)
            if defects_list[i][17] is not None:
                img_icon = db_binary_to_img(defects_list[i][17], 64, 64)
                img_icon = cv2.resize(img_icon, (128, 128))
                qimage = QtGui.QImage(img_icon, 128, 128, QtGui.QImage.Format_Grayscale8)
                pixmap = QtGui.QPixmap.fromImage(qimage)
                item.setIcon(QIcon(pixmap))
            else:
                item.setIcon(QIcon(defects_list[i][16]))

            if defects_list[i][2] == 0:
                item.setForeground(QBrush(Qt.darkRed))
                item.setBackground(QBrush(Qt.lightGray))
                item.setToolTip('上表面')
            else:
                item.setForeground(QBrush(Qt.darkRed))
                item.setBackground(QBrush(Qt.darkGray))
                item.setToolTip('下表面')

            self.addItem(item)

    def get_list_items(self):
        # print(self.currentIndex().row())
        return self.selectedItems()

    def get_curr_index(self):
        return self.currentIndex().row()

    def contextMenuEvent(self, event):
        hitIndex = self.indexAt(event.pos()).column()
        if hitIndex > -1:
            pmenu = QMenu(self)
            pDeleteAct = QAction("删除", pmenu)
            pmenu.addAction(pDeleteAct)
            pDeleteAct.triggered.connect(self.deleteItemSlot)
            if self is self.find('我的好友'):
                pAddItem = QAction("新增好友", pmenu)
                pmenu.addAction(pAddItem)
                pAddItem.triggered.connect(self.addItemSlot)
            if len(self.map_listwidget) > 1:
                pSubMenu = QMenu("转移联系人至", pmenu)
                pmenu.addMenu(pSubMenu)
                for item_dic in self.map_listwidget:
                    if item_dic['listwidget'] is not self:
                        pMoveAct = QAction(item_dic['groupname'], pmenu)
                        pSubMenu.addAction(pMoveAct)
                        pMoveAct.triggered.connect(self.move)
            pmenu.popup(self.mapToGlobal(event.pos()))

    def deleteItemSlot(self):
        dellist = self.getListitems()
        for delitem in dellist:
            del_item = self.takeItem(self.row(delitem))
            del del_item

    def setListMap(self, listwidget):
        self.map_listwidget.append(listwidget)

    def move(self):
        tolistwidget = self.find(self.sender().text())
        movelist = self.getListitems()
        for moveitem in movelist:
            pItem = self.takeItem(self.row(moveitem))
            tolistwidget.addItem(pItem)

    def find(self, pmenuname):
        for item_dic in self.map_listwidget:
            if item_dic['groupname'] == pmenuname:
                return item_dic['listwidget']