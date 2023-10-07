import qrenderdoc as qrd
from PySide2 import QtWidgets, QtCore
import struct as st
import math
from functools import partial

VSM_PAGE_SIZE = 128
VSM_LOG2_L0_PAGES = int(math.log2(VSM_PAGE_SIZE))
VSM_MAX_MIPS = VSM_LOG2_L0_PAGES + 1
VSM_PHYSICAL_PAGE_ANY_MIP_VALID_FLAG = 0x8000000
physPagesXY = (128, 32)
physPageRowMask = physPagesXY[0] - 1
physPageRowShift = int(math.log2(physPagesXY[0]))

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

			proxy.error = "VSM debug fail\n%s" % traceback.format_exc()

	return wrapper

def calcLevelOffsets(Level : int):
	NumBits = Level << 1
	StartBit = (2 * VSM_LOG2_L0_PAGES + 2) - NumBits
	Mask = ((1 << NumBits) - 1) << StartBit

	return 0x55555555 & Mask

VSM_PAGE_TABLE_SIZE = calcLevelOffsets(VSM_MAX_MIPS)

def calcPageAddressFromIndex(index : int):
	mipLevel = 0
	pageAddress = (0xFFFFFFFF, 0xFFFFFFFF)

	for level in range(VSM_MAX_MIPS - 1):
		mipLevel = level
		if index < calcLevelOffsets(mipLevel + 1):
			break
	level0RowMask = ((1 << VSM_LOG2_L0_PAGES) - 1)
	offsetInLevel = index - calcLevelOffsets(mipLevel)
	pageAddress = (offsetInLevel & (level0RowMask >> mipLevel), offsetInLevel >> (VSM_LOG2_L0_PAGES - mipLevel))

	return mipLevel, pageAddress

def physPageAddressToIndex(physPageAddress):
	return (physPageAddress[1] << physPageRowShift) + physPageAddress[0]

def physIndexToPageAddress(pageIndex):
	return (pageIndex & physPageRowMask, pageIndex >> physPageRowShift)

def calcPageTableLevelOffset(vsmID, level):
	return vsmID * VSM_PAGE_TABLE_SIZE + calcLevelOffsets(level)

def calcPageAddressFromFlatIndex(index):
	vsmID = index // VSM_PAGE_TABLE_SIZE
	pageOffset = index % VSM_PAGE_TABLE_SIZE
	mipLevel, pageAddress = calcPageAddressFromIndex(pageOffset)
	return vsmID, mipLevel, pageAddress

class Table:
	page = (-1, -1)
	mip = 0
	isAnyLODValid = False
	isThisLODValid = False

def decodeTable(value : int):
	table = Table()
	table.page = (value & 0x3FF, (value >> 10) & 0x3FF)
	table.mip = (value >> 20) & 0x3F
	table.isAnyLODValid = (value & VSM_PHYSICAL_PAGE_ANY_MIP_VALID_FLAG) != 0
	table.isThisLODValid = table.isAnyLODValid and table.mip == 0
	return table

class PhysPageMetaData:
	flags = 0
	age = 0
	vPage = 0xFFFFFFFF
	vsmID = 0

def decodePhysPage(bytes, offset):
	data = PhysPageMetaData()
	data.flags, data.age, data.vPage, data.vsmID = st.unpack_from('IIII', bytes, offset)
	return data

