# -*- coding: UTF-8 -*-
#
# generated by wxGlade 1.0.4 on Sat Aug 12 17:14:15 2023
#

from AppLogic import *
import wx
# begin wxGlade: dependencies
# end wxGlade

# begin wxGlade: extracode
# end wxGlade


class SelectFirmware(wx.Dialog):
    def __init__(self, app_logic: AppLogic, *args, **kwds):
        self.app_logic = app_logic
        # begin wxGlade: SelectFirmware.__init__
        kwds["style"] = kwds.get("style", 0) | wx.DEFAULT_DIALOG_STYLE
        wx.Dialog.__init__(self, *args, **kwds)
        self.SetTitle("dialog")

        sizer_1 = wx.BoxSizer(wx.VERTICAL)

        label_1 = wx.StaticText(self, wx.ID_ANY, "Select firmware configuration")
        sizer_1.Add(label_1, 0, 0, 0)

        self.choice_configuration = wx.Choice(self, wx.ID_ANY, choices=["choice 1"])
        self.choice_configuration.SetSelection(0)
        sizer_1.Add(self.choice_configuration, 0, wx.ALL | wx.EXPAND, 5)

        sizer_2 = wx.StdDialogButtonSizer()
        sizer_1.Add(sizer_2, 0, wx.ALIGN_RIGHT | wx.ALL, 4)

        self.button_OK = wx.Button(self, wx.ID_OK, "")
        self.button_OK.SetDefault()
        sizer_2.AddButton(self.button_OK)

        self.button_CANCEL = wx.Button(self, wx.ID_CANCEL, "")
        sizer_2.AddButton(self.button_CANCEL)

        sizer_2.Realize()

        self.SetSizer(sizer_1)
        sizer_1.Fit(self)

        self.SetAffirmativeId(self.button_OK.GetId())
        self.SetEscapeId(self.button_CANCEL.GetId())

        self.Layout()

        self.Bind(wx.EVT_CHOICE, self.on_choise_changed, self.choice_configuration)
        # end wxGlade

        # Set configurations
        with self.app_logic.data_lock:
            self.jsons = self.app_logic.JsonConfigs
            self.choice_configuration.Set(self.jsons)
            indx = 0
            self.choice_configuration.Select(indx)
            self.selected_json = self.jsons[0]
            for i in self.jsons:
                if i == self.app_logic.LastJson:
                    self.choice_configuration.Select(indx)
                    break
                indx += 1

    @property
    def SelectedJson(self):
        return self.selected_json

    def on_choise_changed(self, event):  # wxGlade: SelectFirmware.<event_handler>
        self.selected_json = self.jsons[self.choice_configuration.Selection]
        event.Skip()
        
# end of class SelectFirmware