import serial
import time
import threading
from datetime import datetime
from tkinter import ttk
import serial.tools.list_ports
from tkinter import font
from tkinter import *
from tkinter import font, filedialog, messagebox

arduino = None # Initialise arduino var
outputfileSelected = False
inputfileSelected = False
makeNewFile = True

# Establish a connection to the Arduino
#arduino = serial.Serial(port='COM8', baudrate=115200, timeout=.1)

#fileCreated = FALSE # create a new file when the active button is clicked
filename = datetime.now().strftime("Pneumatic_Device_" + "data_%Y%m%d_%H%M%S.txt")

# create root window
root = Tk()

root.title("Pneumatic Device - Control Panel") # root window title and dimension
root.geometry('550x400') # Set geometry(widthxheight)

def getSerialPorts(): # list of serial ports
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

def connectSerial(): # connect to serial
    global arduino  # Declare arduino as global to modify it
    selected_port = portVar.get()
    if selected_port:
        try:
            arduino = serial.Serial(selected_port, 115200, timeout=1)
            statusLabel.config(text=f"Connected to {selected_port}")
        except Exception as e:
            statusLabel.config(text=f"Failed to connect: {e}")
    else:
        statusLabel.config(text="No port selected")
        messagebox.showinfo("Information", "No port selected")

def selectOutputDirectory(): # menu to choose output folder
    global outputfileSelected
    directory = filedialog.askdirectory()
    if directory:
        outputDir.set(directory)
        outputDirLabel.config(text=f"Output Directory: {directory}")
        outputfileSelected = True # so the program can start receving

def selectInputDirectory(): # menu to choose output folder
    global inputfileSelected
    directory = filedialog.askopenfilename()
    if directory:
        inputDir.set(directory)
        inputDirLabel.config(text=f"Input Directory: {directory}")
        inputfileSelected = True # so the program can start receving

#def modeSelection():
    #modeLabel.config(text=f"Mode: {modeSelected.get()}")


def testDevice(): #activeDeviceA
    global arduino
    global makeNewFile
    global filename

    makeNewFile = True
    filename = datetime.now().strftime("Pneumatic_Device_" + "data_%Y%m%d_%H%M%S.txt")

    if arduino:  # Ensure arduino is connected
        serialtoSend = "Test:"
        print(serialtoSend)
        arduino.write(f"{serialtoSend}\n".encode('utf-8')) # write to serial
        #time.sleep(0.05) #wait a moment to receive a responce
        #dataReceived = arduino.readline().decode('utf-8').strip() #read the received serial when it's sent
        #print(dataReceived)


def uploadData():
    global arduino
    global modeSelected
    global inputfileSelected
    global inputDir

    #check mode seelct and input file selected
    if (modeSelected.get() == ""):
        messagebox.showinfo("Information", "Select a Mode")
    
    if (inputfileSelected == False):
        messagebox.showinfo("Information", "Select an Input File")

    if arduino:  # Ensure arduino is connected
        #encode the data
        with open(inputDir.get(), 'r') as file:
            inputFile = file.readlines()

        inputSequence = ":".join(line.strip() for line in inputFile)
        print(inputSequence)

    
        #print(f"There are {len(inputFile)} lines")
        serialtoSend=""

        if (modeSelected.get() == "Hand Tap"):
            serialtoSend = "Hand:"+inputSequence
        elif(modeSelected.get() == "Wrist Actuate"):
            serialtoSend = "Wrst:"+inputSequence
        else:
            print("Mode Select Error")
        
        print(serialtoSend)
        arduino.write(f"{serialtoSend}\n".encode('utf-8')) # write to serial
        time.sleep(0.05) #wait a moment to receive a responce
        dataReceived = arduino.readline().decode('utf-8').strip() #read the received serial when it's sent
        print(dataReceived)
        
    else:
        print("Device not connected!")
        messagebox.showinfo("Information", "Device Not Connected")


def startSignal():
    global arduino
    global waitMRISelected
    global outputfileSelected
    global makeNewFile
    global filename
    makeNewFile = True

    filename = datetime.now().strftime("Pneumatic_Device_" + "data_%Y%m%d_%H%M%S.txt")

    if (inputfileSelected == False):
        messagebox.showinfo("Information", "Select an Output File")

    serialtoSend=""
    if arduino:  # Ensure arduino is connected
        if(waitMRISelected.get() == "WaitMRI"):
            serialtoSend = "Strt:1"
        elif(waitMRISelected.get() == "RunNow"):
            serialtoSend = "Strt:0"
        else:
            print("MRI wait Error") 

        print(serialtoSend)
        arduino.write(f"{serialtoSend}\n".encode('utf-8')) # write to serial
        #time.sleep(0.05) #wait a moment to receive a responce
        #dataReceived = arduino.readline().decode('utf-8').strip() #read the received serial when it's sent
        #print(dataReceived)
        #if dataReceived == "alreadyRunning":
        #    print("was already running") # put in pop up
        #    #popupAlreadyRunning()
    else:
        print("Device not connected!")
        messagebox.showinfo("Information", "Device Not Connected")


def stopSignal():
    print("STOPPING")
    global arduino

    if arduino:  # Ensure arduino is connected
        serialtoSend = "STOP"
        print(serialtoSend)
        arduino.write(f"{serialtoSend}\n".encode('utf-8')) # write to serial


