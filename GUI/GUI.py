import os, sys, time, pandas as pd, openpyxl, threading
sys.path.append("..")
from MachineMotion import *
from PyQt6 import QtWidgets
from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import *

global stop_exec
global process_complete
global state
state = 'NC' # for now change this to have files renamed with the state you are in

class mainWindow(QWidget):
    def __init__(self):
        super().__init__()
        
        l1 = QLabel("WELCOME!")
        l2 = QLabel("Do you want to make changes to the species list?")
        btnyes = QPushButton('YES', self)
        btnyes.clicked.connect(self.optyes)
        btnno = QPushButton('NO', self)
        btnno.clicked.connect(self.optno)

        qbtn = QPushButton('Quit')
        qbtn.clicked.connect(QApplication.instance().quit)
        qbtn.resize(qbtn.sizeHint())
        qbtn.move(330, 260)
        
        grid = QGridLayout()
        grid.setSpacing(10)
        
        grid.addWidget(QLabel("            "), 0, 0)
        grid.addWidget(l1, 0, 2)
        grid.addWidget(l2, 2, 1, 1, 3)
        grid.addWidget(btnyes, 3, 1)
        grid.addWidget(btnno, 3, 3)
        grid.addWidget(qbtn, 5, 4)

        self.setLayout(grid)
        
        self.setGeometry(100, 100, 600, 500)
        self.setWindowTitle('BenchBot')
        self.show()

    def optyes(self):
        mwin.setCurrentIndex(mwin.currentIndex()+1)
    
    def optno(self):
        page2 = Page2()
        mwin.addWidget(page2)
        mwin.setCurrentIndex(mwin.currentIndex()+2)

class Page1(QWidget):
    def __init__(self):
        super().__init__()

        mainl = QLabel("Number of species to add/update?")
        self.total_species = QComboBox()
        self.species_list = [None]*10
        self.row_list = [None]*10
        for i in range(1,10):
            self.total_species.addItem(str(i))
            self.species_list[i-1] = QLineEdit()
            self.row_list[i-1] = QLineEdit()
        self.total_species.currentIndexChanged.connect(self.updateList)
        self.earlier_cnt = 1
        
        self.l1 = QLabel("Name of Species")
        self.l2 = QLabel("Row Numbers")
        self.btn1 = QPushButton('UPDATE', self)
        self.btn1.clicked.connect(self.updateExcel)
        self.btn1.setEnabled(False)

        self.layout = QGridLayout()
        self.layout.addWidget(mainl, 0, 1, 1, 2)
        self.layout.addWidget(self.total_species, 0, 3)
        self.layout.addWidget(self.btn1, 11, 4)

        self.setLayout(self.layout)
        self.setGeometry(100, 100, 600, 500)
        self.setWindowTitle('BenchBot')

    def updateList(self):
        count = int(self.total_species.currentText())
        self.layout.addWidget(self.l1, 1, 2)
        self.layout.addWidget(self.l2, 1, 3)
        if(count>self.earlier_cnt):
            for c in range(self.earlier_cnt-1,count):
                self.layout.addWidget(self.species_list[c], c+2, 2)
                self.layout.addWidget(self.row_list[c], c+2, 3)
        if(count<self.earlier_cnt):
            for c in range(count,self.earlier_cnt):
                self.layout.removeWidget(self.species_list[c])
                self.layout.removeWidget(self.row_list[c])

        self.btn1.setEnabled(True)
        self.earlier_cnt = count
        # issues with too much variance

    def updateExcel(self):
        count = int(self.total_species.currentText())
        wb = openpyxl.load_workbook('SpeciesSheet.xlsx')
        sheet = wb.active
        for c in range(0,count):
            sheet.cell(row = c+2, column = 1).value = self.species_list[c].text()
            sheet.cell(row = c+2, column = 2).value = self.row_list[c].text()
        wb.save('SpeciesSheet.xlsx')
        os.system("python3 sheetupdateSpecies.py")
        #time.sleep(0.1)
        
        page2 = Page2()
        mwin.addWidget(page2)
        mwin.setCurrentIndex(mwin.currentIndex()+1)


