
#include "DebugDialog.h"
#include "CustomListModels.h"


#include <QSettings>
#include <QFileDialog>
#include <QDebug>
#include <QSortFilterProxyModel>
#include <QDialogButtonBox>

#include "qlinemarkpanel.h"
#include "qcodeedit.h"

DebugDialog::DebugDialog(QWidget * par)
    : QMainWindow(par),
    heapModel(0), heapProxyModel(0),
    stackModel(0),stackProxyModel(0),
    memModel(0), memProxyModel(0)
{
    ui = new Ui::DebugDialog();
    ui->setupUi(this);

    QCodeEdit * ce= ui->codeEdit->getQCodeEdit();
    //QLineMarkPanel  * p = dynamic_cast<QLineMarkPanel*>(ce->panels("Line Mark Panel")[0]);
    //p->setDisableClicks(false);

    ui->editorToolbar->addAction(ui->codeEdit->action("undo"));
    ui->editorToolbar->addAction(ui->codeEdit->action("redo"));
    ui->editorToolbar->addSeparator();
    ui->editorToolbar->addAction(ui->codeEdit->action("cut"));
    ui->editorToolbar->addAction(ui->codeEdit->action("copy"));
    ui->editorToolbar->addAction(ui->codeEdit->action("paste"));

    ui->menuEdit->addAction(ui->codeEdit->action("undo"));
    ui->menuEdit->addAction(ui->codeEdit->action("redo"));
    ui->menuEdit->addSeparator();
    ui->menuEdit->addAction(ui->codeEdit->action("cut"));
    ui->menuEdit->addAction(ui->codeEdit->action("copy"));
    ui->menuEdit->addAction(ui->codeEdit->action("paste"));


    //restore settings
    QSettings settings;
    settings.beginGroup("WindowState");

    QByteArray d = settings.value("mainwindow").toByteArray();
    if(d.size() > 0)
    {
      restoreState(d);
      qDebug() << "MainWindow settings restored";
    }

    settings.endGroup();
}


DebugDialog::~DebugDialog()
{
    QSettings settings;
    settings.beginGroup("WindowState");
    settings.setValue("mainwindow",saveState());
    settings.endGroup();

    qDebug() << "MainWindow settings stored";
    delete ui;
}


void DebugDialog::on_actionSave_triggered()
{
    ui->codeEdit->save();
}

void DebugDialog::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "", tr("C++ files (*.cpp *.C *.h)"));

    ui->codeEdit->save(fileName);
}

void DebugDialog::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                               "", tr("C++ files (*.cpp *.C *.h)"));
    ui->codeEdit->loadCppFile(fileName);
}

void DebugDialog::on_actionEditorSettings_triggered()
{
    RoseCodeEdit::showEditorSettingsDialog();
}


void DebugDialog::setEditorMark(const QString & file, int row)
{
    ui->codeEdit->loadCppFile(file);
    ui->codeEdit->markAsWarning(row);
}

void DebugDialog::setHeapVars(RuntimeVariablesType * arr, int arrSize)
{
    if(heapModel)
    {
        delete heapModel;
        delete heapProxyModel;
    }

    heapModel = new RuntimeVariablesModel(arr,arrSize,this);
    heapProxyModel = new QSortFilterProxyModel(this);
    connect(ui->txtHeapFilter,SIGNAL(textChanged(const QString&)),
            heapProxyModel, SLOT(setFilterWildcard(const QString&)));

    heapProxyModel->setSourceModel(heapModel);

    ui->lstHeap->setModel(heapProxyModel);

}

void DebugDialog::setStackVars(RuntimeVariablesType * arr, int arrSize)
{
    if(stackModel)
    {
        delete stackModel;
        delete stackProxyModel;
    }

    stackModel = new RuntimeVariablesModel(arr,arrSize,this);
    stackProxyModel = new QSortFilterProxyModel(this);
    connect(ui->txtStackFilter,SIGNAL(textChanged(const QString&)),
            stackProxyModel, SLOT(setFilterWildcard(const QString&)));

    stackProxyModel->setSourceModel(stackModel);

    ui->lstStack->setModel(stackProxyModel);
}

void DebugDialog::setMemoryLocations(MemoryType * arr, int arrSize)
{
    if(memModel)
    {
        delete memModel;
        delete memProxyModel;
    }

    memModel = new MemoryTypeModel(arr,arrSize,this);
    memProxyModel = new QSortFilterProxyModel(this);
    connect(ui->txtMemFilter,SIGNAL(textChanged(const QString&)),
            memProxyModel, SLOT(setFilterWildcard(const QString&)));


    memProxyModel->setSourceModel(memModel);

    ui->lstMem->setModel(memProxyModel);
}


void DebugDialog::on_lstHeap_clicked(const QModelIndex & ind)
{
    displayRuntimeVariable(heapModel->getRuntimeVariable(ind),ui->propHeap );
}

void DebugDialog::on_lstStack_clicked(const QModelIndex & ind)
{
    displayRuntimeVariable(stackModel->getRuntimeVariable(ind),ui->propStack );

}

void DebugDialog::on_lstMem_clicked(const QModelIndex & ind)
{
    displayMemoryType(memModel->getMemoryType(ind),ui->propMem);
}


void DebugDialog::displayRuntimeVariable(RuntimeVariablesType * rv, PropertyTreeWidget * w)
{
    w->clear();

    int general = w->addSection("RuntimeVariablesType");

    addVariableSection(w,general,rv);

    if(rv->address != NULL)
    {
        int memory = w->addSection("Memory Type");
        addMemorySection(w,memory,rv->address);
    }

    if(rv->arrays != NULL)
    {
        int array = w->addSection("Array");
        addArraySection(w,array,rv->arrays);
    }
}


void DebugDialog::displayMemoryType(MemoryType * mt, PropertyTreeWidget * w)
{
    w->clear();

    int mem = w->addSection("Memory Type");
    addMemorySection(w,mem,mt);

    if(mt->variables != NULL)
    {
        int var = w->addSection("Variable");
        addVariableSection(w,var,mt->variables->variable);
    }
}




void DebugDialog::addVariableSection(PropertyTreeWidget * w, int id, RuntimeVariablesType * rv)
{
    w->addEntryToSection(id,"Name", QString(rv->name) );
    w->addEntryToSection(id,"Mangled Name", QString(rv->mangled_name) );
    w->addEntryToSection(id,"Type", QString(rv->type) );
    w->addEntryToSection(id,"Initialized", rv->initialized );
    w->addEntryToSection(id,"fileOpen", QString(rv->fileOpen) );
    w->addEntryToSection(id,"Value", static_cast<qulonglong>(rv->value) );
}

void DebugDialog::addMemorySection(PropertyTreeWidget * w, int id, MemoryType * mt)
{
    w->addEntryToSection(id,"Address", static_cast<qulonglong>(mt->address));
    w->addEntryToSection(id,"LastVariablePos", mt->lastVariablePos);
    w->addEntryToSection(id,"MaxNrOfVariables", mt->maxNrOfVariables);
    w->addEntryToSection(id,"Size",mt->size);
}

void DebugDialog::addArraySection(PropertyTreeWidget * w, int id, ArraysType * at)
{
    w->addEntryToSection(id,"Name", QString(at->name));
    w->addEntryToSection(id,"Dimension",at->dim);
    w->addEntryToSection(id,"Size Dim1",at->size1);
    w->addEntryToSection(id,"Size Dim2",at->size2);
    w->addEntryToSection(id,"IsMalloc",at->ismalloc);
}
