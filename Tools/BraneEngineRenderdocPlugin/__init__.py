import qrenderdoc as qrd
from typing import Optional
from .VSM import VSMWindow
from .VSM import VSMWindow

cur_window: Optional[VSMWindow] = None

def window_closed():
    global cur_window
    if cur_window is not None:
        cur_window.ctx.RemoveCaptureViewer(cur_window)
    cur_window = None

def error_log(func):
    def wrapper(pyrenderdoc, data):
        manager = pyrenderdoc.Extensions()
        try:
            func(pyrenderdoc, data)
        except:
            import traceback

            manager.MessageDialog("VSM debug init fail\n%s" % traceback.format_exc(), "Error!~")

    return wrapper

@error_log
def open_window_callback(ctx: qrd.CaptureContext, data):
    global cur_window

    if cur_window is None:
        cur_window = VSMWindow(ctx, window_closed)
        cur_window.showAsDock()

def register(version: str, ctx: qrd.CaptureContext):
    # as above ...
    print("Registering VSM Debug Window")

    ctx.Extensions().RegisterWindowMenu(qrd.WindowMenu.Window, ["VSM Debug Window"], open_window_callback)


def unregister():
    print("Unregistering VSM Debug Window")

    global cur_window

    if cur_window is not None:
        # The window_closed() callback will unregister the capture viewer
        cur_window.ctx.Extensions().GetMiniQtHelper().CloseToplevelWidget(cur_window.topWindow)
        cur_window = None