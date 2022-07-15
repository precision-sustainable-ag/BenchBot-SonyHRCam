lenRobot = 133  # Length of the robot (between front and back sensors) measured in cm
widRobot = 215 # Width of the robot (distance between two front wheels) in cm
pi_username = 'benchbot' # if you haven't changed your pi's name the default username is 'pi'
pi_password = 'bb-semi-field' # if you haven't changed your pi's password, the default is 'raspberrypi'
distTravel = 300
wheelMotors = [2,3] # if BB moving backwards change the motors order to [3,2]

import os, sys, time, pandas as pd, openpyxl, threading
import string, shutil, datetime, paramiko, socket, numpy as np
from datetime import date
sys.path.append("..")
from MachineMotion import *
from PyQt6 import QtWidgets
from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import *
from PyQt6.QtGui import QIntValidator

HOST = "192.168.7.3"  # The server's hostname or IP address
PORT = 65432  # The port used by the server

global stop_exec
global process_complete
global state

############################### Ultrasonic sensors and laptop-pi communication functions ###############################

# Helper function for getting distance measurements from sensor
def getDistances(s, offsets):
    # Number of queries of sensor measurements
    N = 3

    dist_list = np.zeros((N, 6))
    dist_raw = np.zeros((N, 6))

    for k in range(0, N):
        # Sending a request for data and receiving it
        s.sendall(b" ")
        data = s.recv(1024)
        time.sleep(0.25)

        # Parsing the measurements
        for i, item in enumerate(data.split()):
            dist_raw[k, i] = float(item)
            dist_list[k, i] = float(item) - offsets[i % 3]
    # print(dist_raw)
    # print(dist_list)
    dist_list = np.median(dist_list, 0)

    return dist_list


# Helper function for computing orientation of robot
def findOrientation(Dist, lenRobot):
    '''
    :param Dist: np array. Average of N distances measured by the ultrasonic sensors. Array[0]=
    front right sensor, Array[1]= back right sensor.
    :param lenRobot: distance in cm from front right to back right sensor
    :return: angle of drift from straight trajectory
    '''
    print('Dist[0]=', Dist[0])
    print('Dist[1]=', Dist[1])
    theta=np.arctan2((Dist[0]-Dist[1]), lenRobot)*180/np.pi
    return theta

############################ End Ultrasonic sensors and laptop-pi communication functions #############################

########################################## Establish connection with pi #############################################

## INITIALIZING SERVER IN RPI

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(HOST, username=pi_username, password=pi_password)
ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command('python /home/benchbot/ultrasonic_calibration/RPI_ServerSensors.py &')
time.sleep(2)


## CONNECTING TO RPI SERVER

# Setting up the connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))


## LOADING OFFSETS
offsets = np.loadtxt('SensorOffsets.csv', delimiter=',', skiprows = 1)# delimiter added

print(offsets)

