#!BPY

""" Registration info for Blender menus:
Name: 'Motion capture (.bvh)...'
Blender: 249b
Group: 'Export'
Tooltip: 'BVH exporter'
"""
__author__ = "FooName"
__url__ = ("blender", "www.j00rURL.com")
__version__ = "1.0"
__bpydoc__ = """\
This is where you document the general workings of the script, keep it short but  informative.

Missing:<br>
    None.

Known issues:<br>
    None.

Notes:<br>
    TODO
"""

import bpy
import Blender
import Blender.Mathutils
from Blender.Mathutils import *

export_type = 'Xrotation Yrotation Zrotation'

class ArmatureInfo:

	__slots__ = ('mNodeInfoMap', 'mRoots')
	
	
	def __init__(self, inArmature):

		self.mNodeInfoMap = {}		
		self.mRoots = []
		
		for bone in inArmature.bones.values():
			node_info = self.GetNodeInfo(bone)
			if bone.parent == None:
				self.mRoots.append(node_info)
			


	def GetNodeInfo(self, inBone):
		
		node_info = None
		
		if (inBone != None):
			if (not self.mNodeInfoMap.has_key(inBone.name)):
				node_info = ArmatureNodeInfo(self, inBone)
				self.mNodeInfoMap[inBone.name] = node_info
			else:
				node_info = self.mNodeInfoMap[inBone.name]
			
		return node_info		


	def DoExport(self, inFilePath):
		
		for root in self.mRoots:
			self.ExportRoot(inFilePath, root)
		
		
	@staticmethod	
	def MakeIndent(inLoopCount):
		return ''.join(['\t' for num in xrange(inLoopCount)])	
		
	
	@staticmethod	
	def Indent(inFile, inLoopCount):
		inFile.write(ArmatureInfo.MakeIndent(inLoopCount))		
		
		
	@staticmethod		
	def ExportTranslationChannels(inFile):
		inFile.write('Xposition Yposition Zposition')
		
	@staticmethod		
	def ExportRotationChannels(inFile):
	#	inFile.write('Xrotation Yrotation Zrotation')	
	#	inFile.write('Xrotation Zrotation Yrotation')	
		global export_type
		inFile.write(export_type)	
	
		
	@staticmethod		
	def ExportRootChannels(inFile, inIndent):
		ArmatureInfo.Indent(inFile, inIndent)
		inFile.write('CHANNELS 6 ')
		ArmatureInfo.ExportTranslationChannels(inFile)
		inFile.write(' ')
		ArmatureInfo.ExportRotationChannels(inFile)
		inFile.write('\n')
		
	@staticmethod		
	def ExportJointChannels(inFile, inIndent):
		ArmatureInfo.Indent(inFile, inIndent)
		inFile.write('CHANNELS 3 ')
		ArmatureInfo.ExportRotationChannels(inFile)
		inFile.write('\n')	
		
	@staticmethod		
	def ExportMatrixTranslation(inFile, inMatrix):
		translation = inMatrix.translationPart()
		inFile.write(str(translation.x) + '\t' + str(translation.y) + '\t' + str(translation.z))

	@staticmethod		
	def GetMatrixTranslationString(inMatrix):
		translation = inMatrix.translationPart()
		return (str(translation.x) + ' ' + str(translation.y) + ' ' + str(translation.z))
		
	@staticmethod		
	def ExportPosition(inFile, inPosition):
		inFile.write(str(inPosition.x) + '\t' + str(inPosition.y) + '\t' + str(inPosition.z))	
		
	@staticmethod	
	def ExportPositionOffset(inFile, inIndent, inPosition):
		ArmatureInfo.Indent(inFile, inIndent)
		inFile.write('OFFSET\t')
		ArmatureInfo.ExportPosition(inFile, inPosition)
		inFile.write('\n')	
		
	@staticmethod	
	def ExportMatrixOffset(inFile, inIndent, inMatrix):
		ArmatureInfo.ExportPositionOffset(inFile, inIndent, inMatrix.translationPart())


	@staticmethod		
	def GetMatrixRotationString(inMatrix):
		euler = inMatrix.toEuler()
		return (str(euler.x) + ' ' + str(euler.y) + ' ' + str(euler.z))

		
	@staticmethod	
	def ExportEndsite(inFile, inIndent, inNode):	
		ArmatureInfo.Indent(inFile, inIndent)		
		inFile.write('END SITE\n')
		ArmatureInfo.Indent(inFile, inIndent)		
		inFile.write('{\n')
		ArmatureInfo.ExportPositionOffset(inFile, inIndent+1, inNode.mLocalEndsiteOffset)
		ArmatureInfo.Indent(inFile, inIndent)		
		inFile.write('}\n')
		
		
	def ExportRoot(self, ioFilePath, inRoot):	
		
		if (len(self.mRoots) > 1):
			ioFilePath += "_" + inRoot.mBone.name
			
		print ioFilePath
		
		f = open(ioFilePath + ".bvh", "w")
		f.write('HIERARCHY\n')
		f.write('ROOT ' + inRoot.mBone.name + '\n')
		f.write('{\n')
		
		indent = 1	
		motion_strings = []
				
		ArmatureInfo.ExportMatrixOffset(f, indent, inRoot.mLocalMatrix)
		ArmatureInfo.ExportRootChannels(f, indent)
		f.write('\n')

		motion_strings.append(ArmatureInfo.GetMatrixTranslationString(inRoot.mLocalMatrix))
		motion_strings.append(ArmatureInfo.GetMatrixRotationString(inRoot.mLocalMatrix))

		ArmatureInfo.ExportEndsite(f, indent, inRoot)
		f.write('\n')

		for child in inRoot.mChildren:
			ArmatureInfo.ExportJoint(f, indent, child, motion_strings)
				
		f.write('}\n')		
		
		f.write('MOTION\n')
		f.write('Frames: 1\n')
		f.write('Frame Time: 0.033333\n')
		
		for motion_string in motion_strings:
			f.write(motion_string)
			f.write(' ')
		
		f.write('\n')	

				
				
	@staticmethod		
	def ExportJoint(inFile, inIndent, inNode, inMotionStrings):
		
		ArmatureInfo.Indent(inFile, inIndent)				
		inFile.write("JOINT " + inNode.mBone.name + '\n')
		ArmatureInfo.Indent(inFile, inIndent)				
		inFile.write('{\n')
		ArmatureInfo.ExportMatrixOffset(inFile, inIndent+1, inNode.mLocalMatrix)
		inMotionStrings.append(ArmatureInfo.GetMatrixRotationString(inNode.mLocalMatrix))
		ArmatureInfo.ExportJointChannels(inFile, inIndent+1)
		
		inFile.write('\n')
		ArmatureInfo.ExportEndsite(inFile, inIndent+1, inNode)
		
		#inFile.write('\n')
		for child in inNode.mChildren:
			ArmatureInfo.ExportJoint(inFile, inIndent+1, child, inMotionStrings)	
		ArmatureInfo.Indent(inFile, inIndent)	
		inFile.write('}\n')
		inFile.write('\n')





