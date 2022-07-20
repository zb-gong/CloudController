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
    self.app_util = 0
    self.curr_power = 0
    self.curr_freq = 2000
    # set the cores affinity
    self.UpdateCores()
    # get the containers pid
    self.GetDockerPID()
    # pre-set the frequency to be 2000MHz
    self.UpdateFreq()
    # get current frequency
    self.curr_power = self.GetPower()
    # get the initial application utilization
    tmp_utils = [0.] * 3
    for i in range(3):
      tmp_utils[i] = self.GetUtil()
    self.app_util = statistics.median(tmp_utils)

  def UpdateCores(self):
    docker_cores_cmd = "sudo docker update --cpuset-cpus 0-" + str(self.num_cores-1) + " " + self.docker_ctr
    os.sytem(docker_cores_cmd)

  def GetDockerPID(self):
    docker_ps_cmd = "sudo docker top " + self.docker_ctr + " | tail -1"
    output = subprocess.check_output(docker_ps_cmd, shell=True).decode("UTF-8").split()
    self.docker_pid = output[0][1]
  
  def UpdateFreq(self):
    freq_cmd = "sudo ../../tools/setspeed -a -f " + str(self.curr_freq)
    os.system(freq_cmd)

  def GetUtil(self):
    util_cmd = "sudo top -b -n 2 -d 0.2 -p " + self.docker_pid + " | tail -1 | awk '{print $9}'"
    output = subprocess.check_output(util_cmd, shell=True).decode("UTF-8").split()
    return float(output[0]) / self.num_cores
  
  def GetPower(self):
    power_cmd = "sudo " + RAPL_POWER_CHK
    output = subprocess.check_output(power_cmd, shell=True)
    return float(output.decode("UTF-8"))

  def SelectCores(self):
    while self.app_util > 50:
      if self.app_util > 80:
        self.num_cores += 2
      else:
        self.num_cores += 1
      self.UpdateCores()

      time.sleep(1)

      tmp_utils = [0.] * 3
      for i in range(3):
        tmp_utils[i] = self.GetUtil()
        if tmp_utils[i] > 80.:
          self.app_util = tmp_utils[i]
          break
      if i == 2:
        self.app_util = statistics.median(tmp_utils)

  def SelectFreq(self):
    while True:
      self.curr_power = self.GetPower()
      if self.curr_power < self.pwr_cap:
        self.curr_freq += 100
        self.UpdateFreq()
        time.sleep(1)
      else:
        self.curr_freq -= 100
        self.UpdateFreq
        break

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
    self.SelectCores()
    self.SelectFreq()

    # pgrep_proc = subprocess.Popen(["pgrep"]+ self.app_name, stdout=subprocess.PIPE)
    # ps_proc = subprocess.Popen("xargs -I {} ps -p {} -L -o psr,pcpu".split(), stdin=pgrep_proc.stdout, stdout=subprocess.PIPE)
    # pgrep_proc.stdout.close()
    # output = ps_proc.communicate()[0]
      

if __name__ == "__main__":
  # command line: controller pwrcap cores app1 app2 ...
  RC = ResourceControl(sys.argv)
  RC.Run()