########################################## End Establish connection with pi ###########################################

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
        l3 = QLabel('State')
        self.statebox = QComboBox()
        self.statebox.addItem('NC')
        self.statebox.addItem('TX')
        self.statebox.addItem('MD')
        
        grid = QGridLayout()
        grid.setSpacing(10)
        
        grid.addWidget(QLabel("            "), 0, 0)
        grid.addWidget(l1, 1, 3)
        grid.addWidget(l2, 2, 2, 1, 3)
        grid.addWidget(l3, 0, 4)
        grid.addWidget(self.statebox, 0, 5)
        grid.addWidget(btnyes, 3, 2)
        grid.addWidget(btnno, 3, 4)
        grid.addWidget(qbtn, 5, 5)

        self.setLayout(grid)
        
        self.setGeometry(100, 100, 600, 500)
        self.setWindowTitle('BenchBot')
        self.show()

    def optyes(self):
        global state
        state = self.statebox.currentText()
        mwin.setCurrentIndex(mwin.currentIndex()+1)
    
    def optno(self):
        global state
        state = self.statebox.currentText()
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
        self.btn1.clicked.connect(self.checkCells)
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
		
    def checkCells(self):
        ## logic for checking contents of the fields before proceeding
        err_code = 0
        count = int(self.total_species.currentText())
        allowed_chars1=['1','2','3','4','5','6','7','8','9','0',',','-']
        allowed_chars2=list(string.ascii_letters)
        allowed_chars2.append(' ')

        for c in range(0,count):
            input_text1 = self.row_list[c].text()
            input_text2 = self.species_list[c].text()
            if input_text1=='' or input_text2=='':
                err_code = 1
            elif any(x not in allowed_chars1 for x in input_text1):
                err_code = 2
            elif any(x not in allowed_chars2 for x in input_text2):
                err_code = 3
            else:
                continue
        
        dlg = QMessageBox(self)
        dlg.setWindowTitle("Error")
        if err_code!=0:
            if err_code == 1:
                dlg.setText("Make sure all fields have entries")
            elif err_code==2:
                dlg.setText("Enter row data in following format: '2' or '1-3' or '1,5,8' or '2-3,5'")
            else:
                dlg.setText("Make sure Species names only contain alphabets")
            dlg.setStandardButtons(QMessageBox.StandardButton.Close)
            if dlg.exec() == QMessageBox.StandardButton.Close:      
                dlg.done(1)
        else:
            self.updateExcel()

    def updateExcel(self):
        count = int(self.total_species.currentText())
        wb = openpyxl.load_workbook('SpeciesSheet.xlsx')
        sheet = wb.active
        for c in range(0,count):
            sheet.cell(row = c+2, column = 1).value = self.species_list[c].text()
            sheet.cell(row = c+2, column = 2).value = self.row_list[c].text()
        for c in range(count,9):
            sheet.cell(row = c+2, column = 1).value = ''
            sheet.cell(row = c+2, column = 2).value = ''
            sheet.cell(row = c+2, column = 3).value = ''
        wb.save('SpeciesSheet.xlsx')
        os.system("python sheetupdateSpecies.py")
        #time.sleep(0.1)
        threading.Thread(target=backupSheet).start()
        
        page2 = Page2()
        mwin.addWidget(page2)
        mwin.setCurrentIndex(mwin.currentIndex()+1)
        
def backupSheet():
    # create a copy of the sheet
    shutil.copy("SpeciesSheet.xlsx", "new.xlsx")
    
    # delete the images column
    wb = openpyxl.load_workbook('new.xlsx')
    sheet = wb.active
    sheet.delete_cols(3, 1)
    wb.save('new.xlsx')

    # include date in file name
    today = datetime.datetime.today().strftime ('%d-%b-%Y')
    os.rename(r'new.xlsx',r'SpeciesSheet_' + state + str(today) + '.xlsx')


class Page2(QWidget):
    def __init__(self):
        super().__init__()

        l1 = QLabel("Species")
        l2 = QLabel("Number of Images to Capture")
        btn = QPushButton('NEXT', self)
        btn.clicked.connect(self.checkContent)
        self.onlyInt = QIntValidator(0,8,self)
        
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
            self.snaps[i].setValidator(self.onlyInt)
            self.layout.addWidget(self.snaps[i], i+2, 2)
            i += 1

        self.setLayout(self.layout)
        self.setGeometry(100, 100, 600, 500)
        self.setWindowTitle('BenchBot')
		
    def checkContent(self):
        err_code = 0
        for c in range(0,len(self.snaps)):
            input_text = self.snaps[c].text()
            if input_text=='':
                err_code = 1
            else:
                continue

        if err_code!=0:
            dlg = QMessageBox(self)
            dlg.setWindowTitle("Error")
            dlg.setText("Make sure all fields have entries")
            dlg.setStandardButtons(QMessageBox.StandardButton.Close)
            if dlg.exec() == QMessageBox.StandardButton.Close:      
                dlg.done(1)
        else:
            self.confirmmsg()

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
            os.system("python sheetupdatePictures.py")
            page3 = Page3()
            mwin.addWidget(page3)
            mwin.setCurrentIndex(mwin.currentIndex()+1)
            dlg.done(1)
        else:
            dlg.done(1)


