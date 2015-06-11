#!/usr/bin/python2.7
# -*- coding: utf-8 -*-

import sys

from os.path import splitext
from os import getcwd

from PyQt4.QtGui import *
from PyQt4.QtCore import QTimer

from coordedit import MolArea
from paramedit import PWTab
from tools.toolarea import ToolArea

def MakeWindow(controller,old):

    mw = QMainWindow()
    mv = MainView(mw,controller,old)
    mw.setCentralWidget(mv)
    mw.setWindowTitle('PWToolBox')
    mw.show()
    return mv

class MainView(QWidget):

        def __init__(self,parent,controller,old):
            super(MainView,self).__init__()
            self.parent = parent
            self.controller = controller
            self.initMenu()
            self.mult =[1,1,1]

        #Central Column:
            #OpenGL Viewport:
            if old:
                from old.viewport import ViewPort
            else:
                from viewport import ViewPort
            self.visual = ViewPort(self)

            #Control visual:
            #Cell multiplication
            self.xspin = QSpinBox()
            self.yspin = QSpinBox()
            self.zspin = QSpinBox()
            for i in [self.xspin,self.yspin,self.zspin]:
                    i.setMinimum(1)
            #Toggle edit column:
            editBut = QPushButton()
            editBut.setText('Edit')
            editBut.setCheckable(True)
            #create Timers
            self.animTimer = QTimer()
            self.animTimer.setInterval(50)
            self.animTimer.timeout.connect(self.incStep)
            #Screenshot test
            screenbut = QPushButton()
            screenbut.setText('Save Screenshot')
            screenbut.clicked.connect(self.makeScreen)
            #Choose step, animate
            incBut = QPushButton()
            incBut.clicked.connect(self.incStep)
            incBut.setAutoRepeat(True)
            incBut.setIcon(self.style().standardIcon(65))
            decBut = QPushButton()
            decBut.clicked.connect(self.decStep)
            decBut.setAutoRepeat(True)
            decBut.setIcon(self.style().standardIcon(66))
            playBut = QPushButton()
            playBut.setIcon(self.style().standardIcon(60))
            playBut.clicked.connect(self.toggleAnim)
            firstBut = QPushButton()
            firstBut.setIcon(self.style().standardIcon(64))
            firstBut.clicked.connect(self.firstStep)
            lastBut = QPushButton()
            lastBut.setIcon(self.style().standardIcon(63))
            lastBut.clicked.connect(self.lastStep)
            self.Step = QSlider()
            self.Step.setOrientation(1)
            self.Step.setMinimum(1)
            self.Step.setMaximum(1)
            self.Step.setTickPosition(self.Step.TicksBelow)
            self.Step.setSingleStep(1)
            self.Step.valueChanged.connect(self.updateMolStep)
            self.curStep = QLabel('1')
            self.maxStep = QLabel('1')
            self.xspin.valueChanged.connect(self.updateMult)
            self.yspin.valueChanged.connect(self.updateMult)
            self.zspin.valueChanged.connect(self.updateMult)
            #Control Layout:
            mult = QHBoxLayout()
            mult.addWidget(QLabel('Cell multiply:'))
            mult.addStretch()
            mult.addWidget(QLabel('x:'))
            mult.addWidget(self.xspin)
            mult.addStretch()
            mult.addWidget(QLabel('y:'))
            mult.addWidget(self.yspin)
            mult.addStretch()
            mult.addWidget(QLabel('z:'))
            mult.addWidget(self.zspin)
            mult.addWidget(screenbut)
            steps = QHBoxLayout()
            steps.addWidget(firstBut)
            steps.addWidget(decBut)
            steps.addWidget(self.Step)
            steps.addWidget(self.curStep)
            steps.addWidget(QLabel('/'))
            steps.addWidget(self.maxStep)
            steps.addWidget(playBut)
            steps.addWidget(incBut)
            steps.addWidget(lastBut)
            steps.addWidget(editBut)

            #Layout:
            viewlay = QVBoxLayout()
            viewlay.addWidget(self.visual)
            viewlay.addLayout(mult)
            viewlay.addLayout(steps)

            #Frame it:
            mcol = QFrame()
            mcol.setLayout(viewlay)
            mcol.setFrameStyle(38)

        #Left column:
            #Molecule list:
            self.mlist = QListWidget()
            self.mlist.currentRowChanged.connect(self.selectMolecule)
            #splitter-container
            mlist=QWidget()
            mlayout=QVBoxLayout()
            mlabel=QLabel('Loaded Molecules:')
            mlayout.addWidget(mlabel)
            mlayout.addWidget(self.mlist)
            mlist.setLayout(mlayout)

            #PWParameter list:
            self.pwlist = QListWidget()
            self.pwlist.currentRowChanged.connect(self.selectPWParam)
            #splitter-container
            pwlist=QWidget()
            pwlayout=QVBoxLayout()
            pwlabel=QLabel('PW Parameter sets:')
            pwlayout.addWidget(pwlabel)
            pwlayout.addWidget(self.pwlist)
            pwlist.setLayout(pwlayout)


            #Edit stuff
            self.edit = ToolArea(self)

            #encapsulate in splitter:
            lcol = QSplitter()
            lcol.addWidget(mlist)
            lcol.addWidget(pwlist)
            lcol.addWidget(self.edit)
            lcol.setOrientation(0)
            lcol.setChildrenCollapsible(False)
            lcol.setFrameStyle(38)
            lcol.setMaximumWidth(300)


        #Right column:
            #Molecule edit area:
            self.coord = MolArea(self)

            #PWParameter edit area:
            self.pw = PWTab()

            #nest edit areas in tabwidget
            rcol = QTabWidget()
            rcol.addTab(self.coord,'Molecule coordinates')
            rcol.addTab(self.pw,'PW Parameters')
            rcol.setFixedWidth(467)
            #connect to toggle button
            rcol.hide()
            editBut.clicked.connect(rcol.setVisible)

        #Lay out columns:
            hbox = QHBoxLayout()
            hbox.addWidget(lcol)
            hbox.addWidget(mcol)
            hbox.addWidget(rcol)
            self.setLayout(hbox)

        def initMenu(self):
            newAction = QAction('&New Molecule',self)
            newAction.setShortcut('Ctrl+N')
            newAction.triggered.connect(self.newHandler)
            loadAction = QAction('&Load Molecule(s)',self)
            loadAction.setShortcut('Ctrl+O')
            loadAction.triggered.connect(self.loadHandler)
            saveAction = QAction('&Save Molecule',self)
            saveAction.setShortcut('Ctrl+S')
            saveAction.triggered.connect(self.saveHandler)
            exitAction = QAction('&Exit',self)
            exitAction.setShortcut('Ctrl+Q')
            exitAction.triggered.connect(qApp.quit)

            fMenu = self.parent.menuBar().addMenu('&File')
            fMenu.addAction(newAction)
            fMenu.addAction(loadAction)
            fMenu.addAction(saveAction)
            fMenu.addSeparator()
            fMenu.addAction(exitAction)

        def newHandler(self):
            self.controller.newMol()
            self.loadView()

        def loadHandler(self):
            fname = QFileDialog.getOpenFileName(self,'Open File',getcwd())
            if not fname: return
            ftype = QInputDialog.getItem(self,'Choose file type','File type:',self.controller.indict.keys(),0,False)
            ftype = str(ftype[0])
            self.controller.readFile(ftype,fname)
            self.loadView()

        def saveHandler(self):
            fname = QFileDialog.getSaveFileName(self,'Save File',getcwd())
            if not fname: return
            ftype = QInputDialog.getItem(self,'Choose File type','File type:',self.controller.outdict.keys(),0,False)
            ftype = str(ftype[0])
            try:
                try:
                    mol = self.getMolecule()
                except:
                    raise IndexError('No Molecule')
                if ftype=='PWScf Input':
                    try:
                        param = self.getParam()
                    except:
                        raise IndexError('No PW Parameter set')
                else:
                        param = False
                coordfmt = self.coord.fmt.currentText()
                self.controller.writeFile(ftype,mol,fname,param,coordfmt)
            except StandardError as e:
                QMessageBox(QMessageBox.Critical,'Error',e.message,QMessageBox.Ok,self).exec_()

        ########################################################
        #insert loaded molecules
        ########################################################
        def loadView(self):
                count = self.mlist.count()
                for i in range(count,self.controller.get_nmol()):
                        self.mlist.addItem("Mol "+str(i+1))
                self.mlist.setCurrentRow(self.mlist.count()-1)
                count = self.pwlist.count()
                for i in range(count,self.controller.get_npw()):
                        self.pwlist.addItem("Param "+str(i+1))
                self.pwlist.setCurrentRow(self.pwlist.count()-1)

        ########################################################
        #get data from controller
        ########################################################
        def selectMolecule(self,sel):
                steps = self.controller.get_lmol(sel)
                self.maxStep.setText(str(steps))
                self.Step.setMaximum(steps)
                self.Step.setValue(steps)
                self.updateMolStep()

        def selectPWParam(self,sel):
                self.pw.setPW(self.controller.get_pw(sel))

        def updateMolStep(self):
                #get current Molecule from MolList
                sel = self.mlist.currentRow()
                step = self.Step.value()-1
                self.curStep.setText(str(step+1))
                mol = self.controller.get_mol(sel,step)
                #Send Molecule to Visualisation and Editor
                self.coord.setMol(mol)
                self.edit.setMol(mol)
                self.visual.setMol(mol,self.mult)

        def updateMult(self):
                self.mult=[self.xspin.value(),self.yspin.value(),self.zspin.value()]
                self.updateMolStep()

        ########################################################
        #to controller
        ########################################################
        def getMolecule(self):
                return self.controller.get_mol(self.mlist.currentRow(),self.Step.value()-1)

        def getParam(self):
                self.pw.saveParam()
                return self.controller.get_pw(self.pwlist.currentRow())


        ########################################################
        #screenshot test
        ########################################################
        def makeScreen(self):
                img = self.visual.grabFrameBuffer(True)
                fn = QFileDialog.getSaveFileName(self,'Save Screenshot',getcwd(),'Portable Network Graphics (*.png)')
                if not fn: return
                if splitext(str(fn))[1] == '': fn+='.png'
                img.save(fn,None,0)

        ########################################################
        #steps and animation:
        ########################################################
        def incStep(self):
                if self.Step.value()==self.Step.maximum():
                        self.animTimer.stop()
                else:
                        self.Step.setValue(self.Step.value()+1)

        def decStep(self):
                if self.Step.value()==1:return
                self.Step.setValue(self.Step.value()-1)

        def firstStep(self):
                self.Step.setValue(1)

        def lastStep(self):
                self.Step.setValue(self.Step.maximum())

        def toggleAnim(self):
                if self.animTimer.isActive():
                        self.animTimer.stop()
                else:
                        self.animTimer.start()

