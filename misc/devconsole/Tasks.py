from __future__ import annotations
import os.path
from abc import ABC, abstractmethod
import inspect
import pytools
import threading


class ITask(ABC):

    def __init__(self, task_name: str):
        self.task_name = task_name
        self.task_id = 'task_' + pytools.replace_all_characters(task_name, '\s', '_').lower()
        self.data_mutex = threading.Lock()
        self.killed = False
        self.thread = None

    def start(self, **kwargs):
        with self.data_mutex:
            if self.thread is not None:
                raise RuntimeError(f'Task ("{self.task_name}") thread is already started.')

            self.thread = threading.Thread(target=self.task_thread, kwargs=kwargs)
            self.thread.start()

    def task_thread_internal(self, **kwargs):
        try:
            self.task_thread(kwargs)
        except Exception as ex:
            raise ex
        finally:
            with self.data_mutex:
                self.thread = None

    def stop(self):
        with self.data_mutex:
            if self.thread is None:
                raise RuntimeError(f'Task ("{self.task_name}") thread is not started.')
            else:
                self.killed = True
                # Also execute kill script
                self.kill_task()

    @abstractmethod
    def task_thread(self, **kwargs):
        raise NotImplemented()

    @abstractmethod
    def kill_task(self):
        raise NotImplemented()


class RemoteBackgroundTask(ITask):
    def __init__(self, task_name: str, ssh_connection):
        super().__init__(task_name)
        self.ssh_connection = ssh_connection

    def task_thread(self, **kwargs):
        raise NotImplemented()

    def kill_task(self):
        raise NotImplemented()


class RemoteShellTask(ITask):
    def __init__(self,
                 task_name: str,
                 host: str,
                 user: str,
                 keyfile: str,
                 command: list[str]):
        super().__init__(task_name)
        self.host = host
        self.user = user
        self.keyfile = keyfile
        self.pid = None
        self.command = ["/usr/bin/gnome-terminal",
             "--disable-factory",
             "--maximize",
             "--",
             f"ssh",
             f"{self.user}@{self.host}",
             "-i",
             f"{self.keyfile}",
             "-t"] + command

    def task_thread(self, **kwargs):
        success, errcode, stdout = pytools.run_shell_adv(
        self.command,
        on_stdout=self.on_stdout,
        on_check_kill=self.on_check_kill,
        on_started=self.on_start,
        on_stopped=self.on_stop)

    def on_start(self, pid):
        with self.data_mutex:
            self.pid = pid
        pass

    def on_stop(self):
        with self.data_mutex:
            self.pid = None
        pass

    def on_stdout(self, s: str):
        print(s)

    def on_check_kill(self):
        with self.data_mutex:
            result = self.killed
        return result

    def kill_task(self):
        pass


