import subprocess
import time
import os
import sys
import numpy
StartTime = time.time()
SET_SPEED = "/home/zibo/Projects/PUPIL/tools/pySetCPUSpeed.py"
POWER_MON = "/home/zibo/Projects/PUPIL/tools/pyWattsup-hank.py"
RAPL_POWER_MON = "/home/zibo/Projects/PUPIL/src/RAPL/RaplPowerMonitor"


class PowerControl:

    def __init__(self, PwrCap, AppName, HeartbeatFileName, CommandLine):
        self.AppName = AppName
        self.AppNameShort = AppName[0:8]
        self.PwrCap = float(PwrCap)
        #  self.isFinished = 0
        self.phase = 0
        self.PerfDictionary = {(0, 0, 0): 0}
        self.PwrDictionary = {(0, 0, 0): 0}
        self.CurConfig = (0, 0, 0)
        # self.NextConfig = (0,0,0)
        self.CoreNumber = 0
        self.frequency = 0
        self.CommandLine = CommandLine
        self.MemoCtrl = 1
        self.PerfFileLength = 0
        self.CurCore = 0
        self.CurFreq = 0
        # self.HeartbeatFileName = HeartbeatFileName
        self.HeartbeatFileName = AppName + "_heartbeat.log"

    def PwrBSearch(self, lowbound, highbound, PwrCap):
        head = lowbound
        tail = highbound
        self.PerfDictionary[(head, 1, 1)], self.PwrDictionary[(
            head, 1, 1)] = self.GetFeedback((head, 1, 1))
        self.PerfDictionary[(tail, 1, 1)], self.PwrDictionary[(
            tail, 1, 1)] = self.GetFeedback((tail, 1, 1))
        while(head + 1 < tail):
            MidPointer = int((head + tail)/2)
            self.PerfDictionary[(MidPointer, 1, 1)], self.PwrDictionary[(
                MidPointer, 1, 1)] = self.GetFeedback((MidPointer, 1, 1))
            if self.PwrDictionary[(MidPointer, 1, 1)] < PwrCap:
                head = MidPointer
            else:
                tail = MidPointer
        if self.PwrDictionary[(tail, 1, 1)] > PwrCap:
            return head
        else:
            return tail

    def PerfBSearch(self, lowbound, highbound):
        self.PerfDictionary[(highbound, 1, 1)], self.PwrDictionary[(
            highbound, 1, 1)] = self.GetFeedback((highbound, 1, 1))
        self.PerfDictionary[(lowbound, 1, 1)], self.PwrDictionary[(
            lowbound, 1, 1)] = self.GetFeedback((lowbound, 1, 1))
        head = lowbound
        tail = highbound
        if self.PerfDictionary[(highbound, 1, 1)] > self.PerfDictionary[(lowbound, 1, 1)]:
            while(head+1 < tail):
                MidPointer = int((head + tail)/2)
                self.PerfDictionary[(MidPointer, 1, 1)], self.PwrDictionary[(
                    MidPointer, 1, 1)] = self.GetFeedback((MidPointer, 1, 1))
                if self.PerfDictionary[(MidPointer, 1, 1)] < self.PerfDictionary[(tail, 1, 1)]:
                    head = MidPointer
                else:
                    TmpPointer = MidPointer + 1
                    self.PerfDictionary[(TmpPointer, 1, 1)], self.PwrDictionary[(
                        TmpPointer, 1, 1)] = self.GetFeedback((TmpPointer, 1, 1))
                    if self.PerfDictionary[(TmpPointer, 1, 1)] > self.PerfDictionary[(MidPointer, 1, 1)]:
                        head = TmpPointer
                    else:
                        tail = MidPointer

        else:
            while(head+1 < tail):
                MidPointer = int((head + tail)/2)
                self.PerfDictionary[(MidPointer, 1, 1)], self.PwrDictionary[(
                    MidPointer, 1, 1)] = self.GetFeedback((MidPointer, 1, 1))
                if self.PerfDictionary[(MidPointer, 1, 1)] < self.PerfDictionary[(head, 1, 1)]:
                    tail = MidPointer
                else:
                    TmpPointer = MidPointer - 1
                    self.PerfDictionary[(TmpPointer, 1, 1)], self.PwrDictionary[(
                        TmpPointer, 1, 1)] = self.GetFeedback((TmpPointer, 1, 1))
                    if self.PerfDictionary[(TmpPointer, 1, 1)] > self.PerfDictionary[(MidPointer, 1, 1)]:
                        tail = TmpPointer
                    else:
                        head = MidPointer
        if self.PerfDictionary[(head, 1, 1)] > self.PerfDictionary[(tail, 1, 1)]:
            return head
        else:
            return tail

    def FreqBsearch(self, CoreNumber):
        head = 1
        tail = 12
        self.PerfDictionary[(CoreNumber, head, 1)], self.PwrDictionary[(
            CoreNumber, head, 1)] = self.GetFeedback((CoreNumber, head, 1))
        self.PerfDictionary[(CoreNumber, tail, 1)], self.PwrDictionary[(
            CoreNumber, tail, 1)] = self.GetFeedback((CoreNumber, tail, 1))
        while (head + 1 < tail):
            MidPointer = int((head + tail)/2)
            self.PerfDictionary[(CoreNumber, MidPointer, 1)], self.PwrDictionary[(
                CoreNumber, MidPointer, 1)] = self.GetFeedback((CoreNumber, MidPointer, 1))
            print("[FreqBsearch] Curr power:", self.PwrDictionary[(CoreNumber,
                  MidPointer, 1)], " PowerCap:", self.PwrCap)
            if (self.PwrDictionary[(CoreNumber, MidPointer, 1)] < self.PwrCap):
                head = MidPointer
            else:
                tail = MidPointer
        if self.PwrDictionary[(CoreNumber, tail, 1)] > self.PwrCap:
            return head
        else:
            return tail

    def Decision(self):
        self.RunApp(2, 1, 1)
        if self.phase == 0:
            self.CurConfig = (2, 1, 1)
            self.PerfDictionary[self.CurConfig], self.PwrDictionary[self.CurConfig] = self.GetFeedback(
                self.CurConfig)
            if self.PwrDictionary[self.CurConfig] > self.PwrCap:
                self.phase = 3
                # Power binary search core number in (1,8)
                TmpCoreNumber = self.PwrBSearch(1, 2, self.PwrCap)
                # Perf BS
                self.CoreNumber = self.PerfBSearch(1, TmpCoreNumber)
                # BS frequency
                self.frequency = self.FreqBsearch(self.CoreNumber)

            else:
                self.phase = 1
                self.CurConfig = (4, 1, 1)
                self.PerfDictionary[self.CurConfig], self.PwrDictionary[self.CurConfig] = self.GetFeedback(
                    self.CurConfig)
                if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(2, 1, 1)]:
                    self.CurConfig = (8, 1, 1)
                    self.phase = 2
                    self.PerfDictionary[self.CurConfig], self.PwrDictionary[self.CurConfig] = self.GetFeedback(
                        self.CurConfig)
                    if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(2, 1, 1)]:
                        self.phase = 3
                        # binary search core number in (1,8)
                        self.CoreNumber = self.PerfBSearch(1, 2)
                        # BS frequency
                        self.frequency = self.FreqBsearch(self.CoreNumber)

                    else:
                        if self.PwrDictionary[self.CurConfig] > self.PwrCap:
                            self.phase = 3
                            # Power binary search core number in (33,40)
                            TmpCoreNumber = self.PwrBSearch(7, 8, self.PwrCap)
                            # binary search core number in (33,tmpCoreNumber)
                            self.CoreNumber = self.PerfBSearch(
                                7, TmpCoreNumber)
                            # BS frequency
                            self.frequency = self.FreqBsearch(self.CoreNumber)
                        else:
                            # binary search core number in (33,40)
                            self.CoreNumber = self.PerfBSearch(7, 8)
                            # BS frequency
                            self.frequency = self.FreqBsearch(self.CoreNumber)

                else:
                    if self.PwrDictionary[self.CurConfig] > self.PwrCap:
                        # Power binary search core number in (9,16)
                        TmpCoreNumber = self.PwrBSearch(2, 4, self.PwrCap)
                        # binary search core number in (9,tmpCoreNumber)
                        self.CoreNumber = self.PerfBSearch(2, TmpCoreNumber)
                        # BS frequency
                        self.frequency = self.FreqBsearch(self.CoreNumber)
                    else:
                        self.CurConfig = (6, 1, 1)
                        self.phase = 2
                        self.PerfDictionary[self.CurConfig], self.PwrDictionary[self.CurConfig] = self.GetFeedback(
                            self.CurConfig)
                        if self.PerfDictionary[self.CurConfig] < self.PerfDictionary[(4, 1, 1)]:
                            print("[Decision]binary search core number in (9, 16)")
                            self.CoreNumber = self.PerfBSearch(3, 4)
                            # BS frequency
                            self.frequency = self.FreqBsearch(self.CoreNumber)
                        else:

                            if self.PwrDictionary[self.CurConfig] > self.PwrCap:
                                print("[Decision]Power binary search core number in (17,32)")
                                TmpCoreNumber = self.PwrBSearch(
                                    5, 6, self.PwrCap)
                                print("[Decision]binary search core number in (17,tmpCoreNumber)")
                                self.CoreNumber = self.PerfBSearch(
                                    5, TmpCoreNumber)
                                # BS frequency
                                self.frequency = self.FreqBsearch(
                                    self.CoreNumber)

                            else:
                                print("[Decision]binary search core number in (17,32)")
                                self.CoreNumber = self.PerfBSearch(5, 6)
                                # BS frequency
                                self.frequency = self.FreqBsearch(
                                    self.CoreNumber)

            return 1

    # get the heartbeat info
    def GetFeedback(self, config):
        print("[GetFeedback] Config:", config)
        #PowerFileName = self.CurFolder+'socket_power.txt'
        PowerFileName = 'socket_power.txt'
        #PerfFileName = self.CurFolder+'heartbeat.log'
        PerfFileName = self.HeartbeatFileName
        if config in self.PerfDictionary:
            print("[GetFeedback] Perf:", self.PerfDictionary[config], "Pwr:", self.PwrDictionary[config])
            return self.PerfDictionary[config], self.PwrDictionary[config]
        else:
            # subprocess.call([SET_SPEED, "-S", str(16-self.CurConfig[1])])
            # subprocess.call(["sudo","-E","numactl","--interleave=0-"+str(self.CurConfig[2]-1),"--physcpubind=0-"+str(self.CurConfig[0]-1),ExeFileName])
            # self.RunApp(config[0],config[1],config[2])
            self.AdjustConfig(config[0], config[1], config[2])
            # PowerFile = open(PowerFileName,'r')
            PerfFile = open(PerfFileName, 'r')
            # PowerFileLines= PowerFile.readlines()
            PerfFileLines = PerfFile.readlines()[1:]
            self.PerfFileLength = len(PerfFileLines)
            TmpTime = time.time()
            time.sleep(1)

            os.system("sudo "+RAPL_POWER_MON+" &")

            time.sleep(2)

            # counter = 0
            # print("len(PerfFileLines)", len(PerfFileLines))
            # print("self.PerfFileLength", self.PerfFileLength)
            print("[GetFeedback] wait....")
            while ((len(PerfFileLines) - self.PerfFileLength) < 10) and ((len(PerfFileLines) - self.PerfFileLength) < 3 or (time.time() - TmpTime < 10)):
                # PowerFile.close()
                PerfFile.close()
                # get Socket Power
                time.sleep(0.1)
                # print "sleep 0.1"
                # print "get Socket Power"
                # os.system("sudo "+RAPL_POWER_MON)
                # PowerFile = open(PowerFileName,'r')
                PerfFile = open(PerfFileName, 'r')
                # PowerFileLines= PowerFile.readlines()
                PerfFileLines = PerfFile.readlines()[1:]
                # counter += 1
                # print PowerFileLines
                # if counter % 10 == 0:
                #   os.system(POWER_MON+" stop > power.txt")

            print("[GetFeedback] waiting time: "+str(time.time() - TmpTime))
            os.system("sudo pkill RaplPowerMonitor")
            PowerFile = open(PowerFileName, 'r')
            PowerFileLines = PowerFile.readlines()

            CurLength = len(PerfFileLines) - self.PerfFileLength
            # CurLength = len(PerfFileLines)

            # print "sleep 0.1"
            # os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print $2}' | sudo xargs kill -9")
            # os.system("ps -ef | grep "+self.CurFolder+self.AppName+" | awk '{print $2}' | sudo xargs kill -9")
            SumPerf = 0
            j = 0
            AvergeInterval = 0.0
            heartbeat = 0.0
            if self.PerfFileLength == 0:
                CurLength = CurLength - 1
            # print "long(PerfFileLines[-1].split()[2])=",long(PerfFileLines[-1].split()[2])
            # print "long(PerfFileLines[-CurLength-1].split()[2])=",long(PerfFileLines[-CurLength-1].split()[2])
            TotalInterval = (
                int(PerfFileLines[-1].split()[2]) - int(PerfFileLines[-CurLength-1].split()[2]))

            AvergeInterval = TotalInterval / float(CurLength)
            TimeIntervalList = []
            for i in range(-CurLength, 0):
                LinePerf = PerfFileLines[i].split()
                TimeInterval = int(LinePerf[2]) - \
                    int(PerfFileLines[i-1].split()[2])
                TimeIntervalList.append(TimeInterval)

            # heartbeat = heartbeat + 1
            # if TimeInterval < AvergeInterval / 100 :
            #   heartbeat = heartbeat - 1
            # SumPerf +=  float(LinePerf[4])
            variance = numpy.var(TimeInterval)
            for interval in TimeIntervalList:
                if interval > AvergeInterval + 3 * variance or interval < AvergeInterval - 3 * variance:
                    TimeIntervalList.remove(interval)
            # AvgPerf = heartbeat/TotalInterval
            AvgPerf = len(TimeIntervalList) / sum(TimeIntervalList)

            # for i in range(-CurLength+1,0):
            #     LinePerf = PerfFileLines[i].split()
            #     SumPerf +=  float(LinePerf[4])
            #     j += 1
            # AvgPerf = SumPerf/j

            j = 0
            tmpPwr = 0
            MaxPwr = 0
            for i in range(len(PowerFileLines)):
                LinePwr = PowerFileLines[i].split()
                tmpPwr = float(LinePwr[0])
                if tmpPwr > MaxPwr:
                    MaxPwr = tmpPwr

            print("[GetFeedback] AvgPerf:", AvgPerf, "MaxPwr:", MaxPwr)
            return float(AvgPerf), float(MaxPwr)

    # def RunApp(self,CoreNumber, freq, MemoCtrl):
    #     if CoreNumber <33:
    #         os.system(SET_SPEED+' -S '+str(12-freq))
    #         #os.system(POWER_MON+" start")
    #         print("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &")
    #         os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &")
    #     else:
    #         os.system(SET_SPEED+' -S '+str(12-freq))
    #         #os.system(POWER_MON+" start")
    #         os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1)+" --physcpubind=0-7,16-"+str(CoreNumber-17)+" "+self.CommandLine+" &")
    #        # os.system(POWER_MON+" stop > power.txt")
    #     self.CurCore = CoreNumber
    def RunApp(self, CoreNumber, freq, MemoCtrl):
        if CoreNumber < 33:
            os.system(SET_SPEED+' -S '+str(12-freq))
            # os.system(POWER_MON+" start")
            print("sudo -E numactl --interleave=0-"+str(MemoCtrl-1) +
                  " --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &")
            os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1) +
                      " --physcpubind=0-"+str(CoreNumber-1)+" "+self.CommandLine+" &")
        else:
            os.system(SET_SPEED+' -S '+str(12-freq))
            # os.system(POWER_MON+" start")
            os.system("sudo -E numactl --interleave=0-"+str(MemoCtrl-1) +
                      " --physcpubind=0-7,16-"+str(CoreNumber-17)+" "+self.CommandLine+" &")
            # os.system(POWER_MON+" stop > power.txt")
        self.CurCore = CoreNumber

    def AdjustConfig(self, CoreNumber, freq, MemoCtrl):
        StartTime = time.time()
        if self.CurFreq != freq:
            os.system(SET_SPEED+' -S '+str(12-freq))
        if self.CurCore != CoreNumber:
            if CoreNumber < 33:
                print("[AdjustConfig]: (", CoreNumber, ",", freq, ",", MemoCtrl, ")")
                # os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i > /dev/null & done")
                result1 = subprocess.check_output(
                    "for i in $(pgrep "+self.AppNameShort+" | xargs pstree -p|grep -o '[[:digit:]]*' |grep -o '[[:digit:]]*');do sudo taskset -pc 0-"+str(CoreNumber-1)+" $i & done", shell=True)
            else:
                # os.system("for i in $(pgrep "+self.AppName+" | xargs ps -mo pid,tid,fname,user,psr -p | awk 'NR > 2  {print $2}');do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i > /dev/null & done")
                result1 = subprocess.check_output(
                    "for i in $(pgrep "+self.AppNameShort+" | xargs pstree -p|grep -o '[[:digit:]]*' |grep -o '[[:digit:]]*');do sudo taskset -pc 0-7,16-"+str(CoreNumber-17)+" $i & done", shell=True)
        self.CurCore = CoreNumber
        self.CurFreq = freq
        EndTime = time.time()
        print("[AdjustConfig] Time:" + str(EndTime - StartTime))


