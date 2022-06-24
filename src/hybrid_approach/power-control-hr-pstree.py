import subprocess
import time
import os
import sys
import math
import numpy
StartTime = time.time()
SET_SPEED = "/home/zibo/Projects/PUPIL/tools/pySetCPUSpeed.py"
POWER_MON = "/home/zibo/Projects/PUPIL/tools/pyWattsup-hank.py"
RAPL_POWER_MON = "/home/zibo/Projects/PUPIL/src/RAPL/RaplPowerMonitor"

class PowerControl:

    def __init__(self, PwrCap, AppName, CommandLine):
        self.AppName = AppName
        self.AppNameShort =AppName[0:8]
        self.PwrCap = float(PwrCap)
      #  self.isFinished = 0
        self.phase = 0
        self.PerfDictionary = {(0,0,0):0}
        self.PwrDictionary = {(0,0,0):0}
        self.CurConfig = (0,0,0)
       # self.NextConfig = (0,0,0)
        self.CoreNumber = 0
        self.frequency = 12
        self.CommandLine = CommandLine
        self.MemoCtrl = 1
        self.PerfFileLength = 0
        self.CurCore= 0
        self.CurFreq= 12
        print(self.AppNameShort)
    
    def PwrBSearch(self,lowbound, highbound, PwrCap):
        head = lowbound
        tail = highbound
        self.PerfDictionary[(head,1,1)],self.PwrDictionary[(head,1,1)] = self.GetFeedback((head,1,1))
        self.PerfDictionary[(tail,1,1)],self.PwrDictionary[(tail,1,1)] = self.GetFeedback((tail,1,1))
        while(head +1 < tail):
            MidPointer = int((head + tail )/2)
            self.PerfDictionary[(MidPointer,1,1)],self.PwrDictionary[(MidPointer,1,1)] = self.GetFeedback((MidPointer,1,1))
            if self.PwrDictionary[(MidPointer,1,1)] < PwrCap:
                head = MidPointer
            else:
                tail = MidPointer
        if self.PwrDictionary[(tail,1,1)] > PwrCap:
            return head
        else:
            return tail
    
    def PerfBSearch(self,lowbound, highbound):
        self.PerfDictionary[(highbound,12,1)],self.PwrDictionary[(highbound,12,1)] = self.GetFeedback((highbound,12,1))
        self.PerfDictionary[(lowbound,12,1)],self.PwrDictionary[(lowbound,12,1)] = self.GetFeedback((lowbound,12,1))
        head = lowbound
        tail = highbound
        if self.PerfDictionary[(highbound,12,1)] > self.PerfDictionary[(lowbound,12,1)]:
            while(head+1<tail):
                MidPointer = int((head +tail)/2)
                self.PerfDictionary[(MidPointer,12,1)],self.PwrDictionary[(MidPointer,12,1)] = self.GetFeedback((MidPointer,12,1))
                if self.PerfDictionary[(MidPointer,12,1)] < self.PerfDictionary[(tail,12,1)]:
                    head = MidPointer
                else:
                    TmpPointer = MidPointer +1
                    self.PerfDictionary[(TmpPointer,12,1)],self.PwrDictionary[(TmpPointer,12,1)] = self.GetFeedback((TmpPointer,12,1))
                    if self.PerfDictionary[(TmpPointer,12,1)] > self.PerfDictionary[(MidPointer,12,1)]:
                        head = TmpPointer
                    else:
                        tail = MidPointer

        else:
            while(head+1<tail):
                MidPointer = int((head +tail)/2)
                self.PerfDictionary[(MidPointer,12,1)],self.PwrDictionary[(MidPointer,12,1)] = self.GetFeedback((MidPointer,12,1))
                if self.PerfDictionary[(MidPointer,12,1)] < self.PerfDictionary[(head,12,1)]:
                    tail = MidPointer
                else:
                    TmpPointer = MidPointer -1
                    self.PerfDictionary[(TmpPointer,12,1)],self.PwrDictionary[(TmpPointer,12,1)] = self.GetFeedback((TmpPointer,12,1))
                    if self.PerfDictionary[(TmpPointer,12,1)] > self.PerfDictionary[(MidPointer,12,1)]:
                        tail = TmpPointer
                    else:
                        head = MidPointer
        if self.PerfDictionary[(head,12,1)] > self.PerfDictionary[(tail,12,1)]:
            return head
        else:
            return tail


    def FreqBsearch(self,CoreNumber):
        head = 1
        tail =12
        self.PerfDictionary[(CoreNumber,head,1)],self.PwrDictionary[(CoreNumber,head,1)] = self.GetFeedback((CoreNumber,head,1))
        self.PerfDictionary[(CoreNumber,tail,1)],self.PwrDictionary[(CoreNumber,tail,1)] = self.GetFeedback((CoreNumber,tail,1))
        while (head +1 <tail):
            MidPointer = int((head + tail)/2)
            self.PerfDictionary[(CoreNumber,MidPointer,1)],self.PwrDictionary[(CoreNumber,MidPointer,1)] = self.GetFeedback((CoreNumber,MidPointer,1))
            if (self.PwrDictionary[(CoreNumber,MidPointer,1)] < self.PwrCap):
                head = MidPointer
            else:
                tail = MidPointer
        if self.PwrDictionary[(CoreNumber,tail,1)] > self.PwrCap:
            return head
        else:
            return tail
            
    def GetPowerDistAndSet(self,CoreNumber):
        power1 =0.0
        power2 =0.0
        if CoreNumber <9 or (CoreNumber >33):
            power1 = self.PwrCap-40.0
            power2 = 0
        else:
            if CoreNumber > 8 and CoreNumber <17:
                power1 = math.ceil(((self.PwrCap-40.0)/CoreNumber ) * 8)
                power2 = self.PwrCap-40.0 -power1
            else:
                if CoreNumber > 16 and CoreNumber < 25:
                    power2 = math.floor(((self.PwrCap-40.0)/CoreNumber ) * 8)
                    power1 = self.PwrCap-40.0 -power2
                else:
                    power1 = math.ceil(((self.PwrCap-40.0)/CoreNumber ) * 16)
                    power2 = self.PwrCap-40.0 -power1
        os.system("sudo /var/tmp/RAPL/RaplSetPowerSeprate "+str(power1+20)+" "+str(power2+20))


    def Decision(self):
        self.RunApp(2,12,1)
        if self.phase == 0:
            self.CurConfig = (2,12,1)
            self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
            self.CurConfig = (4,12,1)
            self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
            
            if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(2,12,1)]:
                self.CurConfig = (1,12,1)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                self.CurConfig = (8,12,1)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(2,12,1)]or self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(1,12,1)]:
                    #bs 1,8
                    self.CoreNumber = self.PerfBSearch(1, 2)
                else:
                    #bs 33,40
                    self.CoreNumber = self.PerfBSearch(6,8)
            else:
                self.CurConfig = (6,12,1)
                self.PerfDictionary[self.CurConfig],self.PwrDictionary[self.CurConfig] = self.GetFeedback(self.CurConfig)
                if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(4,12,1)]:
                    #bs 8,16
                    self.CoreNumber = self.PerfBSearch(2,4)
                else:
                    #bs 16,32
                    self.CoreNumber = self.PerfBSearch(4,6)

            return 1
                
