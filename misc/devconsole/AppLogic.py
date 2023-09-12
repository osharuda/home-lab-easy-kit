import json as json
import os
import pytools
import wx_tools as wx_tools
from typing import Callable
import time as time
import threading as threading

from directory_structure import ProjectBuilder
from enum import IntFlag, auto

class AppLogic:
    def __init__(self,
                 make_log: Callable[[str], None],
                 make_relog: Callable[[str], None],
                 set_status: Callable[[str], None],
                 ask_confirmation: Callable[[str, str], bool]):
        self.app_path = os.path.abspath(__file__)
        self.app_dir_path = os.path.dirname(self.app_path)
        self.config_file_name = os.path.join(self.app_dir_path, "hlek_dev_console.cfg")
        self.load_default()
        self.make_log = make_log
        self.make_relog = make_relog
        self.ask_confirmation = ask_confirmation
        self.set_status = set_status
        self.log_len = 50

        self.KEY_NFS_URI = 'dir_nfs_uri'
        self.KEY_LOCAL_NFS_SHARE = 'dir_local_share'
        self.KEY_LOCAL_HLEK = 'dir_hlek'
        self.KEY_LOCAL_CMSIS = 'dir_cmsis'
        self.KEY_EDITOR = 'bin_editor'
        self.KEY_LAST_CONFIG = 'last_configuration'
        self.KEY_ESTIMATIONS = 'estimations'
        self.KEY_CONFIGURATIONS = 'configurations'

        self.KEY_HOST = 'host'
        self.KEY_USER_NAME = 'user_name'
        self.KEY_PASSWORD = 'password'
        self.KEY_JSONS = 'json'
        self.KEY_LAST_JSON = 'last_json'

        self.last_configuration = None
        self.make_release = False
        self.data_lock = threading.Lock()
        self.thread_result = dict()

        if os.path.isfile(self.app_path):
            try:
                self.load()
            except RuntimeError as e:
                pass