class VSMWindow(qrd.CaptureViewer):
	FORMAT = 0
	BUF_NAME = 1
	TABLE_BUF_NAME = 2
	DEBUG_PIXEL_NAME = 3
	PHYS_PAGE_NAME = 4
	NUM = 5
	def __init__(self, ctx: qrd.CaptureContext, closeCallback):
		super().__init__()

		def addTextBox(name, key):
			horBox = self.mqt.CreateHorizontalContainer()
			self.mqt.AddWidget(self.vertBox, horBox)
			label = self.mqt.CreateLabel()
			self.mqt.SetWidgetText(label, name)
			self.mqt.AddWidget(horBox, label)
			textBox = self.mqt.CreateTextBox(True, partial(self._qt_setValue, key))
			self.mqt.AddWidget(horBox, textBox)
		
		def addButton(name, func):
			button = self.mqt.CreateButton(lambda c, w, t: func())
			self.mqt.SetWidgetText(button, name)
			self.mqt.AddWidget(self.vertBox, button)
		
		self.mqt : qrd.MiniQtHelper = ctx.Extensions().GetMiniQtHelper()
		self.ctx = ctx
		self.topWindow = self.mqt.CreateToplevelWidget("VSM", closeCallback)

		self.values = ['' for _ in range(VSMWindow.NUM) ]

		self.vertBox = self.mqt.CreateVerticalContainer()
		self.mqt.AddWidget(self.topWindow, self.vertBox)

		addTextBox('Buffer', VSMWindow.BUF_NAME)
		addTextBox('Format', VSMWindow.FORMAT)
		addTextBox('TableBuffer', VSMWindow.TABLE_BUF_NAME)
		addTextBox('PhysPageBuffer', VSMWindow.PHYS_PAGE_NAME)
		addTextBox('DebugClipPos [X] [Y]', VSMWindow.DEBUG_PIXEL_NAME)
		addButton('SetDebugPixel', self.setDebugPixel)

		self.debugPixelLabel = self.mqt.CreateLabel()
		self.mqt.SetWidgetText(self.debugPixelLabel, 'DebugPixel:')
		self.mqt.AddWidget(self.vertBox, self.debugPixelLabel)
		debugPixelButton = self.mqt.CreateButton(lambda c, w, t: self.updateDebugPixel())
		self.mqt.SetWidgetText(debugPixelButton, 'UpdateDebugPixel')
		self.mqt.AddWidget(self.vertBox, debugPixelButton)

		addButton('FilterBuffer', self.filterBuffer)
		addButton('MapPageFromFilterBuffer', self.mapPage)
		addButton('ShowPage', self.showPage)
		addButton('ShowPhysPage', self.showPhysPage)

		self.tableView = QtWidgets.QTableWidget()
		self.mqt.AddWidget(self.vertBox, self.tableView)

		ctx.AddCaptureViewer(self)
	
	def error_log(func):
		def wrapper(self):
			manager = self.ctx.Extensions()
			try:
				func(self)
			except:
				import traceback

				manager.MessageDialog("VSM debug fail\n%s" % traceback.format_exc(), "Error!~")

		return wrapper

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
	
	@error_log
	def filterBuffer(self):
		@process_proxy
		def process(data, ctrl):
			buf = self.getBuf(self[VSMWindow.BUF_NAME])
			rawData = ctrl.GetBufferData(buf.resourceId, 0, 0)
			formatStr = self[VSMWindow.FORMAT]
			stride = st.calcsize(formatStr)
			for i in range(0, len(rawData), stride):
				value = st.unpack_from(formatStr, rawData, i)[0]
				if value != 0:
					data.append((int(i / stride), value))
		proxy = ProcessProxy()
		self.ctx.Replay().BlockInvoke(partial(process, proxy))
		if proxy.check(self.ctx):
			self.showInTable(['Value'], proxy.data)
	
	@error_log
	def mapPage(self):
		@process_proxy
		def process(data, ctrl):
			buf = self.getBuf(self[VSMWindow.BUF_NAME])
			rawData = ctrl.GetBufferData(buf.resourceId, 0, 0)
			formatStr = self[VSMWindow.FORMAT]
			stride = st.calcsize(formatStr)
			indices = [int(i / stride) for i in range(0, len(rawData), stride) if st.unpack_from(formatStr, rawData, i)[0] != 0]
			tableBuf = self.getBuf(self[VSMWindow.TABLE_BUF_NAME])
			tableFormat = 'I'
			tableStride = st.calcsize(tableFormat)
			tableData = ctrl.GetBufferData(tableBuf.resourceId, 0, 0)
			table = [decodeTable(st.unpack_from(tableFormat, tableData, i)[0]) for i in range(0, len(tableData), tableStride)]
			for i in indices:
				t = table[i]
				vsmID, mipLevel, address = calcPageAddressFromFlatIndex(i)
				data.append((i, vsmID, mipLevel, address, t.page, t.mip,
				  	t.isAnyLODValid, t.isThisLODValid))
		proxy = ProcessProxy()
		self.ctx.Replay().BlockInvoke(partial(process, proxy))
		if proxy.check(self.ctx):
			self.showInTable(['VPage', 'VSMID', 'Mip', 'Address', 'PPage', 'LodOffset', 'AnyLODValid', 'ThisLODValid'], proxy.data)
	
	@error_log
	def showPage(self):
		@process_proxy
		def process(data, ctrl):
			buf = self.getBuf(self[VSMWindow.TABLE_BUF_NAME])
			rawData = ctrl.GetBufferData(buf.resourceId, 0, 0)
			formatStr = 'I'
			stride = st.calcsize(formatStr)
			for i in range(0, len(rawData), stride):
				value = st.unpack_from(formatStr, rawData, i)[0]
				if value != 0:
					t = decodeTable(value)
					data.append((t.page, t.mip,
				  		t.isAnyLODValid, t.isThisLODValid))
		proxy = ProcessProxy()
		self.ctx.Replay().BlockInvoke(partial(process, proxy))
		if proxy.check(self.ctx):
			self.showInTable(['PPage', 'Mip', 'AnyLODValid', 'ThisLODValid'], proxy.data)
	
	@error_log
	def showPhysPage(self):
		@process_proxy
		def process(data, ctrl):
			buf = self.getBuf(self[VSMWindow.PHYS_PAGE_NAME])
			rawData = ctrl.GetBufferData(buf.resourceId, 0, 0)
			formatStr = 'IIII'
			stride = st.calcsize(formatStr)
			for i in range(0, len(rawData), stride):
				page = decodePhysPage(rawData, i)
				if page.vPage != 0xFFFFFFFF:
					vOffset = page.vPage - calcPageTableLevelOffset(page.vsmID, 0)
					mip, vPage = calcPageAddressFromIndex(vOffset)
					pIndex = int(i / stride)
					pPage = physIndexToPageAddress(pIndex)
					data.append((pPage, pIndex, vPage, page.vPage, vOffset, mip, page.vsmID, page.flags, page.age))
		proxy = ProcessProxy()
		self.ctx.Replay().BlockInvoke(partial(process, proxy))
		if proxy.check(self.ctx):
			self.showInTable(['PPage', 'PIndex', 'VPage', 'VIndex', 'VOffset', 'Mip', 'VSMID', 'Flags', 'Age'], proxy.data)
	
	# @error_log
	# def debugShader(self):
	# 	pipelineState = self.ctx.CurPipelineState()
	# 	if pipelineState is None:
	# 		raise RuntimeError("PipelineState is None")
	# 	stage = rd.ShaderStage.Pixel
	# 	shader = pipelineState.GetShader(stage)
	# 	if shader is None:
	# 		raise RuntimeError("No pixel shader")
	# 	reflection = pipelineState.GetShaderReflection(stage)
	# 	binding = pipelineState.GetBindpointMapping(stage)
	# 	entry = pipelineState.GetShaderEntryPoint(stage)
	# 	trace = rd.ShaderDebugTrace()
	# 	trace.constantBlocks = binding.constantBlocks
	# 	self.ctx.DebugShader(binding, reflection, shader.GetGraphicsPipelineObject())

	@error_log
	def setDebugPixel(self):
		viewer = self.ctx.GetTextureViewer()
		if viewer is None:
			return
		pixelStr = self[VSMWindow.DEBUG_PIXEL_NAME]
		x, y, z, w = pixelStr.split(' ')
		# if not x or not x.isnumeric() or\
		# 	not y or not y.isnumeric() or\
		# 	not z or not z.isnumeric() or\
		# 	not w or not w.isnumeric():
		# 	return
		# viewer.GotoLocation(float(x) * 16384, float(y) * 16384)
		x = float(x)
		y = float(y)
		z = float(z)
		w = float(w)
		x = (x / w * 0.5 + 0.5) * 16384
		y = (y / w * 0.5 + 0.5) * 16384
		viewer.GotoLocation(int(x), int(y))

	@error_log
	def updateDebugPixel(self):
		viewer = self.ctx.GetTextureViewer()
		if viewer is None:
			return
		pos = viewer.GetPickedLocation()
		if pos[0] == -1 or pos[1] == -1:
			return
		grid = (int(pos[0] / VSM_PAGE_SIZE), int(pos[1] / VSM_PAGE_SIZE))
		index = physPageAddressToIndex(grid)
		self.mqt.SetWidgetText(self.debugPixelLabel, f'DebugPixel: {grid} {index}')
