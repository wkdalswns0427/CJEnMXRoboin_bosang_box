import sys, os
from PyQt5 import QtCore
from PyQt5.QtWidgets import *
import numpy as np
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
import matplotlib.animation as animation
import time
from threading import Timer


class MyMplCanvas(FigureCanvas):
    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig = Figure(figsize=(width, height), dpi=dpi)
        
        self.axes = fig.add_subplot(111, xlim=(0, 100), ylim=(0, 200))

        self.compute_initial_figure()
        FigureCanvas.__init__(self, fig)
        self.setParent(parent)
    def compute_initial_figure(self):
        pass
class AnimationWidget(QWidget):
    def __init__(self):
        QMainWindow.__init__(self)
        vbox = QVBoxLayout()
        self.canvas = MyMplCanvas(self, width=10, height=8, dpi=100)
        self.stepNumber = QLCDNumber(self)
        vbox.addWidget(self.canvas)
        vbox.addWidget(self.stepNumber)
        hbox = QHBoxLayout()
        self.start_button = QPushButton("start", self)
        self.stop_button = QPushButton("stop", self)
        self.start_button.clicked.connect(self.on_start)
        self.stop_button.clicked.connect(self.on_stop)
        hbox.addWidget(self.start_button)
        hbox.addWidget(self.stop_button)
        vbox.addLayout(hbox)
        self.setLayout(vbox)

        self.x = np.arange(100)
        self.y = np.ones(100, dtype=np.float)*np.nan
        self.line, = self.canvas.axes.plot(self.x, self.y, animated=True, lw=2)
        
        self.showStepNum()

        
    def update_line(self, i):
        file = open("BPM_output.txt", 'r')
        data = file.read()
        try:
            y = float(data)
            old_y = self.line.get_ydata()
            new_y = np.r_[old_y[1:], y]
            self.line.set_ydata(new_y)
            
        except:
            pass
        
        return [self.line]
        # self.line.set_ydata(y)
                

            
    def on_start(self):
        self.ani = animation.FuncAnimation(self.canvas.figure, self.update_line,blit=True, interval=25)
            
    def on_stop(self):
        self.ani._stop()
        
    def showStepNum(self):
        file = open("STEP_output.txt", 'r')
        data = file.read()
        print(data)
        try:
            step = int(data)
            self.stepNumber.display(step)
        except:
            pass
        timer = Timer(1, self.showStepNum)
        timer.start()


if __name__ == "__main__":
    qApp = QApplication(sys.argv)
    aw = AnimationWidget()
    aw.show()
    sys.exit(qApp.exec_())