# region CONFIGURATION VARIABLES

    def save_config(self):
        js = dict()
        js[self.KEY_NFS_URI] = self.dir_nfs_uri
        js[self.KEY_LOCAL_NFS_SHARE] = self.mount_point
        js[self.KEY_LOCAL_HLEK] = self.dir_hlek
        js[self.KEY_LOCAL_CMSIS] = self.dir_cmsis
        js[self.KEY_EDITOR] = self.bin_editor
        js[self.KEY_LAST_CONFIG] = self.last_configuration
        js[self.KEY_ESTIMATIONS] = self.estimations
        js[self.KEY_CONFIGURATIONS] = self.configurations

        pytools.write_text_file(self.config_file_name, json.dumps(js, indent=4, ))

    @property
    def NfsURI(self):
        return self.dir_nfs_uri

    @NfsURI.setter
    def NfsURI(self, value):
        self.dir_nfs_uri = value


    @property
    def MountPoint(self):
        return self.mount_point

    @MountPoint.setter
    def MountPoint(self, value):
        self.mount_point = value

    @property
    def LocalDirHLEK(self):
        return self.dir_hlek

    @LocalDirHLEK.setter
    def LocalDirHLEK(self, value):
        self.dir_hlek = value

    @property
    def LocalDirCMSIS(self):
        return self.dir_cmsis

    @LocalDirCMSIS.setter
    def LocalDirCMSIS(self, value):
        self.dir_cmsis = value

    @property
    def Editor(self):
        return self.bin_editor

    @Editor.setter
    def Editor(self, value):
        self.bin_editor = value

    @property
    def TimeEstimations(self):
        return self.estimations

    @TimeEstimations.setter
    def TimeEstimations(self, value: tuple[str, float]):
        self.estimations[value[0]] = value[1]

    @property
    def JsonConfigs(self):
        result = []
        if self.LastConfiguration:
            result = self.LastConfiguration[self.KEY_JSONS]
        return result

    @property
    def LastJson(self):
        config = self.LastConfiguration
        if config is None or len(config[self.KEY_JSONS]) == 0:
            return None

        result = config[self.KEY_LAST_JSON]
        if result is None:
            result = self.LastConfiguration[self.KEY_JSONS][0]
            config[self.KEY_LAST_JSON] = result

        return result

    @LastJson.setter
    def LastJson(self, value):
        config = self.LastConfiguration
        if not config:
            config[self.KEY_LAST_JSON] = None
        elif value is None:
            config[self.KEY_LAST_JSON] = config[self.KEY_JSONS][0]
        elif value not in config[self.KEY_JSONS]:
            raise RuntimeError(f'Unknown LastJson value: {value}')
        else:
            config[self.KEY_LAST_JSON] = value

        self.save_config()

    @property
    def Host(self):
        result = None
        if self.LastConfiguration:
            result = self.LastConfiguration[self.KEY_HOST]
        return result


    @property
    def UserName(self):
        result = None
        if self.LastConfiguration:
            result = self.LastConfiguration[self.KEY_USER_NAME]
        return result

    @property
    def Password(self):
        result = None
        if self.LastConfiguration:
            result = self.LastConfiguration[self.KEY_PASSWORD]
        return result

    def load_default(self):
        self.dir_nfs_uri = "<ip or host>:/some/path..."
        self.mount_point = ""
        self.dir_hlek = ""
        self.dir_cmsis = ""
        self.bin_editor = ""
        self.estimations = dict()
        self.configurations = dict()
        self.last_configuration = None

    def load(self):
        try:
            config_text = pytools.read_text_file(self.config_file_name)
            js = json.loads(config_text)
            self.dir_nfs_uri = js[self.KEY_NFS_URI]
            self.mount_point = js[self.KEY_LOCAL_NFS_SHARE]
            self.dir_hlek = js[self.KEY_LOCAL_HLEK]
            self.dir_cmsis = js[self.KEY_LOCAL_CMSIS]
            self.bin_editor = js[self.KEY_EDITOR]
            self.configurations = js[self.KEY_CONFIGURATIONS]
            self.estimations = js[self.KEY_ESTIMATIONS]
            self.last_configuration = js[self.KEY_LAST_CONFIG]

            # Sync LastJson
            for config_name, config in self.configurations.items():
                if len(config[self.KEY_JSONS]) == 0:
                    config[self.KEY_LAST_JSON] = None
                elif config[self.KEY_LAST_JSON] not in config[self.KEY_JSONS]:
                    config[self.KEY_LAST_JSON] = config[self.KEY_JSONS][0]

        except Exception:
            self.load_default()
            return

    @property
    def Configurations(self) -> dict:
        return self.configurations


    @property
    def LastConfigName(self):
        return self.last_configuration

    @LastConfigName.deleter
    def LastConfigName(self):
        del self.configurations[self.last_configuration]
        if len(self.configurations):
            self.last_configuration = next(iter(self.configurations))
        else:
            self.last_configuration = ""

    @LastConfigName.setter
    def LastConfigName(self, value):
        if self.last_configuration != value:
            self.last_configuration = value
        self.save_config()

    @property
    def LastConfiguration(self) -> dict:
        if self.last_configuration:
            return self.configurations[self.last_configuration]
        else:
            return None

    @property
    def BuildType(self):
        return "Release" if self.make_release else "Debug"

    @BuildType.setter
    def BuildType(self, value: str):
        self.make_release = value.upper() == "RELEASE"

# endregion

# region MUTLITHREADING

    def get_thread_result(self, id):
        self.data_lock.acquire()
        result = self.thread_result.get(id, None)
        del self.thread_result[id]
        self.data_lock.release()
        return result

    def set_thread_result(self, success: bool, message: str):
        id = threading.current_thread().native_id
        self.data_lock.acquire()
        if id in self.thread_result:
            raise RuntimeError("Element should'n be among thread results")
        self.thread_result[id] = (success, message)
        self.data_lock.release()

    def task_thread(self, task: Callable[[], str]):
        message = ""
        result = True
        try:
            message = task()
            if message is None:
                message = ""
        except Exception as e:
            result = False
            message = str(e)

        self.set_thread_result(result, message)

    def make_task_names(self, name: str, target: str = None):
        if target:
            return (name, f'{self.LastConfigName}_{target}_{name}')
        else:
           return (name, f'{self.LastConfigName}_{name}')

    def run_on_progress(self, progress: int):
        self.set_status(f' {int(progress)} %')
        pass

    def run_task(self,
                 opname: tuple,
                 task: Callable,
                 on_complete: Callable[[tuple[bool, str]], None],
                 on_progress = None):
        if on_progress is None:
            on_progress = lambda p: self.run_on_progress(p)

        run_thread = threading.Thread(target=self.run_task_internal, args=(opname, task, on_complete, on_progress))
        run_thread.start()
        return run_thread

    def get_estimation(self, names: tuple):
        self.data_lock.acquire()
        if names[1] in self.TimeEstimations:
            value = float(self.TimeEstimations[names[1]])
        elif names[0] in self.TimeEstimations:
            value = float(self.TimeEstimations[names[0]])
        else:
            value = 60.0
        self.data_lock.release()
        return value

    def update_estimation(self, names: tuple, value: float):
        common_task_name, targeted_task_name = names
        self.data_lock.acquire()
        if targeted_task_name not in self.TimeEstimations or value > self.TimeEstimations[targeted_task_name]:
            self.TimeEstimations[names[1]] = value
        if common_task_name not in self.TimeEstimations or value > self.TimeEstimations[common_task_name]:
            self.TimeEstimations[names[0]] = value
        self.data_lock.release()

    def run_task_internal(self,
                          opname: tuple,
                          task: Callable,
                          on_complete: Callable[[tuple[bool, str]], None],
                          on_progress: Callable[[float], None]):
        estimation = self.get_estimation(opname)
        check_interval = estimation / 100
        start_time = time.monotonic()
        task_thread = threading.Thread(target=self.task_thread, args=[task])
        task_thread.start()
        task_thread_id = task_thread.native_id

        task_thread.join(check_interval)
        while task_thread.is_alive():
            progress = (time.monotonic()-start_time)*100.0/estimation
            if int(progress) < 100:
                on_progress(progress)
            task_thread.join(check_interval)

        on_progress(100.0)
        duration = time.monotonic()-start_time
        self.update_estimation(opname, duration)
        self.save_config()
        result = self.get_thread_result(task_thread_id)
        self.set_status('Ready')
        on_complete(result)

