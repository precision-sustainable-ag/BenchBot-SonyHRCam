import os, sys, time, pandas as pd, openpyxl, threading
sys.path.append("..")
from MachineMotion import *
from PyQt6 import QtWidgets
from PyQt6.QtCore import QTimer
from PyQt6.QtWidgets import *
import paramiko
import socket
import numpy as np
from datetime import date
import select
import json

# Before getting started, some things that you may need to change on the script
# line 25 change your STATE to 'MD', 'NC' or 'TX'
# Function def file_rename(pot), change if fname.startswith('MDX'):, ('MDX') to 'NCX' ot 'TXX' 

STATE = 'NC' # for now change this to have files renamed with the STATE you are in
ROBOT_LENGTH = 133  # Length of the robot (between front and back sensors) measured in cm
ROBOT_WIDTH =218 # Width of the robot (distance between two front wheels) in cm
PI_USERNAME = 'pi' # if you haven't changed your pi's name the default username is 'pi'
PI_PASSWORD = 'raspberry' # if you haven't changed your pi's password, the default is 'raspberrypi'
DISTANCE_TRAVELED = 300
WHEEL_MOTORS = [2, 3] # if BB moving backwards change the motors order to [3,2]
NUMBER_OF_SENSORS = 2
GPIO_TRIGGER_LIST = [18, 17]
ULTRASONIC_SENSOR_LISTS = json.dumps({
    "triggers": [18, 17],
    "echoes": [23, 4],
})
DIRECTIONS = ["positive","negative"]



HOST = "192.168.7.3"  # The server's hostname or IP address
PORT = 65432  # The port used by the server

global STOP_EXEC
global PROCESS_COMPLETE
#global STATE

############################### Ultrasonic sensors and laptop-pi communication functions ###############################

# Helper function for getting disntance measurements from sensor
def get_distances(s, offsets):
    dist_list = np.zeros((NUMBER_OF_SENSORS, NUMBER_OF_SENSORS))
    dist_raw = np.zeros((NUMBER_OF_SENSORS, NUMBER_OF_SENSORS))

    for k in range(0, NUMBER_OF_SENSORS):
        # Sending a request for data and receiving it
        s.sendall(bytes(ULTRASONIC_SENSOR_LISTS,encoding="utf-8"))
        data = s.recv(1024)

        # use timeout of 10 seconds to wait for sensor response
        ready = select.select([s], [], [], 10)
        if ready[0]:
            data = s.recv(4096)
        else:
            print("sensor error!")

        time.sleep(0.25)

        # Parsing the measurements
        for i, item in enumerate(data.split()):
            dist_raw[k, i] = float(item)
            dist_list[k, i] = float(item) - offsets[i % 3]

    dist_list = np.median(dist_list, 0)

    return dist_list


# Helper function for computing orientation of robot
def find_orientation(Dist, ROBOT_LENGTH):
    '''
    :param Dist: np array. Average of N distances measured by the ultrasonic sensors. Array[0]=
    front right sensor, Array[1]= back right sensor.
    :param ROBOT_LENGTH: distance in cm from front right to back right sensor
    :return: angle of drift from straight trajectory
    '''
    theta=np.arctan2((Dist[0]-Dist[1]), ROBOT_LENGTH)*180/np.pi
    return theta


############################ End Ultrasonic sensors and laptop-pi communication functions #############################

########################################## Establish connection with pi #############################################

## INITIALIZING SERVER IN RPI

ssh = paramiko.SSHClient()
ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
ssh.connect(HOST, username=PI_USERNAME, password=PI_PASSWORD)
ssh_stdin, ssh_stdout, ssh_stderr = ssh.exec_command('python /home/{}/ultrasonic_calibration/RPI_ServerSensors.py &'.format(PI_USERNAME))
time.sleep(5)


## CONNECTING TO RPI SERVER

