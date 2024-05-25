import wx

def request_sudo_passwd(parent, msg=None) -> str:
    if not msg:
        msg = "Enter sudo password"
    with wx.PasswordEntryDialog(parent, message=msg) as dlg:
        if dlg.ShowModal() == wx.ID_CANCEL:
            return None
        else:
            return dlg.GetValue()

def browse_directory(parent, caption: str) -> str:
    with wx.DirDialog(parent, caption, style=wx.DD_DIR_MUST_EXIST | wx.DD_DEFAULT_STYLE) as dlg:
        if dlg.ShowModal() == wx.ID_CANCEL:
            return None
        else:
            return dlg.GetPath()

def browse_file_save(parent, caption: str, file_description: str, extension: str) -> str:
    with wx.FileDialog(parent, caption,
                       wildcard=f"{file_description} ({extension})|{extension}",
                       style=wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT) as dlg:
        if dlg.ShowModal() == wx.ID_CANCEL:
            return None
        else:
            return dlg.GetPath()

def browse_file_open(parent, caption: str, file_description: str, extension: str) -> str:
    wc = f"{file_description} ({extension})|{extension}" if extension else ""
    with wx.FileDialog(parent, caption,
                       wildcard=wc,
                       style=wx.FD_OPEN) as dlg:
        if dlg.ShowModal() == wx.ID_CANCEL:
            return None
        else:
            return dlg.GetPath()


def enable_controls(controls: list, enable: bool) -> list:
    affected = []
    for c in controls:
        if c.IsEnabled() != enable:
            affected.append(c)
        c.Enable(enable)
    return affected

def show_error(parent, message: str):
    with wx.MessageDialog(parent, caption="Error", style=wx.ICON_ERROR, message=message) as dlg:
        dlg.ShowModal()

def set_choise( control: wx.Choice, s: str):
    items = control.GetItems()
    indx = 0
    if s is None:
        control.Clear()
        return
    else:
        for i in items:
            if s == i:
                control.SetSelection(indx)
                return
            indx += 1

    raise RuntimeError(f"String {str(s)} is not found in choice.")