# endregion

# region HLEK TASKS
    def test_task(self, somearg: str):
        with self.data_lock:
            mp = self.mount_point
        time.sleep(5)
        return f"completed: somearg={mp}"

    def append_error_and_raise(self, errortext, message, stdout, stderr):
        message += f"{os.linesep}{errortext}:{os.linesep}"
        message += f'STDOUT:{os.linesep}-------------------------------{os.linesep}{stdout}{os.linesep}'
        message += f'STDERR:{os.linesep}-------------------------------{os.linesep}{stderr}{os.linesep}'
        raise RuntimeError(message)

    def flash_firmware(self, fw_json: str):
        message = ""

        with self.data_lock:
            self.sanitize_config()

            mount_point = self.MountPoint
            config = self.LastConfiguration
            host = config[self.KEY_HOST]
            login = config[self.KEY_USER_NAME]
            password = config[self.KEY_PASSWORD]
            log_len = self.log_len
            config_name = self.LastConfigName

        flash_cmd = f"cd {mount_point}{os.path.sep}{config_name}{os.path.sep}{fw_json}{os.path.sep}firmware; ./flash.sh"

        try:
            message += f"------ Flashing firmware for ({fw_json}) on {host} ------{os.linesep}"
            message += f"Command -> {flash_cmd} {os.linesep}"

            # Connect
            s = pytools.normalize_string_length(f"Connecting {host} __DOTS__ ", log_len)
            message += s + os.linesep

            with pytools.ssh_connect(host, login, password) as ssh_conn:
                try:
                    # Flash
                    message += pytools.normalize_string_length(f"Flashing __DOTS__ ", log_len) + os.linesep
                    exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, flash_cmd)
                    message += f'STDOUT:{os.linesep}-------------------------------{os.linesep}{stdout}{os.linesep}'
                    message += f'STDERR:{os.linesep}-------------------------------{os.linesep}{stderr}{os.linesep}'
                    if exit_code != 0:
                        raise RuntimeError(message)
                except Exception as iex:
                    raise RuntimeError(str(iex))

        except Exception as ex:
            raise RuntimeError(str(ex))

        return message

    def build_project(self, rebuild: bool):

        with self.data_lock:
            self.sanitize_config()

            mount_point = self.MountPoint
            config = self.LastConfiguration
            host = config[self.KEY_HOST]
            login = config[self.KEY_USER_NAME]
            password = config[self.KEY_PASSWORD]
            log_len = self.log_len
            config_name = self.LastConfigName
            build_release = self.make_release
            project_root = f'{mount_point}{os.path.sep}{config_name}'

        message = ""

        build_cmd = f"cd {project_root}; "
        build_cmd += "./build.sh " + " ".join(config[self.KEY_JSONS])
        if build_release:
            build_cmd += ' "Release"'

        try:
            message += f"------ Building HLEK on {host} ------{os.linesep}"
            message += f"Command -> {build_cmd} {os.linesep}"

            # Connect
            s = pytools.normalize_string_length(f"Connecting {host} __DOTS__ ", log_len)
            message += s + os.linesep

            with pytools.ssh_connect(host, login, password) as ssh_conn:
                try:
                    message += f' [ OK ]{os.linesep}'

                    # Clear cache if rebuild is requested
                    if rebuild:
                        message += pytools.normalize_string_length(f"Clearing the cache __DOTS__ ", log_len) + os.linesep
                        exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, f"rm -f {os.path.join(project_root,'hashes')}")
                        message += f'STDOUT:{os.linesep}-------------------------------{os.linesep}{stdout}{os.linesep}'
                        message += f'STDERR:{os.linesep}-------------------------------{os.linesep}{stderr}{os.linesep}'
                        if exit_code != 0:
                            raise RuntimeError(message)

                    # build
                    message += pytools.normalize_string_length(f"Building __DOTS__ ", log_len) + os.linesep
                    exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, build_cmd)
                    message += f'STDOUT:{os.linesep}-------------------------------{os.linesep}{stdout}{os.linesep}'
                    message += f'STDERR:{os.linesep}-------------------------------{os.linesep}{stderr}{os.linesep}'
                    if exit_code != 0:
                        raise RuntimeError(message)
                except Exception as iex:
                    raise RuntimeError(str(iex))

        except Exception as ex:
            message += f"{os.linesep}{os.linesep}Error: {str(ex)}{os.linesep}"
            raise RuntimeError(message)

        return message

    def create_project_directory(self) -> str:
        with self.data_lock:
            self.sanitize_config()

            mount_point = self.MountPoint
            config = self.LastConfiguration
            host = config[self.KEY_HOST]
            login = config[self.KEY_USER_NAME]
            password = config[self.KEY_PASSWORD]
            log_len = self.log_len
            config_name = self.LastConfigName
            dir_hlek = self.LocalDirHLEK
            dir_cmsis = self.LocalDirCMSIS

        message = ""

        if not self.ask_confirmation("Are you sure?", "Project directory creation"):
            return "canceled"

        try:
            message += f"------ Creating project directory ({config_name}) for {host} ------{os.linesep}"

            # Connect
            s = pytools.normalize_string_length(f"Connecting {host} __DOTS__ ", log_len)
            message += s + os.linesep
            with pytools.ssh_connect(host, login, password) as ssh_conn:
                message += f' [ OK ]{os.linesep}'

                # Create project tree with sym links
                try:
                    message += pytools.normalize_string_length(f"Creating {config_name} __DOTS__ ", log_len)
                    pb = ProjectBuilder(dir_hlek, dir_cmsis, mount_point, config_name)
                    pb_cmds = pb.make_project_tree()
                    for cmd in pb_cmds:
                        message += cmd + os.linesep
                        exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, cmd)
                        message += f'STDOUT:{os.linesep}-------------------------------{os.linesep}{stdout}{os.linesep}'
                        message += f'STDERR:{os.linesep}-------------------------------{os.linesep}{stderr}{os.linesep}'
                        if exit_code != 0:
                            raise RuntimeError(message)

                except Exception as iex:
                    raise RuntimeError(str(iex))

        except Exception as ex:
            message += f"{os.linesep}{os.linesep}Error: {str(ex)}{os.linesep}"
            raise RuntimeError(message)

        return message

    def deploy_software(self):
        with self.data_lock:
            self.sanitize_config()

            mount_point = self.MountPoint
            config = self.LastConfiguration
            host = config[self.KEY_HOST]
            login = config[self.KEY_USER_NAME]
            password = config[self.KEY_PASSWORD]
            log_len = self.log_len
            nfs_uri = self.NfsURI

        message = ""

        try:
            message += f"------ Deploying HLEK ({self.LastConfigName}) to {host} ------{os.linesep}"

            # Connect
            s = pytools.normalize_string_length(f"Connecting {host} __DOTS__ ", self.log_len)
            message += s
            with pytools.ssh_connect(host, login, password) as ssh_conn:
                message += f' [ OK ]{os.linesep}'

                # Get local (geoip) timezone
                message += pytools.normalize_string_length(f"Requesting geoip timezone __DOTS__ ", self.log_len)
                #command = """wget -O - -q http://geoip.ubuntu.com/lookup | sed -n -e 's/.*<TimeZone>\(.*\)<\/TimeZone>.*/\1/ p'"""
                command = """wget -O - -q http://geoip.ubuntu.com/lookup"""
                exit_code, timezone, stderr = pytools.ssh_run(ssh_conn, command)
                if exit_code != 0:
                    self.append_error_and_raise("Unable to get local time zone", message, timezone, stderr)
                timezone, i = pytools.extract_between_markers(timezone, "<TimeZone>", "</TimeZone>")

                # Asjust time zone
                message += pytools.normalize_string_length(f"Adjusting time zone __DOTS__ ", self.log_len)
                command = f"""echo {timezone} | sudo tee /etc/timezone > /dev/null; dpkg-reconfigure -f noninteractive tzdata >/dev/null 2>&1; timedatectl set-timezone {timezone}"""
                exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, command)
                if exit_code != 0:
                    self.append_error_and_raise("Unable to adjust time zone", message, stdout, stderr)

                # apt update
                message += pytools.normalize_string_length(f"Updating repositories __DOTS__ ", self.log_len)
                exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, "sudo apt-get update")
                if exit_code != 0:
                    self.append_error_and_raise("Can't update repositories", message, stdout, stderr)

                # install packages
                package_list = ["libicu-dev", "binutils-arm-none-eabi", "gcc-arm-none-eabi", "gdb-multiarch", "openocd",
                                "stlink-tools", "i2c-tools", "doxygen", "libncurses-dev", "vim", "git", "mc", "nfs-kernel-server",
                                "nfs-common", "autofs", "ninja-build", "cmake"]

                for p in package_list:
                    message += pytools.normalize_string_length(f"Installing {p} __DOTS__ ", log_len)
                    exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, f"sudo apt-get --yes install {p}")
                    if exit_code != 0:
                        self.append_error_and_raise(f"Can't install {p}", message, stdout, stderr)

                # Setup NFS share using autofs
                message += pytools.normalize_string_length(f"Configuring NFS share __DOTS__ ", log_len)

                exit_code, stdout, stderr = pytools.ssh_run(ssh_conn,
                    f"sudo /bin/bash -c 'echo \"/-  /etc/auto.nfs --ghost\" >> /etc/auto.master'")
                if exit_code != 0:
                    self.append_error_and_raise(f"Can't change /etc/auto.master {p}", message, stdout, stderr)

                exit_code, stdout, stderr = pytools.ssh_run(ssh_conn,
                    f"sudo /bin/bash -c 'echo \"{mount_point} {nfs_uri}\" > /etc/auto.nfs'")
                if exit_code != 0:
                    self.append_error_and_raise(f"Can't change /etc/auto.nfs {p}", message, stdout, stderr)

                exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, f"sudo service autofs reload")
                if exit_code != 0:
                    self.append_error_and_raise(f"Can't reload autofs {p}", message, stdout, stderr)

                exit_code, stdout, stderr = pytools.ssh_run(ssh_conn, f"sudo systemctl restart autofs")
                if exit_code != 0:
                    self.append_error_and_raise(f"Can't restart autofs {p}", message, stdout, stderr)

        except Exception as ex:
            message = f"{os.linesep}{os.linesep}Error: {str(ex)}"
            raise RuntimeError(message)

        return message