class ArmatureNodeInfo:

	__slots__ = ('mBone', 'mIsRoot', 'mArmatureMatrix', 'mLocalMatrix', 'mLocalEndsiteOffset' 'mParent', 'mChildren')


	def __init__(self, inArmatureInfo, inBone):
		
		print(inBone.name)
		
		self.mChildren = []
		self.mParent = inArmatureInfo.GetNodeInfo(inBone.parent)
		
		if (self.mParent != None):
			self.mParent.AddChild(self)
		
		
		self.mBone = inBone
		self.mIsRoot = (inBone.parent == None)
		self.mArmatureMatrix = inBone.matrix["ARMATURESPACE"]
		
		parent_world_inv = Matrix()
		parent_world_inv.identity()
		
		if (self.mParent != None):
			parent_world_inv = self.mParent.mArmatureMatrix.copy()
			parent_world_inv.invert()
			
		self.mLocalMatrix = self.mArmatureMatrix * parent_world_inv

		head_world_matrix_inv = self.mArmatureMatrix.copy().invert()
		tail_world_matrix = self.mArmatureMatrix.rotationPart().copy() 
		tail_world_matrix.resize4x4()
		tail_world_matrix = tail_world_matrix * TranslationMatrix(self.mBone.tail["ARMATURESPACE"])

		tail_local_matrix = tail_world_matrix * head_world_matrix_inv
		self.mLocalEndsiteOffset = tail_local_matrix.translationPart()

#		print(self.mArmatureMatrix)			
#		print(self.mLocalMatrix)
#		print(tail_world_matrix)
#		print(tail_local_matrix.translationPart())
		
		#print(self.mArmatureMatrix.translationPart())
		#print(self.mLocalMatrix.translationPart())
		#euler = self.mArmatureMatrix.toEuler()
		#print(euler)
	

	def __str__(self):
		return "Info for " + self.mBone.name		


	def AddChild(self, inChild):
		self.mChildren.append(inChild)






def Export():
	
	export_armature = list(bpy.data.armatures)[0]
	print(export_armature)
	armature_info = ArmatureInfo(export_armature)
	
	global export_type
	
	export_type = 'Xrotation Yrotation Zrotation'
	armature_info.DoExport("d:\jad\Dev\AiGameDev\BlenderToBVH\proto1_export_test_xyz")
	
	export_type = 'Xrotation Zrotation Yrotation'
	armature_info.DoExport("d:\jad\Dev\AiGameDev\BlenderToBVH\proto1_export_test_xzy")
	
	export_type = 'Yrotation Xrotation Zrotation'
	armature_info.DoExport("d:\jad\Dev\AiGameDev\BlenderToBVH\proto1_export_test_yxz")
	
	export_type = 'Yrotation Zrotation Xrotation'
	armature_info.DoExport("d:\jad\Dev\AiGameDev\BlenderToBVH\proto1_export_test_yzx")
	
	export_type = 'Zrotation Xrotation Yrotation'
	armature_info.DoExport("d:\jad\Dev\AiGameDev\BlenderToBVH\proto1_export_test_zxy")
	
	export_type = 'Zrotation Yrotation Xrotation'
	armature_info.DoExport("d:\jad\Dev\AiGameDev\BlenderToBVH\proto1_export_test_zyx")
	

#
#matrix operation order test
#conslusion is: Op2 * Op1 meaning my_world = my_local * my_parent_world
#
#trans_m = TranslationMatrix(Vector([1,2,3]))
#trans_m.resize4x4()
#rot_m = RotationMatrix(30, 3, "r", Vector([0.2, 0.1, 0.3]))
#rot_m.resize4x4()
#print(trans_m * rot_m)
#print(rot_m * trans_m)
	
Export()	