#get the heartbeat info
    def GetFeedback(self,config):
        print(config)
        #PowerFileName = self.CurFolder+'socket_power.txt'
        PowerFileName = 'socket_power.txt'
        #PerfFileName = self.CurFolder+'heartbeat.log'
        PerfFileName = self.AppName +'_heartbeat.log'
        if config in self.PerfDictionary:
            return self.PerfDictionary[config], self.PwrDictionary[config]
        else:
           # subprocess.call([SET_SPEED, "-S", str(16-self.CurConfig[1])])
           # subprocess.call(["sudo","-E","numactl","--interleave=0-"+str(self.CurConfig[2]-1),"--physcpubind=0-"+str(self.CurConfig[0]-1),ExeFileName])
         #   self.RunApp(config[0],config[1],config[2])
            self.AdjustConfig(config[0],config[1],config[2])
            # self.GetPowerDistAndSet(config[0])

        #    PowerFile = open(PowerFileName,'r')
            PerfFile = open(PerfFileName,'r')
        #    PowerFileLines= PowerFile.readlines()
            PerfFileLines= PerfFile.readlines()[1:]
            self.PerfFileLength = len(PerfFileLines)
            TmpTime = time.time()

            os.system("sudo "+RAPL_POWER_MON + " &")

            time.sleep(2)
           # if int(config[0]) == 40 and int(config[1]==1):
            #    time.sleep(4)
            #    print("sleep 5"
         #   counter = 0
            print("len(PerfFileLines)", len(PerfFileLines))
            print("self.PerfFileLength",self.PerfFileLength)
            print("wait....")
            while ((len(PerfFileLines) - self.PerfFileLength) <10)and((len(PerfFileLines) - self.PerfFileLength) < 3 or (time.time()- TmpTime < 10)):
              #  PowerFile.close()
                PerfFile.close()
                #get Socket Power
                time.sleep(0.1)
              #  print("sleep 0.1"
               # print("get Socket Power"
              #  os.system("sudo "+RAPL_POWER_MON)
              #  PowerFile = open(PowerFileName,'r')
                PerfFile = open(PerfFileName,'r')
             #   PowerFileLines= PowerFile.readlines()
                PerfFileLines= PerfFile.readlines()[1:]
             #   counter += 1
             #   print(PowerFileLines
                # if counter % 10 == 0:
                #   os.system(POWER_MON+" stop > power.txt")
            print("waiting time: "+str(time.time() - TmpTime))
            os.system("sudo pkill RaplPowerMonitor")
            PowerFile = open(PowerFileName,'r')
            PowerFileLines= PowerFile.readlines()
            
            CurLength = len(PerfFileLines) - self.PerfFileLength

            print("CurLength=",CurLength)
                # print("sleep 0.1"
            #os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print($2}' | sudo xargs kill -9")
           # os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print($2}' | sudo xargs kill -9")
            SumPerf = 0
            j =0
            AvergeInterval = 0.0
            heartbeat = 0.0
            if self.PerfFileLength == 0:
                CurLength = CurLength -1
           # print("int(PerfFileLines[-1].split()[2])=",int(PerfFileLines[-1].split()[2])
           # print("int(PerfFileLines[-CurLength-1].split()[2])=",int(PerfFileLines[-CurLength-1].split()[2])
            TotalInterval =(int(PerfFileLines[-1].split()[2]) - int(PerfFileLines[-CurLength-1].split()[2]))

            AvergeInterval = TotalInterval/ float(CurLength)
            TimeIntervalList =[]
            for i in range(-CurLength,0):
                LinePerf = PerfFileLines[i].split()
                TimeInterval = int(LinePerf[2]) - int(PerfFileLines[i-1].split()[2])
                TimeIntervalList.append(TimeInterval)
                
             #   heartbeat = heartbeat + 1
             #   if TimeInterval < AvergeInterval / 100 :
             #       heartbeat = heartbeat - 1
             #   SumPerf +=  float(LinePerf[4])
            variance = numpy.var(TimeInterval)
            for interval in TimeIntervalList:
                if interval > AvergeInterval + 3 * variance or interval < AvergeInterval - 3 * variance :
                    TimeIntervalList.remove(interval)
          #  AvgPerf = heartbeat/TotalInterval
            AvgPerf = len(TimeIntervalList)/ sum(TimeIntervalList)
            j = 0
            SumPwr = 0
            for i in range(len(PowerFileLines)):
                LinePwr = PowerFileLines[i].split()
                SumPwr += float(LinePwr[0])
                j +=1
            AvgPwr = float(SumPwr)/j
            print(AvgPerf, AvgPwr)
            return float(AvgPerf), float(AvgPwr)



    def RunApp(self,CoreNumber, freq, MemoCtrl):
        if CoreNumber <33:
            os.system(SET_SPEED+' -S '+str(12-freq))
            #os.system(POWER_MON+" start")
            print("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &")
            os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &")
        else:
            os.system(SET_SPEED+' -S '+str(12-freq))
            #os.system(POWER_MON+" start")
            os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-7,16-"+str(CoreNumber-17)+" "+self.CommandLine+" &")
           # os.system(POWER_MON+" stop > power.txt")
        self.CurCore = CoreNumber
        # self.GetPowerDistAndSet(CoreNumber)

    def AdjustConfig(self, CoreNumber,freq,MemoCtrl):
        StartTime = time.time()

        if self.CurFreq != freq:
            os.system(SET_SPEED+' -S '+str(12-freq))
        
        if self.CurCore != CoreNumber:
            if CoreNumber <33:
                print("for i in $(pgrep "+self.AppNameShort+" | xargs pstree -p|grep -o '[[:digit:]]*' |grep -o '[[:digit:]]*');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i & done")
                print(self.AppNameShort)
            #    os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print($2}');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i > /dev/null & done")
                result1 = subprocess.check_output("for i in $(pgrep "+self.AppNameShort+" | xargs pstree -p|grep -o '[[:digit:]]*' |grep -o '[[:digit:]]*');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i & done",shell=True)
            else:
                #os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print($2}');do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i > /dev/null & done")
                result1 = subprocess.check_output("for i in $(pgrep "+self.AppNameShort+" | xargs pstree -p|grep -o '[[:digit:]]*' |grep -o '[[:digit:]]*');do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i & done",shell=True)
        # self.GetPowerDistAndSet(CoreNumber)
        self.CurCore = CoreNumber
        self.CurFreq = freq
        EndTime = time.time()
        print((EndTime - StartTime))

