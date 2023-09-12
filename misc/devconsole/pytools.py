import subprocess
import os
from typing import Callable
import paramiko as paramiko

#region STRING TOOLS
def normalize_string_length(s: str, dst_len: int, normalize_str: str=".", token: str="__DOTS__") -> str:
    token_index = s.find(token)
    if token_index < 0:
        raise RuntimeError(f'Token "{token}" is not found in string "{s}"')

    if s.find(token, token_index+len(token)) > 0:
        raise RuntimeError(f'Token "{token}" occurs more tha once is string "{s}"')

    if len(normalize_str) == 0:
        raise RuntimeError(f'Empty string is passed as normalize_str')

    s1 = s[0:token_index]
    s2 = s[token_index + len(token):]
    pad = ""

    pad_len = dst_len - len(s1) - len(s2)
    if pad_len > 0:
        n = pad_len // len(normalize_str)
        if n > 0:
            pad = normalize_str*n

        n = pad_len - n
        pad += normalize_str[0:n]

    return s1 + pad + s2


def extract_all_between_markers(s: str, begin: str, end: str) -> list[str]:
    result = list()
    text, indx = extract_between_markers(s, begin, end, start_search=0)
    while indx > 0:
        result.append(text)
        text, indx = extract_between_markers(s, begin, end, start_search=indx)
    return result


def extract_between_markers(s: str, begin: str, end: str, start_search=0) -> tuple[int, str]:
    begin_indx = s.find(begin, start_search)
    if begin_indx < 0:
        return "", begin_indx
    res_indx = begin_indx + len(begin)
    end_indx = s.find(end, res_indx)
    if end_indx < 0:
        return "", end_indx
    next_indx = end_indx + len(end)
    return s[res_indx: end_indx], next_indx

#endregion


#region FILE TOOLS

def write_text_file(fn : str, text: str) -> dict:
    with open(fn, "w") as f:
        f.write(text)


def read_text_file(fn : str) -> str:
    with open(fn, "r") as f:
        return f.read()


def is_parent_of(path: str, parent: str):
    p = os.path.abspath(path).split(os.path.sep)
    r = os.path.abspath(parent).split(os.path.sep)

    if len(p) == len(r):
        return False

    for i in r:
        if p[0] != i:
            return False
        else:
            p = p[1:]
    return True


def path_rebase(src: str, dst: str, p: str) -> str:
    abs_src = os.path.abspath(src).split(os.path.sep)
    abs_dst = os.path.abspath(dst).split(os.path.sep)
    abs_p = os.path.abspath(p).split(os.path.sep)
    for rsp in abs_src:
        node = abs_p[0]
        abs_p = abs_p[1:]

        if node != rsp:
            raise RuntimeError('Path (src) is not parent for path to be rebased (p)')

    result_components = abs_dst + abs_p
    return os.path.join(os.path.sep.join(result_components))


def is_executable_file(fn: str):
    return os.path.isfile(fn) and os.access(fn, os.X_OK)
#endregion

#region PROCESS TOOLS


def run_shell_adv(params : list, cwd=None, input: list = None, print_stdout = True, envvars : dict = None) -> tuple:
    stdout_accum = ""
    env_vars = os.environ.copy()
    if envvars:
        env_vars = {**env_vars, **envvars}
    proc = subprocess.Popen(params,
        cwd=cwd,
        stderr=subprocess.STDOUT, stdout=subprocess.PIPE, stdin=subprocess.PIPE,
        close_fds=True,
        env=env_vars)

    if input:
        data = os.linesep.join(map(str, input))
        ba = data.encode("UTF-8")
        proc.stdin.write(ba)

    proc.stdin.flush()
    proc.stdin.close()

    for line in iter(proc.stdout.readline, b''):
        l = line.rstrip().decode("UTF-8")
        stdout_accum += l + os.linesep
        if print_stdout:
            print(l)

    proc.wait()
    return (proc.returncode==0, proc.returncode, stdout_accum)


#endregion

#region NFS TOOLS
def verify_nfs_share(sudopasswd, nfs_path):
    test_script = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'test_nfs.sh');
    success, exit_code, output = run_shell_adv(params=["sudo", "-S", "/bin/bash", test_script, nfs_path], input=[sudopasswd])
    if not success:
        raise RuntimeError("Can't mount NFS share.")
#endregion

#region PARAMICO TOOLS

def ssh_connect(host: str, user_name: str, password: str) -> paramiko.SSHClient:
    ssh_client = paramiko.client.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.client.AutoAddPolicy)

    ssh_client.connect(host,
                   username=user_name,
                   password=password,
                   look_for_keys=False)
    return ssh_client


def ssh_disconnect(client: paramiko.SSHClient):
    client.close()


def ssh_test_connection(host: str, user_name: str, password: str):
    client = ssh_connect(host=host, user_name=user_name, password=password)
    ssh_disconnect(client)


def ssh_run(client: paramiko.client.SSHClient,
            command: str) -> tuple[int, str, str]:
    (stdin_file, stdout_file, stderr_file) = client.exec_command(command)
    exit_code = stdout_file.channel.recv_exit_status()
    return exit_code, stdout_file.read().decode("utf-8") , stderr_file.read().decode("utf-8")
#endregion