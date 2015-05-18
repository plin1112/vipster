#!/usr/bin/python2.7
# -*- coding: utf-8 -*-

from PyQt4.QtGui import *
from numpy.linalg import norm
from numpy import degrees,arccos,dot,cross
from itertools import combinations

class ToolArea(QWidget):
        def __init__(self,parent):
                super(ToolArea,self).__init__()
                self.parent = parent
                self.stack = QStackedWidget()
                self.combo = QComboBox()
                self.combo.currentIndexChanged.connect(self.stack.setCurrentIndex)
                vbox = QVBoxLayout()
                hbox = QHBoxLayout()
                hbox.addWidget(QLabel('Tools:'))
                hbox.addWidget(self.combo)
                vbox.addLayout(hbox)
                vbox.addWidget(self.stack)
                self.setLayout(vbox)
                #initialize childwidgets (in order):
                self.initPicker()
                self.initScript()
                self.initMod()
                self.initVol()
                self.initPlane()

        def setMol(self,mol):
                self.mol = mol
                #inform childwidgets about new molecule:
                self.volUpdate()
                self.pickUpdate()

####################################
# Modify unit cell
####################################

        def initMod(self):
            self.mult = QWidget()
            self.combo.addItem('Mod. cell')
            self.stack.addWidget(self.mult)

            wrapBut = QPushButton('Wrap atoms')
            wrapBut.clicked.connect(self.modHandler)
            cropBut = QPushButton('Crop atoms')
            cropBut.clicked.connect(self.modHandler)

            self.xmult = QSpinBox()
            self.xmult.setMinimum(1)
            self.ymult = QSpinBox()
            self.ymult.setMinimum(1)
            self.zmult = QSpinBox()
            self.zmult.setMinimum(1)
            multBut = QPushButton('Multiply cell')
            multBut.clicked.connect(self.modHandler)
            mbox = QHBoxLayout()
            mbox.addWidget(QLabel('x:'))
            mbox.addWidget(self.xmult)
            mbox.addWidget(QLabel('y:'))
            mbox.addWidget(self.ymult)
            mbox.addWidget(QLabel('z:'))
            mbox.addWidget(self.zmult)

            self.reshape = QTableWidget()
            self.reshape.setColumnCount(3)
            self.reshape.setRowCount(3)
            self.reshape.setFixedHeight(120)
            self.reshape.setColumnWidth(0,84)
            self.reshape.setColumnWidth(1,84)
            self.reshape.setColumnWidth(2,84)
            self.reshape.setHorizontalHeaderLabels(['x','y','z'])
            for i in range(3):
                for j in range(3):
                    self.reshape.setItem(i,j,QTableWidgetItem(str(0.0)))
            rBut = QPushButton('Reshape cell')
            rBut.clicked.connect(self.modHandler)

            self.alignVec = QComboBox()
            self.alignDir = QComboBox()
            for i,j in enumerate(['x','y','z']):
                self.alignVec.addItem(str(i))
                self.alignDir.addItem(j)
            self.alignBut = QPushButton('Align cell')
            self.alignBut.clicked.connect(self.modHandler)
            abox = QHBoxLayout()
            abox.addWidget(self.alignVec)
            abox.addWidget(self.alignDir)

            vbox = QVBoxLayout()
            vbox.addWidget(wrapBut)
            vbox.addWidget(cropBut)
            vbox.addLayout(mbox)
            vbox.addWidget(multBut)
            vbox.addWidget(self.reshape)
            vbox.addWidget(rBut)
            vbox.addLayout(abox)
            vbox.addWidget(self.alignBut)
            vbox.addStretch()
            self.mult.setLayout(vbox)

        def modHandler(self):
            if not hasattr(self,'mol'): return
            reason=self.sender().text()
            if reason=='Multiply cell':
                self.mol.mult(int(self.xmult.text()),int(self.ymult.text()),int(self.zmult.text()))
            elif reason=='Reshape cell':
                vec=[[0,0,0],[0,0,0],[0,0,0]]
                for i in [0,1,2]:
                        for j in [0,1,2]:
                                vec[i][j]=float(self.reshape.item(i,j).text())
                self.mol.reshape(vec)
            elif reason=='Wrap atoms':
                self.mol.wrap()
            elif reason=='Crop atoms':
                self.mol.crop()
            elif reason=='Align cell':
                self.mol.align(self.alignVec.currentText(),self.alignDir.currentText())
            self.mol.set_bonds()
            self.parent.updateMolStep()