class Page2(QWidget):
    def __init__(self):
        super().__init__()

        l1 = QLabel("Species")
        l2 = QLabel("Number of Images to Capture")
        btn = QPushButton('NEXT', self)
        btn.clicked.connect(self.confirmmsg)
        
        self.layout = QGridLayout()
        self.layout.addWidget(QLabel("            "), 0, 0)
        self.layout.addWidget(l1, 0, 1)
        self.layout.addWidget(l2, 0, 2)
        self.layout.addWidget(btn, 10, 3)

        df  = pd.read_excel('SpeciesSheet.xlsx')
        species_names = df[['SpeciesName']].values
        i = 0
        self.snaps = [None] * species_names.size
        for sname in species_names:
            self.layout.addWidget(QLabel(sname[0]), i+2, 1)
            self.snaps[i] = QLineEdit()
            self.layout.addWidget(self.snaps[i], i+2, 2)
            i += 1

        self.setLayout(self.layout)
        self.setGeometry(100, 100, 600, 500)
        self.setWindowTitle('BenchBot')

    def confirmmsg(self):
        dlg = QMessageBox(self)
        dlg.setWindowTitle("Confirm")
        dlg.setText("Are you sure you want to begin the image acquisition process?")
        dlg.setStandardButtons(QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
        dlg.setIcon(QMessageBox.Icon.Question)
        button = dlg.exec()

        if button == QMessageBox.StandardButton.Yes:      
            wb = openpyxl.load_workbook('SpeciesSheet.xlsx')
            sheet = wb.active
            for c in range(0,len(self.snaps)):
                sheet.cell(row = c+2, column = 3).value = self.snaps[c].text()
            wb.save('SpeciesSheet.xlsx')
            os.system("python3 sheetupdatePictures.py")
            page3 = Page3()
            mwin.addWidget(page3)
            mwin.setCurrentIndex(mwin.currentIndex()+1)
            dlg.done(1)
        else:
            dlg.done(1)


class Page3(QWidget):
    def __init__(self):
        super().__init__()
        
        self.mm = MachineMotion(DEFAULT_IP_ADDRESS.usb_windows)
        self.camMotor = 1

        self.l1 = QLabel("Image Acquisition Process Started")
        self.l2 = QLabel("     Time Elapsed: 0 hrs 0 mins")
        self.sbtn = QPushButton('CANCEL', self)
        self.sbtn.clicked.connect(self.stop_thread)
        
        self.layout = QGridLayout()
        self.layout.addWidget(QLabel("            "), 0, 0)
        self.layout.addWidget(self.l1, 0, 1)
        self.layout.addWidget(self.l2, 1, 1)
        self.layout.addWidget(self.sbtn, 4, 1)
        self.layout.addWidget(QLabel("            "), 3, 2)
        
        self.setLayout(self.layout)
        self.setGeometry(100, 100, 600, 500)
        self.setWindowTitle('BenchBot')
        
        self.start_time = time.time()
        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_label)
        self.timer.start(60000)  # every 1 min
        
        threading.Thread(target=self.acquisition_process).start()
    
    def acquisition_process(self):
        global stop_exec
        stop_exec = False
        global process_complete
        process_complete = False
        self.mm.configAxis(self.camMotor, MICRO_STEPS.ustep_8, MECH_GAIN.ballscrew_10mm_turn)
        self.mm.configAxisDirection(self.camMotor, DIRECTION.POSITIVE)

        wheelMotors = [2,3]
        directions = ["positive","negative"]
        for axis in wheelMotors:
            self.mm.configAxis(axis, MICRO_STEPS.ustep_8, MECH_GAIN.enclosed_timing_belt_mm_turn)
            self.mm.configAxisDirection(axis, directions[axis-2])
        self.mm.emitAcceleration(50)
        self.mm.emitSpeed(80)
        path = os.getcwd()+'\\out\\build\\x64-Debug\\RemoteCli.exe'
        
        df  = pd.read_excel('ImagesSheet.xlsx')
        colvalues = df[['ImagesCount']].values
        i = 0
        pots = [None] * colvalues.size
        for num in colvalues:
            pots[i] = num[0]
            i = i+1
        
        self.mm.releaseEstop()
        self.mm.resetSystem()
        os.chdir(os.getcwd()+'\\Images')
        
        dist = 300 #distance between rows of pots
        positions = [dist, dist] #position variable
        row = 1
        for j in pots:
            if j==0:
                if stop_exec:
                    break
                self.mm.moveRelativeCombined(wheelMotors, positions)
                self.mm.waitForMotionCompletion()
                row += 1
                continue
            else:
                c = int(600/(j-1)) #total distance between home and end sensor
                for i in range(1,j):
                    if stop_exec:
                        break
                    #Trigger capture of image
                    os.startfile(path)
                    time.sleep(15)
                    threading.Thread(target=filerename(row)).start()
                    #Move camera plate to next point
                    self.mm.moveRelative(self.camMotor, c)
                    self.mm.waitForMotionCompletion()
                if stop_exec:
                    break
                #Trigger image capture at last point
                os.startfile(path)
                time.sleep(15)
                threading.Thread(target=filerename(row)).start()
                #move camera plate to start location
                self.mm.moveToHome(self.camMotor)
                self.mm.waitForMotionCompletion()
            # move the bot to next set of pots
            self.mm.moveRelativeCombined(wheelMotors, positions)
            self.mm.waitForMotionCompletion()
            row += 1

        if stop_exec:
            self.stop
        else:
            process_complete = True
            self.mm.triggerEstop()
            self.l1.setText('Acquisition Process Complete')
            current_time = time.time()
            elapsed_time = int(current_time-self.start_time)
            elapsed_hr = int(elapsed_time/3600)
            elapsed_min = int(elapsed_time/60) - 60*elapsed_hr
            self.l2.setText('Total time taken: '+str(elapsed_hr)+ ' hrs '+str(elapsed_min)+ ' mins')
        
    def stop_thread(self):
        stopping = threading.Thread(target=self.stop)
        stopping.start()
        
    def stop(self):
        global stop_exec
        global process_complete
        stop_exec = True
        self.mm.moveToHome(self.camMotor)
        self.mm.waitForMotionCompletion()
        self.mm.triggerEstop()
        if not process_complete:
            self.l1.setText('Acquisition Process Disrupted')
            self.l2.setText('     Close the Application')

    def update_label(self):
        global process_complete
        global stop_exec
        if process_complete or stop_exec:
            self.timer.stop()
        else:
            current_time = time.time()
            elapsed_time = int(current_time-self.start_time)
            elapsed_hr = int(elapsed_time/3600)
            elapsed_min = int(elapsed_time/60) - 60*elapsed_hr
            self.l2.setText('     Time Elapsed: '+str(elapsed_hr)+ ' hrs '+str(elapsed_min)+ ' mins')


def filerename(pot):
    global state
    time.sleep(2)
    t = str(int(time.time()))
    for fname in os.listdir('.'):
        if fname.startswith('DSC'):
            if fname.endswith('.JPG'):
                dst = f"{state}_Row-{str(pot)}_{t}.JPG" 
            elif fname.endswith('.ARW'):
                dst = f"{state}_Row-{str(pot)}_{t}.ARW"
        else:
            continue
        os.rename(f"{fname}", f"{dst}")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    mwin = QtWidgets.QStackedWidget()
    win = mainWindow()
    page1 = Page1()
    mwin.addWidget(win)
    mwin.addWidget(page1)
    mwin.setFixedHeight(400)
    mwin.setFixedWidth(500)
    mwin.show()
    sys.exit(app.exec())