# Setting up the connection
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.setblocking(0)


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
        for i in range(1, 10):
            self.total_species.addItem(str(i))
            self.species_list[i-1] = QLineEdit()
            self.row_list[i-1] = QLineEdit()
        self.total_species.currentIndexChanged.connect(self.update_list)
        self.earlier_cnt = 1
        
        self.l1 = QLabel("Name of Species")
        self.l2 = QLabel("Row Numbers")
        self.btn1 = QPushButton('UPDATE', self)
        self.btn1.clicked.connect(self.update_excel)
        self.btn1.setEnabled(False)

        self.layout = QGridLayout()
        self.layout.addWidget(mainl, 0, 1, 1, 2)
        self.layout.addWidget(self.total_species, 0, 3)
        self.layout.addWidget(self.btn1, 11, 4)

        self.setLayout(self.layout)
        self.setGeometry(100, 100, 600, 500)
        self.setWindowTitle('BenchBot')

    def update_list(self):
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

    def update_excel(self):
        count = int(self.total_species.currentText())
        wb = openpyxl.load_workbook('SpeciesSheet.xlsx')
        sheet = wb.active
        for c in range(0,count):
            sheet.cell(row = c+2, column = 1).value = self.species_list[c].text()
            sheet.cell(row = c+2, column = 2).value = self.row_list[c].text()
        wb.save('SpeciesSheet.xlsx')
        os.system("python sheetupdateSpecies.py")
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
        btn.clicked.connect(self.confirm_message)
        
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

    def confirm_message(self):
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
        self.mm = MachineMotion(DEFAULT_IP)
        print('mm initialized')

        # Remove the software stop
        print("--> Removing software stop")
        self.mm.releaseEstop()
        print("--> Resetting system")
        self.mm.resetSystem()
        print("successfully created mm object")
                
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
        print("entered acquisition")
        global STOP_EXEC
        STOP_EXEC = False
        global PROCESS_COMPLETE
        PROCESS_COMPLETE = False

        self.mm.configAxis(self.camMotor, MICRO_STEPS.ustep_8, MECH_GAIN.ballscrew_10mm_turn)
        self.mm.configAxisDirection(self.camMotor, DIRECTION.POSITIVE)

        # DIRECTIONS = ["negative","positive"]
        for axis in WHEEL_MOTORS:
            self.mm.configAxis(axis, MICRO_STEPS.ustep_8, MECH_GAIN.enclosed_timing_belt_mm_turn)
            self.mm.configAxisDirection(axis, DIRECTIONS[axis-2])
        self.mm.emitAcceleration(50)
        self.mm.emitSpeed(80)
        path = os.getcwd()+'\\RemoteCli\\RemoteCli.exe'
        
        df  = pd.read_excel('ImagesSheet.xlsx')
        colvalues = df[['ImagesCount']].values
        i = 0
        pots = [None] * colvalues.size
        for num in colvalues:
            print(pots[i])
            print(num)
            pots[i] = num[0]
            i = i+1
        
        # self.mm.releaseEstop()
        # self.mm.resetSystem()

        # Create new directory where today's images will be saved.
        dirName=f'{STATE}_{date.today()}'
        if dirName in os.listdir():
            os.chdir(f'{os.getcwd()}\{dirName}')
            print('Folder already created')
        else:
            os.mkdir(f'{os.getcwd()}\{dirName}')
            os.chdir(f'{os.getcwd()}\{dirName}')
            
        row = 1
        print("starting for loop")
        print(pots)
        for j in pots:
            print(j)
            if j==0:
                print("j = 0")
                if STOP_EXEC:
                    break
                dist_corrected = get_distances(s, offsets)

                # Getting angle
                ang = find_orientation(dist_corrected, ROBOT_LENGTH)
                print('debug_angle=', ang)

                # ang = 0.5

                #Calculate how much distance motor needs to move to align platform
                d_correction_mm = 2*np.pi*ROBOT_WIDTH*(abs(ang)/360)*10
                print('debug_d_correction_mm=', d_correction_mm)
                #Create if statement to indicate which motor moves

                if ang > 0.5:
                    self.mm.moveRelative(WHEEL_MOTORS[0], d_correction_mm)
                    self.mm.waitForMotionCompletion()
                elif ang < -0.5:
                    self.mm.moveRelative(WHEEL_MOTORS[1], d_correction_mm)
                    self.mm.waitForMotionCompletion()

                self.mm.moveRelativeCombined(WHEEL_MOTORS, [DISTANCE_TRAVELED, DISTANCE_TRAVELED])
                self.mm.waitForMotionCompletion()
                row += 1
                continue
            else:
                print("j not 0")
                dist_corrected = get_distances(s, offsets)
                print("finished get_distances")

                # Getting angle
                ang = find_orientation(dist_corrected, ROBOT_LENGTH)
                # ang = 0.5
                print("finished find_orientation")
                print('debug_angle=', ang)

                #Calculate how much distance motor needs to move to align platform
                d_correction_mm = 2*np.pi*ROBOT_WIDTH*(abs(ang)/360)*10
                print('debug_d_correction_mm=', d_correction_mm)
                #Create if statement to indicate which motor moves

                if ang > 0.5:
                    self.mm.moveRelative(WHEEL_MOTORS[0], d_correction_mm)
                    self.mm.waitForMotionCompletion()
                elif ang < -0.5:
                    self.mm.moveRelative(WHEEL_MOTORS[1], d_correction_mm)
                    self.mm.waitForMotionCompletion()
                
                if j == 1:
                    c = int(1000/(j)) #total distance between home and end sensor
                else:
                    c = int(1000/(j-1)) #total distance between home and end sensor

                for i in range(1,j):
                    if STOP_EXEC:
                        break
                    #Trigger capture of image
                    os.startfile(path)
                    time.sleep(15)
                    threading.Thread(target=file_rename(row)).start()
                    #Move camera plate to next point
                    self.mm.moveRelative(self.camMotor, c)
                    self.mm.waitForMotionCompletion()
                if STOP_EXEC:
                    break
                #Trigger image capture at last point
                os.startfile(path)
                time.sleep(15)
                threading.Thread(target=file_rename(row)).start()
                #move camera plate to start location
                self.mm.moveToHome(self.camMotor)
                self.mm.waitForMotionCompletion()

                self.mm.moveRelativeCombined(WHEEL_MOTORS, [DISTANCE_TRAVELED, DISTANCE_TRAVELED])
                self.mm.waitForMotionCompletion()
                row += 1

        if STOP_EXEC:
            self.stop()
        else:
            PROCESS_COMPLETE = True
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
        global STOP_EXEC
        global PROCESS_COMPLETE
        STOP_EXEC = True
        self.mm.moveToHome(self.camMotor)
        self.mm.waitForMotionCompletion()
        self.mm.triggerEstop()
        if not PROCESS_COMPLETE:
            self.l1.setText('Acquisition Process Disrupted')
            self.l2.setText('     Close the Application')

    def update_label(self):
        global PROCESS_COMPLETE
        global STOP_EXEC
        if PROCESS_COMPLETE or STOP_EXEC:
            self.timer.stop()
        else:
            current_time = time.time()
            elapsed_time = int(current_time-self.start_time)
            elapsed_hr = int(elapsed_time/3600)
            elapsed_min = int(elapsed_time/60) - 60*elapsed_hr
            self.l2.setText('     Time Elapsed: '+str(elapsed_hr)+ ' hrs '+str(elapsed_min)+ ' mins')


def file_rename(pot):
    global STATE
    #time.sleep(2)
    time.sleep(3) #changed to 10 to test since file renaming not working
    t = str(int(time.time()))
    for fname in os.listdir('.'):
        if fname.startswith('NCX'):
            if fname.endswith('.JPG'):
                dst = f"{STATE}_Row-{str(pot)}_{t}.JPG" 
            elif fname.endswith('.ARW'):
                dst = f"{STATE}_Row-{str(pot)}_{t}.ARW"
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