CommandLine =""
# os.system("sudo /home/zibo/Projects/PUPIL/src/RAPL/RaplPowerLimitDisable")
os.system("sudo /home/zibo/Projects/PUPIL/src/RAPL/RaplSetPower " + str(sys.argv[1]))
print("power set time:", (time.time() - StartTime))
for i in range(3,len(sys.argv)):
    CommandLine  = CommandLine+" "+sys.argv[i]
PC= PowerControl(sys.argv[1],sys.argv[2],CommandLine)
print("CommandLine", CommandLine)
log_file = sys.argv[2]+"_heartbeat.log"
tmp1 = PC.Decision()
print(PC.PerfDictionary)
print(PC.PwrDictionary)
print(PC.CoreNumber,PC.frequency,PC.MemoCtrl)
PC.AdjustConfig(PC.CoreNumber,PC.frequency,PC.MemoCtrl)
file = open(log_file,'r')
StartLength= len(file.readlines()[1:])
file.close()
print((time.time() - StartTime))
result = subprocess.check_output("pgrep "+PC.AppNameShort+" > /dev/null; echo $?", shell=True)
while (result =='0\n'):
    result = subprocess.check_output("pgrep "+PC.AppNameShort+" > /dev/null; echo $?", shell=True)
    time.sleep(1.0)

file = open(log_file,'r')
Endlength= len(file.readlines()[1:])
file.close()
os.system("echo "+str(Endlength - StartLength)+" >> Length.txt")

file = open("converged_configuration",'a')
file.write(str(PC.PwrCap)+" ("+str(PC.CoreNumber)+","+str(PC.frequency)+","+str(PC.MemoCtrl)+")")
print(result,"finished")

