#!/usr/bin/env python3
import subprocess, shlex
import sys

class Interactor:
	def __init__(self,command,errtoout=False,errto=None):
		if type(command)==str:
			command=shlex.split(command)
		if errtoout:
			errto=subprocess.STDOUT
		elif errto==None:
			errto=subprocess.PIPE
		self.proc=subprocess.Popen(
			command,
			bufsize=0,
			stdin=subprocess.PIPE,
			stdout=subprocess.PIPE,
			stderr=errto)

		self.readbuffer=["",""]

	def __enter__(self):
		return self

	def __exit__(self,exc_type,exc_value,traceback):
		self.proc.kill()
		return False

	def write(self,data):
		if type(data)==str:
			data=data.encode("utf-8")
		cursor=0
		while cursor<len(data):
			count=self.proc.stdin.write(data if cursor==0 else data[cursor:])
			if count==None:
				raise EOFError("Could not write to stdin of process")
			if count==0:
				raise EOFError("Stdin of process closed unexpectedly")
			cursor+=count

	def getline(self,terminator="\n",stderr=False):
		bufidx=1 if stderr else 0
		idx=self.readbuffer[bufidx].find(terminator)
		prevlen=0
		if idx==-1:
			prevlen+=len(self.readbuffer[bufidx])
		while idx==-1:
			buf=(self.proc.stderr if stderr else self.proc.stdout).read(1024)
			if buf==None:
				raise EOFError("Could not read from "+("stderr" if stderr else "stdout")+" of process")
			if len(buf)==0:
				raise EOFError("EOF on read from "+("stderr" if stderr else "stdout")+" of process")
			buf=buf.decode("utf-8")
			self.readbuffer[bufidx]+=buf
			idx=self.readbuffer[bufidx].find(terminator,prevlen)
			if idx==-1:
				prevlen+=len(buf)
		# print("buffer="+repr(self.readbuffer[bufidx])+" idx="+str(idx)+" prevlen="+str(prevlen))
		idx+=prevlen
		line=self.readbuffer[bufidx][:idx]
		self.readbuffer[bufidx]=self.readbuffer[bufidx][idx+len(terminator):]
		return line

	def expect(self,s,stderr=False):
		if len(s)==0:
			return
		bufidx=1 if stderr else 0
		doraise=False
		while len(self.readbuffer[bufidx])<len(s):
			buf=(self.proc.stderr if stderr else self.proc.stdout).read(1024)
			if buf==None:
				raise EOFError("Could not read from "+("stderr" if stderr else "stdout")+" of process")
			if len(buf)==0:
				raise EOFError("EOF on read from "+("stderr" if stderr else "stdout")+" of process")
			buf=buf.decode("utf-8")
			self.readbuffer[bufidx]+=buf
			if len(self.readbuffer[bufidx])<len(s) and self.readbuffer[bufidx]!=s[:len(self.readbuffer[bufidx])]:
				doraise=True
				break
		if doraise or self.readbuffer[bufidx][:len(s)]!=s:
			raise AssertionError("Expected string "+repr(s)+" not found on "+("stderr" if stderr else "stdout")+" of process, rather "+repr(self.readbuffer[bufidx]))
		self.readbuffer[bufidx]=self.readbuffer[bufidx][len(s):]

	def pipe(self,out,stderr=False):
		bufidx=1 if stderr else 0
		out.write(self.readbuffer[bufidx])
		self.readbuffer[bufidx]=""
		while True:
			buf=(self.proc.stderr if stderr else self.proc.stdout).read(1024)
			if buf==None:
				raise EOFError("Could not read from "+("stderr" if stderr else "stdout")+" of process")
			if len(buf)==0: #actual EOF
				break
			out.write(buf.decode("utf-8"))
		out.flush()


def setequal(a,b):
	if len(a)!=len(b):
		return False
	return sorted(a)==sorted(b)

def changedcheck(expected):
	return lambda s: s[:9]=="Changed: " and setequal(s[9:].split(),expected)

