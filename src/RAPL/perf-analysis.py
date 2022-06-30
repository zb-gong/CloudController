import numpy
PerfFile = open("jacobi_heartbeat.log", 'r')
# PerfFile = open("STREAM_heartbeat.log", 'r')
PerfFileLines = PerfFile.readlines()[1:]
CurLength = len(PerfFileLines) - 1
TotalInterval =(int(PerfFileLines[-1].split()[2]) - int(PerfFileLines[-CurLength-1].split()[2]))
AvergeInterval = TotalInterval/ float(CurLength)
TimeIntervalList =[]
for i in range(-CurLength,0):
    LinePerf = PerfFileLines[i].split()
    TimeInterval = int(LinePerf[2]) - int(PerfFileLines[i-1].split()[2])
    TimeIntervalList.append(TimeInterval)
variance = numpy.var(TimeInterval)
for interval in TimeIntervalList:
    if interval > AvergeInterval + 3 * variance or interval < AvergeInterval - 3 * variance :
        TimeIntervalList.remove(interval)
AvgPerf = len(TimeIntervalList)/ sum(TimeIntervalList)
print(AvgPerf)
