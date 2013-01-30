import sublime, sublime_plugin
import subprocess
import os
import sys

def runPiped(args):
	proc = subprocess.Popen(args, stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell=False)
	return proc.communicate()

def runHiddenPiped(args):
	startupinfo = subprocess.STARTUPINFO()
	startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
	proc = subprocess.Popen(args, stdout = subprocess.PIPE, stderr = subprocess.PIPE, shell=False, startupinfo=startupinfo)
	return proc.communicate()


class JadP4InfoCommand(sublime_plugin.TextCommand):
	def run(self, edit):
		(out, err) = runHiddenPiped(["p4", "-c", "jad.nohra.sandy1", "-p", "10.80.1.2:1666", "-u", "jad.nohra", "info"])
		print out, err


class JadCheckoutCommand(sublime_plugin.TextCommand):
	def run(self, edit):
		args=["p4", "-c", "jad.nohra.sandy1", "-p", "10.80.1.2:1666", "-u", "jad.nohra", "edit", self.view.file_name()]
		(out, err) = runHiddenPiped(args)
		print out, err


class JadCheckoutSaveCommand(sublime_plugin.TextCommand):
	def run(self, edit):
			self.view.run_command('jad_checkout')
			self.view.run_command('save')


class JadCopyPathCommand(sublime_plugin.TextCommand):
	def run(self, edit):
			sublime.set_clipboard(self.view.file_name())


def isHavokBuildDir(path):
	dirEntries = os.listdir(path)
	for entry in dirEntries:
		if entry == "Build":
			return True
	return False

def dirBack(path):
	dirs = path.split('\\')
	dirs.pop()
	return '\\'.join(dirs)

def findView(window, name):
	views = window.views()
	for entry in views:
		if entry.name() == name:
			return entry
	return None		



class JadBuildPyCommand(sublime_plugin.TextCommand):
	def run(self, edit):
		path = dirBack(self.view.file_name().replace('/', '\\'))
		print path
		while len(path) > 0:
			if isHavokBuildDir(path):
				buildpy=path + '\\Build\\ReleaseSystem\\build.py'
				(out, err) = runPiped(["python", '-u', buildpy, 'again', '--no-gui'])
				outview=findView(self.view.window(), buildpy)
				if outview is None:
					outview=self.view.window().new_file()
					outview.set_name(buildpy)
					outview.set_scratch(True)
				else:
					outview.set_read_only(False)
					edit=outview.begin_edit()
					outview.erase(edit, sublime.Region(0, outview.size()))
					outview.end_edit(edit)
				edit=outview.begin_edit()
				outview.insert(edit, 0, out)
				outview.insert(edit, 0, "\n")
				outview.insert(edit, 0, err)
				outview.end_edit(edit)
				outview.set_read_only(True)
				self.view.window().focus_view(outview)
				#print out, err
				return
			path=dirBack(path)



class JadCloseBuildPyCommand(sublime_plugin.TextCommand):
	def run(self, edit):
		views = self.view.window().views()
		for entry in views:
			if entry.name().endswith('build.py'):
				self.view.window().focus_view(entry)
				entry.window().run_command("close_file")