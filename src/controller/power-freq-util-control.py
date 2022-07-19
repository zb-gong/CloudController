# import os
# import sys
# import subprocess

# SPEED_CONTROLLER = "/home/cc/PUPIL/tools/setspeed"
# RAPL_POWER_CHK = "/home/cc/PUPIL/src/RAPL/RaplPowerCheck"

# class ResourceControl:
#   def __init__(self, argv):
#     self.app_name = argv[2:]
#     self.pwr_cap = float(argv[1])
#     self.app_util = {}
#     for app in self.app_name:
#       self.app_util[app] = {}
#   def Run(self):
#     # PowerFileName = "socket_power.txt"
#     # os.system("sudo " + RAPL_POWER_MON + " &")

#     power_cmd = "sudo " + RAPL_POWER_CHK
#     output = subprocess.check_output(power_cmd, shell=True)
#     power = float(output.decode("UTF-8"))
#     print(power)

#     for app in self.app_name:
#       util_cmd = "pgrep " + app + " | xargs -I {} ps -p {} -L -o psr,pcpu"
#       output = subprocess.check_output(util_cmd, shell=True)
#       for ele in output.decode("UTF-8").split("\n"):
#         if ele and ele[0] != 'P':
#           util_per_core = ele.split()
#           core = int(util_per_core[0])
#           util = float(util_per_core[1])
#           if core in self.app_util[app]:
#             self.app_util[app][core] += util
#           else:
#             self.app_util[app][core] = util
#     print(self.app_util)

#     # pgrep_proc = subprocess.Popen(["pgrep"]+ self.app_name, stdout=subprocess.PIPE)
#     # ps_proc = subprocess.Popen("xargs -I {} ps -p {} -L -o psr,pcpu".split(), stdin=pgrep_proc.stdout, stdout=subprocess.PIPE)
#     # pgrep_proc.stdout.close()
#     # output = ps_proc.communicate()[0]
      

# if __name__ == "__main__":
#   # command line: controller pwrcap app1 app2 ...
#   RC = ResourceControl(sys.argv)
#   RC.Run()


import os
import sys
import subprocess
import statistics

SPEED_CONTROLLER = "/home/cc/PUPIL/tools/setspeed"
RAPL_POWER_CHK = "/home/cc/PUPIL/src/RAPL/RaplPowerCheck"
LOW_FREQ = 1000
HIGH_FREQ = 3700

class ResourceControl:
  def __init__(self, argv):
    self.pwr_cap = float(argv[1])
    self.num_cores = int(argv[2])
    self.docker_containers = argv[3:]
    self.app_util = {}
    for app in self.docker_containers:
      self.app_util[app] = {}
    freq_cmd = "sudo ../../tools/setspeed -a -f 2000"
    os.system(freq_cmd)
    self.curr_freq = 2000
  def GetUtil(self):
    for app in self.docker_containers:
      tmp_utils = [0,0,0]
      # util_cmd = "sudo docker stats --no-stream " + app + " | tail -1 | awk '{print $3}'"
      util_cmd = "sudo top -b -n 2 -d 0.2 -p " + app + " | tail -1 | awk '{print $9}'"
      for i in range(3):
        output = subprocess.check_output(util_cmd, shell=True).decode("UTF-8").split()
        tmp_utils[i] = float(output[0])
      self.app_util[app] = statistics.median(tmp_utils) / self.num_cores
  def Schedule(self):
    for app in self.docker_containers:
      # if self.app_util[app] >= 10. and self.app_util[app] < 20.:
      #   self.curr_freq = max(self.curr_freq-400, LOW_FREQ)
      #   freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq - 400)
      #   os.system(freq_cmd)
      # elif self.app_util[app] >= 20. and self.app_util[app] < 25.:
      #   self.curr_freq = max(self.curr_freq-200, LOW_FREQ)
      #   freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq - 200)
      #   os.system(freq_cmd)
      if self.app_util[app] >= 10. and self.app_util[app] < 25.:
        self.curr_freq = max(self.curr_freq-100, LOW_FREQ)
        freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq - 100)
        os.system(freq_cmd)
      elif self.app_util[app] >= 30. and self.app_util[app] < 32.:
        self.curr_freq = min(self.curr_freq+100, HIGH_FREQ)
        freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq)
        os.system(freq_cmd)
      elif self.app_util[app] >= 32. and self.app_util[app] < 37.:
        self.curr_freq = min(self.curr_freq+200, HIGH_FREQ)
        freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq + 200)
        os.system(freq_cmd)
      elif self.app_util[app] >= 37.:
        self.curr_freq = min(self.curr_freq+400, HIGH_FREQ)
        freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq + 400)
        os.system(freq_cmd)

  def Run(self):
    # PowerFileName = "socket_power.txt"
    # os.system("sudo " + RAPL_POWER_MON + " &")

    power_cmd = "sudo " + RAPL_POWER_CHK
    output = subprocess.check_output(power_cmd, shell=True)
    power = float(output.decode("UTF-8"))
    print(power)

    while True:
      self.GetUtil()
      self.Schedule()
      print(self.app_util, "current freq:", self.curr_freq)

    # pgrep_proc = subprocess.Popen(["pgrep"]+ self.app_name, stdout=subprocess.PIPE)
    # ps_proc = subprocess.Popen("xargs -I {} ps -p {} -L -o psr,pcpu".split(), stdin=pgrep_proc.stdout, stdout=subprocess.PIPE)
    # pgrep_proc.stdout.close()
    # output = ps_proc.communicate()[0]
      

if __name__ == "__main__":
  # command line: controller pwrcap cores app1 app2 ...
  RC = ResourceControl(sys.argv)
  RC.Run()

