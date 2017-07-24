#include <QFile>
#include <QSettings>
#include "lthemeengine.h"
#include "qsseditordialog.h"
#include "ui_qsseditordialog.h"

QSSEditorDialog::QSSEditorDialog(const QString &filePath, QWidget *parent) : QDialog(parent), m_ui(new Ui::QSSEditorDialog){
  m_ui->setupUi(this);
  m_filePath = filePath;
  QFile file(filePath);
  file.open(QIODevice::ReadOnly);
  m_ui->textEdit->setPlainText(QString::fromUtf8(file.readAll()));
  setWindowTitle(tr("%1 - Style Sheet Editor").arg(file.fileName()));
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  restoreGeometry(settings.value("QSSEditor/geometry").toByteArray());
}

QSSEditorDialog::~QSSEditorDialog(){
  delete m_ui;
}

void QSSEditorDialog::save(){
  QFile file(m_filePath);
  file.open(QIODevice::WriteOnly);
  file.write(m_ui->textEdit->toPlainText().toUtf8());
}

void QSSEditorDialog::hideEvent(QHideEvent *){
  QSettings settings(lthemeengine::configFile(), QSettings::IniFormat);
  settings.setValue("QSSEditor/geometry", saveGeometry());
}

void QSSEditorDialog::on_buttonBox_clicked(QAbstractButton *button){
  QDialogButtonBox::StandardButton id = m_ui->buttonBox->standardButton(button);
  if(id == QDialogButtonBox::Ok){
    save();
    accept();
    }
  else if(id == QDialogButtonBox::Save){
    save();
    }
  else{
  reject();
  }
}
