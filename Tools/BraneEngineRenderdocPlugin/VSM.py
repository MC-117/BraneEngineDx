import qrenderdoc as qrd
from PySide2 import QtWidgets, QtCore
from .Core import *
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

class VSMWindow(BaseWindow):
	FORMAT = 0
	BUF_NAME = 1
	TABLE_BUF_NAME = 2
	DEBUG_PIXEL_NAME = 3
	PHYS_PAGE_NAME = 4
	NUM = 5
	def __init__(self, ctx: qrd.CaptureContext, closeCallback):
		super().__init__("VSM", ctx, closeCallback)

		self.values = ['' for _ in range(VSMWindow.NUM) ]

		self.addTextBox('Buffer', VSMWindow.BUF_NAME)
		self.addTextBox('Format', VSMWindow.FORMAT)
		self.addTextBox('TableBuffer', VSMWindow.TABLE_BUF_NAME)
		self.addTextBox('PhysPageBuffer', VSMWindow.PHYS_PAGE_NAME)
		self.addTextBox('DebugClipPos [X] [Y]', VSMWindow.DEBUG_PIXEL_NAME)
		self.addButton('SetDebugPixel', self.setDebugPixel)

		self.debugPixelLabel = self.mqt.CreateLabel()
		self.mqt.SetWidgetText(self.debugPixelLabel, 'DebugPixel:')
		self.mqt.AddWidget(self.vertBox, self.debugPixelLabel)
		debugPixelButton = self.mqt.CreateButton(lambda c, w, t: self.updateDebugPixel())
		self.mqt.SetWidgetText(debugPixelButton, 'UpdateDebugPixel')
		self.mqt.AddWidget(self.vertBox, debugPixelButton)

		self.addButton('FilterBuffer', self.filterBuffer)
		self.addButton('MapPageFromFilterBuffer', self.mapPage)
		self.addButton('ShowPage', self.showPage)
		self.addButton('ShowPhysPage', self.showPhysPage)

		self.tableView = QtWidgets.QTableWidget()
		self.mqt.AddWidget(self.vertBox, self.tableView)

		ctx.AddCaptureViewer(self)
	
	@BaseWindow.error_log
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
	
	@BaseWindow.error_log
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
	
	@BaseWindow.error_log
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
	
	@BaseWindow.error_log
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

	@BaseWindow.error_log
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

	@BaseWindow.error_log
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
