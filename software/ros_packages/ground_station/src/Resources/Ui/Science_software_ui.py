# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'Science_software.ui'
#
# Created by: PyQt5 UI code generator 5.15.4
#
# WARNING: Any manual changes made to this file will be lost when pyuic5 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt5 import QtCore, QtGui, QtWidgets
from pyqtgraph import PlotWidget, plot
import pyqtgraph as pg
import sys  # We need sys so that we can pass argv to QApplication
import os
from random import randint

import rospy
from geometry_msgs.msg import Point


class Ui_MainWindow(object):

    def setupUi(self, MainWindow):

        self.spectrometer_x = [0]  # 100 time points
        self.spectrometer_y = [0]  # 100 data points
        self.temp_x = [0]
        self.temp_y = [0]
        self.co2_x = [0]
        self.co2_y = [0]
        self.voc_x = [0]
        self.voc_y = [0]

        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(960, 1080)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.tabWidget = QtWidgets.QTabWidget(self.centralwidget)
        self.tabWidget.setGeometry(QtCore.QRect(0, 0, 960, 1080))
        self.tabWidget.setObjectName("tabWidget")
        self.tab = QtWidgets.QWidget()
        self.tab.setObjectName("tab")

        #self.centralwidget = QtWidgets.QWidget(self.tab)
        #self.centralwidget.setObjectName("centralwidget")
        #setting up graph for spectrometer data in tab 1
        self.graphicsView = PlotWidget(self.tab)
        self.graphicsView.setGeometry(QtCore.QRect(10, 30, 900, 400))
        self.graphicsView.setObjectName("graphicsView")
        self.graphicsView.setBackground('w')
        pen = pg.mkPen(color=(255, 0, 0))
        self.spectrometer_data_line =  self.graphicsView.plot(self.spectrometer_x, self.spectrometer_y, pen=pen)

        self.lcdNumber = QtWidgets.QLCDNumber(self.tab)
        self.lcdNumber.setGeometry(QtCore.QRect(10, 490, 131, 51))
        self.lcdNumber.setObjectName("lcdNumber")
        self.lcdNumber_2 = QtWidgets.QLCDNumber(self.tab)
        self.lcdNumber_2.setGeometry(QtCore.QRect(180, 490, 131, 51))
        self.lcdNumber_2.setObjectName("lcdNumber_2")
        self.lcdNumber_3 = QtWidgets.QLCDNumber(self.tab)
        self.lcdNumber_3.setGeometry(QtCore.QRect(350, 490, 131, 51))
        self.lcdNumber_3.setObjectName("lcdNumber_3")
        self.tabWidget.addTab(self.tab, "")

        self.tab_2 = QtWidgets.QWidget()
        self.tab_2.setObjectName("tab_2")

        #setting up graphs for tab 2
        self.graphicsView = PlotWidget(self.tab_2)
        self.graphicsView.setGeometry(QtCore.QRect(10, 30, 900, 230))
        self.graphicsView.setObjectName("graphicsView")
        self.graphicsView.setBackground('w')
        pen = pg.mkPen(color=(255, 0, 0))
        self.temp_data_line =  self.graphicsView.plot(self.temp_x, self.temp_y, pen=pen)

        self.graphicsView = PlotWidget(self.tab_2)
        self.graphicsView.setGeometry(QtCore.QRect(10, 260, 900, 230))
        self.graphicsView.setObjectName("graphicsView")
        self.graphicsView.setBackground('w')
        pen = pg.mkPen(color=(255, 0, 0))
        self.co2_data_line =  self.graphicsView.plot(self.co2_x, self.co2_y, pen=pen)

        self.graphicsView = PlotWidget(self.tab_2)
        self.graphicsView.setGeometry(QtCore.QRect(10, 520, 900, 230))
        self.graphicsView.setObjectName("graphicsView")
        self.graphicsView.setBackground('w')
        pen = pg.mkPen(color=(255, 0, 0))
        self.voc_data_line =  self.graphicsView.plot(self.voc_x, self.voc_y, pen=pen)

        #finishing setup for tab 2
        self.tabWidget.addTab(self.tab_2, "")

        MainWindow.setCentralWidget(self.centralwidget)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

        rospy.init_node('listener', anonymous=True)

        rospy.Subscriber("science_sensor/temp", Point, spectrometer_callback)
        rospy.Subscriber("science_sensor/temp", Point, temp_callback)
        rospy.Subscriber("science_sensor/temp", Point, co2_callback)
        rospy.Subscriber("science_sensor/temp", Point, voc_callback)

        self.timer = QtCore.QTimer()
        self.timer.setInterval(200)
        self.timer.timeout.connect(self.update_data)
        self.timer.start()

    

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), _translate("MainWindow", "Spectrometer/Soil Probes"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), _translate("MainWindow", "Other"))

    def update_data(self):
        self.spectrometer_data_line.setData(self.spectrometer_x, self.spectrometer_y)  # Update the data.
        self.temp_data_line.setData(self.temp_x, self.temp_y)
        self.co2_data_line.setData(self.co2_x, self.co2_y)
        self.voc_data_line.setData(self.voc_x, self.voc_y)


        self.lcdNumber.display(self.spectrometer_y[-1])
        self.lcdNumber_2.display(self.spectrometer_y[-1])
        self.lcdNumber_3.display(self.spectrometer_y[-1])

def spectrometer_callback(data):
        #rospy.loginfo(rospy.get_caller_id() + ',' + str(data.y))
        #ui.x = ui.x[1:]  # Remove the first x element.
        ui.spectrometer_x.append(ui.spectrometer_x[-1] + 1)  # Add a new value 1 higher than the last.

        #ui.y = ui.y[1:]  # Remove the first  y element.
        ui.spectrometer_y.append(data.y)  # Add a new value.

        #ui.data_line.setData(ui.x, ui.y)

def temp_callback(data):
        #ui.x = ui.x[1:]  # Remove the first x element.
        ui.temp_x.append(ui.temp_x[-1] + 1)  # Add a new value 1 higher than the last.

        #ui.y = ui.y[1:]  # Remove the first  y element.
        ui.temp_y.append(data.y)  # Add a new value.

def co2_callback(data):
        #ui.x = ui.x[1:]  # Remove the first x element.
        ui.co2_x.append(ui.co2_x[-1] + 1)  # Add a new value 1 higher than the last.

        #ui.y = ui.y[1:]  # Remove the first  y element.
        ui.co2_y.append(data.y)  # Add a new value.

def voc_callback(data):
        #ui.x = ui.x[1:]  # Remove the first x element.
        ui.voc_x.append(ui.voc_x[-1] + 1)  # Add a new value 1 higher than the last.

        #ui.y = ui.y[1:]  # Remove the first  y element.
        ui.voc_y.append(data.y)  # Add a new value.

if __name__ == "__main__":
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    sys.exit(app.exec_())