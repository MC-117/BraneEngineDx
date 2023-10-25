import qrenderdoc as qrd
from PySide2 import QtWidgets, QtCore
from functools import partial

class ProcessProxy:
	def __init__(self):
		self.data = []
		self.error = ''

	def check(self, ctx):
		if self.error:
			ctx.Extensions().MessageDialog(self.error, "Error!~")
			return False
		return True

def process_proxy(func):
	def wrapper(proxy, ctrl):
		try:
			func(proxy.data, ctrl)
		except:
			import traceback

			proxy.error = "Debug fail\n%s" % traceback.format_exc()

	return wrapper

class BaseWindow(qrd.CaptureViewer):
	def __init__(self, title : str, ctx: qrd.CaptureContext, closeCallback):
		super().__init__()
		self.title = title
		self.mqt : qrd.MiniQtHelper = ctx.Extensions().GetMiniQtHelper()
		self.ctx = ctx
		self.topWindow = self.mqt.CreateToplevelWidget(title, closeCallback)

		self.values = []

		self.tableView = QtWidgets.QTableWidget()

		self.vertBox = self.mqt.CreateVerticalContainer()
		self.mqt.AddWidget(self.topWindow, self.vertBox)
	
	def error_log(func):
		def wrapper(self):
			manager = self.ctx.Extensions()
			try:
				func(self)
			except:
				import traceback

				manager.MessageDialog(f"{self.title} debug fail\n%s" % traceback.format_exc(), "Error!~")

		return wrapper
	
	def addTextBox(self, name, key):
		horBox = self.mqt.CreateHorizontalContainer()
		self.mqt.AddWidget(self.vertBox, horBox)
		label = self.mqt.CreateLabel()
		self.mqt.SetWidgetText(label, name)
		self.mqt.AddWidget(horBox, label)
		textBox = self.mqt.CreateTextBox(True, partial(self._qt_setValue, key))
		self.mqt.AddWidget(horBox, textBox)
		
	def addButton(self, name, func):
		button = self.mqt.CreateButton(lambda c, w, t: func())
		self.mqt.SetWidgetText(button, name)
		self.mqt.AddWidget(self.vertBox, button)
	
	@error_log
	def showAsDialog(self):
		self.mqt.ShowWidgetAsDialog(self.topWindow)
	
	@error_log
	def showAsDock(self):
		self.ctx.AddDockWindow(self.topWindow, qrd.DockReference.MainToolArea, None)
		self.ctx.RaiseDockWindow(self.topWindow)
	
	def __getitem__(self, index):
		return self.values[index]
	
	def __setitem__(self, index, value):
		self.values[index] = value
	
	def _qt_setValue(self, key, c, w, v):
		self[key] = v

	def getBuf(self, name):
		for buf in self.ctx.GetBuffers():
			if self.ctx.GetResourceName(buf.resourceId) == name:
				return buf
	
	def showInTable(self, headers, data):
		self.tableView.clear()
		rows = len(data)
		cols = len(headers)
		self.tableView.setRowCount(rows)
		self.tableView.setColumnCount(cols)
		h = 0
		for header in headers:
			self.tableView.setHorizontalHeaderItem(h, QtWidgets.QTableWidgetItem(header))
			h += 1
		row = 0
		for d in data:
			for col in range(cols):
				self.tableView.setItem(row, col, QtWidgets.QTableWidgetItem(str(d[col])))
			row += 1
		