def readSerial():  # read serial data (once called it will recall itself every 50ms)
    global arduino
    global outputfileSelected
    global makeNewFile
    buffer = [] 
    

    while True:
        if makeNewFile == True: #make a new file if set
            newFile = 'w'
        else:
            newFile = 'a'

        if arduino and outputfileSelected:  # check arduino is connected (also if file seselected)
            print("read")
            data = arduino.readline()##.decode('utf-8')
            
            if data:
                
                print(f"Received: {data}")
                buffer.append(f"{data}\n")  # Add data to buffer
                #print(f"Received: {data}")
                #if data.startswith("time"): 
                 # Write to file only when the buffer reaches a certain size
                if len(buffer) >= 50:  # Adjust this threshold as needed
                    with open(f"{outputDir.get()}/{filename}", 'a') as file:
                        file.writelines(buffer)
                        newFile = 'a'
                    buffer = []  # Clear the buffer after writing

                #with open(f"{outputDir.get()}/{filename}", 'a') as file:
                #    file.write(f"{data}\n")
        #root.after(10, readSerial)  # run again after 1m
        #time.sleep(0.0001)  # Sleep for 0.1ms




## INTERFACE

#choose serial port
labelSerial = Label(root, text = "  Select Serial Port:", pady=10)
labelSerial.grid(column =0, row =0, sticky="w", padx = 5)

portVar = StringVar() #also try without?

ports = getSerialPorts() # Get the list of serial ports
portDropdown = ttk.Combobox(root, textvariable=portVar, values=ports, state="readonly",  width=10)
portDropdown.grid(column =1, row =0, sticky="w")

connect_button = ttk.Button(root, text="Connect", command=connectSerial, width=15) #button
connect_button.grid(column =2, row =0, padx = 15, pady=5, sticky="w")

statusLabel = Label(root, text="Not connected                   ") # Label to display the connection status
statusLabel.grid(column =3, row =0, sticky="w")

#Incomming serial info
labelIncommingSerial = Label(root, text = "  Incomming Serial Data: ", pady=2)
labelIncommingSerial.grid(column =0, row =1, sticky="w", padx = 5, pady = 2)


separator = ttk.Separator(root, orient="horizontal")
separator.grid(row=2, column=0, columnspan=4, sticky="ew", pady=10, padx=10)


#input directory
inputDir = StringVar()

inputDirButton = ttk.Button(root, text="Input File", command=selectInputDirectory, width=15)
inputDirButton.grid(column=0, row=3, padx = 15)

inputDirLabel = Label(root, text="None selected", wraplength=600, pady=10)
inputDirLabel.grid(column=1, row=3, columnspan=3, sticky="w")


#output directory
outputDir = StringVar()
#outputDir.set("No directory selectedd")  # Default value

outputDirButton = ttk.Button(root, text="Output Folder", command=selectOutputDirectory, width=15)
outputDirButton.grid(column=0, row=4, padx = 15)

outputDirLabel = Label(root, text="None selected", wraplength=600, pady=10)
outputDirLabel.grid(column=1, row=4, columnspan=5, sticky="w")



#root.grid_rowconfigure(5, minsize=20)  # Adjust the row index and height as needed
separator = ttk.Separator(root, orient="horizontal")
separator.grid(row=5, column=0, columnspan=4, sticky="ew", pady=10, padx=10)





#Mode select button
modeSelected = StringVar()
modeSelected.set("")
modeLabel = Label(root, text="   Mode:", wraplength=150, pady=10)
modeLabel.grid(column=0, row=6, columnspan=1)

handTapMode = Radiobutton(root, text="Hand Tap", variable=modeSelected, value="Hand Tap")
handTapMode.grid(column=1, row=6, columnspan=1, sticky="w")
wristActuateMode = Radiobutton(root, text="Wrist Actuate", variable=modeSelected, value="Wrist Actuate") #, command=modeSelection
wristActuateMode.grid(column=2, row=6, columnspan=1, sticky="w")

#MRI wait button
waitMRISelected = StringVar()
waitMRISelected.set("WaitMRI")

waitMRILabel = Label(root, text="   MRI Wait:", wraplength=150, pady=10)
waitMRILabel.grid(column=0, row=7, columnspan=1,)

mriCheckButton = Checkbutton(root, text = "Wait for MRI", variable = waitMRISelected, 
                             onvalue = "WaitMRI", offvalue = "RunNow",  width = 10,) #height=1,
mriCheckButton.grid(column=1, row=7, columnspan=1, sticky="w")



separator = ttk.Separator(root, orient="horizontal")
separator.grid(row=8, column=0, columnspan=4, sticky="ew", pady=10, padx=10)


#upload, start, test and stop button

#Test
testButton = Button(root, text="Test Device", command=testDevice, width=13, bg ="lavender", activebackground = "purple")
testButton.grid(column=1, row=9, padx = 15, pady = 10)


#Upload
uploadButton = Button(root, text="Upload Data", command=uploadData, width=13, bg ="lightblue", activebackground = "blue")
uploadButton.grid(column=1, row=10, padx = 15, pady = 5)


#Start
startButton = Button(root, text="Start", command=startSignal, width=13, bg ="lightgreen", activebackground = "green")
startButton.grid(column=2, row=9, padx = 15, pady = 5)

#Stop
stopButton = Button(root, text="Stop", command=stopSignal, width=13, bg ="lightcoral", activebackground = "red")
stopButton.grid(column=2, row=10, padx = 15, pady = 5)

#root.after(1, readSerial) # read serial after 50ms
# Start the readSerial function in a separate thread
serial_thread = threading.Thread(target=readSerial, daemon=True)
serial_thread.start()

root.mainloop() # Execute Tkinter