####################################
# Crystal plane
####################################

        def initPlane(self):
                self.plane = QWidget()
                self.combo.addItem('Plane')
                self.stack.addWidget(self.plane)
                self.planeBut = QPushButton('Show/Hide')
                self.planeBut.clicked.connect(self.planeBHandler)
                hbox2 = QHBoxLayout()
                hbox2.addWidget(QLabel('h:'))
                hbox2.addWidget(QLabel('k:'))
                hbox2.addWidget(QLabel('l:'))
                hbox = QHBoxLayout()
                self.ph = QSpinBox()
                self.pk = QSpinBox()
                self.pl = QSpinBox()
                for i in [self.ph,self.pk,self.pl]:
                    hbox.addWidget(i)
                    i.valueChanged.connect(self.planeSHandler)
                    i.setMinimum(-99)
                vbox = QVBoxLayout()
                vbox.addLayout(hbox2)
                vbox.addLayout(hbox)
                vbox.addWidget(self.planeBut)
                vbox.addStretch()
                self.plane.setLayout(vbox)

        def planeSHandler(self):
                self.parent.visual.setPlane('c',[
                    1./self.ph.value() if self.ph.value() else 0,
                    1./self.pk.value() if self.pk.value() else 0,
                    1./self.pl.value() if self.pl.value() else 0])

        def planeBHandler(self):
                self.planeSHandler()
                self.parent.visual.togglePlane()

####################################
# Volume-PP
####################################

        def initVol(self):
                self.vol = QWidget()
                self.combo.addItem('Volume')
                self.stack.addWidget(self.vol)
                volMin = QLabel('1')
                self.volDir = QComboBox()
                for i in 'xyz':
                    self.volDir.addItem(i)
                self.volDir.currentIndexChanged.connect(self.volSHandler)
                self.volDir.setDisabled(True)
                self.volSel = QSlider()
                self.volSel.setDisabled(True)
                self.volSel.setOrientation(1)
                self.volSel.setMinimum(1)
                self.volSel.setMaximum(1)
                self.volSel.setTickPosition(self.volSel.TicksBelow)
                self.volSel.setSingleStep(1)
                self.volSel.valueChanged.connect(self.volSHandler)
                self.volMax = QLabel('1')
                self.volBut = QPushButton('Show/Hide')
                self.volBut.clicked.connect(self.volBHandler)
                self.volBut.setDisabled(True)
                hbox=QHBoxLayout()
                hbox.addWidget(volMin)
                hbox.addWidget(self.volSel)
                hbox.addWidget(self.volMax)
                hbox2=QHBoxLayout()
                hbox2.addWidget(QLabel('Plane:'))
                hbox2.addWidget(self.volDir)
                vbox=QVBoxLayout()
                vbox.addLayout(hbox2)
                vbox.addLayout(hbox)
                vbox.addWidget(self.volBut)
                vbox.addStretch()
                self.vol.setLayout(vbox)

        def volSHandler(self):
                self.parent.visual.setPlane(str(self.volDir.currentText()),self.volSel.value()-1.)

        def volBHandler(self):
                self.volSHandler()
                self.parent.visual.togglePlane()

        def volUpdate(self):
                if hasattr(self.mol,'_vol'):
                    lim=self.mol.get_vol().shape[2]
                    self.volSel.setMaximum(lim)
                    self.volSel.setTickInterval(lim/10)
                    self.volMax.setText(str(lim))
                    self.volSel.setEnabled(True)
                    self.volBut.setEnabled(True)
                    self.volDir.setEnabled(True)
                else:
                    self.volSel.setMaximum(1)
                    self.volMax.setText('1')
                    self.volSel.setDisabled(True)
                    self.volBut.setDisabled(True)
                    self.volDir.setDisabled(True)


