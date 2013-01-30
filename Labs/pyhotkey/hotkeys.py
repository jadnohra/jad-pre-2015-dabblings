import os
import sys
import ctypes
from ctypes import wintypes
import win32con

byref = ctypes.byref
user32 = ctypes.windll.user32


HOTKEYS = {
  1 : (win32con.VK_F5, win32con.MOD_WIN),
#  2 : (win32con.VK_F1, win32con.MOD_WIN),
#  3 : (win32con.VK_F2, win32con.MOD_WIN),
#  4 : (win32con.VK_F3, win32con.MOD_WIN),
#  5 : (win32con.VK_F4, win32con.MOD_WIN),
}


def handle_win_quit ():
  return

def handle_win_f1 ():
  return

def handle_win_f2 ():
  return
  
def handle_win_f3 ():
  return

def handle_win_f4 ():
  return


HOTKEY_ACTIONS = {
  1 : handle_win_quit,
 # 2 : handle_win_f1,
 # 3 : handle_win_f2,
 # 4 : handle_win_f3,
 # 5 : handle_win_f4
}

for id, (vk, modifiers) in HOTKEYS.items ():
  #print "Registering id", id, "for key", vk
  if not user32.RegisterHotKey (None, id, modifiers, vk):
    sys.exit("Failed to register hotkey:" + str(id))


# Spin a message loop waiting for WM_HOTKEY.
def runLoop():
  while 1 :

    msg = wintypes.MSG()
    while user32.GetMessageA(byref(msg), None, 0, 0) != 0:
      if msg.message == win32con.WM_HOTKEY:
          action_to_take = HOTKEY_ACTIONS.get (msg.wParam)
          if action_to_take:
            action_to_take ()
          if action_to_take == handle_win_quit:
            return

    user32.TranslateMessage(byref(msg))
    user32.DispatchMessageA(byref(msg))


runLoop()