CommandLine = ""
for i in range(4, len(sys.argv)):
    CommandLine = CommandLine+" "+sys.argv[i]
StartTime = time.time()
PC = PowerControl(sys.argv[1], sys.argv[2], sys.argv[3], CommandLine)
print("CommandLine", CommandLine)
tmp1 = PC.Decision()
print("PerfDictionary:", PC.PerfDictionary)
print("PwrDictionary:", PC.PwrDictionary)
print("Converging settings:(", PC.CoreNumber, ",", PC.frequency, ",", PC.MemoCtrl, ")")
EndTime = time.time()

PC.AdjustConfig(PC.CoreNumber, PC.frequency, PC.MemoCtrl)
file = open(PC.HeartbeatFileName, 'r')
StartLength = len(file.readlines()[1:])
file.close()
print("Converging time: " + str(time.time() - StartTime))
result = subprocess.check_output(
    "pgrep "+PC.AppName+" > /dev/null; echo $?", shell=True)
while (result == '0\n'):
    result = subprocess.check_output(
        "pgrep "+PC.AppName+" > /dev/null; echo $?", shell=True)
    time.sleep(1.0)

file = open(PC.HeartbeatFileName, 'r')
Endlength = len(file.readlines()[1:])
file.close()
os.system("echo "+str(Endlength - StartLength)+" >> Length.txt")

file = open("settling_time.txt", 'a')
file.write(str(PC.PwrCap)+" "+str(EndTime - StartTime)+"\n")
file.close()
file = open("converged_configuration", 'a')
file.write(str(PC.PwrCap)+" ("+str(PC.CoreNumber)+"," +
           str(PC.frequency)+","+str(PC.MemoCtrl)+")")
file.close()
print(result, "finished")
