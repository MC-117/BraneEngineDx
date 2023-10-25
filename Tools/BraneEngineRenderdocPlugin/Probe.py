import qrenderdoc as qrd
from PySide2 import QtWidgets, QtCore
from .Core import *
import struct as st
import math
from functools import partial

def getProbeSphere(data):
	return st.unpack_from("ffff", data)

def getProbeType(data, probeDataStride):
	return st.unpack_from("I", data, probeDataStride - 4)[0]

def getProbeTypeName(typeID):
	typeNames = ["PointLight", "Reflection", "EnvLight"]
	if typeID >= len(typeNames):
		return "Error"
	return typeNames[typeID]

class EnvLightProbe:
	def __init__(self, data):
		vec = st.unpack_from("ffff", data)
		self.position = vec[0:3]
		self.radius = vec[3]
		vec = st.unpack_from("fffI", data, 16)
		self.tintColor = vec[0:3]
		self.reverseIndex = vec[3]
		self.falloff, self.cutoff, self.shDataOffset = st.unpack_from("ffI", data, 32)

PROBE_TYPE_LIGHT = 0
PROBE_TYPE_REF = 1
PROBE_TYPE_ENV = 2

class ProbeWindow(BaseWindow):
	PROBE_DATA_SIZE = 0
	PROBE_BUF_NAME = 1
	NUM = 2
	def __init__(self, ctx: qrd.CaptureContext, closeCallback):
		super().__init__("Probe", ctx, closeCallback)
		
		self.values = ['' for _ in range(ProbeWindow.NUM) ]
		self.addTextBox('ProbeDataSize', ProbeWindow.PROBE_DATA_SIZE)
		self.addTextBox('ProbeBuffer', ProbeWindow.PROBE_BUF_NAME)

		self.addButton('ShowProbe', self.showProbe)
		self.addButton('ShowEnvProbe', self.showEnvProbe)
		self.mqt.AddWidget(self.vertBox, self.tableView)

		ctx.AddCaptureViewer(self)
	
	@BaseWindow.error_log
	def showProbe(self):
		@process_proxy
		def process(data, ctrl):
			buf = self.getBuf(self[ProbeWindow.PROBE_BUF_NAME])
			rawData = ctrl.GetBufferData(buf.resourceId, 0, 0)
			stride = int(self[ProbeWindow.PROBE_DATA_SIZE])
			for i in range(0, len(rawData), stride):
				probeData = rawData[i:i+stride]
				typeID = getProbeType(probeData, stride)
				sphere = getProbeSphere(probeData)
				data.append((getProbeTypeName(typeID), sphere[0:3], sphere[3]))
		proxy = ProcessProxy()
		self.ctx.Replay().BlockInvoke(partial(process, proxy))
		if proxy.check(self.ctx):
			self.showInTable(['Type', 'Pos', 'Radius'], proxy.data)
	
	@BaseWindow.error_log
	def showEnvProbe(self):
		@process_proxy
		def process(data, ctrl):
			buf = self.getBuf(self[ProbeWindow.PROBE_BUF_NAME])
			rawData = ctrl.GetBufferData(buf.resourceId, 0, 0)
			stride = int(self[ProbeWindow.PROBE_DATA_SIZE])
			for i in range(0, len(rawData), stride):
				probeData = rawData[i:i+stride]
				typeID = getProbeType(probeData, stride)
				if typeID == PROBE_TYPE_ENV:
					envProbe = EnvLightProbe(probeData)
					data.append((envProbe.position, envProbe.radius,
				  		envProbe.tintColor, envProbe.reverseIndex,
						envProbe.falloff, envProbe.cutoff,
						envProbe.shDataOffset))
		proxy = ProcessProxy()
		self.ctx.Replay().BlockInvoke(partial(process, proxy))
		if proxy.check(self.ctx):
			self.showInTable(['Pos', 'Radius', 'Tint', 'RIdx', 'Fall', 'Cut', 'SHOff'], proxy.data)
