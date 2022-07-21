import os
import sys
import time
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
    self.docker_ctr = argv[3]
    self.docker_pid = 0
    self.total_util = 0
    self.app_util = 0
    self.curr_power = 0
    self.curr_freq = 1000
    # self.pwr_dict = {}
    # # init power dictonary
    # for pwr in range(LOW_FREQ, HIGH_FREQ+1, 100):
    #   self.pwr_dict[pwr] = 0.
    # set the cores affinity
    self.UpdateCores()
    # get the containers pid
    self.GetDockerPID()
    # pre-set the frequency to be 1000MHz
    self.UpdateFreq()
    # get current frequency
    self.curr_power = self.GetPower()
    # get the initial application utilization
    tmp_total_utils = [0., 0., 0.]
    for i in range(3):
      tmp_total_utils[i] = self.GetUtil()
    self.total_util = statistics.median(tmp_total_utils) 
    self.app_util = self.total_util / self.num_cores

  def UpdateCores(self):
    docker_cores_cmd = "sudo docker update --cpuset-cpus 0-" + str(self.num_cores-1) + " " + self.docker_ctr + " > /dev/null"
    # print("docker_cores_cmd:", docker_cores_cmd)
    os.system(docker_cores_cmd)

  def GetDockerPID(self):
    docker_ps_cmd = "sudo docker top " + self.docker_ctr + " | tail -1"
    # print("docker_ps_cmd:", docker_ps_cmd)
    output = subprocess.check_output(docker_ps_cmd, shell=True).decode("UTF-8").split()
    self.docker_pid = output[1]
  
  def UpdateFreq(self):
    freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq)
    # print("freq_cmd:", freq_cmd)
    os.system(freq_cmd)

  def GetUtil(self):
    util_cmd = "sudo top -b -n 2 -d 0.2 -p " + self.docker_pid + " | tail -1 | awk '{print $9}'"
    # print("util_cmd:", util_cmd)
    output = subprocess.check_output(util_cmd, shell=True).decode("UTF-8").split()
    return float(output[0])
  
  def GetPower(self):
    power_cmd = "sudo " + RAPL_POWER_CHK
    # print("power_cmd:", power_cmd)
    output = subprocess.check_output(power_cmd, shell=True)
    return float(output.decode("UTF-8"))

  def SelectCores(self):
    # wait for the applications running
    tmp_total_utils = [0., 0., 0.]
    while self.app_util < 15.:
      self.total_util = self.GetUtil()
      self.app_util = self.total_util / self.num_cores

    time.sleep(4)

    for i in range(len(tmp_total_utils)):
      tmp_total_utils[i] = self.GetUtil()
    self.total_util = statistics.median(tmp_total_utils)
    self.app_util = self.total_util / self.num_cores
    # print("total util2:", self.total_util)

    # increase the cores until the application util reaches 50
    while self.app_util > 50.:
      if self.app_util > 80.:
        self.num_cores += 2
      else:
        self.num_cores += 1
      self.UpdateCores()

      time.sleep(2)

      tmp_total_utils = [0., 0., 0.]
      for i in range(len(tmp_total_utils)):
        tmp_total_utils[i] = self.GetUtil()
        tmp_utils = tmp_total_utils[i] / self.num_cores
        if tmp_utils > 80.:
          self.total_util = tmp_total_utils[i]
          self.app_util = tmp_utils
          break
      if i == len(tmp_total_utils) - 1:
        self.total_util = statistics.median(tmp_total_utils)
        self.app_util = self.total_util / self.num_cores
        # print("total util3:", tmp_total_utils[i])

    # choose the cores where util stays less than 50 #
    target_cores = int(self.total_util / 50) + 1
    if self.num_cores != target_cores:
      self.num_cores = target_cores
      self.UpdateCores()
    print("number of cores:", self.num_cores)

  def SelectFreq(self):
    while True:
      self.curr_power = self.GetPower()
      if self.curr_power < self.pwr_cap:
        if self.curr_freq == HIGH_FREQ:
          break
        self.curr_freq += 100
        self.UpdateFreq()
        time.sleep(1)
      else:
        if self.curr_freq == LOW_FREQ:
          break
        self.curr_freq -= 100
        self.UpdateFreq()
        break
    print("frequency:", self.curr_freq)

  def Run(self):
    self.SelectCores()
    self.SelectFreq()

    print("docker ctr:", self.docker_ctr, " docker pid:", self.docker_pid, " current cores:", self.num_cores, " curr power:", self.curr_power, " curr freq", self.curr_freq)
      
if __name__ == "__main__":
  # command line: controller pwrcap cores app1 app2 ...
  RC = ResourceControl(sys.argv)
  RC.Run()

