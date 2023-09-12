import wx as wx
from HLEKDevEnvironment import *



if __name__ == '__main__':
    app = wx.App(False)


    frame = main_frame(parent=None, dev_logic=dev_logic)
    dev_logic.set_write_console_cb(frame.print_console)

    frame.Show()
    app.MainLoop()