testcases=[]
testcases.append(("CellAddress conversion",[
	["ca 0 0","A1"],
	["ca 0 1","B1"],
	["ca 1 1","B2"],
	["ca 100 1","B101"],
	["ca 1 100","CW2"],
	["ca A1","row=0 column=0"],
	["ca B1","row=0 column=1"],
	["ca B2","row=1 column=1"],
	["ca B101","row=100 column=1"],
	["ca CW2","row=1 column=100"],
]))
testcases.append(("Ensure size",[
	["disp D2","Out of bounds address"],
	["ensure 4 4"],
	["disp D2","\"\""],
]))
testcases.append(("Changing",[
	["ensure 3 3"],
	["change C3 10","Changed: C3"],
	["change B1 20","Changed: B1"],
]))
testcases.append(("Display string",[
	["ensure 3 3"],
	["change C3 10","Changed: C3"],
	["disp C3","\"10\""],
	["change B1 20","Changed: B1"],
	["disp B1","\"20\""],
]))
testcases.append(("Edit string",[
	["ensure 3 3"],
	["change C3 10","Changed: C3"],
	["edit C3","\"10\""],
	["change B1 20","Changed: B1"],
	["edit B1","\"20\""],
]))
testcases.append(("Formulas",[
	["ensure 3 3"],
	["change A1 10","Changed: A1"],
	["change B1 20","Changed: B1"],
	["change C1 30","Changed: C1"],

	["change A2 =A1 B1 C1","Changed: A2"],
	["disp A2","\"10 20 30\""],
	["edit A2","\"=A1 B1 C1\""],
]))
testcases.append(("Formula updating",[
	["ensure 3 3"],
	["change A1 10","Changed: A1"],
	["change B1 20","Changed: B1"],
	["change C1 30","Changed: C1"],

	["change A2 =A1 B1 C1","Changed: A2"],

	["change B1 42",changedcheck(["B1","A2"])],
	["disp B1","\"42\""],
	["disp A2","\"10 42 30\""],
	["edit A2","\"=A1 B1 C1\""],
]))
testcases.append(("Formula updating restore",[
	["ensure 3 3"],
	["change A1 10","Changed: A1"],
	["change B1 20","Changed: B1"],
	["change C1 30","Changed: C1"],
	["change A2 =A1 B1 C1","Changed: A2"],
	["change B1 42",changedcheck(["B1","A2"])],
	["change A2 hoi","Changed: A2"],
	["change C1 41","Changed: C1"],
]))
testcases.append(("Formula updating deeper",[
	["ensure 3 3"],
	["change A1 1","Changed: A1"],
	["change A2 2","Changed: A2"],
	["change A3 3","Changed: A3"],
	["change B1 4","Changed: B1"],
	["change B2 5","Changed: B2"],
	["change B3 6","Changed: B3"],
	["change C1 =A1 A2 A3","Changed: C1"],
	["change C2 =B1 B2 B3","Changed: C2"],
	["change C3 =C1 C2","Changed: C3"],
	["change B1 42",changedcheck(["B1","C2","C3"])],
	["disp C1","\"1 2 3\""],
	["disp C2","\"42 5 6\""],
	["disp C3","\"1 2 3 42 5 6\""],
	["change A2 hoi",changedcheck(["A2","C1","C3"])],
	["disp C1","\"1 hoi 3\""],
	["disp C2","\"42 5 6\""],
	["disp C3","\"1 hoi 3 42 5 6\""],
	["change C1",changedcheck(["C1","C3"])],
	["change A3 x","Changed: A3"],
	["change B3 x",changedcheck(["B3","C2","C3"])],
	["disp C3","\" 42 5 x\""]
]))
testcases.append(("Formula circular dependency detection",[
	["ensure 3 3"],
	["change A1 a","Changed: A1"],
	["change A2 =A1","Changed: A2"],
	["change A3 =A2","Changed: A3"],
	["disp A1","\"a\""],
	["disp A2","\"a\""],
	["disp A3","\"a\""],

	["change A1 rip",changedcheck(["A1","A2","A3"])],
	["disp A1","\"rip\""],
	["disp A2","\"rip\""],
	["disp A3","\"rip\""],

	["change A1 =A3",changedcheck(["A1","A2","A3"])],
	["disp A1","\"ERR:Circular reference chain\""],
	["disp A2","\"ERR:Circular reference chain\""],
	["disp A3","\"ERR:Circular reference chain\""],

	["change A2 42",changedcheck(["A1","A2","A3"])],
	["disp A1","\"42\""],
	["disp A2","\"42\""],
	["disp A3","\"42\""],
	["edit A1","\"=A3\""],
	["edit A2","\"42\""],
	["edit A3","\"=A2\""],
]))


if __name__=="__main__":
	with Interactor("make",errtoout=True) as proc:
		line=proc.getline()
		if line!="make: Nothing to be done for `all'.":
			# print("\x1B[34m",end="")
			print(line)
			proc.pipe(sys.stdout)
			# print("\x1B[0m",end="")

	class TestingError(Exception): pass

	try:
		for tcidx,testcase in enumerate(testcases,start=1):
			with Interactor("./main",errto=sys.stderr) as proc:
				try:
					for itidx,item in enumerate(testcase[1],start=1):
						proc.expect("> ")
						proc.write(item[0])
						proc.write("\n")
						if len(item)>1:
							if type(item[1])==str:
								proc.expect(item[1])
								proc.expect("\n")
							else:
								line=proc.getline()
								if not item[1](line):
									raise AssertionError("Lambda returned false on "+repr(line))
					proc.expect("> ")
				except EOFError as e:
					print("EOFError while running testcase '"+testcase[0]+"' item "+str(itidx)+":")
					print(e)
					raise TestingError
				except AssertionError as e:
					print("AssertionError while running testcase '"+testcase[0]+"' item "+str(itidx)+":")
					print(e)
					raise TestingError
			print("\x1B[32m-- Test case OK: '"+testcase[0]+"'\x1B[0m")
	except TestingError:
		print("\x1B[31m-- Test case ERR: '"+testcase[0]+"'\x1B[0m")
		# for itidx2,item in enumerate(testcase[1],start=1):
		# 	print(item[0])
		# 	if itidx2>=itidx:
		# 		break