# endregion

    def add_json(self, name: str):
        self.configurations

    def set_nfs_uri(self, uri: str):
        self.uri = uri

    def set_write_console_cb(self, cb):
        self.write_console_cb = cb



    def get_default_configuration(self):
        return { self.KEY_HOST : '',
                 self.KEY_USER_NAME: 'pi',
                 self.KEY_PASSWORD: 'pi',
                 self.KEY_LAST_JSON: '',
                 self.KEY_JSONS: []}

    def get_configurations(self):
        return self.configurations

    def set_configuration(self,
                          name: str,
                          host: str,
                          user_name: str,
                          password: str,
                          last_json: str,
                          jsons: list):
        if last_json not in jsons:
            last_json = jsons[0]

        self.configurations[name] = {self.KEY_HOST: host,
                                     self.KEY_USER_NAME: user_name,
                                     self.KEY_PASSWORD: password,
                                     self.KEY_LAST_JSON: last_json,
                                     self.KEY_JSONS: jsons}

        self.last_configuration = name


    def get_last_configuration(self):
        if not self.configurations:
            return None, self.get_default_configuration()

        if self.last_configuration not in self.configurations:
            raise RuntimeError(f'Corrupted configuration {self.last_configuration}')

        return self.last_configuration, self.configurations[self.last_configuration]

    def get_configuration_names(self):
        return list(self.configurations.keys())

    def del_configuration(self, name: str):
        if name not in self.configurations.keys():
            raise RuntimeError('Unknown configuration')

        del self.configurations[name]
        if self.last_configuration == name:
            self.last_configuration = next(iter(self.configurations))


    def update_nfs_share(self, nfs_share, local_mount_point, do_not_mount_local):
        self.js["nfs_share"] = nfs_share
        self.js["local_mount_point"] = local_mount_point
        self.js["do_not_mount_local"] = do_not_mount_local


    def sanitize_config(self):
        if not self.LastConfiguration:
            raise RuntimeError("Configuration is not set")