class Page3(QWidget):
    def __init__(self):
        super().__init__()
        
        # Initialize the machine motion object
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

        directions = ["negative", "positive"]
        for axis in wheelMotors:
            self.mm.configAxis(axis, MICRO_STEPS.ustep_8, MECH_GAIN.enclosed_timing_belt_mm_turn)
            self.mm.configAxisDirection(axis, directions[axis-2])
        self.mm.emitAcceleration(50)
        self.mm.emitSpeed(80)
        path = os.getcwd()+'\\RemoteCli\\RemoteCli.exe'

        df  = pd.read_excel('ImagesSheet.xlsx')
        colvalues = df[['ImagesCount']].values
        pots = []
        for num in colvalues:
            pots.append(num[0])
        
        self.mm.releaseEstop()
        self.mm.resetSystem()
        # Create new directory where today's images will be saved.
        dirName=f'{state}_{date.today()}'
        if dirName not in os.listdir():
            os.mkdir(f'{os.getcwd()}\{dirName}')
        os.chdir(f'{os.getcwd()}\{dirName}')
            
        direction = True
        
        for j in pots:
            if j==0:
                if stop_exec:
                    break
                dist_corrected = getDistances(s, offsets)

                # Getting angle
                ang = findOrientation(dist_corrected, lenRobot)
                print('debug_angle=', ang)

                #Calculate how much distance motor needs to move to align platform
                d_correction_mm = 2*np.pi*widRobot*(abs(ang)/360)*10
                print('debug_d_correction_mm=', d_correction_mm)
                #Create if statement to indicate which motor moves

                if ang > 0.5:
                    self.mm.moveRelative(wheelMotors[1], d_correction_mm)
                    self.mm.waitForMotionCompletion()
                elif ang < -0.5:
                    self.mm.moveRelative(wheelMotors[0], d_correction_mm)
                    self.mm.waitForMotionCompletion()

                self.mm.moveRelativeCombined(wheelMotors, [distTravel, distTravel])
                self.mm.waitForMotionCompletion()
                continue
            else:
                dist_corrected = getDistances(s, offsets)

                # Getting angle
                ang = findOrientation(dist_corrected, lenRobot)
                print('debug_angle=', ang)

                #Calculate how much distance motor needs to move to align platform
                d_correction_mm = 2*np.pi*widRobot*(abs(ang)/360)*10
                print('debug_d_correction_mm=', d_correction_mm)
                #Create if statement to indicate which motor moves

                if ang > 0.5:
                    self.mm.moveRelative(wheelMotors[1], d_correction_mm)
                    self.mm.waitForMotionCompletion()
                elif ang < -0.5:
                    self.mm.moveRelative(wheelMotors[0], d_correction_mm)
                    self.mm.waitForMotionCompletion()
                
                ct = int(1000/(j-1))
                    
                if direction:
                    c = ct
                    direction = False
                else:
                    c = -ct
                    direction = True

                for i in range(1,j):
                    if stop_exec:
                        break
                    #Trigger capture of image
                    os.startfile(path)
                    time.sleep(15)
                    threading.Thread(target=filerename()).start()
                    #Move camera plate to next point
                    self.mm.moveRelative(self.camMotor, c)
                    self.mm.waitForMotionCompletion()
                if stop_exec:
                    break
                #Trigger image capture at last point
                os.startfile(path)
                time.sleep(15)
                threading.Thread(target=filerename()).start()
                
                self.mm.moveRelativeCombined(wheelMotors, [distTravel, distTravel])
                self.mm.waitForMotionCompletion()

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

def filerename():
    global state
    time.sleep(2)
    t = str(int(time.time()))
    for fname in os.listdir('.'):
        if fname.startswith(state+'X'):
            if fname.endswith('.JPG'):
                dst = f"{state}_{t}.JPG" 
            elif fname.endswith('.ARW'):
                dst = f"{state}_{t}.ARW"
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
