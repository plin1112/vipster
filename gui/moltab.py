# -*- coding: utf-8 -*-

from PyQt4.QtGui import *
from PyQt4.QtCore import Qt

from .collapsiblewidget import collapsibleWidget

class MolTab(QWidget):

    def __init__(self,parent):
        super(MolTab,self).__init__()
        self.parent = parent
        coordCollapse = self.initCoord()
        cellCollapse = self.initCell()
        kCollapse = self.initKpoints()
        vbox = QVBoxLayout()
        vbox.addWidget(coordCollapse)
        vbox.setStretchFactor(coordCollapse,1)
        vbox.addWidget(cellCollapse)
        vbox.addWidget(kCollapse)
        self.setLayout(vbox)

    def initCoord(self):
        # coordinate format
        self.fmt = QComboBox()
        self.fmt.setToolTip('Select format of coordinates')
        self.fmt.addItems(['angstrom','bohr','crystal','alat'])
        self.fmt.setCurrentIndex(2)
        self.fmt.currentIndexChanged.connect(self.fillTab)
        hbox = QHBoxLayout()
        hbox.addWidget(QLabel('Format:'))
        hbox.addWidget(self.fmt)

        # coordinate table
        self.table = QTableWidget()
        self.table.setColumnCount(4)
        self.table.setHorizontalHeaderLabels(['Type','x','y','z'])
        def cellHandler(row,col):
            if self.updatedisable: return
            self.mol.init_undo()
            name = str(self.table.item(row,0).text())
            coord = [0,0,0]
            fix = [0,0,0]
            for j in [0,1,2]:
                coord[j]=float(self.table.item(row,j+1).text())
                fix[j]=int(not self.table.item(row,j+1).checkState()/2)
            self.mol.set_atom(row,name,coord,self.fmt.currentText(),fix)
            self.mol.save_undo('set coordinates')
            self.parent.updateMolStep()
        self.table.cellChanged.connect(cellHandler)
        def selHandler():
            if self.updatedisable: return
            sel = self.mol.get_selection()
            keep = []
            for i in self.table.selectedRanges():
                for j in range(i.topRow(),i.bottomRow()+1):
                    l = [k for k in sel if k[0] == j]
                    if l:
                        keep.extend(l)
                    else:
                        keep.append([j,(0,0,0)])
            self.mol.del_selection()
            for i in keep:
                self.mol.add_selection(i)
            self.parent.updateMolStep()
        self.table.itemSelectionChanged.connect(selHandler)

        coordWidget=QWidget()
        coordLay=QVBoxLayout()
        coordLay.addLayout(hbox)
        coordLay.addWidget(self.table)
        coordLay.setContentsMargins(0,0,0,0)
        coordLay.setStretchFactor(self.table,0)
        coordWidget.setLayout(coordLay)
        return coordWidget

    def initCell(self):
        # show celldm
        def cdmHandler():
            if self.updatedisable: return
            if float(self.cellDm.text()) == self.mol.get_celldm(self.fmt.currentText()):return
            par= self.parent
            if self.appAll.isChecked():
                self.mol.set_all_celldm(float(self.cellDm.text()),self.scale.isChecked(),self.fmt.currentText())
            else:
                self.mol.set_celldm(float(self.cellDm.text()),self.scale.isChecked(),self.fmt.currentText())
            self.parent.updateMolStep()

        self.cellDm = QLineEdit()
        self.cellDm.setValidator(QDoubleValidator())
        self.cellDm.editingFinished.connect(cdmHandler)

        # show cell vectors in table
        def vecHandler():
            if self.updatedisable: return
            vec=[[0,0,0],[0,0,0],[0,0,0]]
            for i in [0,1,2]:
                for j in [0,1,2]:
                    vec[i][j]=float(self.vtable.item(i,j).text())
            if vec == self.mol.get_vec().tolist(): return
            par = self.parent
            if self.appAll.isChecked():
                self.mol.set_all_vec(vec,self.scale.isChecked())
            else:
                self.mol.set_vec(vec,self.scale.isChecked())
            self.parent.updateMolStep()

        self.vtable = QTableWidget()
        self.vtable.setColumnCount(3)
        self.vtable.setRowCount(3)
        self.vtable.setFixedHeight(120)
        self.vtable.setHorizontalHeaderLabels(['x','y','z'])
        self.vtable.itemChanged.connect(vecHandler)

        #cell header with label and celldm
        hbox=QHBoxLayout()
        hbox.addWidget(QLabel('Cell vectors:'))
        hbox.addStretch()
        hbox.addWidget(QLabel('Cell dimension:'))
        hbox.addWidget(self.cellDm)

        # Action modifiers
        self.appAll = QCheckBox('Apply to all Molecules')
        self.scale = QCheckBox('Scale coordinates with cell')
        hbox2 = QHBoxLayout()
        hbox2.addWidget(self.appAll)
        hbox2.addWidget(self.scale)

        #nest in collapsible widget
        cellWidget=QWidget()
        cellLay=QVBoxLayout()
        cellLay.addLayout(hbox)
        cellLay.addWidget(self.vtable)
        cellLay.addLayout(hbox2)
        cellLay.setContentsMargins(0,0,0,0)
        cellWidget.setLayout(cellLay)
        return collapsibleWidget('Cell geometry',cellWidget)

    def initKpoints(self):
        self.kp = QWidget()
        kp=self.kp
        kp.updatedisable=False
        #choose k point format
        kp.fmt = QComboBox()
        kp.fmts = ['gamma','automatic','tpiba','crystal','tpiba_b','crystal_b']
        for i in kp.fmts:
            kp.fmt.addItem(i)

        #Automatic: x,y,z, offset(x,y,z)
        def saveAutoK():
            if kp.updatedisable:return
            auto=[str(kp.auto.widg[i].text()) for i in [0,1,2]]
            auto+=[str(int(kp.auto.widg[i].isChecked())) for i in [3,4,5]]
            self.mol.set_kpoints('automatic',auto)
        kp.auto = QWidget()
        kp.auto.widg = []
        for i in [0,1,2]:
            kp.auto.widg.append(QLineEdit())
            kp.auto.widg[-1].setValidator(QIntValidator(1,40000))
            kp.auto.widg[-1].setText(str(1))
            kp.auto.widg[-1].editingFinished.connect(saveAutoK)
        for i in [0,1,2]:
            kp.auto.widg.append(QCheckBox())
            kp.auto.widg[-1].stateChanged.connect(saveAutoK)
        kp.auto.hbox1=QHBoxLayout()
        kp.auto.hbox1.addWidget(QLabel('x:'))
        kp.auto.hbox1.addWidget(kp.auto.widg[0])
        kp.auto.hbox1.addWidget(QLabel('y:'))
        kp.auto.hbox1.addWidget(kp.auto.widg[1])
        kp.auto.hbox1.addWidget(QLabel('z:'))
        kp.auto.hbox1.addWidget(kp.auto.widg[2])
        kp.auto.hbox2 = QHBoxLayout()
        kp.auto.hbox2.addWidget(QLabel('x offs.:'))
        kp.auto.hbox2.addWidget(kp.auto.widg[3])
        kp.auto.hbox2.addWidget(QLabel('y offs.:'))
        kp.auto.hbox2.addWidget(kp.auto.widg[4])
        kp.auto.hbox2.addWidget(QLabel('z offs.:'))
        kp.auto.hbox2.addWidget(kp.auto.widg[5])
        kp.auto.vbox = QVBoxLayout()
        kp.auto.vbox.addLayout(kp.auto.hbox1)
        kp.auto.vbox.addLayout(kp.auto.hbox2)
        kp.auto.setLayout(kp.auto.vbox)

        #discrete kpoints
        def cellHandler():
            if kp.updatedisable: return
            disc = []
            for i in range(kp.disc.rowCount()):
                disc.append([str(kp.disc.item(i,j).text()) for j in range(4)])
            self.mol.set_kpoints('disc',disc)
        kp.disc = QTableWidget()
        kp.disc.setColumnCount(4)
        kp.disc.setHorizontalHeaderLabels(['x','y','z','weight'])
        kp.disc.setContextMenuPolicy(Qt.ActionsContextMenu)
        kp.disc.cellChanged.connect(cellHandler)
        def newKpoint():
            kp.updatedisable=True
            kp.disc.setRowCount(kp.disc.rowCount()+1)
            for i in range(4):
                kp.disc.setItem(kp.disc.rowCount()-1,i,QTableWidgetItem('1'))
            kp.updatedisable=False
            cellHandler()
        newKp = QAction('New k-point',kp.disc)
        newKp.setShortcut('Ctrl+K')
        newKp.triggered.connect(newKpoint)
        kp.disc.addAction(newKp)
        def delKpoint():
            kp.disc.removeRow(kp.disc.currentRow())
            cellHandler()
        delKp = QAction('Delete k-point',kp.disc)
        delKp.triggered.connect(delKpoint)
        kp.disc.addAction(delKp)
        #stacked display of various formats
        kp.disp = QStackedWidget()
        kp.disp.addWidget(QLabel('Gamma point only'))
        kp.disp.addWidget(kp.auto)
        kp.disp.addWidget(kp.disc)
        kp.disp.setFixedHeight(120)
        kp.fmt.currentIndexChanged.connect(self.changeKpoint)

        #layout
        hbox = QHBoxLayout()
        hbox.addWidget(QLabel('Format:'))
        hbox.addWidget(kp.fmt)
        vbox = QVBoxLayout()
        vbox.addLayout(hbox)
        vbox.addWidget(kp.disp)
        kp.setLayout(vbox)
        return collapsibleWidget('K-Points',kp,show=False)

    def changeKpoint(self,index):
        kp=self.kp
        kp.updatedisable=True
        kp.disp.setCurrentIndex(min(index,2))
        fmt=kp.fmts[index]
        self.mol.set_kpoints('active',fmt)
        if fmt=='automatic':
            auto=self.mol.get_kpoints(fmt)
            for i in range(6):
                kp.auto.widg[i].setText(auto[i])
        elif fmt in kp.fmts[2:]:
            disc=self.mol.get_kpoints(fmt)
            kp.disc.setRowCount(len(disc))
            for i in range(len(disc)):
                for j in range(4):
                    kp.disc.setItem(i,j,QTableWidgetItem(disc[i][j]))
        kp.updatedisable=False

    def setMol(self,mol):
        #connect molecule
        self.mol = mol
        #fill if visible
        if self.isVisible(): self.fillTab()

    def showEvent(self,e):
        if hasattr(self,'mol'): self.fillTab()

    ##############################################################
    # MAIN WIDGET UPDATE FUNCTION
    #############################################################

    def fillTab(self):
        #prevent handling of cell changes during fill
        self.updatedisable = True
        fmt = self.fmt.currentText()
        #fill atom coordinate widget
        self.table.setRowCount(self.mol.get_nat())
        #make table count from zero
        self.table.setVerticalHeaderLabels([str(x) for x in range(self.mol.get_nat())])
        for i in range(self.mol.get_nat()):
            at = self.mol.get_atom(i,fmt)
            self.table.setItem(i,0,QTableWidgetItem(at[0]))
            for j in [0,1,2]:
                self.table.setItem(i,j+1,QTableWidgetItem(str(at[1][j])))
                self.table.item(i,j+1).setFlags(Qt.ItemFlag(51))
                self.table.item(i,j+1).setCheckState(int(not at[3][j])*2)
        #update selection
        self.table.setSelectionMode(QAbstractItemView.MultiSelection)
        self.table.clearSelection()
        for j in set(i[0] for i in self.mol.get_selection()):
            self.table.selectRow(j)
        self.table.setSelectionMode(QAbstractItemView.ExtendedSelection)
        #fill cell geometry widget
        self.cellDm.setText(str(self.mol.get_celldm(fmt)))
        vec = self.mol.get_vec()
        for i in [0,1,2]:
            for j in [0,1,2]:
                self.vtable.setItem(i,j,QTableWidgetItem(str(vec[i,j])))
        #update k-point widgets
        self.kp.fmt.setCurrentIndex(self.kp.fmts.index(self.mol.get_kpoints('active')))
        self.changeKpoint(self.kp.fmts.index(self.mol.get_kpoints('active')))
        #reenable handling
        self.updatedisable = False
