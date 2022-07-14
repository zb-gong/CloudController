import os
import sys
import subprocess

SPEED_CONTROLLER = "/home/cc/PUPIL/tools/setspeed"
RAPL_POWER_CHK = "/home/cc/PUPIL/src/RAPL/RaplPowerCheck"

class ResourceControl:
  def __init__(self, argv):
    self.app_name = argv[2:]
    self.pwr_cap = float(argv[1])
    self.app_util = {}
    for app in self.app_name:
      self.app_util[app] = {}
  def Run(self):
    # PowerFileName = "socket_power.txt"
    # os.system("sudo " + RAPL_POWER_MON + " &")

    power_cmd = "sudo " + RAPL_POWER_CHK
    output = subprocess.check_output(power_cmd, shell=True)
    power = float(output.decode("UTF-8"))
    print(power)

    for app in self.app_name:
      util_cmd = "pgrep " + app + " | xargs -I {} ps -p {} -L -o psr,pcpu"
      output = subprocess.check_output(util_cmd, shell=True)
      for ele in output.decode("UTF-8").split("\n"):
        if ele and ele[0] != 'P':
          util_per_core = ele.split()
          core = int(util_per_core[0])
          util = float(util_per_core[1])
          if core in self.app_util[app]:
            self.app_util[app][core] += util
          else:
            self.app_util[app][core] = util
    print(self.app_util)

    # pgrep_proc = subprocess.Popen(["pgrep"]+ self.app_name, stdout=subprocess.PIPE)
    # ps_proc = subprocess.Popen("xargs -I {} ps -p {} -L -o psr,pcpu".split(), stdin=pgrep_proc.stdout, stdout=subprocess.PIPE)
    # pgrep_proc.stdout.close()
    # output = ps_proc.communicate()[0]
      

if __name__ == "__main__":
  # command line: controller pwrcap app1 app2 ...
  RC = ResourceControl(sys.argv)
  RC.Run()

