import qrenderdoc as qrd
from typing import Optional
from .VSM import VSMWindow
from .Probe import ProbeWindow

cur_vsm_window: Optional[VSMWindow] = None
cur_probe_window: Optional[ProbeWindow] = None

def error_log(func):
    def wrapper(pyrenderdoc, data):
        manager = pyrenderdoc.Extensions()
        try:
            func(pyrenderdoc, data)
        except:
            import traceback

            manager.MessageDialog("Debug init fail\n%s" % traceback.format_exc(), "Error!~")

    return wrapper

def vsm_window_closed():
    global cur_vsm_window
    if cur_vsm_window is not None:
        cur_vsm_window.ctx.RemoveCaptureViewer(cur_vsm_window)
    cur_vsm_window = None

@error_log
def open_vsm_window_callback(ctx: qrd.CaptureContext, data):
    global cur_vsm_window

    if cur_vsm_window is None:
        cur_vsm_window = VSMWindow(ctx, vsm_window_closed)
        cur_vsm_window.showAsDock()

def probe_window_closed():
    global cur_probe_window
    if cur_probe_window is not None:
        cur_probe_window.ctx.RemoveCaptureViewer(cur_probe_window)
    cur_probe_window = None

@error_log
def open_probe_window_callback(ctx: qrd.CaptureContext, data):
    global cur_probe_window

    if cur_probe_window is None:
        cur_probe_window = ProbeWindow(ctx, probe_window_closed)
        cur_probe_window.showAsDock()

def register(version: str, ctx: qrd.CaptureContext):
    # as above ...
    print("Registering BraneEngine Debug Window")

    ctx.Extensions().RegisterWindowMenu(qrd.WindowMenu.Window, ["VSM Debug Window"], open_vsm_window_callback)
    ctx.Extensions().RegisterWindowMenu(qrd.WindowMenu.Window, ["Probe Debug Window"], open_probe_window_callback)


def unregister():
    print("Unregistering BraneEngine Debug Window")

    global cur_vsm_window
    global cur_probe_window

    if cur_vsm_window is not None:
        # The window_closed() callback will unregister the capture viewer
        cur_vsm_window.ctx.Extensions().GetMiniQtHelper().CloseToplevelWidget(cur_vsm_window.topWindow)
        cur_vsm_window = None

    if cur_probe_window is not None:
        # The window_closed() callback will unregister the capture viewer
        cur_probe_window.ctx.Extensions().GetMiniQtHelper().CloseToplevelWidget(cur_probe_window.topWindow)
        cur_probe_window = None