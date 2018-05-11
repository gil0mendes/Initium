import os
import shutil

def execute(cmd, directory = '.', expected = 0):
    """
    Helper function to execute a command and throw an exception is
    required status not returned.
    """
    print(f'+ {cmd}')
    old_cwd = os.getcwd()
    os.chdir(directory)
    if os.system(cmd) != expected:
        os.chdir(old_cwd)
        raise Exception('Command did not return expected value')
    os.chdir(old_cwd)

def makedirs(path):
    try:
        os.makedirs(path)
    except:
        pass

def remove(path):
    if not os.path.lexists(path):
        return

    # Handle symbolic links first as isfile() and isdir() follow links.
    if os.path.islink(path) or os.path.isfile(path):
        os.remove(path)
    elif os.path.isdir(path):
        shutil.rmtree(path)
    else:
        raise Exception('Unhandled type during remove (%s)' % (path))