####################################
# Script handling
####################################

        def initScript(self):
            scriptWidget = QWidget()
            self.combo.addItem('Script')
            self.stack.addWidget(scriptWidget)
            self.scriptArea = QTextEdit()
            self.scriptResult = QLabel()
            scriptBut = QPushButton('Evaluate')
            scriptBut.clicked.connect(self.scriptHandler)
            hbox=QHBoxLayout()
            hbox.addWidget(self.scriptResult)
            hbox.addWidget(scriptBut)
            vbox=QVBoxLayout()
            vbox.addWidget(self.scriptArea)
            vbox.addLayout(hbox)
            scriptWidget.setLayout(vbox)

        def scriptHandler(self):
            self.scriptResult.setText(self.mol.evalScript(str(self.scriptArea.toPlainText())))
            self.mol.set_bonds()
            self.parent.updateMolStep()

####################################
# Selected Atoms
####################################

        def initPicker(self):
            self.pickArea = QTextEdit()
            self.pickArea.setReadOnly(True)
            tooltip = QLabel()
            tooltip.setText('Pick up to 4 atoms:')
            self.pickWarn = QLabel()
            vbox=QVBoxLayout()
            vbox.addWidget(tooltip)
            vbox.addWidget(self.pickArea)
            vbox.addWidget(self.pickWarn)
            pickWidget = QWidget()
            pickWidget.setLayout(vbox)
            self.combo.addItem('Pick')
            self.stack.addWidget(pickWidget)

        def pickHandler(self,sel):
            br=0.52917721092
            self.pickWarn.setText('')
            if len(sel)==0:
                self.pickArea.setPlainText('')
            else:
                output ='Atoms: '+str([a[1] for a in sel])+'\n'
                output+='Types: '+str([a[2] for a in sel])+'\n'
                ids = [a[1] for a in sel]
                if len(sel)>1:
                    diff01 = sel[0][3]-sel[1][3]
                    output+=u'Dist {1}-{2}: {0:3.3f} Å\n'.format(norm(diff01)*br,*ids[:2])
                if len(sel)>2:
                    diff12 = sel[2][3]-sel[1][3]
                    output+=u'Dist {1}-{2}: {0:3.3f} Å\n'.format(norm(diff12)*br,*ids[1:3])
                    if len(sel)>3:
                        diff23 = sel[2][3]-sel[3][3]
                        output+=u'Dist {1}-{2}: {0:3.3f} Å\n'.format(norm(diff23)*br,*ids[2:])
                    a012 = degrees(arccos(dot(diff01,diff12)/(norm(diff01)*norm(diff12))))
                    output+=u'Angle {1}-{2}-{3}: {0:3.3f}°\n'.format(a012,*ids[:3])
                if len(sel)>3:
                    a123 = degrees(arccos(dot(diff12,diff23)/(norm(diff12)*norm(diff23))))
                    c012 = cross(diff01,diff12)
                    c123 = cross(diff12,diff23)
                    d0123 = degrees(arccos(dot(c012,c123)/(norm(c012)*norm(c123))))
                    output+=u'Angle {1}-{2}-{3}: {0:3.3f}°\n'.format(a123,*ids[1:])
                    output+=u'Dihedral {1}-{2}-{3}-{4}: {0:3.3f}°\n'.format(d0123,*ids)
                self.pickArea.setPlainText(output)

        def pickUpdate(self):
            if self.pickArea.toPlainText():
                self.pickWarn.setText('Data has changed!')
